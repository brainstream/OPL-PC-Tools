/***********************************************************************************************
 * Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                   *
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
#include <QObject>
#include <OplPcTools/MediaType.h>
#include <OplPcTools/GameInstallationType.h>

namespace OplPcTools {

class Game final
{
public:
    inline Game(const QString & _id, GameInstallationType _type);
    Game(const Game &) = default;
    Game & operator = (const Game &) = default;
    inline const QString & id() const;
    inline void setTitle(const QString & _title);
    inline const QString & title() const;
    inline void setMediaType(MediaType _media_type);
    inline MediaType mediaType() const;
    inline void setPartCount(quint8 _count);
    inline quint8 partCount() const;
    inline GameInstallationType installationType() const;

private:
    QString m_id;
    QString m_title;
    MediaType m_media_type;
    quint8 m_part_count;
    GameInstallationType m_installation_type;
};

Game::Game(const QString & _id, GameInstallationType _type) :
    m_id(_id),
    m_media_type(MediaType::Unknown),
    m_part_count(1),
    m_installation_type(_type)
{
    m_title = QObject::tr("<Untitled>");
}

const QString & Game::id() const
{
    return m_id;
}

void Game::setTitle(const QString & _title)
{
    if(m_title != _title)
        m_title = _title;
}

const QString & Game::title() const
{
    return m_title;
}

void Game::setMediaType(MediaType _media_type)
{
    m_media_type = _media_type;
}

MediaType Game::mediaType() const
{
    return m_media_type;
}

void Game::setPartCount(quint8 _count)
{
    m_part_count = _count;
}

quint8 Game::partCount() const
{
    return m_part_count;
}

GameInstallationType Game::installationType() const
{
    return m_installation_type;
}

} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAME__
