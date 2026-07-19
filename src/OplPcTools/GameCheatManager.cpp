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

#include <OplPcTools/GameCheatManager.h>
#include <OplPcTools/StandardPaths.h>
#include <OplPcTools/File.h>
#include <OplPcTools/ApplicationInfo.h>
#include <QNetworkAccessManager>
#include <QDir>

using namespace OplPcTools;

GameCheatManager::GameCheatManager(const QDir & _library_path) :
    m_library_path(_library_path)
{
}

std::optional<QString> GameCheatManager::load(const QString & _game_id) const
{
    QFile file(makeFilename(_game_id));
    if(file.exists())
    {
        openFile(file, QFile::ReadOnly);
        return file.readAll();
    }
    return std::nullopt;
}

QString GameCheatManager::makeFilename(const QString & _game_id) const
{
    return m_library_path.absoluteFilePath(QString(StandardDirectories::cht) + QDir::separator() + _game_id + ".cht");
}

void GameCheatManager::save(const QString & _game_id, const QString & _cheat) const
{
    QFile file(makeFilename(_game_id));
    if(_cheat.isEmpty())
    {
        if(file.exists())
            removeFile(file);
        return;
    }
    QFileInfo(file).absoluteDir().mkpath(".");
    openFile(file, QFile::WriteOnly | QFile::Truncate);
    file.write(_cheat.toUtf8());
}

void GameCheatManager::remove(const QString & _game_id) const
{
    QFile file(makeFilename(_game_id));
    if(file.exists())
        removeFile(file);
}

GameCheatDownloader::GameCheatDownloader(const QString & _game_id, QObject * _parent) :
    QObject(_parent),
    m_game_id(_game_id),
    mp_network_replay(nullptr),
    m_is_canceled(false)
{
}

void GameCheatDownloader::start()
{
    if(mp_network_replay)
    {
        emit error(tr("The cheat download has already started"));
        return;
    }
    QNetworkAccessManager * network = new QNetworkAccessManager(this);
    const QString url =
        QString("https://raw.githubusercontent.com/PS2-Widescreen/OPL-Widescreen-Cheats/refs/heads/main/CHT/%1")
        .arg(m_game_id + ".cht");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, APPLICATION_DISPLAY_NAME);
    request.setTransferTimeout(std::chrono::seconds(15));
    mp_network_replay = network->get(request);
    connect(mp_network_replay, &QNetworkReply::finished, this, [this, network] {
        mp_network_replay->deleteLater();
        network->deleteLater();
        if(!m_is_canceled)
        {
            switch(mp_network_replay->error())
            {
            case QNetworkReply::NoError:
                emit downloaded(mp_network_replay->readAll());
                break;
            case QNetworkReply::ContentNotFoundError:
                emit error(tr("Cheat for this game not found"));
                break;
            default:
                emit error(tr("Unable to download the cheat, a network error occurred"));
                break;
            }
        }
        emit finished();
        mp_network_replay = nullptr;
    });
}

void GameCheatDownloader::cancel()
{
    m_is_canceled = true;
    if(mp_network_replay)
        mp_network_replay->abort();
}
