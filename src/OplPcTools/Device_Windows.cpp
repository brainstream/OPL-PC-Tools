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

#ifdef _WIN32

#include <windows.h>
#include <OplPcTools/Device.h>

using namespace OplPcTools;

namespace {

bool isOpticalDrive(char letter)
{
    char root_dir[] = "?:\\";
    root_dir[0] = letter;
    return GetDriveTypeA(root_dir) == DRIVE_CDROM;
}

}

QList<DeviceName> OplPcTools::loadDriveList()
{
    DWORD drives = GetLogicalDrives();
    QList<DeviceName> result;
    for(int i = 0; i < 26; ++i)
    {
        if(drives & (1 << i))
        {
            char letter = 'A' + i;
            if(isOpticalDrive(letter))
            {
                DeviceName name;
                name.filename = QString("\\\\.\\%1:").arg(letter);
                name.name = QString("%1:").arg(letter);
                result.append(name);
            }
        }
    }
    return result;
}

#endif // _WIN32
