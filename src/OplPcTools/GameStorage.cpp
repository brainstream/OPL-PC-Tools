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
    return m_games[_index];
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

Game * GameStorage::createGame(const QString & _id)
{
    Game * game = new Game(_id, installationType());
    m_games.append(game);
    return game;
}

bool GameStorage::renameGame(const QString & _id, const QString & _title)
{
    return renameGame(findNonConstGame(_id), _title);
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
        emit gameRenamed(_game->id());
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
        emit gameRegistered(game->id());
        return true;
    }
    return false;
}

void GameStorage::validateId(const QString & _id)
{
    if(_id.toLatin1().size() > max_id_length)
        throw ValidationException(QObject::tr("Maximum image name length is %1 bytes").arg(max_id_length));
}

bool GameStorage::deleteGame(const QString & _id)
{
    for(int i = m_games.count() - 1; i >= 0; --i)
    {
        Game * game = m_games.at(i);
        if(game->id() == _id)
        {
            if(performDeletion(*game))
            {
                emit gameAboutToBeDeleted(_id);
                m_games.remove(i);
                delete game;
                emit gameDeleted(_id);
                return true;
            }
            break;
        }
    }
    return false;
}
