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

#include <QVector>
#include <OplPcTools/Exception.h>
#include <OplPcTools/DirectoryGameStorage.h>
#include <OplPcTools/Device.h>
#include <OplPcTools/Iso9660DeviceSource.h>

using namespace OplPcTools;

const QString DirectoryGameStorage::cd_directory("CD");
const QString DirectoryGameStorage::dvd_directory("DVD");

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
    if(!base_directory.cd(_media_type == MediaType::CD ? cd_directory : dvd_directory))
        return;
    for(const QString & iso : base_directory.entryList({ "*.iso" }))
    {
        Device image(QSharedPointer<DeviceSource>(new Iso9660DeviceSource(base_directory.absoluteFilePath(iso))));
        if(!image.init())
            break;
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
    validateTitle(_title);
    QDir directory(m_base_directory);
    if(!directory.cd(_game.mediaType() == MediaType::CD ? cd_directory : dvd_directory))
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
    return true;
}

QString DirectoryGameStorage::makeIsoFilename(const QString & _title, const QString & _id)
{
    return QString("%1.%2.iso").arg(_id).arg(_title);
}

QString DirectoryGameStorage::makeIsoFilename(const QString & _title)
{
    return _title + ".iso";
}

void DirectoryGameStorage::validateTitle(const QString & _title)
{
    static const QString disallowed_characters("<>:\"/\\|?*");
    for(const QChar & ch : disallowed_characters)
    {
       if(_title.contains(ch))
           throw ValidationException(QObject::tr("Name must not contain following symbols: ") + disallowed_characters);
    }
}

QString DirectoryGameStorage::makeGameIsoFilename(const QString & _title, const QString & _id)
{
    return QString("%1.%2.iso").arg(_id).arg(_title);
}

bool DirectoryGameStorage::performDeletion(const Game & _game)
{
    QDir dir(m_base_directory);
    dir.cd(_game.mediaType() == MediaType::CD ? cd_directory : dvd_directory);
    QString path = dir.absoluteFilePath(_game.title()) + ".iso";
    if(QFile::exists(path) && QFile::remove(path))
        return true;
    path = dir.absoluteFilePath(makeGameIsoFilename(_game.title(), _game.id()));
    return QFile::exists(path) && QFile::remove(path);
}
