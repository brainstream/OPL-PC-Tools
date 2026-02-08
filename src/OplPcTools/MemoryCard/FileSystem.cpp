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

#include <OplPcTools/MemoryCard/FileSystem.h>
#include <OplPcTools/File.h>
#include <QRegularExpression>

using namespace OplPcTools;
using namespace OplPcTools::MemoryCard;

#define INVALID_CLUSTER_PTR static_cast<uint32_t>(-1)
#define NULL_CLUSTER_PTR 0

namespace {

[[noreturn]] inline void throwNotFormatted()
{
    throw MemoryCardFileSystemException(QObject::tr("The VMC is corrupted or not formatted correctly"));
}

[[noreturn]] inline void throwPathNotFound(const Path & _path)
{
    throw MemoryCardFileException(QObject::tr("Path not found"), _path.path());
}

[[noreturn]] inline void throwNoSpace()
{
    throw MemoryCardFileSystemException(QObject::tr("Not enough free space"));
}

} // namespace

struct File::Private
{
    FileSystem * fs;
    QByteArray name;
    uint32_t size;
    uint32_t position;
    QList<uint32_t> clusters;
};

File::File(Private * _private) :
    mp_private(_private)
{
}

File::~File()
{
    delete mp_private;
}

uint32_t File::size() const
{
    return mp_private->size;
}

const QByteArray & File::name() const
{
    return mp_private->name;
}

bool File::seek(uint32_t _pos)
{
    if(_pos == mp_private->position)
        return true;
    if(_pos >= mp_private->size)
        return false;
    mp_private->position = _pos;
    return true;
}

int64_t File::read(char * _buffer, int64_t _max_size)
{
    return mp_private->fs->readFile(*this->mp_private, _buffer, _max_size);
}

FileSystem::FileSystem(const QString & _filepath) :
    m_file(_filepath),
    mp_info(nullptr)
{
}

FileSystem::~FileSystem()
{
    deinit();
}

void FileSystem::deinit()
{
    if(m_file.isOpen())
        m_file.close();
    delete mp_info;
    mp_info = nullptr;
    m_fat.reset();
}

const FSInfo * FileSystem::info() const
{
    return mp_info;
}

void FileSystem::load()
{
    deinit();
    ::openFile(m_file, QIODevice::OpenMode(QIODevice::ReadWrite | QIODevice::ExistingOnly));
    readSuperblock();
    readFAT();
}

void FileSystem::readSuperblock()
{
    QScopedPointer<Superblock> sb(new Superblock);
    read(0, reinterpret_cast<char *>(sb.data()), sizeof(Superblock));
    validateSuperblock(*sb);
    mp_info = new FSInfo;
    mp_info->magic = QString::fromLatin1(sb->magic, sizeof(sb->magic));
    mp_info->version= QString::fromLatin1(sb->version, sizeof(sb->version));
    mp_info->pagesize = sb->pagesize;
    mp_info->pages_per_cluster = sb->pages_per_cluster;
    mp_info->pages_per_block = sb->pages_per_block;
    mp_info->clusters_per_card = sb->clusters_per_card;
    mp_info->alloc_offset = sb->alloc_offset;
    mp_info->alloc_end = sb->alloc_end;
    mp_info->rootdir_cluster = sb->rootdir_cluster;
    mp_info->backup_block1 = sb->backup_block1;
    mp_info->backup_block2 = sb->backup_block2;
    std::memcpy(mp_info->ifc_ptr_list, sb->ifc_ptr_list, sizeof(sb->ifc_ptr_list));
    std::memcpy(mp_info->bad_block_list, sb->bad_block_list, sizeof(sb->bad_block_list));
    mp_info->cardtype = sb->cardtype;
    mp_info->cardflags = sb->cardflags;
    mp_info->cluster_size = sb->cluster_size;
    mp_info->fat_entries_per_cluster = sb->fat_entries_per_cluster;
    mp_info->fs_entries_per_cluster = sb->cluster_size / sizeof(FSEntry);
    mp_info->clusters_per_block = sb->clusters_per_block;
    mp_info->cardform = sb->cardform;
    mp_info->max_allocatable_clusters = sb->max_allocatable_clusters;
}

