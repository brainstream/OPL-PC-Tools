/***********************************************************************************************
 * Copyright © 2017-2018 Sergey Smolyannikov aka brainstream                                   *
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
 * You should have received a copy of the GNU General Public License along with MailUnit.      *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#include <cstring>
#include <QFile>
#include <OplPcTools/Endianness.h>
#include <OplPcTools/File.h>
#include <OplPcTools/Exception.h>
#include <OplPcTools/NrgDeviceSource.h>

using namespace OplPcTools;

#define NER5_FOOTER_LENGTH  12
#define CHUNK_HEADER_LENGTH 8
#define CHUNK_ID_LENGTH     4
#define CHUNK_SIZE_LENGTH   4


namespace {

struct TrackHeader // TODO: is it necessary
{
    char isrc[12];
    quint8 sector_size[2];
    quint8 mode[2];
    quint8 unknown[2];
    quint8 pre_gap[8];
    quint8 track_begin[8];
    quint8 track_end[8];
} __attribute__((packed));

} // namespace


class NrgDeviceSource::NrgImage final
{
    Q_DISABLE_COPY(NrgImage)

    struct ChunkHeader
    {
        QString id;
        quint32 size;
    };

    struct TrackLocation
    {
        TrackLocation() :
            begin(0),
            size(0)
        {
        }

        quint64 begin;
        quint64 size;
    };

public:
    explicit NrgImage(const QString & _filepath);
    void open();
    void close();
    inline bool isOpen() const;
    inline QString filepath() const;
    bool seek(quint64 _offset);
    qint64 read(QByteArray & _buffer);

private:
    quint64 readFirstChunkOffset();
    ChunkHeader readChunkHeader(quint64 _offset);
    TrackLocation readDaoHeader(quint64 _offset, quint32 _size);

private:
    QFile m_file;
    TrackLocation m_track;
};




NrgDeviceSource::NrgImage::NrgImage(const QString & _filepath) :
    m_file(_filepath)
{
}

void NrgDeviceSource::NrgImage::open()
{
    openFile(m_file, QIODevice::ReadOnly);
    quint64 offset = readFirstChunkOffset();
    for(;;)
    {
        ChunkHeader header = readChunkHeader(offset);
        if(header.id == "END!")
            break;
        if(header.id == "DAOX")
        {
            m_track = readDaoHeader(offset, header.size);
            break;
        }
        offset += header.size;
    }
}

quint64 NrgDeviceSource::NrgImage::readFirstChunkOffset()
{
    if(!m_file.seek(m_file.size() - NER5_FOOTER_LENGTH))
        throw IOException(QObject::tr("Unable to locate the NER5 chunk")); // TODO: generic solution
    char buffer[12];
    if(m_file.read(buffer, NER5_FOOTER_LENGTH) != NER5_FOOTER_LENGTH)
        throw IOException(QObject::tr("Unable to read the NER5 chunk")); // TODO: generic solution
    if(std::strcmp("NER5", buffer) != 0)
        throw IOException(QObject::tr("Format is not supported. Only NRG version 2 is supported."));

    // TODO: ugly!
    std::array<quint8, sizeof(quint64)> offset;
    std::copy(&buffer[CHUNK_ID_LENGTH], &buffer[CHUNK_ID_LENGTH + sizeof(quint64)], offset.begin());

    return readBigEndian<quint64>(offset);
}

NrgDeviceSource::NrgImage::ChunkHeader NrgDeviceSource::NrgImage::readChunkHeader(quint64 _offset)
{
    if(!m_file.seek(_offset))
        throw IOException(QObject::tr("Unable to locate a NRG chunk header")); // TODO: generic solution
    char buffer[CHUNK_HEADER_LENGTH];
    if(!m_file.read(buffer, CHUNK_HEADER_LENGTH))
        throw IOException(QObject::tr("Unable to read a NRG chunk header")); // TODO: generic solution
    ChunkHeader header;
    header.id = QString::fromLatin1(buffer, CHUNK_ID_LENGTH);

    // TODO: ugly!
    std::array<uint8_t, CHUNK_SIZE_LENGTH> size;
    std::copy(&buffer[CHUNK_ID_LENGTH], &buffer[CHUNK_HEADER_LENGTH], size.begin());

    header.size = readBigEndian<uint32_t>(size) + CHUNK_HEADER_LENGTH;
    return header;
}

NrgDeviceSource::NrgImage::TrackLocation NrgDeviceSource::NrgImage::readDaoHeader(quint64 _offset, quint32 _size)
{
    const quint32 session_header_size = 30;
    m_file.seek(_offset + session_header_size);
    // while(read_bytes < _size) // TODO: read all tracks
    TrackHeader header;
    m_file.read(reinterpret_cast<char *>(&header), sizeof(TrackHeader)); // TODO: check result (generic solution)

    // TODO: ugly!
    std::array<quint8, sizeof(quint64)> track_begin;
    std::copy(header.track_begin, &header.track_begin[sizeof(header.track_begin)], track_begin.begin());
    // TODO: ugly!
    std::array<quint8, sizeof(quint64)> track_end;
    std::copy(header.track_end, &header.track_end[sizeof(header.track_begin)], track_end.begin());

    TrackLocation location;
    location.begin = readBigEndian<quint64>(track_begin);
    location.size = readBigEndian<quint64>(track_end) - location.begin - 1;
    return location;
}

void NrgDeviceSource::NrgImage::close()
{
    m_file.close();
}

bool NrgDeviceSource::NrgImage::isOpen() const
{
    return m_file.isOpen();
}

QString NrgDeviceSource::NrgImage::filepath() const
{
    return m_file.fileName();
}

bool NrgDeviceSource::NrgImage::seek(quint64 _offset)
{
    if(m_track.size == 0)
        return false;
    return m_file.seek(m_track.begin + _offset);
}

qint64 NrgDeviceSource::NrgImage::read(QByteArray & _buffer)
{
    if(m_track.size == 0)
        return 0;
    return m_file.read(_buffer.data(), _buffer.size());
}

NrgDeviceSource::NrgDeviceSource(const QString & _nrg_filepath) :
    mp_image(new NrgDeviceSource::NrgImage(_nrg_filepath))
{
}

NrgDeviceSource::~NrgDeviceSource()
{
    delete mp_image;
}

QString NrgDeviceSource::filepath() const
{
    return mp_image->filepath();
}

bool NrgDeviceSource::isReadOnly() const
{
    return true;
}

bool NrgDeviceSource::open()
{
    mp_image->open();
    return true;
}

bool NrgDeviceSource::isOpen() const
{
    return mp_image->isOpen();
}

void NrgDeviceSource::close()
{
    mp_image->close();
}

bool NrgDeviceSource::seek(qint64 _offset)
{
    return mp_image->seek(_offset);
}

qint64 NrgDeviceSource::read(QByteArray & _buffer)
{
    return mp_image->read(_buffer);
}
