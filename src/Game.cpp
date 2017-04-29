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

#include <algorithm>
#include <QFile>
#include <QList>
#include "Game.h"

Game::Game(UlConfig & _config, const QString & _image) :
    mr_config(_config),
    m_image(_image)
{
}

void Game::rename(const QString & _new_name)
{
    const UlConfigRecord & config_record = configRecord();
    QList<QString> files;
    QDir root_dir(mr_config.directory());
    for(quint8 part = 0; part < config_record.parts; ++part)
    {
        QString part_path = root_dir.absoluteFilePath(makeGamePartName(m_image, config_record.name, part));
        if(!QFile::exists(part_path))
            throw ValidationException(QObject::tr("File \"%1\" was not found").arg(part_path));
        files.append(part_path);
    }
    for(int part = 0; part < config_record.parts; ++part)
    {
        QString new_path = root_dir.absoluteFilePath(makeGamePartName(m_image, _new_name, part));
        QFile::rename(files[part], new_path);
    }
    mr_config.renameRecord(m_image, _new_name);
}

void Game::remove()
{
    UlConfigRecord config_record = configRecord();
    mr_config.deleteRecord(m_image);
    QDir root_dir(mr_config.directory());
    for(int part = 0; part < config_record.parts; ++part)
    {
        QString part_path = root_dir.absoluteFilePath(makeGamePartName(m_image, config_record.name, part));
        QFile::remove(part_path);
    }
}

const UlConfigRecord & Game::configRecord() const
{
    const QList<UlConfigRecord> & records = mr_config.records();
    auto it = std::find_if(records.cbegin(), records.cend(), [this](const UlConfigRecord & _record) {
        return _record.image == m_image;
    });
    if(it == records.cend())
        throw ValidationException(QObject::tr("Record \"%1\" was not found in the config file").arg(m_image));
    return *it;
}

QString Game::makeGamePartName(const QString & _id, const QString & _name, quint8 _part)
{
    QString crc = QString("%1").arg(crc32(_name.toUtf8().constData()), 8, 16, QChar('0')).toUpper();
    return QString("ul.%1.%2.%3").arg(crc).arg(_id).arg(_part, 2, 10, QChar('0'));
}

// This fucnction is taken form the original OPL project (iso2opl.c).
quint32 Game::crc32(const QString & _string)
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
