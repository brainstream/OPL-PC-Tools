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

#ifndef __QPCOPL_GAME__
#define __QPCOPL_GAME__

#include <QString>
#include <QMetaType>
#include <QPixmap>
#include "MediaType.h"

#define MAX_GAME_ID_LENGTH   15
#define MAX_GAME_NAME_LENGTH 32

struct Game
{
    QString id;
    QString name;
    MediaType media_type;
    quint8 part_count;
    QPixmap cover;
    QString cover_filepath;
};

#endif // __QPCOPL_GAME__
