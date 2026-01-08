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

#ifndef __OPLPCTOOLS_VMCFS__
#define __OPLPCTOOLS_VMCFS__

#include <QString>
#include <QSharedPointer>
#include <OplPcTools/VmcPath.h>
#include <OplPcTools/MCFS/FileSystemDriver.h>

namespace OplPcTools {

using VmcFsInfo = MCFS::FSInfo;
using VmcFile = MCFS::MemoryCardFile;

struct VmcFsEntryInfo final
{
    VmcFsEntryInfo()
    {
    }

    VmcFsEntryInfo(const MCFS::EntryInfo & _entry) :
        name(_entry.name),
        is_directory(_entry.is_directory),
        size(_entry.length)
    {
    }

    QByteArray name;
    bool is_directory;
    uint32_t size;
};

class VmcFileManager final
{
    Q_DISABLE_COPY(VmcFileManager)

public:
    ~VmcFileManager();
    const VmcFsInfo * fileSystemInfo() const;
    QList<VmcFsEntryInfo> enumerateEntries(const VmcPath & _path) const;
    QSharedPointer<VmcFile> openFile(const VmcPath & _path);
    void writeFile(const VmcPath & _path, const QByteArray & _data);
    uint32_t totalUsedBytes() const { return mp_driver->totalUsedBytes(); }
    uint32_t totalFreeBytes() const { return mp_driver->totalFreeBytes(); }

    static QSharedPointer<VmcFileManager> load(const QString & _filepath);
    static void createVmc(const QString & _filepath, uint32_t _size_mib);

private:
    explicit VmcFileManager(MCFS::FileSystemDriver * _private);

private:
    MCFS::FileSystemDriver * mp_driver;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMCFS__