void FileSystem::readCluster(uint32_t _cluster, bool _is_absolute, char * _buffer, uint32_t _size /*= 0*/)
{
    qint64 offset = _cluster * mp_info->cluster_size;
    if(!_is_absolute)
        offset += mp_info->alloc_offset * mp_info->cluster_size;
    read(offset, _buffer, _size == 0 ? mp_info->cluster_size : _size);
}

void FileSystem::read(quint64 _offset, char * _buffer, uint32_t _size)
{
    if(!m_file.seek(_offset) || m_file.read(_buffer, _size) != _size)
        throwNotFormatted();
}

void FileSystem::validateSuperblock(const Superblock & _sb) const
{
    static QRegularExpression version_regex("^1\\.[012]\\.0\\.0$");
    if(
        std::strncmp(g_mcfs_magic, _sb.magic, strlen(g_mcfs_magic)) != 0 ||
        !version_regex.match(_sb.version).hasMatch() ||
        _sb.pagesize != 512 ||
        (_sb.cluster_size != 1024 && _sb.cluster_size != 512) ||
        _sb.pages_per_cluster != _sb.cluster_size / _sb.pagesize ||
        _sb.alloc_offset == INVALID_CLUSTER_PTR ||
        _sb.alloc_offset < 2 ||
        _sb.alloc_end == INVALID_CLUSTER_PTR ||
        _sb.alloc_end < _sb.alloc_offset ||
        _sb.ifc_ptr_list[0] >= _sb.alloc_offset)
    {
        throwNotFormatted();
    }
}

void FileSystem::readFAT()
{
    m_fat.reset();

    //
    // Loading IFC
    //
    size_t ifc_ptr_count = 0;
    for(; ifc_ptr_count < sizeof(Superblock::ifc_ptr_list) / sizeof(uint32_t); ++ifc_ptr_count)
    {
        uint32_t ptr = mp_info->ifc_ptr_list[ifc_ptr_count];
        if(ptr == INVALID_CLUSTER_PTR || ptr == NULL_CLUSTER_PTR)
            break;
    }

    //
    // Loading FAT pointers
    //
    const size_t fat_ptrs_count = ifc_ptr_count * mp_info->fat_entries_per_cluster;
    QScopedArrayPointer<uint32_t> fat_ptrs(new uint32_t[fat_ptrs_count]);
    for(size_t i = 0; i < ifc_ptr_count; ++i)
    {
        char * address = reinterpret_cast<char *>(fat_ptrs.data()) + (mp_info->cluster_size * i);
        readCluster(mp_info->ifc_ptr_list[i], true, address);
    }

    //
    // Loading FATs
    //
    QByteArray buffer(mp_info->cluster_size, Qt::Uninitialized);
    for(size_t i = 0; i < fat_ptrs_count; ++i)
    {
        const uint32_t fat_cluster = fat_ptrs[i];
        if(fat_cluster == INVALID_CLUSTER_PTR)
            break;
        readCluster(fat_cluster, true, buffer.data());
        FATEntry * fat = reinterpret_cast<FATEntry *>(buffer.data());
        m_fat.append(fat_cluster, QList<FATEntry>(fat, &fat[mp_info->fat_entries_per_cluster]));
    }
}

QList<EntryInfo> FileSystem::enumerateEntries(const Path & _path)
{
    std::optional<EntryPath> entry_path = resolvePath(_path);
    QList<EntryInfo> result;
    if(entry_path.has_value())
    {
        forEachEntry(entry_path->entry, [&](const EntryPath & next_entry_path) -> bool {
            if(next_entry_path.entry.name().compare(".") != 0  && next_entry_path.entry.name().compare("..") != 0)
                result << next_entry_path.entry;
            return true;
        });
    }
    else
    {
        throwPathNotFound(_path);
    }
    return result;
}

