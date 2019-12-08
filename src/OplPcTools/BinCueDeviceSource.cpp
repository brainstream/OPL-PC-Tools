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

#include <QFileInfo>
#include <QDir>
#include <OplPcTools/BinCueDeviceSource.h>

#define BIN_HEADER_SIZE    24
#define BIN_SECTOR_SIZE    2352
#define BIN_SECTOR_OFFSET  0
#define ISO_SECTOR_SIZE    2048

using namespace OplPcTools;

BinCueDeviceSource::BinCueDeviceSource(const QString & _bin_filepath) :
    m_bin_file(_bin_filepath)
{
}

QString BinCueDeviceSource::filepath() const
{
    return m_bin_file.fileName();
}

bool BinCueDeviceSource::isReadOnly() const
{
    return true;
}

bool BinCueDeviceSource::open()
{
    return m_bin_file.open(QIODevice::ReadOnly);
}

bool BinCueDeviceSource::isOpen() const
{
    return m_bin_file.isOpen();
}

void BinCueDeviceSource::close()
{
    m_bin_file.close();
}

bool BinCueDeviceSource::seek(qint64 _offset)
{
    quint32 sector = _offset / ISO_SECTOR_SIZE;
    quint32 bytes = _offset % ISO_SECTOR_SIZE;
    qint64 real_offset = (BIN_SECTOR_SIZE * sector) + bytes + BIN_SECTOR_OFFSET + BIN_HEADER_SIZE;
    return m_bin_file.seek(real_offset);
}

qint64 BinCueDeviceSource::read(QByteArray & _buffer)
{
    qint64 read_bytes = 0;
    for(qint64 remains_to_read = _buffer.size(); remains_to_read > 0; remains_to_read = _buffer.size() - read_bytes)
    {
        quint64 bin_pos = m_bin_file.pos();
        quint32 sector = (bin_pos - BIN_HEADER_SIZE) / BIN_SECTOR_SIZE;
        quint64 iso_sector_begin = (BIN_SECTOR_SIZE * sector) + BIN_SECTOR_OFFSET + BIN_HEADER_SIZE;
        quint64 iso_sector_end = iso_sector_begin + ISO_SECTOR_SIZE;
        quint32 available_to_read = iso_sector_end - bin_pos;
        quint32 to_read = remains_to_read < available_to_read ? remains_to_read : available_to_read;
        if(to_read > 0)
        {
            qint64 current_read = m_bin_file.read(&_buffer.data()[read_bytes], to_read);
            if(current_read < 0)
                return current_read;
            read_bytes += current_read;
            if(current_read < to_read)
                return read_bytes;
        }
        m_bin_file.seek((sector + 1) * BIN_SECTOR_SIZE + BIN_HEADER_SIZE + BIN_SECTOR_OFFSET);
    }
    return read_bytes;
}
