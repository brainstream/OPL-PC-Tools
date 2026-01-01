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

#ifndef __OPLPCTOOLS_GAMEARTMANAGER__
#define __OPLPCTOOLS_GAMEARTMANAGER__

#include <QDir>
#include <OplPcTools/Maybe.h>
#include <OplPcTools/GameArt.h>
#include <OplPcTools/GameArtSource.h>

namespace OplPcTools {

class GameArtManager final : public QObject
{
    Q_OBJECT

    using GameCache = QMap<GameArtType, Maybe<QPixmap>>;
    using CacheMap = QMap<QString, Maybe<GameCache>>;

public:
    explicit GameArtManager(const QDir & _base_directory, QObject * _parent = nullptr);
    void addCacheType(GameArtType _type);
    void removeCacheType(GameArtType _type, bool _clear_cache);
    QPixmap load(const QString & _game_id, GameArtType _type);
    void deleteArt(const QString & _game_id, GameArtType _type);
    void clearArts(const QString & _game_id);
    void setArt(const QString & _game_id, GameArtType _type, const GameArtSource & _source);

    static QString makeArtFilename(const QString & _game_id, const GameArtProperties & _porperties)
    {
        return _game_id + _porperties.suffix + ".png";
    }

signals:
    void artChanged(const QString & _game_id, OplPcTools::GameArtType _type);

private:
    void clearCache(GameArtType _type);
    Maybe<QPixmap> findInCache(const QString & _game_id, GameArtType _type) const;
    void cacheArt(const QString & _game_id, GameArtType _type, const QPixmap & _pixmap);

private:
    QString m_directory_path;
    CacheMap m_cache;
    QFlags<GameArtType> m_cached_types;
    QMap<GameArtType, GameArtProperties> m_art_props;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMEARTMANAGER__