std::optional<EntryInfo> FileSystem::entry(const Path & _path)
{
    std::optional<EntryPath> ep = resolvePath(_path);
    if(ep) return ep->entry;
    return std::nullopt;
}

std::optional<EntryPath> FileSystem::resolvePath(const Path & _path)
{
    EntryPath entry_path = getRootEntry();
    for(const QByteArray & path_part : _path.parts())
    {
        bool matched = false;
        forEachEntry(entry_path.entry, [&](const EntryPath & __next_entry_path) -> bool {
            if(path_part.compare(__next_entry_path.entry.name(), Qt::CaseInsensitive) == 0)
            {
                matched = true;
                entry_path = __next_entry_path;
                return false;
            }
            return true;
        });
        if(!matched)
            return std::nullopt;
    }
    return entry_path;
}

EntryPath FileSystem::getRootEntry()
{
    QByteArray buffer(sizeof(FSEntry), Qt::Uninitialized);
    readCluster(mp_info->rootdir_cluster, false, buffer.data(), sizeof(FSEntry));
    return EntryPath
    {
        .entry = EntryInfo(*reinterpret_cast<FSEntry *>(buffer.data())),
        .address = EntryAddress
        {
            .cluster = mp_info->rootdir_cluster,
            .entry = 0
        }
    };
}

void FileSystem::forEachEntry(const EntryInfo & _dir, std::function<bool(const EntryPath &)> _callback)
{
    QList<uint32_t> clusters = getEntryClusters(_dir);
    QByteArray buffer(mp_info->cluster_size, Qt::Uninitialized);
    uint32_t read_count = 0;
    foreach(uint32_t cluster, clusters)
    {
        readCluster(cluster, false, buffer.data());
        const FSEntry * entries = reinterpret_cast<const FSEntry *>(buffer.data());
        for(size_t i = 0; i < mp_info->fs_entries_per_cluster; ++i)
        {
            if(++read_count > _dir.length())
                return;
            const FSEntry & entry = entries[i];
            if(!(entry.mode & EM_EXISTS))
                continue;
            EntryPath ep
            {
                .entry = entry,
                .address = EntryAddress { .cluster = cluster, .entry = static_cast<uint32_t>(i) }
            };
            if(!_callback(ep))
                return;
        }
    }
}

QList<uint32_t> FileSystem::getEntryClusters(const EntryInfo & _entry) const
{
    if(_entry.cluster() > mp_info->max_allocatable_clusters)
        throwNotFormatted();
    QList<uint32_t> result;
    for(uint32_t cluster = _entry.cluster();;) // TODO: check overflow, cycles, etc.
    {
        result.append(cluster);
        FATEntry fat_entry = m_fat[cluster];
        if(fat_entry.isEndOfFile())
            break;
        cluster = fat_entry.cluster;
    }
    return result;
}

QSharedPointer<File> FileSystem::openFile(const Path & _path)
{
    std::optional<EntryPath> entry_path = resolvePath(_path);
    if(!entry_path.has_value())
        throw MemoryCardFileException(QObject::tr("File not found"), _path);
    if(entry_path->entry.isDirectory())
        throw MemoryCardFileException(QObject::tr("The entry is not a file"), _path);
    File::Private * pr = new File::Private;
    pr->clusters = getEntryClusters(entry_path->entry);
    pr->fs = this;
    pr->position = 0;
    pr->size = entry_path->entry.length();
    pr->name = entry_path->entry.name();
    return QSharedPointer<File>(new File(pr));
}

