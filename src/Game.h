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

#include <QObject>
#include "UlConfig.h"
#include "ValidationException.h"

class Game
{
public:
    const static int max_game_name_length = 32;
    const static int max_image_name_length = 15;

public:
    Game(UlConfig & _config, const QString & _image);
    Game(const Game &) = default;
    Game & operator = (const Game &) = default;
    void rename(const QString & _new_name);
    void remove();
    static QString makeGamePartName(const QString & _id, const QString & _name, quint8 _part);
    inline static void validateGameName(const QString & _name);
    inline static void validateGameImageName(const QString & _name);

private:
    static quint32 crc32(const QString & _string);
    QString imageId() const;
    const UlConfigRecord & configRecord() const;

private:
    UlConfig & mr_config;
    QString m_image;
};

void Game::validateGameName(const QString & _name)
{
    if(_name.toUtf8().size() > max_game_name_length)
        throw ValidationException(QObject::tr("Maximum name length is %1 bytes").arg(max_game_name_length));
}

void Game::validateGameImageName(const QString & _name)
{
    if(_name.toLatin1().size() > max_image_name_length)
        throw ValidationException(QObject::tr("Maximum image name length is %1 bytes").arg(max_image_name_length));
}


#endif // __QPCOPL_GAME__
