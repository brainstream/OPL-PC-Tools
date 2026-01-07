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

#include <OplPcTools/MCFS/FileSystemDriver.h>
#include <OplPcTools/File.h>
#include <QRegularExpression>

using namespace OplPcTools;
using namespace OplPcTools::MCFS;

#define INVALID_CLUSTER_PTR static_cast<uint32_t>(-1)
#define NULL_CLUSTER_PTR 0

namespace {

[[noreturn]] inline void throwNotFormatted()
{
    throw MCFSException(QObject::tr("The VMC is corrupted or not formatted correctly"));
}

[[noreturn]] inline void throwPathNotFound()
{
    throw MCFSException(QObject::tr("Path not found"));
}

[[noreturn]] inline void throwNoSpace()
{
    throw MCFSException(QObject::tr("Not enough free space"));
}

} // namespace

struct MemoryCardFile::Private
{
    FileSystemDriver * driver;
    QByteArray name;
    uint32_t size;
    uint32_t position;
    QList<uint32_t> clusters;
};

MemoryCardFile::MemoryCardFile(Private * _private) :
    mp_private(_private)
{
}

MemoryCardFile::~MemoryCardFile()
{
    delete mp_private;
}

uint32_t MemoryCardFile::size() const
{
    return mp_private->size;
}

const QByteArray & MemoryCardFile::name() const
{
    return mp_private->name;
}

bool MemoryCardFile::seek(uint32_t _pos)
{
    if(_pos == mp_private->position)
        return true;
    if(_pos >= mp_private->size)
        return false;
    mp_private->position = _pos;
    return true;
}

int64_t MemoryCardFile::read(char * _buffer, int64_t _max_size)
{
    return mp_private->driver->readFile(*this->mp_private, _buffer, _max_size);
}

FileSystemDriver::FileSystemDriver(const QString & _filepath) :
    m_file(_filepath),
    mp_info(nullptr)
{
}

FileSystemDriver::~FileSystemDriver()
{
    deinit();
}

void FileSystemDriver::deinit()
{
    if(m_file.isOpen())
        m_file.close();
    delete mp_info;
    mp_info = nullptr;
    m_fat.reset();
}

const FSInfo * FileSystemDriver::info() const
{
    return mp_info;
}

void FileSystemDriver::load()
{
    deinit();
    ::openFile(m_file, QIODevice::OpenMode(QIODevice::ReadWrite | QIODevice::ExistingOnly));
    readSuperblock();
    readFAT();
}

void FileSystemDriver::readSuperblock()
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
    mp_info->clusters_per_block = sb->clusters_per_block;
    mp_info->cardform = sb->cardform;
    mp_info->max_allocatable_clusters = sb->max_allocatable_clusters;
}

void FileSystemDriver::readCluster(uint32_t _cluster, bool _is_absolute, char * _buffer, uint32_t _size /*= 0*/)
{
    qint64 offset = _cluster * mp_info->cluster_size;
    if(!_is_absolute)
        offset += mp_info->alloc_offset * mp_info->cluster_size;
    read(offset, _buffer, _size == 0 ? mp_info->cluster_size : _size);
}

void FileSystemDriver::read(quint64 _offset, char * _buffer, uint32_t _size)
{
    if(!m_file.seek(_offset) || m_file.read(_buffer, _size) != _size)
        throwNotFormatted();
}

void FileSystemDriver::validateSuperblock(const Superblock & _sb) const
{
    static QRegularExpression version_regex("^1\\.[012]\\.0\\.0$");
    if(
        std::strncmp(g_mcfs_magic, _sb.magic, strlen(g_mcfs_magic)) != 0 ||
        !version_regex.match(_sb.version).hasMatch() ||
        _sb.pagesize != 512 ||
        (
            _sb.cluster_size != 1024 && _sb.cluster_size != 512) ||
            _sb.pages_per_cluster != _sb.cluster_size / _sb.pagesize ||
            _sb.alloc_offset == INVALID_CLUSTER_PTR ||
            _sb.alloc_offset < 2 ||
            _sb.alloc_end == INVALID_CLUSTER_PTR ||
            _sb.alloc_end < _sb.alloc_offset ||
            _sb.ifc_ptr_list[0] >= _sb.alloc_offset
        )
    {
        throwNotFormatted();
    }
}

void FileSystemDriver::readFAT()
{
    m_fat.reset();
    const size_t entries_per_cluster = mp_info->cluster_size / sizeof(uint32_t);

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
    const size_t fat_ptrs_count = ifc_ptr_count * entries_per_cluster;
    QScopedArrayPointer<uint32_t> fat_ptrs(new uint32_t[fat_ptrs_count]);
    for(size_t i = 0; i < ifc_ptr_count; ++i)
    {
        char * address = reinterpret_cast<char *>(fat_ptrs.data()) + (mp_info->cluster_size * i);
        readCluster(mp_info->ifc_ptr_list[i], true, address);
    }

    //
    // Loading FATs
    //
    QList<FATEntry> fat(entries_per_cluster);
    for(size_t i = 0; i < fat_ptrs_count; ++i)
    {
        const uint32_t fat_cluster = fat_ptrs[i];
        if(fat_cluster == INVALID_CLUSTER_PTR)
            break;
        readCluster(fat_cluster, true, reinterpret_cast<char *>(fat.data()));
        m_fat.append(fat_cluster, fat);
    }
}