int64_t FileSystem::readFile(File::Private & _file, char * _buffer, uint32_t _max_size)
{
    const uint32_t start_cluster_index = _file.position / mp_info->cluster_size;
    const uint32_t cluster_count = static_cast<uint32_t>(_file.clusters.size());
    if(_file.position >= _file.size || start_cluster_index >= cluster_count)
        return -1;
    uint32_t position_in_cluster = _file.position % mp_info->cluster_size;
    uint32_t position_in_file = _file.position;
    uint32_t position_in_buffer = 0;
    QByteArray cluster_buff(mp_info->cluster_size, Qt::Uninitialized);
    for(int cluster_index = start_cluster_index; cluster_index < _file.clusters.size(); ++cluster_index)
    {
        const uint32_t available_to_read = std::min(
            mp_info->cluster_size - position_in_cluster,
            _file.size - position_in_file);
        const uint32_t bytes_to_read = std::min(available_to_read, _max_size - position_in_buffer);
        const bool is_last_cluster = available_to_read + position_in_cluster < mp_info->cluster_size ||
                                     bytes_to_read + position_in_buffer == _max_size;
        readCluster(_file.clusters[cluster_index], false, cluster_buff.data());
        std::memcpy(&_buffer[position_in_buffer], &cluster_buff.data()[position_in_cluster], bytes_to_read);
        position_in_buffer += bytes_to_read;
        position_in_file += bytes_to_read;
        position_in_cluster = 0;
        if(is_last_cluster)
            break;
    }
    return position_in_buffer;
}

void FileSystem::writeFile(const Path & _path, const QByteArray & _data)
{
    Path dir_path = _path.up();
    std::optional<EntryPath> dir_entry_path = resolvePath(dir_path);
    if(!dir_entry_path.has_value())
        throwPathNotFound(dir_path);
    writeFile(*dir_entry_path, _path.filename(), _data, false);
}

void FileSystem::createDirectory(const Path & _path)
{
    Path parent_dir_path = _path.up();
    std::optional<EntryPath> parent_dir_entry_path = resolvePath(parent_dir_path);
    if(!parent_dir_entry_path.has_value())
        throwPathNotFound(parent_dir_path);

    const DateTime now = DateTime::now();
    QByteArray buffer(sizeof(FSEntry) * 2, '\0');
    FSEntry * entries = reinterpret_cast<FSEntry *>(buffer.data());
    for(int i = 0; i < 2; ++i)
    {
        entries[i].mode = EM_READ | EM_WRITE | EM_EXECUTE | EM_DIRECTORY | EM_EXISTS;
        entries[i].created = now;
        entries[i].modified = now;
        entries[i].name[0] = '.';
    }
    entries[0].cluster = parent_dir_entry_path->address.cluster;
    entries[0].dir_entry = parent_dir_entry_path->address.entry;
    entries[1].name[1] = '.';

    writeFile(*parent_dir_entry_path, _path.filename(), buffer, true);
}

void FileSystem::writeFile(
    const EntryPath & _directory_path,
    const QByteArray & _filename,
    const QByteArray & _data,
    bool _is_directory)
{
    if(!_directory_path.entry.isDirectory())
    {
        throw MemoryCardFileSystemException(
            QObject::tr("The entry %1 in cluster %2 is not a directory")
                .arg(_directory_path.address.entry)
                .arg(_directory_path.address.cluster));
    }

    std::optional<QList<uint32_t>> file_data_clusters = alloc(_data.size());
    if(!file_data_clusters)
        throwNoSpace();

    EntryInfo file_entry(
        _filename,
        _is_directory,
        file_data_clusters->first(),
        // New directory contains two entries: . and ..
        _is_directory ? 2 : static_cast<uint32_t>(_data.length()));
    if(!allocEntry(_directory_path, file_entry))
    {
        freeFAT(*file_data_clusters);
        throwNoSpace();
    }

    {
        QByteArray buffer(mp_info->cluster_size, Qt::Uninitialized);
        uint32_t chunk_idx = 0;
        foreach(uint32_t cluster, *file_data_clusters)
        {
            const qsizetype offset = chunk_idx * mp_info->cluster_size;
            std::memcpy(
                buffer.data(),
                &_data.data()[offset],
                std::min(mp_info->cluster_size, static_cast<uint32_t>(_data.size() - offset)));
            writeCluster(cluster, false, buffer.data());
            ++chunk_idx;
        }
    }
}

