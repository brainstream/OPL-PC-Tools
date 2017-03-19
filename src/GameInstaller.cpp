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

GameInstaller::GameInstaller(GameInstallerSource & _source, const QString _dest_dir_path, QObject * _parent /*= nullptr*/) :
    QObject(_parent),
    mp_sourrce(&_source),
    m_dest_dir_path(_dest_dir_path),
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
    mp_installed_game_info = new Ul::ConfigRecord();
    if(m_game_name.toUtf8().size() > UL_MAX_GAME_NAME_LENGTH)
        throw ValidationException(tr("Game name is too logn"));
    QString iso_id = mp_sourrce->gameId();
    mp_installed_game_info->image = QString("ul.") + iso_id;
    mp_installed_game_info->name = m_game_name.isEmpty() ? tr("Untitled Game") : m_game_name;
    const quint64 iso_size = mp_sourrce->size();
    mp_installed_game_info->type = iso_size > 681984000 ? MediaType::dvd : MediaType::cd;
    const size_t part_size = 1073741824;
    const size_t read_part_size = 32768;
    QString crc = QString("%1").arg(crc32(mp_installed_game_info->name.toUtf8().constData()), 8, 16, QChar('0')).toUpper();
    size_t processed_bytes = 0;
    QDir dest_dir(m_dest_dir_path);
    for(mp_installed_game_info->parts = 0; processed_bytes < iso_size; ++mp_installed_game_info->parts)
    {
        QString part_filename = QString("ul.%1.%2.%3").arg(crc).arg(iso_id).arg(mp_installed_game_info->parts, 2, 10, QChar('0'));
        QFile part(dest_dir.absoluteFilePath(part_filename));
        if(!part.open(QIODevice::Truncate | QIODevice::WriteOnly))
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
    m_written_parts.clear();
    return true;
}

// This fucnction is taken form the original OPL project (iso2opl.c).
quint32 GameInstaller::crc32(const QString & _string)
{
    const char * string = _string.toUtf8().constData();
    quint32 * crctab = new quint32[0x400];
    int crc, table, count, byte;
    for(table = 0; table < 256; ++table)
    {
        crc = table << 24;
        for (count = 8; count > 0; --count)
        {
            if (crc < 0)
                crc = crc << 1;
            else
                crc = (crc << 1) ^ 0x04C11DB7;
        }
        crctab[255 - table] = crc;
    }
    do
    {
        byte = string[count++];
        crc = crctab[byte ^ ((crc >> 24) & 0xFF)] ^ ((crc << 8) & 0xFFFFFF00);
    } while (string[count - 1] != 0);
    delete [] crctab;
    return crc;
}

void GameInstaller::rollback()
{
    emit rollbackStarted();
    for(const QString & path : m_written_parts)
        QFile::remove(path);
    m_written_parts.clear();
    emit rollbackFinished();
}
