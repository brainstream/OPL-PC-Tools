/***********************************************************************************************
 * Copyright © 2017-2025 Sergey Smolyannikov aka brainstream                                   *
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

#include <OplPcTools/Exception.h>
#include <OplPcTools/GameStorage.h>

using namespace OplPcTools;

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
    foreach(Game * game, m_games)
        delete game;
    m_games.clear();
}

const Game * GameStorage::operator [](int _index) const
{
    return gameAt(_index);
}

Game * GameStorage::gameAt(int _index) const
{
    return m_games[_index];
}

const Game * GameStorage::findGame(const Uuid & _uuid) const
{
    return findNonConstGame(_uuid);
}

Game * GameStorage::findNonConstGame(const Uuid & _uuid) const
{
    int count = m_games.count();
    for(int i = 0; i < count; ++i)
    {
        if(_uuid == m_games[i]->uuid())
            return m_games[i];
    }
    return nullptr;
}

bool GameStorage::load(const QDir & _directory)
{
    clear();
    if(performLoading(_directory))
    {
        emit loaded();
        return true;
    }
    return false;
}

int GameStorage::count() const
{
    return m_games.count();
}

Game * GameStorage::createGame(const QString & _game_id)
{
    Game * game = new Game(_game_id, installationType());
    m_games.append(game);
    return game;
}

bool GameStorage::renameGame(const Uuid & _uuid, const QString & _title)
{
    return renameGame(findNonConstGame(_uuid), _title);
}

bool GameStorage::renameGame(const int _index, const QString & _title)
{
    return renameGame(m_games[_index], _title);
}

bool GameStorage::renameGame(Game * _game, const QString & _title)
{
    if(_game && performRenaming(*_game, _title))
    {
        _game->setTitle(_title);
        emit gameRenamed(_game->uuid());
        return true;
    }
    return false;
}

bool GameStorage::registerGame(const Game & _game)
{
    if(performRegistration(_game))
    {
        Game * game = new Game(_game);
        m_games.append(game);
        emit gameRegistered(game->uuid());
        return true;
    }
    return false;
}

void GameStorage::validateId(const QString & _id)
{
    if(_id.toLatin1().size() > max_id_length)
        throw ValidationException(QObject::tr("Maximum image name length is %1 bytes").arg(max_id_length));
}

bool GameStorage::deleteGame(const Uuid & _uuid)
{
    for(int i = m_games.count() - 1; i >= 0; --i)
    {
        Game * game = m_games.at(i);
        if(game->uuid() == _uuid)
        {
            if(performDeletion(*game))
            {
                emit gameAboutToBeDeleted(_uuid);
                m_games.remove(i);
                delete game;
                emit gameDeleted(_uuid);
                return true;
            }
            break;
        }
    }
    return false;
}
