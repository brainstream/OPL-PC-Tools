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

#include <QFile>
#include <OplPcTools/Core/GameArtManager.h>

using namespace OplPcTools::Core;

namespace {

const QString art_directory("ART");
const QString sfx_icon("_ICO");
const QString sfx_front_cover("_COV");
const QString sfx_back_cover("_COV2");
const QString sfx_spine_cover("_LAB");
const QString sfx_screenshot1("_SCR");
const QString sfx_screenshot2("_SCR2");
const QString sfx_background("_BG");
const QString sfx_logo("_LGO");

QString suffix(GameArtType _art_type)
{
    switch(_art_type)
    {
    case GameArtType::Icon:
        return sfx_icon;
    case GameArtType::Front:
        return sfx_front_cover;
    case GameArtType::Back:
        return sfx_back_cover;
    case GameArtType::Spine:
        return sfx_spine_cover;
    case GameArtType::Screenshot1:
        return sfx_screenshot1;
    case GameArtType::Screenshot2:
        return sfx_screenshot2;
    case GameArtType::Background:
        return sfx_background;
    case GameArtType::Logo:
        return sfx_logo;
    default:
        return QString();
    }
}

} // namespace

GameArtManager::GameArtManager(const QDir & _base_directory) :
    m_cached_types(0)
{
    m_directory_path = _base_directory.absoluteFilePath(art_directory);
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
    const QString sfx = suffix(_type);
    for(const QString & ext : exts)
    {
        QFile file(dir.absoluteFilePath(_game_id + sfx + ext));
        if(!file.exists()) continue;
        QPixmap pixmap(file.fileName()); // TODO: scale
        if(pixmap.isNull()) continue;
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
