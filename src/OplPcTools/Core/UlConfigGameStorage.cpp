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

#include <OplPcTools/Core/UlConfigGameStorage.h>

using namespace OplPcTools::Core;

UlConfigGameStorage::UlConfigGameStorage(QObject * _parent /*= nullptr*/) :
    GameStorage(_parent)
{
}

UlConfigGameStorage::~UlConfigGameStorage()
{
}

bool UlConfigGameStorage::load(const QDir & _directory)
{
    clear();
    for(int i = 0; i < 20; ++i)
    {
        Game * game = createGame(QString("UL_ID_#%1").arg(i));
        game->setTitle(QString("#%1. The test UL game").arg(i));
    }
    return true;
}

bool UlConfigGameStorage::renameGame(const QString & _id, const QString & _title)
{
    // TODO: exception
    Game * game = findNonConstGame(_id);
    return game && renameGame(*game, _title);
}

bool UlConfigGameStorage::renameGame(const int _index, const QString & _title)
{
    // TODO: exception
    return renameGame(*gameAt(_index), _title);
}

bool UlConfigGameStorage::renameGame(Game & _game, const QString & _title)
{
    if(renameGameInConfig(_game.id(), _title))
    {
        _game.setTitle(_title);
        return true;
    }
    return false;
}

bool UlConfigGameStorage::renameGameInConfig(const QString & _id, const QString & _title)
{
    // TODO: update ul.conf
    return true;
}

bool UlConfigGameStorage::registerGame(const Game & _game)
{
    // TODO: register game
    return true;
}
