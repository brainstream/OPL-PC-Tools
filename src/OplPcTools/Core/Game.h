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

#ifndef __OPLPCTOOLS_GAME__
#define __OPLPCTOOLS_GAME__

#include <QString>
#include <QMetaType>
#include <QPixmap>
#include <OplPcTools/Core/ValidationException.h>
#include <OplPcTools/Core/MediaType.h>
#include <OplPcTools/Core/GameInstallationType.h>

const quint16 g_max_game_id_length = 15;
const quint16 g_max_game_name_length = 32;

struct Game
{
    Game() :
        media_type(MediaType::Unknown),
        part_count(0)
    {
    }

    QString id;
    QString title;
    MediaType media_type;
    GameInstallationType installation_type;
    quint8 part_count;
    QPixmap cover;
    QString cover_filepath;
    QPixmap icon;
    QString icon_filepath;
};

void validateGameName(const QString & _name, GameInstallationType _installation_type);

void validateGameId(const QString & _id);

QString makeGamePartName(const QString & _id, const QString & _name, quint8 _part);

QString makeGameIsoFilename(const QString & _title, const QString & _id);

#endif // __OPLPCTOOLS_GAME__
