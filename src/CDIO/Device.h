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

#ifndef __QPCOPL_CDIO_DEVICE__
#define __QPCOPL_CDIO_DEVICE__

#include <QStringList>

QStringList loadDriveList();

class Device
{   
    Q_DISABLE_COPY(Device)

private:
    class Iso9660Data;

public:
    explicit Device(const QString & _filepath);
    virtual ~Device();
    virtual bool init();
    bool isInitialized() const;
    const QString & gameId() const;

private:
    const QString m_filepath;
    Iso9660Data * mp_iso9660;
};

#endif // __QPCOPL_CDIO_DEVICE__
