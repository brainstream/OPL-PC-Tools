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

#include <cstring>
#include <QFile>
#include <OplPcTools/BigEndian.h>
#include <OplPcTools/Device/NrgDeviceSource.h>

#define INVALID_OFFSET (~(quint64)0)

using namespace OplPcTools;

namespace {

struct __attribute__((packed)) Chunk
{
    char id[4];               // 0
    BigEndian<quint32> size;  // 4
};

struct __attribute__((packed)) Ner5
{
    char id[4];
    BigEndian<quint64> offset_of_first_chunk;
};

struct __attribute__((packed)) Daox : Chunk
{
    BigEndian<quint32> size2;                // 8
    char upc[13];                            // 12
    quint8 padding;                          // 25
    BigEndian<quint16> toc_type;             // 26
    quint8 first_track_in_session;           // 28
    quint8 last_track_in_session;            // 29
    char isrc[12];                           // 30
    BigEndian<quint16> image_sector_size;    // 42
    BigEndian<quint16> mode;                 // 44
    BigEndian<quint16> unknown;              // 46
    BigEndian<quint64> padding2;             // 48
    BigEndian<quint64> begin_of_track;       // 56
    BigEndian<quint64> end_of_track_plus_1b; // 64
                                             // 72
};

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
    qint64 isoSize() const;

private:
    quint64 readFirstChunkOffset();
    bool readChunkHeader(quint64 _offset, Chunk * _chunk);
    bool readDaox(quint64 _offset, Daox * _daox);

private:
    QFile m_file;
    quint64 m_track_location;
    qint64 m_iso_size;
};


NrgDeviceSource::NrgImage::NrgImage(const QString & _filepath) :
    m_file(_filepath),
    m_track_location(INVALID_OFFSET),
    m_iso_size(-1)
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
        Chunk header = { };
        if(!readChunkHeader(offset, &header))
            return false;
        if(std::strncmp("END!", header.id, sizeof(header.id)) == 0)
            break;
        if(std::strncmp("DAOX", header.id, sizeof(header.id)) == 0)
        {
            Daox daox;
            if(!readDaox(offset, &daox))
                return false;
            m_track_location = daox.begin_of_track.toIntLE();
            m_iso_size = daox.end_of_track_plus_1b.toIntLE() - m_track_location;
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
    if(std::strncmp("NER5", ner5.id, sizeof(ner5.id)) != 0)
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

bool NrgDeviceSource::NrgImage::readDaox(quint64 _offset, Daox * _daox)
{
    if(!m_file.seek(_offset))
        return false;
    if(!m_file.read(reinterpret_cast<char *>(_daox), sizeof(Daox)))
        return false;
    return true;
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

inline qint64 NrgDeviceSource::NrgImage::isoSize() const
{
    return m_iso_size;
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

qint64 NrgDeviceSource::isoSize() const
{
    return mp_image->isoSize();
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
