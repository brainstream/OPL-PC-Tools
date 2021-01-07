/***********************************************************************************************
 * Copyright © 2017-2021 Sergey Smolyannikov aka brainstream                                   *
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

#ifndef __OPLPCTOOLS_VMCFS__
#define __OPLPCTOOLS_VMCFS__

#include <QString>
#include <QSharedPointer>
#include <OplPcTools/Exception.h>

namespace OplPcTools {

DECLARE_EXCEPTION(VmcFSException)

struct VmcInfo final
{
    QString magic;
    QString version;
    int16_t pagesize;
    uint16_t pages_per_cluster;
    uint16_t pages_per_block;
    uint32_t clusters_per_card;
    uint32_t alloc_offset;
    uint32_t alloc_end;
    uint32_t rootdir_cluster;
    uint32_t backup_block1;
    uint32_t backup_block2;
    uint32_t ifc_ptr_list[32];
    int32_t bad_block_list[32];
    uint8_t cardtype;
    uint8_t cardflags;
    uint32_t cluster_size;
    uint32_t fat_entries_per_cluster;
    uint32_t clusters_per_block;
    int32_t cardform;
    uint32_t max_allocatable_clusters;
};

struct VmcEntryInfo
{
    QString name;
    bool is_directory;
    uint32_t size;
};

class VmcFile final
{
    Q_DISABLE_COPY(VmcFile)

public:
    struct Private;

public:
    explicit VmcFile(Private * _private);
    ~VmcFile();
    const QString & name() const;
    uint32_t size() const;
    bool seek(uint32_t _pos);
    int64_t read(char * _buffer, int64_t _max_size);

private:
    Private * mp_private;
};

class VmcFS final
{
    Q_DISABLE_COPY(VmcFS)

protected:
    VmcFS();

public:
    class Private;

public:
    ~VmcFS();
    const VmcInfo * info() const;
    QList<VmcEntryInfo> enumerateEntries(const QString & _path);
    QSharedPointer<VmcFile> openFile(const QString & _path);

    static QSharedPointer<VmcFS> load(const QString & _filepath);
    static void create(const QString & _filepath, uint8_t _size_mib);
    static QString concatPaths(const QString & _base, const QString &  _part);

public:
    static const char path_separator;

private:
    Private * mp_private;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMCFS__
