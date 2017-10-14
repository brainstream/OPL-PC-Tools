/***********************************************************************************************
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

#include <QStorageInfo>
#include <QThread>
#include <OplPcTools/Misc/IOException.h>
#include <OplPcTools/Core/DirectoryGameInstaller.h>


DirectoryGameInstaller::DirectoryGameInstaller(Device & _device, GameCollection & _collection, QObject * _parent /*= nullptr*/) :
    GameInstaller(_device, _collection, _parent),
    m_move_file(false),
    m_rename_file(false)
{
}

bool DirectoryGameInstaller::performInstallation()
{
    if(m_move_file && mr_device.isReadOnly())
    {
        throw IOException(tr("It is impossible to move the file \"%1\". "
            "Probably it is read-only or not an ISO image.").arg(mr_device.filepath()));
    }
    QDir dest_dir(mr_collection.directory());
    QString dest_subdir = deviceMediaType() == MediaType::CD ? GameCollection::cdDirectory() : GameCollection::dvdDirectory();
    if(!dest_dir.cd(dest_subdir))
        dest_dir.mkdir(dest_subdir);
    dest_dir.cd(dest_subdir);
    mp_installed_game->id = mr_device.gameId();
    mp_installed_game->media_type = deviceMediaType();
    mp_installed_game->title = mr_device.title();
    mp_installed_game->installation_type = GameInstallationType::Directory;
    mp_installed_game->part_count = 1;
    QString dest_filepath = m_rename_file ?
        dest_dir.absoluteFilePath(makeGameIsoFilename(mp_installed_game->title, mp_installed_game->id)) :
        dest_dir.absoluteFilePath(mp_installed_game->title + ".iso");
    if(m_move_file && QStorageInfo(mr_device.filepath()).device() == QStorageInfo(dest_dir).device())
    {
        quint64 iso_size = mr_device.size();
        QFile::rename(mr_device.filepath(), dest_filepath);
        emit progress(iso_size, iso_size);
    }
    else
    {
        if(!mr_device.open())
            throw IOException(tr("Unable to open device to read: \"%1\"").arg(mr_device.filepath()));
        try
        {
        if(!copyDeviceTo(dest_filepath))
            return false;
        }
        catch(...)
        {
            mr_device.close();
            throw;
        }
        mr_device.close();
        if(m_move_file)
            QFile::remove(mr_device.filepath());
    }
    try
    {
        registerGame();
    }
    catch(...)
    {
        rollback(dest_filepath);
        throw;
    }
    return true;
}

bool DirectoryGameInstaller::copyDeviceTo(const QString & _dest)
{
    const ssize_t read_size = 4194304;
    QByteArray bytes(read_size, Qt::Initialization::Uninitialized);
    QFile dest(_dest);
    if(dest.exists())
        throw IOException(tr("File already exists: \"%1\"").arg(dest.fileName()));
    if(!dest.open(QIODevice::WriteOnly))
        throw IOException(tr("Unable to open file to write: \"%1\"").arg(dest.fileName()));
    const quint64 iso_size = mr_device.size();
    mr_device.seek(0);
    for(quint64 total_read_bytes = 0, write_operation = 0; total_read_bytes < iso_size; ++write_operation)
    {
        qint64 read_bytes = mr_device.read(bytes);
        if(read_bytes < 0)
        {
            dest.close();
            rollback(_dest);
            throw IOException(tr("An error occurred during reading the source medium"));
        }
        else if(read_bytes > 0)
        {
            if(dest.write(bytes.constData(), read_bytes) != read_bytes)
            {
                dest.close();
                rollback(_dest);
                throw IOException(tr("Unable to write a data into the file: \"%1\"").arg(dest.fileName()));
            }
            if(++write_operation % 5 == 0)
                dest.flush();
            total_read_bytes += read_bytes;
            emit progress(iso_size, total_read_bytes);
        }
        if(read_bytes < read_size)
        {
            emit progress(iso_size, iso_size);
            break;
        }
        if(QThread::currentThread()->isInterruptionRequested())
        {
            rollback(_dest);
            return false;
        }
    }
    return true;
}

void DirectoryGameInstaller::rollback(const QString & _dest)
{
    emit rollbackStarted();
    if(m_move_file && !QFile::exists(mr_device.filepath()))
        QFile::rename(_dest, mr_device.filepath());
    else
        QFile::remove(_dest);
    emit rollbackFinished();
}

void DirectoryGameInstaller::registerGame()
{
    emit registrationStarted();
    mr_collection.addGame(*mp_installed_game);
    emit registrationFinished();
}
