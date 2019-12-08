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

#include <functional>
#include <QFile>
#include <OplPcTools/Exception.h>
#include <OplPcTools/GameArtManager.h>

using namespace OplPcTools;

struct GameArtManager::GameArtProperties
{
    QString suffix;
    QSize size;
};

GameArtManager::GameArtManager(const QDir & _base_directory, QObject * _parent /*= nullptr*/) :
    QObject(_parent),
    m_cached_types(0)
{
    m_directory_path = _base_directory.absoluteFilePath("ART");
    initArtProperties();
}

GameArtManager::~GameArtManager()
{
    for(GameArtProperties * props : m_art_props)
        delete props;
}

void GameArtManager::initArtProperties()
{
    m_art_props[GameArtType::Icon] = new GameArtProperties {
        "_ICO",
        QSize(64, 64)
    };
    m_art_props[GameArtType::Front] = new GameArtProperties {
        "_COV",
        QSize(140, 200)
    };
    m_art_props[GameArtType::Back] = new GameArtProperties {
        "_COV2",
        QSize(242, 344)
    };
    m_art_props[GameArtType::Spine] = new GameArtProperties {
        "_LAB",
        QSize(18, 240)
    };
    m_art_props[GameArtType::Screenshot1] = new GameArtProperties {
        "_SCR",
        QSize(250, 188)
    };
    m_art_props[GameArtType::Screenshot2] = new GameArtProperties {
        "_SCR2",
        QSize(250, 188)
    };
    m_art_props[GameArtType::Background] = new GameArtProperties {
        "_BG",
        QSize(640, 480)
    };
    m_art_props[GameArtType::Logo] = new GameArtProperties {
        "_LGO",
        QSize(300, 125)
    };
}

void GameArtManager::addCacheType(GameArtType _type)
{
    m_cached_types |= _type;
}

void GameArtManager::removeCacheType(GameArtType _type, bool _clear_cache)
{
    if((m_cached_types & _type) == 0)
        return;
    m_cached_types ^= _type;
    if(_clear_cache)
        clearCache(_type);
}

void GameArtManager::clearCache(GameArtType _type)
{
    for(auto & game_cache : m_cache)
    {
        auto it = game_cache->find(_type);
        if(it != game_cache->end())
            game_cache->erase(it);
    }
}

QPixmap GameArtManager::load(const QString & _game_id, GameArtType _type)
{
    Maybe<QPixmap> cached_pixmap = findInCache(_game_id, _type);
    if(m_cached_types & _type && cached_pixmap.hasValue())
        return cached_pixmap.value();
    QDir dir(m_directory_path);
    if(!dir.exists())
        return QPixmap();
    static const QStringList exts { ".png", ".jpeg", ".jpg", ".bmp" };
    const QString sfx = m_art_props[_type]->suffix;
    for(const QString & ext : exts)
    {
        QFile file(dir.absoluteFilePath(_game_id + sfx + ext));
        if(!file.exists()) continue;
        QPixmap pixmap(file.fileName());
        if(pixmap.isNull()) continue;
        const GameArtProperties * props = m_art_props[_type];
        if(pixmap.size() != props->size)
            pixmap = pixmap.scaled(props->size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if(m_cached_types & _type)
            cacheArt(_game_id, _type, pixmap);
        return pixmap;
    }
    return QPixmap();
}

Maybe<QPixmap> GameArtManager::findInCache(const QString & _game_id, GameArtType _type) const
{
    Maybe<GameCache> game_cache = m_cache[_game_id];
    if(!game_cache.hasValue())
        return Maybe<QPixmap>();
    return (*game_cache)[_type];
}

void GameArtManager::cacheArt(const QString & _game_id, GameArtType _type, const QPixmap & _pixmap)
{
    if(m_cache.contains(_game_id))
    {
        (*m_cache[_game_id])[_type] = _pixmap;
    }
    else
    {
        GameCache game_cache;
        game_cache[_type] = _pixmap;
        m_cache[_game_id] = game_cache;
    }
}

void GameArtManager::deleteArt(const QString & _game_id, GameArtType _type)
{
    Maybe<GameCache> & game_cache = m_cache[_game_id];
    if(game_cache.hasValue())
        game_cache->remove(_type);
    QStringList file_filter { QString("%1%2.*").arg(_game_id).arg(m_art_props[_type]->suffix) };
    bool changed = false;
    for(const QFileInfo & file_info : QDir(m_directory_path).entryInfoList(file_filter, QDir::Files))
    {
        if(QFile::remove(file_info.absoluteFilePath()))
            changed = true;
    }
    if(changed)
    {
        emit artChanged(_game_id, _type, nullptr);
    }
}

void GameArtManager::clearArts(const QString & _game_id)
{
    m_cache.remove(_game_id);
    QStringList file_filter { _game_id + "*.*" };
    for(const QFileInfo & file_info : QDir(m_directory_path).entryInfoList(file_filter, QDir::Files))
        QFile::remove(file_info.absoluteFilePath());
}

QPixmap GameArtManager::setArt(const QString & _game_id, GameArtType _type, const QString & _filepath)
{
    QPixmap pixmap(_filepath);
    if(pixmap.isNull())
        return pixmap;
    const GameArtProperties * props = m_art_props[_type];
    if(pixmap.size() != props->size)
        pixmap = pixmap.scaled(props->size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QDir dir(m_directory_path);
    if(!dir.exists())
        dir.mkpath(".");
    QString filename = dir.absoluteFilePath(_game_id + props->suffix + ".png");
    if(!pixmap.save(filename))
        throw IOException(QObject::tr("Unable to save file \"%1\"").arg(filename));
    if(m_cached_types & _type)
        cacheArt(_game_id, _type, pixmap);
    emit artChanged(_game_id, _type, &pixmap);
    return pixmap;
}
