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

#include <OplPcTools/DirectoryGameStorage.h>
#include <OplPcTools/Iso9660DeviceSource.h>
#include <OplPcTools/Device.h>
#include <OplPcTools/File.h>
#include <OplPcTools/StandardDirectories.h>
#include <QVector>

using namespace OplPcTools;

DirectoryGameStorage::DirectoryGameStorage(QObject * _parent /*= nullptr*/) :
    GameStorage(_parent)
{
}

GameInstallationType DirectoryGameStorage::installationType() const
{
    return GameInstallationType::Directory;
}

bool DirectoryGameStorage::performLoading(const QDir & _directory)
{
    m_base_directory = _directory.absolutePath();
    loadDirectory(MediaType::CD);
    loadDirectory(MediaType::DVD);
    return true;
}

void DirectoryGameStorage::loadDirectory(MediaType _media_type)
{
    QDir base_directory(m_base_directory);
    if(!base_directory.cd(_media_type == MediaType::CD ? StandardDirectories::cd : StandardDirectories::dvd))
        return;
    foreach(const QString & iso, base_directory.entryList({ "*.iso" }))
    {
        Device image(QSharedPointer<DeviceSource>(new Iso9660DeviceSource(base_directory.absoluteFilePath(iso))));
        if(!image.init())
            continue;
        Game * game = createGame(image.gameId());
        game->setMediaType(_media_type);
        game->setPartCount(1);
        QString title = QFileInfo(image.filepath()).fileName();
        title = title.left(title.lastIndexOf('.'));
        if(title.startsWith(image.gameId()))
            game->setTitle(title.right(title.size() - image.gameId().size() - 1));
        else
            game->setTitle(title);
    }
}

bool DirectoryGameStorage::performRenaming(const Game & _game, const QString & _title)
{
    if(_game.title() == _title)
        return true;
    QDir directory(m_base_directory);
    if(!directory.cd(_game.mediaType() == MediaType::CD ? StandardDirectories::cd : StandardDirectories::dvd))
        return false;
    QString old_filename = directory.absoluteFilePath(makeIsoFilename(_game.title(), _game.id()));
    bool is_name_included_id = true;
    if(!QFile::exists(old_filename))
    {
        is_name_included_id = false;
        old_filename = directory.absoluteFilePath(makeIsoFilename(_game.title()));
        if(!QFile::exists(old_filename))
            return false;
    }
    QString new_filename = directory.absoluteFilePath(
        is_name_included_id ? makeIsoFilename(_title, _game.id()) : makeIsoFilename(_title));
    return QFile::rename(old_filename, new_filename);
}

bool DirectoryGameStorage::performRegistration(const Game & _game)
{
    Q_UNUSED(_game)
    return true;
}

QString DirectoryGameStorage::makeIsoFilename(const QString & _title, const QString & _id)
{
    return QString("%1.%2.iso").arg(_id, _title);
}

QString DirectoryGameStorage::makeIsoFilename(const QString & _title)
{
    return _title + ".iso";
}

QString DirectoryGameStorage::makeGameIsoFilename(const QString & _title, const QString & _id)
{
    return QString("%1.%2.iso").arg(_id, _title);
}

bool DirectoryGameStorage::performDeletion(const Game & _game)
{
    QDir dir(m_base_directory);
    dir.cd(_game.mediaType() == MediaType::CD ? StandardDirectories::cd : StandardDirectories::dvd);
    QString path = dir.absoluteFilePath(_game.title()) + ".iso";
    if(QFile::exists(path) && QFile::remove(path))
        return true;
    path = dir.absoluteFilePath(makeGameIsoFilename(_game.title(), _game.id()));
    return QFile::exists(path) && QFile::remove(path);
}
