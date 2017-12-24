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

#include <OplPcTools/Core/GameCollection.h>

using namespace OplPcTools::Core;

namespace {

template<class TCollection>
auto findGameById(TCollection & _collection, const QString & _id) -> typename TCollection::value_type
{
    for(auto item : _collection)
    {
        if(item->id() == _id)
            return item;
    }
    return nullptr;
}

} // namespace

GameCollection::GameCollection(QObject * _parent /*= nullptr*/) :
    QObject(_parent)
{
}

GameCollection::~GameCollection()
{
    for(Game * game : m_games)
        delete game;
}

void GameCollection::load(const QString & _directory)
{
    m_games.clear();
    for(int i = 0; i < 20; ++i)
    {
        Game * game = new Game(QString("ID_#%1").arg(i));
        game->setTitle(QString("The test game #%1").arg(i));
        m_games.append(game);
    }
    m_directory = _directory;
    emit loaded();
}

const Game * GameCollection::findGame(const QString & _id) const
{
    return findGameById(m_games, _id);
}

Game * GameCollection::findGame(const QString & _id)
{
    return findGameById(m_games, _id);
}