bool FileSystem::allocEntry(const EntryPath & _parent, const EntryInfo & _entry)
{
    QList<uint32_t> parent_dir_clusters = getEntryClusters(_parent.entry);

    EntrySearchResult parent_free_entry = {};
    if(!findFreeEntry(parent_dir_clusters, parent_free_entry))
    {
        // If all entries of the parent cluster are occupied,
        // we allocate a new cluster and link it to the parent via FAT.

        std::optional<QList<uint32_t>> new_cluster = alloc(mp_info->cluster_size);
        if(!new_cluster)
            return false;

        writeFATEntry(parent_dir_clusters.last(), FATEntry::pointer(new_cluster->first()));
        writeFATEntry(new_cluster->first(), FATEntry::endOfFile());

        parent_free_entry.entry_index = 0;
        parent_free_entry.cluster_index = new_cluster->first();
        parent_free_entry.cluster_entries.resize(mp_info->fs_entries_per_cluster);
        for(size_t i = 0; i < mp_info->fs_entries_per_cluster; ++i)
        {
            parent_free_entry.cluster_entries[i] = {};
            parent_free_entry.cluster_entries[i].cluster = 0xFFFFFFFF;
        }
    }

    { // Creating a new entry
        FSEntry & new_entry = parent_free_entry.cluster_entries[parent_free_entry.entry_index];
        new_entry.mode = EM_EXISTS | EM_READ | EM_WRITE;
        if(_entry.isDirectory())
            new_entry.mode |= EM_DIRECTORY | EM_EXECUTE;
        else
            new_entry.mode |= EM_FILE;
        new_entry.length = _entry.length();
        new_entry.cluster = _entry.cluster();
        new_entry.created = new_entry.modified = DateTime::now();
        std::strncpy(
            new_entry.name,
            _entry.name().data(),
            std::min(sizeof(FSEntry::name), static_cast<size_t>(_entry.name().length())));
    }

    // Updating an old or new cluster
    writeCluster(
        parent_free_entry.cluster_index,
        false,
        reinterpret_cast<const char *>(parent_free_entry.cluster_entries.data()));

    // Increment parent's length
    changeEntryLength(_parent.address, 1);

    return true;
}

void FileSystem::changeEntryLength(const EntryAddress & _address, int8_t _amount)
{
    QByteArray buffer(mp_info->cluster_size, Qt::Uninitialized);
    readCluster(_address.cluster, false, buffer.data());
    FSEntry * entries = reinterpret_cast<FSEntry *>(buffer.data());
    entries[_address.entry].length += _amount;
    writeCluster(_address.cluster, false, reinterpret_cast<const char *>(entries));
}

void FileSystem::writeFATEntry(uint32_t _cluster, FATEntry _entry)
{
    const uint32_t fat_cluster = m_fat.fatCluster(_cluster);
    const uint32_t entry_index = _cluster % mp_info->fat_entries_per_cluster;
    m_file.seek(fat_cluster * mp_info->cluster_size + entry_index * sizeof(FATEntry));
    m_file.write(reinterpret_cast<const char *>(&_entry), sizeof(FATEntry));
    m_fat.setEntry(_cluster, _entry);
}

bool FileSystem::findFreeEntry(const QList<uint32_t> & _parent_clusters, EntrySearchResult & _result)
{
    QByteArray buffer(mp_info->cluster_size, Qt::Uninitialized);
    foreach(uint32_t parent_cluster, _parent_clusters)
    {
        readCluster(parent_cluster, false, buffer.data());
        FSEntry * entries = reinterpret_cast<FSEntry *>(buffer.data());
        for(size_t entry_index = 0; entry_index < mp_info->fs_entries_per_cluster; ++entry_index)
        {
            FSEntry & entry = entries[entry_index];
            if(!(entry.mode & EM_EXISTS))
            {
                _result.cluster_entries = QVector<FSEntry>(entries, &entries[mp_info->fs_entries_per_cluster]);
                _result.cluster_index = parent_cluster;
                _result.entry_index = entry_index;
                return true;
            }
        }
    }
    return false;
}

