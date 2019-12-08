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

#include <QFile>
#include <QDir>
#include <QThread>
#include <OplPcTools/Exception.h>
#include <OplPcTools/UlConfigGameInstaller.h>

using namespace OplPcTools;

UlConfigGameInstaller::UlConfigGameInstaller(Device & _device, GameCollection & _collection, QObject * _parent) :
    GameInstaller(_device, _collection, _parent),
    mp_game(nullptr)
{
}

UlConfigGameInstaller::~UlConfigGameInstaller()
{
    delete mp_game;
}

bool UlConfigGameInstaller::install()
{
    if(!mr_device.open())
    {
        throw IOException(tr("Unable to open device file to read: \"%1\"").arg(mr_device.filepath()));
    }
    delete mp_game;
    mp_game = new Game(mr_device.gameId(), GameInstallationType::UlConfig);
    mp_game->setTitle(mr_device.title());
    mp_game->setMediaType(deviceMediaType());
    UlConfigGameStorage::validateId(mp_game->id());
    UlConfigGameStorage::validateTitle(mp_game->title());
    const quint64 iso_size = mr_device.size();
    const ssize_t part_size = 1073741824;
    const ssize_t read_part_size = 4194304;
    quint64 processed_bytes = 0;
    unsigned int write_operation = 0;
    QDir dest_dir(mr_collection.directory());
    QByteArray bytes(read_part_size, Qt::Initialization::Uninitialized);
    mr_device.seek(0);
    quint8 part_count = 0;
    for(bool unexpected_finish = false; !unexpected_finish && processed_bytes < iso_size; ++part_count)
    {
        QString part_filename = UlConfigGameStorage::makePartFilename(mp_game->id(), mp_game->title(), part_count);
        QFile part(dest_dir.absoluteFilePath(part_filename));
        if(part.exists())
        {
            rollback();
            throw IOException(tr("File already exists: \"%1\"").arg(part.fileName()));
        }
        if(!part.open(QIODevice::WriteOnly))
        {
            rollback();
            throw IOException(tr("Unable to open file to write: \"%1\"").arg(part.fileName()));
        }
        m_written_parts.append(part.fileName());
        for(quint64 total_read_bytes = 0; total_read_bytes < part_size;)
        {
            qint64 read_bytes = mr_device.read(bytes);
            if(read_bytes < 0)
            {
                part.close();
                rollback();
                throw IOException(tr("An error occurred during reading the source medium"));
            }
            else if(read_bytes > 0)
            {
                if(part.write(bytes.constData(), read_bytes) != read_bytes)
                {
                    part.close();
                    rollback();
                    throw IOException(tr("Unable to write a data into the file: \"%1\"").arg(part.fileName()));
                }
                if(++write_operation % 5 == 0)
                    part.flush();
                total_read_bytes += read_bytes;
                processed_bytes += read_bytes;
            }
            if(read_bytes < read_part_size)
            {
                // Yes. It is a real scenario. The "Final Fantasy XII" declares the ISO FS size larger than it is.
                unexpected_finish = true;
                emit progress(processed_bytes, processed_bytes);
                break;
            }
            emit progress(iso_size, processed_bytes);
            if(QThread::currentThread()->isInterruptionRequested())
            {
                rollback();
                return false;
            }
        }
    }
    mp_game->setPartCount(part_count);
    try
    {
        registerGame();
    }
    catch(...)
    {
        rollback();
        throw;
    }
    mr_device.close();
    m_written_parts.clear();
    return true;
}

void UlConfigGameInstaller::rollback()
{
    if(mr_device.isOpen())
        mr_device.close();
    emit rollbackStarted();
    for(const QString & path : m_written_parts)
        QFile::remove(path);
    m_written_parts.clear();
    delete mp_game;
    mp_game = nullptr;
    emit rollbackFinished();
}

void UlConfigGameInstaller::registerGame()
{
    emit registrationStarted();
    mr_collection.addGame(*mp_game);
    emit registrationFinished();
}
