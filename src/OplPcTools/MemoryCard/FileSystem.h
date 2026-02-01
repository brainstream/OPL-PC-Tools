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

#include <OplPcTools/MemoryCard/FATable.h>
#include <OplPcTools/MemoryCard/FSEntry.h>
#include <OplPcTools/MemoryCard/Superblock.h>
#include <OplPcTools/MemoryCard/Path.h>
#include <QFile>
#include <cstring>

namespace OplPcTools {
namespace MemoryCard {

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
    uint32_t fs_entries_per_cluster;
    uint32_t clusters_per_block;
    int32_t cardform;
    uint32_t max_allocatable_clusters;
};

struct EntryAddress
{
    uint32_t cluster;
    uint32_t entry;
};

struct EntryInfo
{
    static EntryInfo fromFSEntry(const FSEntry & _fs_entry)
    {
        return EntryInfo
        {
            .name = QByteArray(_fs_entry.name, std::min(sizeof(FSEntry::name), std::strlen(_fs_entry.name))),
            .is_directory = (_fs_entry.mode & EM_DIRECTORY) != 0,
            .cluster = _fs_entry.cluster,
            .length = _fs_entry.length
        };
    }

    QByteArray name;
    bool is_directory;
    uint32_t cluster;
    uint32_t length;
};

struct EntryPath
{
    EntryInfo entry;
    EntryAddress address;
};

class File final
{
    Q_DISABLE_COPY_MOVE(File)

public:
    struct Private;

public:
    explicit File(Private * _private);
    ~File();
    const QByteArray & name() const;
    uint32_t size() const;
    bool seek(uint32_t _pos);
    int64_t read(char * _buffer, int64_t _max_size);

private:
    Private * mp_private;
};

class FileSystem final
{
    Q_DISABLE_COPY_MOVE(FileSystem)

private:
    struct EntrySearchResult
    {
        uint32_t cluster_index;
        QVector<FSEntry> cluster_entries;
        size_t entry_index;
    };

public:
    explicit FileSystem(const QString & _filepath);
    ~FileSystem();
    void load();
    const FSInfo * info() const;
    QList<EntryInfo> enumerateEntries(const Path & _path);
    void exportEntry(const Path & _vmc_path, const QString & _dest_path);
    QSharedPointer<File> openFile(const Path & _path);
    int64_t readFile(File::Private & _file, char * _buffer, uint32_t _max_size);
    void writeFile(const Path & _path, const QByteArray & _data);
    void createDirectory(const Path & _path);
    void remove(const Path & _path);
    void rename(const Path & _path, const QByteArray & _new_name);
    uint32_t totalUsedBytes() const;
    uint32_t totalFreeBytes() const;

private:
    void deinit();
    void readSuperblock();
    void readCluster(uint32_t _cluster, bool _is_absolute, char * _buffer, uint32_t _size = 0);
    void read(quint64 _offset, char * _buffer, uint32_t _size);
    void writeCluster(uint32_t _cluster, bool _is_absolute, const char * _buffer);
    void validateSuperblock(const Superblock & _sb) const;
    void readFAT();
    std::optional<EntryPath> resolvePath(const Path & _path);
    EntryPath getRootEntry();
    void forEachEntry(const EntryInfo & _dir, std::function<bool(const EntryPath &)> _callback);
    QList<uint32_t> getEntryClusters(const EntryInfo & _entry) const;
    void writeFile(
        const EntryPath & _directory_path,
        const QByteArray & _filename,
        const QByteArray & _data,
        bool _is_directory);
    bool allocEntry(const EntryPath & _parent, const EntryInfo & _entry);
    void changeEntryLength(const EntryAddress & _address, int8_t _amount);
    void writeFATEntry(uint32_t _cluster, FATEntry _entry);
    bool findFreeEntry(const QList<uint32_t> & _parent_clusters, EntrySearchResult & _result);
    std::optional<QList<uint32_t>> alloc(uint32_t _allocation_size);
    void freeFAT(const QList<uint32_t> & _clusters);
    void eraseEntriesRecursive(const EntryPath & _path);

private:
    QFile m_file;
    FSInfo * mp_info;
    FATable m_fat;
};

} // namespace MemoryCard
} // namespace OplPcTools
