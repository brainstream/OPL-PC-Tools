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

#pragma once

#include <OplPcTools/MCFS/FATable.h>
#include <OplPcTools/MCFS/FSEntry.h>
#include <OplPcTools/MCFS/Superblock.h>
#include <OplPcTools/VmcPath.h>
#include <QFile>

namespace OplPcTools {
namespace MCFS {

struct FSInfo final
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

struct EntryInfo
{
    QByteArray name;
    bool is_directory;
    uint32_t cluster;
    uint32_t length;
};

class MemoryCardFile final
{
    Q_DISABLE_COPY(MemoryCardFile)

public:
    struct Private;

public:
    explicit MemoryCardFile(Private * _private);
    ~MemoryCardFile();
    const QByteArray & name() const;
    uint32_t size() const;
    bool seek(uint32_t _pos);
    int64_t read(char * _buffer, int64_t _max_size);

private:
    Private * mp_private;
};

class FileSystemDriver final
{
private:
    struct FSEntrySearchResult
    {
        FSEntrySearchResult() :
            cluster_index(0),
            cluster_entries(std::unique_ptr<FSEntry[]>()),
            entry_index(0)
        {
        }

        uint32_t cluster_index;
        std::unique_ptr<FSEntry[]> cluster_entries;
        size_t entry_index;
    };

public:
    explicit FileSystemDriver(const QString & _filepath);
    ~FileSystemDriver();
    void load();
    void create(uint8_t _size_mib);
    const FSInfo * info() const;
    QList<EntryInfo> enumerateEntries(const VmcPath & _path);
    void exportEntry(const VmcPath & _vmc_path, const QString & _dest_path);
    QSharedPointer<MemoryCardFile> openFile(const VmcPath & _path);
    int64_t readFile(MemoryCardFile::Private & _file, char * _buffer, uint32_t _max_size);
    void writeFile(const VmcPath & _path, const QByteArray & _data);

private:
    void deinit();
    void readSuperblock();
    void readCluster(uint32_t _cluster, bool _is_absolute, char * _buffer, uint32_t _size = 0);
    void read(quint64 _offset, char * _buffer, uint32_t _size);
    void writeCluster(uint32_t _cluster, bool _is_absolute, const char * _buffer);
    void validateSuperblock(const Superblock & _sb) const;
    void readFAT();
    std::optional<EntryInfo> resolvePath(const VmcPath & _path);
    EntryInfo getRootEntry();
    inline EntryInfo map(const FSEntry & _fs_entry) const;
    void enumerateEntries(const EntryInfo & _dir, std::function<bool(const EntryInfo &)> _callback);
    QList<uint32_t> getEntryClusters(const EntryInfo & _entry) const;
    bool allocEntry(const EntryInfo & _parent, const EntryInfo & _entry);
    void writeFATEntry(uint32_t _cluster, FATEntry _entry);
    bool findFreeEntry(const QList<uint32_t> & _parent_clusters, FSEntrySearchResult & _result);
    std::optional<QList<uint32_t>> alloc(uint32_t _allocation_size);
    void free(const QList<uint32_t> & _clusters);

private:
    QFile m_file;
    FSInfo * mp_info;
    FATable m_fat;
};

} // namespace MCFS
} // namespace OplPcTools
