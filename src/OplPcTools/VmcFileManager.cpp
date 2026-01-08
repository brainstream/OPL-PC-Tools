/***********************************************************************************************
 * Copyright © 2017-2026 Sergey Smolyannikov aka brainstream                                   *
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
 * You should have received a copy of the GNU General Public License along with OPL PC Tools   *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#include <OplPcTools/VmcFileManager.h>
#include <OplPcTools/MCFS/VmcFormatter.h>

#define INVALID_CLUSTER_PTR static_cast<uint32_t>(-1)
#define NULL_CLUSTER_PTR 0

using namespace OplPcTools;
using namespace OplPcTools::MCFS;


VmcFileManager::VmcFileManager(FileSystemDriver * _private) :
    mp_driver(_private)
{
}

VmcFileManager::~VmcFileManager()
{
    delete mp_driver;
}

const VmcFsInfo * VmcFileManager::fileSystemInfo() const
{
    return mp_driver->info();
}

QSharedPointer<VmcFileManager> VmcFileManager::load(const QString & _filepath)
{
    FileSystemDriver * driver = new FileSystemDriver(_filepath);
    try
    {
        driver->load();
    }
    catch(...)
    {
        delete driver;
        throw;
    }
    return QSharedPointer<VmcFileManager>(new VmcFileManager(driver));
}

void VmcFileManager::createVmc(const QString & _filepath, uint32_t _size_mib)
{
    VmcFormatter::format(_filepath, _size_mib);
}

QList<VmcFsEntryInfo> VmcFileManager::enumerateEntries(const VmcPath & _path) const
{
    const QList<EntryInfo> entries = mp_driver->enumerateEntries(_path);
    QList<VmcFsEntryInfo> result;
    result.reserve(entries.count());
    foreach(const EntryInfo & entry, entries)
        result.append(VmcFsEntryInfo(entry));
    return result;
}

QSharedPointer<VmcFile> VmcFileManager::openFile(const VmcPath & _path)
{
    return mp_driver->openFile(_path);
}

void VmcFileManager::writeFile(const VmcPath & _path, const QByteArray & _data)
{
    mp_driver->writeFile(_path, _data);
}
