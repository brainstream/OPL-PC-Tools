/***********************************************************************************************
 * Copyright © 2017-2021 Sergey Smolyannikov aka brainstream                                   *
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

#ifndef __OPLPCTOOLS_GAMEARTMANAGER__
#define __OPLPCTOOLS_GAMEARTMANAGER__

#include <QDir>
#include <QPixmap>
#include <QMap>
#include <QSet>
#include <QSize>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <OplPcTools/Maybe.h>

namespace OplPcTools {

enum class GameArtType
{
    Icon         = 0x1,
    Front        = 0x2,
    Back         = 0x4,
    Spine        = 0x8,
    Screenshot1  = 0x10,
    Screenshot2  = 0x20,
    Background   = 0x40,
    Logo         = 0x80
};

class GameArtManager final : public QObject
{
    Q_OBJECT

    using GameCache = QMap<GameArtType, Maybe<QPixmap>>;
    using CacheMap = QMap<QString, Maybe<GameCache>>;
    struct GameArtProperties;

public:
    explicit GameArtManager(const QDir & _base_directory, QObject * _parent = nullptr);
    ~GameArtManager();
    void addCacheType(GameArtType _type);
    void removeCacheType(GameArtType _type, bool _clear_cache);
    QPixmap load(const QString & _game_id, GameArtType _type);
    void deleteArt(const QString & _game_id, GameArtType _type);
    void clearArts(const QString & _game_id);
    QPixmap setArt(const QString & _game_id, GameArtType _type, const QString & _filepath);
    void downloadArt(const QString & _game_id, GameArtType _type);
    void downloadAllArt(const QString & _game_id, bool _skip_existing = false);
    bool isBulkDownloadActive(const QString & _game_id) const;
    bool hasArt(const QString & _game_id, GameArtType _type) const;

signals:
    void artChanged(const QString & _game_id, GameArtType _type);
    void downloadStarted(const QString & _game_id, GameArtType _type);
    void downloadProgress(const QString & _game_id, GameArtType _type, qint64 _received, qint64 _total);
    void downloadCompleted(const QString & _game_id, GameArtType _type, bool _success);
    void allDownloadsCompleted(const QString & _game_id, int _successful, int _total);

private:
    void initArtProperties();
    void clearCache(GameArtType _type);
    Maybe<QPixmap> findInCache(const QString & _game_id, GameArtType _type) const;
    void cacheArt(const QString & _game_id, GameArtType _type, const QPixmap & _pixmap);

private:
    QString m_directory_path;
    CacheMap m_cache;
    QFlags<GameArtType> m_cached_types;
    QMap<GameArtType, GameArtProperties *> m_art_props;
    QNetworkAccessManager * mp_network_manager;
    QMap<QString, QSet<GameArtType>> m_bulk_downloads;
    QMap<QString, int> m_bulk_download_successful;
    QMap<QString, int> m_bulk_download_total;
};

// Hash function for GameArtType to enable QSet usage
inline uint qHash(GameArtType key, uint seed = 0) noexcept
{
    return ::qHash(static_cast<int>(key), seed);
}

} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMEARTMANAGER__
