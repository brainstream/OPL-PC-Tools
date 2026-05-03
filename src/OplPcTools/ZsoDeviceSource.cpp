/***********************************************************************************************
 * Copyright © 2017-2026 Sergey Smolyannikov aka brainstream                                   *
 *                                                                                             *
 * This file is part of the OPL PC Tools project, the graphical PC tools for Open PS2 Loader.  *
 *                                                                                             *
 * OPL PC Tools is free software: you can redistribute it and/or modify it under the terms of  *
 * the GNU General Public License as published by the Free Software Foundation,                *
 * either version 3 of the License, or (at your option) any later version.                     *
 *                                                                                             *
 * OPL PC Tools is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;  *
 * without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  *
 * See the GNU General Public License for more details.                                        *
 *                                                                                             *
 * You should have received a copy of the GNU General Public License along with OPL PC Tools   *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#include <OplPcTools/ZsoDeviceSource.h>
#include <lz4.h>
#include <QFile>
#include <QList>

static_assert(Q_BYTE_ORDER == Q_LITTLE_ENDIAN, "ZsoDeviceSource: only little-endian hosts (ZSO file is LE)");

using namespace OplPcTools;

namespace {

constexpr quint32 g_zso_magic = 0x4F53495A;

struct __attribute__((packed)) ZsoHeader
{
    quint32 magic;       // 0
    quint32 header_size; // 4
    quint64 total_bytes; // 8
    quint32 block_size;  // 16
    quint8 version;      // 20
    quint8 align;        // 21
    quint8 reserved[2];  // 22
};

struct  ZsoIndex
{
    quint32 address;
    bool is_compressed;
};

struct CachedBlock
{
    quint32 index;
    QByteArray data;

    CachedBlock() :
        index(0)
    {
    }

    bool isValide() const
    {
        return !data.isEmpty();
    }

    void reset()
    {
        index = 0;
        data.clear();
    }
};

} // namespace

class ZsoDeviceSource::ZsoImage
{
    Q_DISABLE_COPY_MOVE(ZsoImage)

public:
    explicit ZsoImage(const QString & _filename);
    bool open();
    void close();
    bool isOpen() const;
    QString filepath() const;
    bool seek(quint64 _offset);
    qint64 read(QByteArray & _buffer);

private:
    bool initIndex();
    QByteArray readBlock(quint32 _index);

private:
    QFile m_file;
    ZsoHeader m_header;
    QList<ZsoIndex> m_index;
    quint64 m_logical_offset;
    CachedBlock m_cache;
};

ZsoDeviceSource::ZsoImage::ZsoImage(const QString & _filename) :
    m_file(_filename),
    m_header{},
    m_logical_offset(0)
{
}

bool ZsoDeviceSource::ZsoImage::open()
{
    if(!m_file.open(QIODevice::ReadOnly))
        return false;
    if(
        m_file.read(reinterpret_cast<char *>(&m_header), sizeof(ZsoHeader)) != sizeof(ZsoHeader) ||
        m_header.magic != g_zso_magic ||
        !initIndex())
    {
        m_file.close();
        return false;
    }
    return true;
}

bool ZsoDeviceSource::ZsoImage::initIndex()
{
    m_index.clear();

    if(!m_file.seek(sizeof(ZsoHeader)))
        return false;

    // The pointer after the last one points to a location just beyond the end of the file
    const qsizetype total_index_count = m_header.total_bytes / m_header.block_size + 1;
    m_index.reserve(total_index_count);

    const qsizetype buffer_size = sizeof(uint32_t) * 2048;
    QByteArray buffer(buffer_size, Qt::Uninitialized);

    for(;;)
    {
        const qsizetype read_bytes = m_file.read(buffer.data(), buffer_size);
        if(read_bytes <= 0)
        {
            m_index.clear();
            return false;
        }
        const uint32_t * indices = reinterpret_cast<const uint32_t *>(buffer.data());
        const qsizetype index_count = read_bytes / sizeof(uint32_t);
        for(qsizetype i = 0; i < index_count; ++i)
        {
            m_index.append({
                .address = (indices[i] & 0b01111111'11111111'11111111'11111111) << m_header.align,
                .is_compressed = (indices[i] & 0b10000000'00000000'00000000'00000000) == 0
            });
            if(m_index.size() == total_index_count)
                return true;
        }
    }
}

inline void ZsoDeviceSource::ZsoImage::close()
{
    m_file.close();
}

inline bool ZsoDeviceSource::ZsoImage::isOpen() const
{
    return m_file.isOpen() && !m_index.empty();
}

inline QString ZsoDeviceSource::ZsoImage::filepath() const
{
    return m_file.fileName();
}

bool ZsoDeviceSource::ZsoImage::seek(quint64 _offset)
{
    if(_offset >= m_header.total_bytes)
        return false;
    m_logical_offset = _offset;
    return true;
}

qint64 ZsoDeviceSource::ZsoImage::read(QByteArray & _buffer)
{
    if(_buffer.isEmpty())
        return 0;

    quint64 block_index = m_logical_offset / m_header.block_size;
    quint64 position_in_block = m_logical_offset % m_header.block_size;
    qsizetype can_read_bytes = m_header.block_size - position_in_block;
    qsizetype need_to_read = _buffer.size();
    qsizetype output_position = 0;

    do
    {
        QByteArray block = readBlock(block_index);
        if(block.isEmpty())
            return output_position;

        qsizetype bytes_to_copy = qMin(need_to_read, can_read_bytes);
        std::memcpy(&_buffer.data()[output_position], &block.constData()[position_in_block], bytes_to_copy);

        need_to_read -= bytes_to_copy;
        output_position += bytes_to_copy;
        ++block_index;
        can_read_bytes = m_header.block_size;
        position_in_block = 0;
        m_logical_offset += bytes_to_copy;
    } while(need_to_read > 0);


    return _buffer.size();
}

QByteArray ZsoDeviceSource::ZsoImage::readBlock(quint32 _index)
{
    if(m_index.empty() || _index >= m_index.size() - 1)
        return QByteArray();

    if(m_cache.isValide() && m_cache.index == _index)
        return m_cache.data;

    if(!m_file.seek(m_index[_index].address))
        return QByteArray();

    if(m_cache.data.size() < m_header.block_size)
        m_cache.data.resize(m_header.block_size);

    m_cache.index = _index;

    if(m_index[_index].is_compressed)
    {
        const quint32 block_address = m_index[_index].address;
        const quint32 next_block_address = m_index[_index + 1].address;
        if(next_block_address <= block_address)
        {
            m_cache.reset();
            return m_cache.data;
        }
        const quint32 compressed_size = m_index[_index + 1].address - m_index[_index].address;
        QByteArray compressed_data(compressed_size, Qt::Uninitialized);
        if(m_file.read(compressed_data.data(), compressed_size) != compressed_size)
        {
            m_cache.reset();
            return m_cache.data;
        }
        const int decompressed_bytes = LZ4_decompress_safe(
            compressed_data.constData(),
            m_cache.data.data(),
            compressed_size,
            m_header.block_size);
        if(decompressed_bytes < 0)
        {
            m_cache.reset();
            return m_cache.data;
        }
    }
    else
    {
        if(m_file.read(m_cache.data.data(), m_header.block_size) != m_header.block_size)
        {
            m_cache.reset();
            return m_cache.data;
        }
    }

    return m_cache.data;
}

ZsoDeviceSource::ZsoDeviceSource(const QString & _zso_filepath) :
    mp_image(new ZsoDeviceSource::ZsoImage(_zso_filepath))
{
}

ZsoDeviceSource::~ZsoDeviceSource()
{
    delete mp_image;
}

QString ZsoDeviceSource::filepath() const
{
    return mp_image->filepath();
}

bool ZsoDeviceSource::isReadOnly() const
{
    return true;
}

bool ZsoDeviceSource::open()
{
    return mp_image->open();
}

bool ZsoDeviceSource::isOpen() const
{
    return mp_image->isOpen();
}

void ZsoDeviceSource::close()
{
    mp_image->close();
}

bool ZsoDeviceSource::seek(qint64 _offset)
{
    return mp_image->seek(_offset);
}

qint64 ZsoDeviceSource::read(QByteArray & _buffer)
{
    return mp_image->read(_buffer);
}
