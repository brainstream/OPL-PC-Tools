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

#include <functional>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QImageReader>
#include <QUrl>
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
    m_cached_types(),
    mp_network_manager(nullptr)
{
    m_directory_path = _base_directory.absoluteFilePath("ART");
    initArtProperties();
    mp_network_manager = new QNetworkAccessManager(this);
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
        emit artChanged(_game_id, _type);
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
    emit artChanged(_game_id, _type);
    return pixmap;
}

void GameArtManager::downloadArt(const QString & _game_id, GameArtType _type)
{
    const GameArtProperties * props = m_art_props[_type];
    QString url = QString("https://ia903209.us.archive.org/view_archive.php?archive=/34/items/ps2-opl-cover-art-set/PS2_OPL_ART_kira.7z&file=ART/%1%2.png")
        .arg(_game_id)
        .arg(props->suffix);
    
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::UserAgentHeader, "OPL PC Tools");
    
    QNetworkReply * reply = mp_network_manager->get(request);
    
    emit downloadStarted(_game_id, _type);
    
    connect(reply, &QNetworkReply::downloadProgress, [this, _game_id, _type](qint64 received, qint64 total) {
        emit downloadProgress(_game_id, _type, received, total);
    });
    
    connect(reply, &QNetworkReply::finished, [this, reply, _game_id, _type]() {
        reply->deleteLater();
        
        bool success = false;
        if(reply->error() == QNetworkReply::NoError)
        {
            QByteArray data = reply->readAll();
            if(!data.isEmpty())
            {
                // Create temporary file to validate the image
                QTemporaryFile temp_file;
                if(temp_file.open())
                {
                    temp_file.write(data);
                    temp_file.flush();
                    
                    // Validate that it's a PNG image
                    QImageReader reader(temp_file.fileName());
                    if(reader.canRead() && reader.format().toLower() == "png")
                    {
                        // Load the image as a pixmap
                        QPixmap pixmap(temp_file.fileName());
                        if(!pixmap.isNull())
                        {
                            // Scale to appropriate size
                            const GameArtProperties * props = m_art_props[_type];
                            if(pixmap.size() != props->size)
                                pixmap = pixmap.scaled(props->size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                            
                            // Save to ART directory
                            QDir dir(m_directory_path);
                            if(!dir.exists())
                                dir.mkpath(".");
                            
                            QString filename = dir.absoluteFilePath(_game_id + props->suffix + ".png");
                            if(pixmap.save(filename))
                            {
                                if(m_cached_types & _type)
                                    cacheArt(_game_id, _type, pixmap);
                                emit artChanged(_game_id, _type);
                                success = true;
                            }
                        }
                    }
                }
            }
        }
        
        emit downloadCompleted(_game_id, _type, success);
        
        // Check if this was part of a bulk download
        if(m_bulk_downloads.contains(_game_id))
        {
            QSet<GameArtType> & remaining = m_bulk_downloads[_game_id];
            remaining.remove(_type);
            
            if(success)
                m_bulk_download_successful[_game_id]++;
            
            if(remaining.isEmpty())
            {
                // All downloads completed
                int successful = m_bulk_download_successful[_game_id];
                int total = m_bulk_download_total[_game_id];
                emit allDownloadsCompleted(_game_id, successful, total);
                
                // Clean up
                m_bulk_downloads.remove(_game_id);
                m_bulk_download_successful.remove(_game_id);
                m_bulk_download_total.remove(_game_id);
            }
        }
    });
}

void GameArtManager::downloadAllArt(const QString & _game_id, bool _skip_existing)
{
    QSet<GameArtType> allArtTypes = {
        GameArtType::Icon,
        GameArtType::Front,
        GameArtType::Back,
        GameArtType::Spine,
        GameArtType::Screenshot1,
        GameArtType::Screenshot2,
        GameArtType::Background,
        GameArtType::Logo
    };
    
    QSet<GameArtType> artTypesToDownload;
    
    if(_skip_existing)
    {
        // Only download missing artwork
        for(GameArtType artType : allArtTypes)
        {
            if(!hasArt(_game_id, artType))
            {
                artTypesToDownload.insert(artType);
            }
        }
    }
    else
    {
        // Download all artwork (replace existing)
        artTypesToDownload = allArtTypes;
    }
    
    if(artTypesToDownload.isEmpty())
    {
        // No artwork to download, emit completion immediately
        emit allDownloadsCompleted(_game_id, 0, 0);
        return;
    }
    
    // Initialize bulk download tracking
    m_bulk_downloads[_game_id] = artTypesToDownload;
    m_bulk_download_successful[_game_id] = 0;
    m_bulk_download_total[_game_id] = artTypesToDownload.size();
    
    // Start downloading selected artwork types
    for(GameArtType artType : artTypesToDownload)
    {
        downloadArt(_game_id, artType);
    }
}

bool GameArtManager::isBulkDownloadActive(const QString & _game_id) const
{
    return m_bulk_downloads.contains(_game_id);
}

bool GameArtManager::hasArt(const QString & _game_id, GameArtType _type) const
{
    // Check if artwork file exists on disk
    QDir dir(m_directory_path);
    if(!dir.exists())
        return false;
    
    static const QStringList exts { ".png", ".jpeg", ".jpg", ".bmp" };
    const QString sfx = m_art_props[_type]->suffix;
    
    for(const QString & ext : exts)
    {
        QFile file(dir.absoluteFilePath(_game_id + sfx + ext));
        if(file.exists())
            return true;
    }
    
    return false;
}
