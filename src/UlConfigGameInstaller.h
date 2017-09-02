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

#ifndef __OPLPCTOOLS_ULCONFIGGAMEINSTALLER__
#define __OPLPCTOOLS_ULCONFIGGAMEINSTALLER__

#include "GameInstaller.h"

class UlConfigGameInstaller : public GameInstaller
{
    Q_OBJECT

public:
    UlConfigGameInstaller(Device & _device, GameCollection & _collection, QObject * _parent = nullptr);

protected:
    bool performInstallation() override;

private:
    void rollback();
    void registerGame();

private:
    QStringList m_written_parts;
};

#endif // __OPLPCTOOLS_ULCONFIGGAMEINSTALLER__
