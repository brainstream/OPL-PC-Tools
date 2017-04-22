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

#ifndef __QPCOPL_GAMEINSTALLERSOURCE__
#define __QPCOPL_GAMEINSTALLERSOURCE__

#include <cdio/iso9660.h>
#include <QString>
#include <QByteArray>
#include <QObject>
#include "MediaType.h"

class GameInstallerSource
{
public:
    inline GameInstallerSource();
    virtual ~GameInstallerSource() { }
    virtual QString gameId() const = 0;
    inline const QString & gameName() const;
    inline void setGameName(const QString & _name);
    virtual void seek(quint64 _offset) = 0;
    virtual ssize_t read(QByteArray & _buffer) = 0;
    virtual quint64 size() const = 0;
    virtual MediaType type() const = 0;

protected:
    QString readGameId(CdioList_t * _root_dir) const;
    virtual QByteArray read(lsn_t _lsn, quint32 _blocks) const = 0;

private:
    QString m_game_name;
};

GameInstallerSource::GameInstallerSource() :
    m_game_name(QObject::tr("Untitled Game"))
{
}

const QString & GameInstallerSource::gameName() const
{
    return m_game_name;
}

void GameInstallerSource::setGameName(const QString & _name)
{
    m_game_name = _name;
}

#endif // __QPCOPL_GAMEINSTALLERSOURCE__
