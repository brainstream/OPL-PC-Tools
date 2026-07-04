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

#include <OplPcTools/Library.h>
#include <OplPcTools/DirectoryGameInstaller.h>
#include <OplPcTools/DirectoryGameStorage.h>
#include <OplPcTools/File.h>
#include <OplPcTools/Exception.h>
#include <OplPcTools/StandardPaths.h>
#include <OplPcTools/Constants.h>
#include <QStorageInfo>

using namespace OplPcTools;

DirectoryGameInstaller::DirectoryGameInstaller(
    DeviceReader & _reader,
    std::unique_ptr<DeviceWriter> && _writer,
    QObject * _parent /*= nullptr*/
) :
    GameInstaller(_reader, _parent),
    m_move_file(false),
    m_rename_file(false),
    mp_game(nullptr),
    m_writer_ptr(std::move(_writer))
{
    connect(m_writer_ptr.get(), &DeviceWriter::progress, this, &DirectoryGameInstaller::progress);
}

DirectoryGameInstaller::~DirectoryGameInstaller()
{
    delete mp_game;
}

bool DirectoryGameInstaller::performInstallation()
{
    delete mp_game;
    mp_game = new Game(
        mr_device.gameId(),
        mr_device.isCompressed() ? GameInstallationType::Ziso : GameInstallationType::Iso9660);
    mp_game->setMediaType(deviceMediaType());
    mp_game->setTitle(mr_device.title());
    QDir dest_dir(Library::instance().directory());
    QString dest_subdir = mp_game->mediaType() == MediaType::CD ? StandardDirectories::cd : StandardDirectories::dvd;
    if(!dest_dir.cd(dest_subdir))
        dest_dir.mkdir(dest_subdir);
    dest_dir.cd(dest_subdir);
    const QString dest_file_ext(mr_device.isCompressed() ? g_file_ext_zso : g_file_ext_iso);
    QString dest_filepath = m_rename_file
        ? dest_dir.absoluteFilePath(DirectoryGameStorage::makeGameIsoFilename(mp_game->title(), mp_game->id(), dest_file_ext))
        : dest_dir.absoluteFilePath(mp_game->title() + dest_file_ext);
    if(m_move_file && QStorageInfo(mr_device.filepath()).device() == QStorageInfo(dest_dir).device())
    {
        quint64 iso_size = mr_device.size();
        if(!QFile::rename(mr_device.filepath(), dest_filepath))
            throw IOException(tr("It is impossible to move the file \"%1\"").arg(mr_device.filepath()));
        emit progress(iso_size, iso_size);
    }
    else
    {
        if(!mr_device.open())
            throw IOException(tr("Unable to open device to read: \"%1\"").arg(mr_device.filepath()));
        try
        {
            if(!m_writer_ptr->write(mr_device, dest_filepath))
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

void DirectoryGameInstaller::rollback(const QString & _dest)
{
    emit rollbackStarted();
    if(m_move_file && !QFile::exists(mr_device.filepath()))
        QFile::rename(_dest, mr_device.filepath());
    else
        QFile::remove(_dest);
    delete mp_game;
    mp_game = nullptr;
    emit rollbackFinished();
}

void DirectoryGameInstaller::registerGame()
{
    emit registrationStarted();
    Library::instance().games().addGame(*mp_game);
    emit registrationFinished();
}