QList<EntryInfo> FileSystemDriver::enumerateEntries(const VmcPath & _path)
{
    std::optional<EntryInfo> entry = resolvePath(_path);
    QList<EntryInfo> result;
    if(entry.has_value())
    {
        enumerateEntries(*entry, [&](const EntryInfo & next_entry) -> bool {
            if(next_entry.name.compare(".") != 0  && next_entry.name.compare("..") != 0)
                result << next_entry;
            return true;
        });
    }
    else
    {
        throwPathNotFound();
    }
    return result;
}

std::optional<EntryInfo> FileSystemDriver::resolvePath(const VmcPath & _path)
{
    EntryInfo entry = getRootEntry();
    for(const QByteArray & path_part : _path.parts())
    {
        bool matched = false;
        enumerateEntries(entry, [&](const EntryInfo & next_entry) -> bool {
            if(path_part.compare(next_entry.name, Qt::CaseInsensitive) == 0)
            {
                matched = true;
                entry = next_entry;
                return false;
            }
            return true;
        });
        if(!matched)
            return std::nullopt;
    }
    return entry;
}

EntryInfo FileSystemDriver::getRootEntry()
{
    QScopedArrayPointer<char> ptr(new char[sizeof(FSEntry)]);
    readCluster(mp_info->rootdir_cluster, false, ptr.data(), sizeof(FSEntry));
    return map(*reinterpret_cast<FSEntry *>(ptr.data()));
}

EntryInfo FileSystemDriver::map(const FSEntry & _fs_entry) const
{
    EntryInfo info;
    info.is_directory = _fs_entry.mode & EM_DIRECTORY;
    info.cluster = _fs_entry.cluster;
    info.name = QByteArray(_fs_entry.name, std::min(sizeof(FSEntry::name), std::strlen(_fs_entry.name)));
    info.length = _fs_entry.length;
    return info;
}

void FileSystemDriver::enumerateEntries(const EntryInfo & _dir, std::function<bool(const EntryInfo &)> _callback)
{
    const size_t entry_count_per_cluster = mp_info->cluster_size / sizeof(FSEntry);
    QList<uint32_t> clusters = getEntryClusters(_dir);
    QScopedArrayPointer<FSEntry> entries(new FSEntry[entry_count_per_cluster]);
    uint32_t read_count = 0;
    foreach(uint32_t cluster, clusters)
    {
        readCluster(cluster, false, reinterpret_cast<char *>(entries.data()));
        for(size_t i = 0; i < entry_count_per_cluster; ++i)
        {
            if(++read_count > _dir.length)
                return;
            const FSEntry & entry = entries[i];
            if(!(entry.mode & EM_EXISTS))
                continue;
            if(!_callback(map(entry)))
                return;
        }
    }
}

QList<uint32_t> FileSystemDriver::getEntryClusters(const EntryInfo & _entry) const
{
    if(_entry.cluster > mp_info->max_allocatable_clusters)
        throwNotFormatted();
    QList<uint32_t> result;
    for(uint32_t cluster = _entry.cluster;;) // TODO: check overflow, cycles, etc.
    {
        result.append(cluster);
        FATEntry fat_entry = m_fat[cluster];
        if(fat_entry.isEndOfFile())
            break;
        cluster = fat_entry.cluster;
    }
    return result;
}

QSharedPointer<MemoryCardFile> FileSystemDriver::openFile(const VmcPath & _path)
{
    std::optional<EntryInfo> entry = resolvePath(_path);
    if(!entry.has_value())
        throw MCFSException(QObject::tr("File not found"));
    if(entry->is_directory)
        throw MCFSException(QObject::tr("\"%1\" is not a file").arg(_path));
    MemoryCardFile::Private * pr = new MemoryCardFile::Private;
    pr->clusters = getEntryClusters(*entry);
    pr->driver = this;
    pr->position = 0;
    pr->size = entry->length;
    pr->name = entry->name;
    return QSharedPointer<MemoryCardFile>(new MemoryCardFile(pr));
}