void FileSystem::writeCluster(uint32_t _cluster, bool _is_absolute, const char * _buffer)
{
    const uint32_t offset = ((_is_absolute ? 0 : mp_info->alloc_offset) + _cluster) * mp_info->cluster_size;
    m_file.seek(offset);
    m_file.write(_buffer, mp_info->cluster_size);
}

std::optional<QList<uint32_t>> FileSystem::alloc(uint32_t _allocation_size)
{
    qsizetype cluster_count = _allocation_size / mp_info->cluster_size;
    if(_allocation_size % mp_info->cluster_size)
        ++cluster_count;

    std::optional<QList<uint32_t>> result = m_fat.findFreeClusters(cluster_count);
    if(!result)
        return std::nullopt;

    for(qsizetype cluster = 0; cluster < cluster_count; ++cluster)
    {
        writeFATEntry(
            result->at(cluster),
            cluster == cluster_count - 1 ? FATEntry::endOfFile() : FATEntry::pointer(result->at(cluster + 1)));
    }

    return result;
}

void FileSystem::freeFAT(const QList<uint32_t> & _clusters)
{
    const FATEntry free = FATEntry::free();
    foreach(uint32_t cluster, _clusters)
    {
        m_fat.setEntry(cluster, free);
        writeFATEntry(cluster, free);
    }
}

void FileSystem::remove(const Path & _path)
{
    std::optional<EntryPath> entry_path = resolvePath(_path);
    if(!entry_path.has_value())
        throwPathNotFound(_path);

    const std::optional<EntryPath> parent = resolvePath(_path.up());

    { // Mark parent's entry as unused
        QByteArray buffer(mp_info->cluster_size, Qt::Uninitialized);
        readCluster(entry_path->address.cluster, false, buffer.data());
        FSEntry * entries = reinterpret_cast<FSEntry *>(buffer.data());
        entries[entry_path->address.entry].mode = 0;
        writeCluster(entry_path->address.cluster, false, buffer.data());
    }

    eraseEntriesRecursive(*entry_path);

    // Decrement parent's length
    changeEntryLength(parent->address, -1);
}

void FileSystem::eraseEntriesRecursive(const EntryPath & _path)
{
    if(_path.entry.isDirectory())
    {
        forEachEntry(_path.entry, [this](const EntryPath & __child_path) -> bool {
            if(__child_path.entry.name() != "." && __child_path.entry.name() != "..")
                eraseEntriesRecursive(__child_path);
            return true;
        });
    }
    else
    {
        freeFAT(getEntryClusters(_path.entry));
    }
}

void FileSystem::rename(const Path & _path, const QByteArray & _new_name)
{
    std::optional<EntryPath> entry_path = resolvePath(_path);
    if(!entry_path)
        throwPathNotFound(_path);
    QByteArray buffer(mp_info->cluster_size, Qt::Uninitialized);
    readCluster(entry_path->address.cluster, false, buffer.data());
    FSEntry * entries = reinterpret_cast<FSEntry *>(buffer.data());
    memset(entries[entry_path->address.entry].name, 0, sizeof(FSEntry::name));
    strncpy(
        entries[entry_path->address.entry].name,
        _new_name.data(),
        std::min(sizeof(FSEntry::name), static_cast<size_t>(_new_name.size())));
    writeCluster(entry_path->address.cluster, false, buffer.data());
}

uint32_t FileSystem::totalUsedBytes() const
{
    return m_fat.allocatedCount() * mp_info->cluster_size;
}

uint32_t FileSystem::totalFreeBytes() const
{
    const uint32_t allocable_space = (mp_info->alloc_end - mp_info->alloc_offset) * mp_info->cluster_size;
    return allocable_space - totalUsedBytes();
}
