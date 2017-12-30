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

#include "DirectoryGameStorage.h"

using namespace OplPcTools::Core;

DirectoryGameStorage::DirectoryGameStorage(QObject * _parent /*= nullptr*/) :
    GameStorage(_parent)
{
}

bool DirectoryGameStorage::load(const QDir & _directory)
{
    clear();
    for(int i = 0; i < 20; ++i)
    {
        Game * game = createGame(QString("DIR_ID_#%1").arg(i));
        game->setTitle(QString("#%1. The test DIR game").arg(i));
    }
    return true;
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
