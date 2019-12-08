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

#ifndef __OPLPCTOOLS_GAMEINSTALLER__
#define __OPLPCTOOLS_GAMEINSTALLER__

#include <QObject>
#include <QStringList>
#include <OplPcTools/GameCollection.h>
#include <OplPcTools/Device.h>

namespace OplPcTools {

class GameInstaller : public QObject
{
    Q_OBJECT

public:
    GameInstaller(Device & _device, GameCollection & _collection, QObject * _parent = nullptr);
    virtual bool install() = 0;
    virtual const Game * installedGame() const = 0;

signals:
    void progress(quint64 _total_bytes, quint64 _done_bytes);
    void registrationStarted();
    void registrationFinished();
    void rollbackStarted();
    void rollbackFinished();

protected:
    MediaType deviceMediaType() const;  

protected:
    Device & mr_device;
    GameCollection & mr_collection;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMEINSTALLER__
