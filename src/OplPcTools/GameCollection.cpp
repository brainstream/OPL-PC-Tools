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
#include <OplPcTools/GameCollection.h>

using namespace OplPcTools;

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
    QObject(_parent),
    mp_ul_conf_storage(new UlConfigGameStorage),
    mp_dir_storage(new DirectoryGameStorage)
{
    connect(mp_dir_storage, &DirectoryGameStorage::gameRenamed, this, &GameCollection::gameRenamed);
    connect(mp_ul_conf_storage, &UlConfigGameStorage::gameRenamed, this, &GameCollection::gameRenamed);
    connect(mp_dir_storage, &DirectoryGameStorage::gameRegistered, this, &GameCollection::gameAdded);
    connect(mp_ul_conf_storage, &UlConfigGameStorage::gameRegistered, this, &GameCollection::gameAdded);
    connect(mp_dir_storage, &DirectoryGameStorage::gameAboutToBeDeleted, this, &GameCollection::gameAboutToBeDeleted);
    connect(mp_ul_conf_storage, &UlConfigGameStorage::gameAboutToBeDeleted, this, &GameCollection::gameAboutToBeDeleted);
    connect(mp_dir_storage, &DirectoryGameStorage::gameDeleted, this, &GameCollection::gameDeleted);
    connect(mp_ul_conf_storage, &UlConfigGameStorage::gameDeleted, this, &GameCollection::gameDeleted);
}

GameCollection::~GameCollection()
{
    delete mp_ul_conf_storage;
    delete mp_dir_storage;
}

void GameCollection::load(const QDir & _directory)
{
    mp_ul_conf_storage->load(_directory);
    mp_dir_storage->load(_directory);
    m_directory = _directory.absolutePath();
    emit loaded();
}

bool GameCollection::isLoaded() const
{
    return !m_directory.isEmpty();
}

const QString & GameCollection::directory() const
{
    return m_directory;
}

int GameCollection::count() const
{
    return mp_ul_conf_storage->count() + mp_dir_storage->count();
}

const Game * GameCollection::operator [](int _index) const
{
    int dir_index = _index - mp_ul_conf_storage->count();
    const Game * game = dir_index < 0 ? mp_ul_conf_storage->operator [](_index) :
        mp_dir_storage->operator [](dir_index);
    return game;
}

const Game * GameCollection::findGame(const QString & _id) const
{
    const Game * game = mp_ul_conf_storage->findGame(_id);
    if(!game) game = mp_dir_storage->findGame(_id);
    return game;
}

void GameCollection::addGame(const Game & _game)
{
    if(findGame(_game.id()))
        throw ValidationException(QObject::tr("Game \"%1\" already registered").arg(_game.id()));
    storage(_game.installationType()).registerGame(_game);
}

GameStorage & GameCollection::storage(GameInstallationType _installation_type) const
{
    if(_installation_type == GameInstallationType::Directory)
        return *mp_dir_storage;
    else
        return *mp_ul_conf_storage;
}

void GameCollection::renameGame(const Game & _game, const QString & _title)
{
    if(!storage(_game.installationType()).renameGame(_game.id(), _title))
        throw Exception(tr("Unable to rename game \"%1\" to \"%2\"").arg(_game.title()).arg(_title));
}

void GameCollection::deleteGame(const Game & _game)
{
    if(!storage(_game.installationType()).deleteGame(_game.id()))
        throw Exception(tr("Unable to delete game \"%1\"").arg(_game.title()));
}
