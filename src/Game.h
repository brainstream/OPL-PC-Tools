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
#include "ValidationException.h"
#include "MediaType.h"

#define MAX_GAME_ID_LENGTH   15
#define MAX_GAME_NAME_LENGTH 32

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
    quint8 part_count;
    QPixmap cover;
    QString cover_filepath;
    QPixmap icon;
    QString icon_filepath;
};

inline void validateGameName(const QString & _name)
{
    if(_name.toUtf8().size() > MAX_GAME_NAME_LENGTH)
        throw ValidationException(QObject::tr("Maximum name length is %1 bytes").arg(MAX_GAME_NAME_LENGTH));
}

inline void validateGameId(const QString & _id)
{
    if(_id.toLatin1().size() > MAX_GAME_ID_LENGTH)
        throw ValidationException(QObject::tr("Maximum image name length is %1 bytes").arg(MAX_GAME_ID_LENGTH));
}

QString makeGamePartName(const QString & _id, const QString & _name, quint8 _part);

#endif // __OPLPCTOOLS_GAME__
