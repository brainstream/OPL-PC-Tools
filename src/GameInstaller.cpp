/***********************************************************************************************
 *                                                                                             *
 * This file is part of the qpcopl project, the graphical PC tools for Open PS2 Loader.        *
 *                                                                                             *
 * qpcopl is free software: you can redistribute it and/or modify it under the terms of        *
 * the GNU General Public License as published by the Free Software Foundation,                *
 * either version 3 of the License, or (at your option) any later version.                     *
 *                                                                                             *
 * qpcopl is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;        *
 * without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  *
 * See the GNU General Public License for more details.                                        *
 *                                                                                             *
 * You should have received a copy of the GNU General Public License along with MailUnit.      *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#include <QFile>
#include <QDir>
#include <QThread>
#include "IOException.h"
#include "ValidationException.h"
#include "GameInstaller.h"
#include "Game.h"

GameInstaller::GameInstaller(GameInstallerSource & _source, UlConfig & _config, QObject * _parent /*= nullptr*/) :
    QObject(_parent),
    mp_sourrce(&_source),
    mr_config(_config),
    mp_installed_game_info(nullptr)
{
}

GameInstaller::~GameInstaller()
{
    delete mp_installed_game_info;
}

bool GameInstaller::install()
{
    delete mp_installed_game_info;
    mp_installed_game_info = new UlConfigRecord();
    Game::validateGameName(m_game_name);
    QString iso_id = mp_sourrce->gameId();
    mp_installed_game_info->image = QString("ul.") + iso_id;
    mp_installed_game_info->name = m_game_name.isEmpty() ? tr("Untitled Game") : m_game_name;
    const quint64 iso_size = mp_sourrce->size();
    mp_installed_game_info->type = mp_sourrce->type();
    if(mp_installed_game_info->type == MediaType::unknown)
        mp_installed_game_info->type = iso_size > 681984000 ? MediaType::dvd : MediaType::cd;
    const size_t part_size = 1073741824;
    const size_t read_part_size = 32768;
    size_t processed_bytes = 0;
    QDir dest_dir(mr_config.directory());
    while(processed_bytes < iso_size)
    {
        QString part_filename = Game::makeGamePartName(iso_id, mp_installed_game_info->name, mp_installed_game_info->parts++);
        QFile part(dest_dir.absoluteFilePath(part_filename));
        if(part.exists())
        {
            rollback();
            throw IOException(tr("File already exists: \"%1\"").arg(part.fileName()));
        }
        if(!part.open(QIODevice::WriteOnly))
        {
            rollback();
            throw IOException(tr("Unable to open file to write: \"%1\"").arg(part.fileName()));
        }
        m_written_parts.append(part.fileName());
        for(quint64 read_bytes = 0; read_bytes < part_size;)
        {
            QByteArray bytes = mp_sourrce->read(read_part_size);
            if(bytes.isEmpty())
                break;
            part.write(bytes);
            read_bytes += bytes.size();
            processed_bytes += bytes.size();
            emit progress(iso_size, processed_bytes);
            if(static_cast<size_t>(bytes.size()) < read_part_size)
                break;
            if(QThread::currentThread()->isInterruptionRequested())
            {
                rollback();
                return false;
            }
        }
    }
    try
    {
        registerGame();
    }
    catch(...)
    {
        rollback();
        throw;
    }
    m_written_parts.clear();
    return true;
}


void GameInstaller::rollback()
{
    emit rollbackStarted();
    for(const QString & path : m_written_parts)
        QFile::remove(path);
    m_written_parts.clear();
    delete mp_installed_game_info;
    mp_installed_game_info = nullptr;
    emit rollbackFinished();
}

void GameInstaller::registerGame()
{
    emit registrationStarted();
    mr_config.addRecord(*mp_installed_game_info);
    emit registrationFinished();
}