int64_t FileSystemDriver::readFile(MemoryCardFile::Private & _file, char * _buffer, uint32_t _max_size)
{
    const uint32_t start_cluster_index = _file.position / mp_info->cluster_size;
    const uint32_t cluster_count = static_cast<uint32_t>(_file.clusters.size());
    if(_file.position >= _file.size || start_cluster_index >= cluster_count)
        return -1;
    uint32_t position_in_cluster = _file.position % mp_info->cluster_size;
    uint32_t position_in_file = _file.position;
    uint32_t position_in_buffer = 0;
    QScopedArrayPointer<char> cluster_buff(new char[mp_info->cluster_size]);
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

void FileSystemDriver::writeFile(const VmcPath & _path, const QByteArray & _data)
{
    VmcPath dir_path = _path.up();
    std::optional<EntryInfo> dir_entry = resolvePath(dir_path);
    if(!dir_entry.has_value())
        throwPathNotFound();
    if(!dir_entry->is_directory)
        throw MCFSException(QObject::tr("\"%1\" is not a directory").arg(dir_path.path()));

    std::optional<QList<uint32_t>> file_data_clusters = alloc(_data.size());
    if(!file_data_clusters)
        throwNoSpace();

    EntryInfo file_entry
    {
        .name = _path.filename(),
        .is_directory = false,
        .cluster = file_data_clusters->first(),
        .length = static_cast<uint32_t>(_data.length())
    };
    if(!allocEntry(*dir_entry, file_entry))
    {
        free(*file_data_clusters);
        throwNoSpace();
    }

    {
        QByteArray buffer(mp_info->cluster_size, Qt::Uninitialized);
        foreach(uint32_t cluster, *file_data_clusters)
        {
            const qsizetype offset = cluster * mp_info->cluster_size;
            std::memcpy(
                buffer.data(),
                &_data.data()[offset],
                std::min(mp_info->cluster_size, static_cast<uint32_t>(_data.size() - offset)));
            writeCluster(cluster, false, buffer.data());
        }
    }
}

bool FileSystemDriver::allocEntry(const EntryInfo & _parent, const EntryInfo & _entry)
{
    const size_t entry_count_per_cluster = mp_info->cluster_size / sizeof(FSEntry);
    QList<uint32_t> parent_dir_clusters = getEntryClusters(_parent);

    FSEntrySearchResult parent_free_entry;
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
        parent_free_entry.cluster_entries.reset(new FSEntry[entry_count_per_cluster]);
        for(size_t i = 0; i < entry_count_per_cluster; ++i)
        {
            parent_free_entry.cluster_entries[i] = {};
            parent_free_entry.cluster_entries[i].cluster = 0xFFFFFFFF;
        }
    }

    FSEntry & new_entry = parent_free_entry.cluster_entries[parent_free_entry.entry_index];
    new_entry.mode = EM_EXISTS;
    if(_entry.is_directory)
        new_entry.mode |= EM_DIRECTORY;
    else
        new_entry.mode |= EM_READ | EM_WRITE;
    new_entry.length = _entry.length;
    new_entry.cluster = _entry.cluster;
    new_entry.created = new_entry.modified = FSDateTime::now();
    std::strncpy(
        new_entry.name,
        _entry.name.data(),
        std::min(sizeof(FSEntry::name), static_cast<size_t>(_entry.name.length())));

    // Updating an old or new cluster
    writeCluster(
        parent_free_entry.cluster_index,
        false,
        reinterpret_cast<const char *>(parent_free_entry.cluster_entries.get()));

    return true;
}

void FileSystemDriver::writeFATEntry(uint32_t _cluster, FATEntry _entry)
{
    const uint32_t fat_cluster = m_fat.fatCluster(_cluster);
    const uint32_t entry_index = _cluster % mp_info->fat_entries_per_cluster;
    m_file.seek(fat_cluster * mp_info->cluster_size + entry_index * sizeof(FATEntry));
    m_file.write(reinterpret_cast<const char *>(&_entry), sizeof(FATEntry));
    m_fat[_cluster] = _entry;
}

bool FileSystemDriver::findFreeEntry(const QList<uint32_t> & _parent_clusters, FSEntrySearchResult & _result)
{
    const size_t entry_count_per_cluster = mp_info->cluster_size / sizeof(FSEntry);
    std::unique_ptr<FSEntry[]> entries(new FSEntry[entry_count_per_cluster]);
    foreach(uint32_t parent_cluster, _parent_clusters)
    {
        readCluster(parent_cluster, false, reinterpret_cast<char *>(entries.get()));
        for(size_t entry_index = 1; entry_index < entry_count_per_cluster; ++entry_index)
        {
            FSEntry & entry = entries[entry_index];
            if(!(entry.mode & EM_EXISTS))
            {
                _result.cluster_entries.swap(entries);
                _result.cluster_index = parent_cluster;
                _result.entry_index = entry_index;
                return true;
            }
        }
    }
    return false;
}

void FileSystemDriver::writeCluster(uint32_t _cluster, bool _is_absolute, const char * _buffer)
{
    const uint32_t offset = ((_is_absolute ? 0 : mp_info->alloc_offset) + _cluster) * mp_info->cluster_size;
    m_file.seek(offset);
    m_file.write(_buffer, mp_info->cluster_size);
}

std::optional<QList<uint32_t>> FileSystemDriver::alloc(uint32_t _allocation_size)
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

void FileSystemDriver::free(const QList<uint32_t> & _clusters)
{
    foreach(uint32_t cluster, _clusters)
    writeFATEntry(cluster, m_fat[cluster] = FATEntry::free());
}
