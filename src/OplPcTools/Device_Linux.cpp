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

#ifdef __linux__

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <OplPcTools/Device.h>

using namespace OplPcTools;

namespace {

bool isBlockDevice(const char * _file)
{
    struct stat file_stat;
    lstat(_file, &file_stat);
    return S_ISBLK(file_stat.st_mode);
}

bool isOpticalDrive(const char * _file)
{
    if(!isBlockDevice(_file)) return false;
    int fd = open(_file, O_RDONLY | O_NONBLOCK);
    if(fd < 0) return false;
    bool result = ioctl(fd, CDROM_GET_CAPABILITY) >= 0;
    close(fd);
    return result;
}

} // namespace

QList<DeviceName> OplPcTools::loadDriveList()
{
    QList<DeviceName> result;
    std::string dev_dir_path("/dev/");
    DIR * dev_dir = opendir(dev_dir_path.c_str());
    if(!dev_dir) return result;
    for(struct dirent * entry = readdir(dev_dir); entry != nullptr; entry = readdir(dev_dir))
    {
        std::string filename = dev_dir_path + entry->d_name;
        if(isOpticalDrive(filename.c_str()))
        {
            DeviceName device_name;
            device_name.name = entry->d_name;
            device_name.filename = QString::fromStdString(filename);
            result.append(device_name);
        }
    }
    closedir(dev_dir);
    return result;
}

#endif // __linux__
