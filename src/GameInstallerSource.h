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

#include <QString>
#include <QByteArray>

class GameInstallerSource
{
public:
    virtual ~GameInstallerSource() { }
    virtual QString gameId() = 0;
    virtual QByteArray read(quint64 _length) = 0;
    virtual quint64 size() = 0;
};

#endif // __QPCOPL_GAMEINSTALLERSOURCE__
