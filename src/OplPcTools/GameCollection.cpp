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

#include <OplPcTools/GameCollection.h>
#include <OplPcTools/UlConfigGameStorage.h>
#include <OplPcTools/DirectoryGameStorage.h>
#include <OplPcTools/GameInstallationTypeUtils.h>
#include <OplPcTools/Exception.h>

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

class GameCollection::Storages final : public GameInstallationTypeUtilityFactory<GameStorage &>
{
public:
    Storages() :
        m_storages
        {
            { GameInstallationType::UlConfig, new UlConfigGameStorage },
            { GameInstallationType::Ziso, new ZisoGameStorage },
            { GameInstallationType::Iso9660, new Iso9660GameStorage }
        }
    {
    }

    ~Storages() override
    {
        for(auto & pair : m_storages)
            delete pair.second;
    }

    void forEach(std::function<void(GameStorage &)> _cb)
    {
        for(auto & pair : m_storages)
            _cb(*pair.second);
    }

    const Game * findGame(std::function<std::optional<const Game *>(const GameStorage &)> _cb) const
    {
        for(auto & pair : m_storages)
        {
            std::optional<const Game *> result = _cb(*pair.second);
            if(result.has_value())
                return *result;
        }
        return nullptr;
    }

    template<typename T>
    T aggregate(T _seed, std::function<T(T, const GameStorage &)> _cb) const
    {
        T val = _seed;
        for(auto & pair : m_storages)
            val = _cb(val, *pair.second);
        return val;
    }

protected:
    GameStorage & produceForUlConfig() const override
    {
        return *m_storages.at(GameInstallationType::UlConfig);
    }

    GameStorage & produceForIso9660() const override
    {
        return *m_storages.at(GameInstallationType::Iso9660);
    }

    GameStorage & produceForZiso() const override
    {
        return *m_storages.at(GameInstallationType::Ziso);
    }

private:
    std::map<GameInstallationType, GameStorage *> m_storages;
};

GameCollection::GameCollection(QObject * _parent /*= nullptr*/) :
    QObject(_parent),
    mp_storages(new Storages)
{
    mp_storages->forEach([this](GameStorage & __storage)
    {
        connect(&__storage, &GameStorage::gameRenamed, this, &GameCollection::gameRenamed);
        connect(&__storage, &GameStorage::gameRegistered, this, &GameCollection::gameAdded);
        connect(&__storage, &GameStorage::gameAboutToBeDeleted, this, &GameCollection::gameAboutToBeDeleted);
        connect(&__storage, &GameStorage::gameDeleted, this, &GameCollection::gameDeleted);
    });
}

GameCollection::~GameCollection()
{
    delete mp_storages;
}

void GameCollection::load(const QDir & _directory)
{
    mp_storages->forEach([_directory](GameStorage & __storage) { __storage.load(_directory); });
    m_directory = _directory.absolutePath();
}

bool GameCollection::isLoaded() const
{
    return !m_directory.isEmpty();
}

int GameCollection::count() const
{
    return mp_storages->aggregate<int>(
        0,
        [](int __count, const GameStorage & __storage) { return __count + __storage.count(); });
}

const Game * GameCollection::operator [](int _index) const
{
    int idx = _index;
    return mp_storages->findGame(
        [&idx](const GameStorage & __storage)
        {
            if(idx < __storage.count())
                return std::make_optional(__storage[idx]);
            idx -= __storage.count();
            return std::optional<const Game *>();
        }
    );
}

const Game * GameCollection::findGame(const Uuid & _uuid) const
{
    return mp_storages->findGame(
        [_uuid](const GameStorage & __storage)
        {
            const Game * game = __storage.findGame(_uuid);
            return game ? std::make_optional(game) : std::optional<const Game *>();
        }
    );
}

const Game * GameCollection::findGame(const QString & _game_id) const
{
    for(int i = count() - 1; i >= 0; --i)
    {
        const Game * game = (*this)[i];
        if(game->id() == _game_id)
            return game;
    }
    return nullptr;
}

void GameCollection::addGame(const Game & _game)
{
    if(findGame(_game.uuid()))
        throw ValidationException(QObject::tr("Game \"%1\" already registered").arg(_game.id()));
    mp_storages->produce(_game.installationType()).registerGame(_game);
}

void GameCollection::renameGame(const Game & _game, const QString & _title)
{
    if(!mp_storages->produce(_game.installationType()).renameGame(_game.uuid(), _title))
        throw Exception(tr("Unable to rename game \"%1\" to \"%2\"").arg(_game.title(), _title));
}

void GameCollection::deleteGame(const Game & _game)
{
    if(!mp_storages->produce(_game.installationType()).deleteGame(_game.uuid()))
        throw Exception(tr("Unable to delete game \"%1\"").arg(_game.title()));
}

bool GameCollection::contains(const QString & _game_id) const
{
    return findGame(_game_id) != nullptr;
}
