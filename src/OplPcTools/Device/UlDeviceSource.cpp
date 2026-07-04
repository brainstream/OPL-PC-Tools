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

#include <OplPcTools/Device/UlDeviceSource.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/UlConfigGameStorage.h>
#include <OplPcTools/StandardPaths.h>
#include <OplPcTools/File.h>

using namespace OplPcTools;

UlDeviceSource::UlDeviceSource(const Game & _game) :
    m_game(_game),
    m_size(-1),
    mp_open_part(nullptr)
{
}

QString UlDeviceSource::filepath() const
{
    return QDir(Library::instance().directory()).absoluteFilePath(StandardFiles::ul_config);
}

bool UlDeviceSource::open()
{
    if(isOpen())
        return true;

    m_parts_ptr.reset();
    QDir games_dir(Library::instance().directory());
    qint64 begin = 0;
    qint64 size = 0;
    Part * tail = nullptr;
    for(quint8 part_idx = 0; part_idx < m_game.partCount(); ++part_idx)
    {
        const QString filename = games_dir.absoluteFilePath(
            UlConfigGameStorage::makePartFilename(m_game.id(), m_game.title(), part_idx));
        const QFileInfo file_info(filename);
        if(!file_info.isFile() || !file_info.isReadable())
            return false;
        const qint64 part_size = file_info.size();
        size += part_size;
        const qint64 end = begin + part_size;
        Part * part = new Part(file_info.absoluteFilePath(), begin, end);
        begin = end;
        if(tail)
        {
            tail->next = part;
            tail = part;
        }
        else
        {
            tail = part;
            m_parts_ptr.reset(part);
        }
    }
    if(!m_parts_ptr)
        return false;
    openPart(*m_parts_ptr);
    m_size = size;
    return true;
}

bool UlDeviceSource::isOpen() const
{
    return m_parts_ptr != nullptr && mp_open_part != nullptr;
}

void UlDeviceSource::close()
{
    m_file_ptr.reset();
    m_size = -1;
    m_parts_ptr.reset();
    mp_open_part = nullptr;
}

qint64 UlDeviceSource::isoSize() const
{
    return m_size;
}

bool UlDeviceSource::seek(qint64 _offset)
{
    if(!isOpen())
        return false;

    if(mp_open_part && mp_open_part->begin <= _offset && mp_open_part->end > _offset)
    {
        return m_file_ptr->seek(_offset - mp_open_part->begin);
    }
    for(const Part * part = m_parts_ptr.get(); part != nullptr; part = part->next)
    {
        if(part->begin >= _offset)
        {
            try
            {
                openPart(*part);
                return m_file_ptr->seek(_offset - mp_open_part->begin);
            }
            catch(...)
            {
                return false;
            }
        }
    }
    return false;
}

void UlDeviceSource::openPart(const Part & _part)
{
    std::unique_ptr<QFile> file(new QFile(_part.path));
    openFile(*file, QIODevice::ReadOnly);
    mp_open_part = &_part;
    m_file_ptr.swap(file);
}

qint64 UlDeviceSource::read(QByteArray & _buffer)
{
    if(!isOpen())
        return -1;

    quint64 remaining_bytes = _buffer.size();
    qsizetype buffer_pos = 0;
    qint64 total_read_bytes = 0;
    for(const Part * part = mp_open_part; part != nullptr; part = part->next)
    {
        qint64 read_bytes = m_file_ptr->read(&_buffer.data()[buffer_pos], remaining_bytes);
        remaining_bytes -= read_bytes;
        total_read_bytes += read_bytes;
        if(remaining_bytes == 0)
            return total_read_bytes;
        if(part->next)
        {
            openPart(*part->next);
            buffer_pos += read_bytes;
        }
        else
        {
            break;
        }
    }
    return total_read_bytes;
}
