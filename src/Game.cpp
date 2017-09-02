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

#include <QVector>
#include "Game.h"

namespace {

const QVector<QChar> g_disallowed_filename_characters = QVector<QChar> { '<', '>', ':', '"', '/', '\\', '|', '?', '*' };

// This function originally was taken from the OPL project (iso2opl.c).
// https://github.com/ifcaro/Open-PS2-Loader
quint32 crc32(const QString & _string)
{
    std::string string = _string.toStdString();
    quint32 * crctab = new quint32[0x400];
    int crc = 0;
    int count = 0;
    for(int table = 0; table < 256; ++table)
    {
        crc = table << 24;
        for(count = 8; count > 0; --count)
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
        int byte = string[count++];
        crc = crctab[byte ^ ((crc >> 24) & 0xFF)] ^ ((crc << 8) & 0xFFFFFF00);
    } while (string[count - 1] != 0);
    delete [] crctab;
    return crc;
}

} // namespace

void validateGameId(const QString & _id)
{
    if(_id.toLatin1().size() > g_max_game_id_length)
        throw ValidationException(QObject::tr("Maximum image name length is %1 bytes").arg(g_max_game_id_length));
}


void validateGameName(const QString & _name, GameInstallationType _installation_type)
{
    switch(_installation_type)
    {
    case GameInstallationType::UlConfig:
        if(_name.toUtf8().size() > g_max_game_name_length)
            throw ValidationException(QObject::tr("Maximum name length is %1 bytes").arg(g_max_game_name_length));
        break;
    case GameInstallationType::Directory:
        for(const QChar & ch : g_disallowed_filename_characters)
        {
           if(_name.contains(ch))
           {
               throw ValidationException(QObject::tr("Name must not contain following symbols: ") +
                    QString::fromRawData(g_disallowed_filename_characters.data(), g_disallowed_filename_characters.size()));
           }
        }
        break;
    }

}

QString makeGamePartName(const QString & _id, const QString & _name, quint8 _part)
{
    QString crc = QString("%1").arg(crc32(_name.toUtf8().constData()), 8, 16, QChar('0')).toUpper();
    return QString("ul.%1.%2.%3").arg(crc).arg(_id).arg(_part, 2, 10, QChar('0'));
}

