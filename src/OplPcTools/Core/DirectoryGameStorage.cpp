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

#include <OplPcTools/Core/DirectoryGameStorage.h>
#include <OplPcTools/Core/Device.h>
#include <OplPcTools/Core/Iso9660DeviceSource.h>

using namespace OplPcTools::Core;

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

bool DirectoryGameStorage::load(const QDir & _directory)
{
    clear();
    load(_directory, MediaType::CD);
    load(_directory, MediaType::DVD);
    return true;
}

void DirectoryGameStorage::load(QDir _base_directory, MediaType _media_type)
{
    if(!_base_directory.cd(_media_type == MediaType::CD ? cd_directory : dvd_directory))
        return;
    for(const QString & iso : _base_directory.entryList({ "*.iso" }))
    {
        Device image(QSharedPointer<DeviceSource>(new Iso9660DeviceSource(_base_directory.absoluteFilePath(iso))));
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

bool DirectoryGameStorage::renameGame(const QString & _id, const QString & _title)
{
    // TODO: exception
    Game * game = findNonConstGame(_id);
    return game && renameGame(*game, _title);
}

bool DirectoryGameStorage::renameGame(const int _index, const QString & _title)
{
    // TODO: exception
    return renameGame(*gameAt(_index), _title);
}

bool DirectoryGameStorage::renameGame(Game & _game, const QString & _title)
{
    if(renameGameFiles(_game.id(), _title))
    {
        _game.setTitle(_title);
        return true;
    }
    return false;
}

bool DirectoryGameStorage::renameGameFiles(const QString & _id, const QString & _title)
{
    // TODO: rename files
    return true;
}

bool DirectoryGameStorage::registerGame(const Game & _game)
{
    // TODO: register game
    return true;
}
