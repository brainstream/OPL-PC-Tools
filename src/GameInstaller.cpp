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
    const ssize_t part_size = 1073741824;
    const ssize_t read_part_size = 4194304;
    size_t processed_bytes = 0;
    QDir dest_dir(mr_config.directory());
    QByteArray bytes(read_part_size, Qt::Initialization::Uninitialized);
    mp_sourrce->seek(0);
    for(bool unexpected_finish = false; !unexpected_finish && processed_bytes < iso_size;)
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
        for(size_t total_read_bytes = 0; total_read_bytes < part_size;)
        {
            ssize_t read_bytes = mp_sourrce->read(bytes);
            if(read_bytes < 0)
            {
                part.close();
                rollback();
                throw IOException(tr("An error occurred during reading the source medium"));
            }
            if(part.write(bytes.constData(), read_bytes) != read_bytes)
            {
                part.close();
                rollback();
                throw IOException(tr("Unable to write a data into the file: \"%1\"").arg(part.fileName()));
            }
            total_read_bytes += read_bytes;
            processed_bytes += read_bytes;
            if(read_bytes < read_part_size)
            {
                // Yes. It is a real scenario. The "Final Fantasy XII" declares the ISO FS size longer than it is.
                unexpected_finish = true;
                emit progress(processed_bytes, processed_bytes);
                break;
            }
            emit progress(iso_size, processed_bytes);
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
