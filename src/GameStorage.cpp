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

#include <OplPcTools/Core/GameStorage.h>

using namespace OplPcTools::Core;

GameStorage::GameStorage(QObject * _parent /*= nullptr*/) :
    QObject(_parent)
{
}

GameStorage::~GameStorage()
{
    clear();
}

void GameStorage::clear()
{
    for(Game * game : m_games)
        delete game;
    m_games.clear();
}

const Game * GameStorage::operator [](int _index) const
{
    return gameAt(_index);
}

Game * GameStorage::gameAt(int _index) const
{
    // TODO: exception
    return m_games[_index];
}

int GameStorage::count() const
{
    return m_games.count();
}

Game * GameStorage::createGame(const QString & _id)
{
    Game * game = new Game(_id);
    m_games.append(game);
    return game;
}

const Game * GameStorage::findGame(const QString & _id) const
{
    return findNonConstGame(_id);
}

Game * GameStorage::findNonConstGame(const QString & _id) const
{
    int count = m_games.count();
    for(int i = 0; i < count; ++i)
    {
        if(_id == m_games[i]->id())
            return m_games[i];
    }
    return nullptr;
}
