/***********************************************************************************************
 * Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                   *
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
#include <OplPcTools/BigEndian.h>
#include <OplPcTools/NrgDeviceSource.h>

#define INVALID_OFFSET (~(quint64)0)

using namespace OplPcTools;

namespace {

struct Chunk
{
    char id[4];
    BigEndian<quint32> size;
} __attribute__((packed));

struct Ner5
{
    char id[4];
    BigEndian<quint64> offset_of_first_chunk;
} __attribute__((packed));

struct Daox
{
    quint8 header[30];
} __attribute__((packed));

struct DaoxTrack
{
    char isrc[12];
    quint8 sector_size[2];
    quint8 mode[2];
    quint8 unknown[2];
    quint8 pre_gap[8];
    BigEndian<quint64> track_begin;
    BigEndian<quint64> track_end;
} __attribute__((packed));

} // namespace


class NrgDeviceSource::NrgImage final
{
    Q_DISABLE_COPY(NrgImage)

public:
    explicit NrgImage(const QString & _filepath);
    bool open();
    void close();
    inline bool isOpen() const;
    inline QString filepath() const;
    bool seek(quint64 _offset);
    qint64 read(QByteArray & _buffer);

private:
    quint64 readFirstChunkOffset();
    bool readChunkHeader(quint64 _offset, Chunk * _chunk);
    quint64 getTrackLocation(quint64 _dao_header_offset);

private:
    QFile m_file;
    quint64 m_track_location;
};


NrgDeviceSource::NrgImage::NrgImage(const QString & _filepath) :
    m_file(_filepath),
    m_track_location(INVALID_OFFSET)
{
}

bool NrgDeviceSource::NrgImage::open()
{
    if(!m_file.open(QIODevice::ReadOnly))
        return false;
    quint64 offset = readFirstChunkOffset();
    if(offset == INVALID_OFFSET)
        return false;
    for(;;)
    {
        Chunk header;
        if(!readChunkHeader(offset, &header))
            return false;
        if(std::strcmp("END!", header.id) == 0)
            break;
        if(std::strcmp("DAOX", header.id) == 0)
        {
            m_track_location = getTrackLocation(offset);
            return true;
        }
        offset += header.size.toIntLE() + sizeof(Chunk);
    }
    return false;
}

quint64 NrgDeviceSource::NrgImage::readFirstChunkOffset()
{
    if(!m_file.seek(m_file.size() - sizeof(Ner5)))
        return INVALID_OFFSET;
    Ner5 ner5;
    if(m_file.read(reinterpret_cast<char *>(&ner5), sizeof(Ner5)) != sizeof(Ner5))
        return INVALID_OFFSET;
    if(std::strcmp("NER5", ner5.id) != 0)
        return INVALID_OFFSET;
    return ner5.offset_of_first_chunk.toIntLE();
}

bool NrgDeviceSource::NrgImage::readChunkHeader(quint64 _offset, Chunk * _chunk)
{
    if(!m_file.seek(_offset))
        return false;
    if(!m_file.read(reinterpret_cast<char *>(_chunk), sizeof(Chunk)))
        return false;
    return true;
}

quint64 NrgDeviceSource::NrgImage::getTrackLocation(quint64 _dao_header_offset)
{
    if(!m_file.seek(_dao_header_offset + sizeof(Daox::header)))
        return INVALID_OFFSET;
    DaoxTrack first_track_header;
    if(!m_file.read(reinterpret_cast<char *>(&first_track_header), sizeof(DaoxTrack)))
        return INVALID_OFFSET;
    return first_track_header.track_begin.toIntLE();
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
    if(m_track_location == INVALID_OFFSET)
        return false;
    return m_file.seek(m_track_location + _offset);
}

qint64 NrgDeviceSource::NrgImage::read(QByteArray & _buffer)
{
    if(m_track_location == INVALID_OFFSET)
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
    try {
        mp_image->open();
    } catch(...) {
        if(mp_image->isOpen())
            mp_image->close();
        return false;
    }
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
