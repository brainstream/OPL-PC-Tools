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


// ================================================================================================
//
//  List of Inderect FAT Clusters (IFC) (Superblock offset: 0x50).
//  On a standard 8M card there's only one indirect FAT cluster.
//  ┌─┬─┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┐
//  │8│9│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│FF│
//  └┰┴┰┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┘
//   ┃ ┃
//   ┃ ┃    Lists of FAT clusters. Every value points to a FAT.
//   ┃ ┃    ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
//   ┃ ┗━━━━┥261│262│263│264│265│266│267│268│269│26A│26B│26C│26D│26E│26F│...│360│
//   ┃      └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
//   ┃      ┌─┬─┬─┬─┬─┬─┬─┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬───┬───┐
//   ┗━━━━━━┥F│A│B│C│E│E│F│10│11│12│13│14│15│16│17│18│19│1A│1B│1C│...│260│
//          └┰┴─┴─┴─┴─┴─┴─┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴───┴───┘
//           ┃
//           ┃   FAT. The cluster numbers are relative to the alloc_offset.
//           ┃   ┌──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬───┐
//           ┗━━━┥01│02│FF│04│05│FF│06│07│FF│09│0A│0B│0C│FF│...│
//               └──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴───┘
//
// ================================================================================================
//
// Assume that the cluster size is 1024 bytes (it is default size).
// Then every FAT contains 256 entries (4 bytes per entry).
// Every entry points to a cluster.
// So, a single FAT can address 256 * 1024 = 262144 bytes.
//
// Every FAT inderect list contains 256 entries. So, there are 256 FAT.
// Thus, a single FAT inderect list can address 256 * 262144 = 67108864 bytes (or 64 MiB).
//
// ================================================================================================

#include <cstring>
#include <cmath>
#include <functional>
#include <optional>
#include <QRegExp>
#include <QScopedPointer>
#include <QDateTime>
#include <QTimeZone>
#include <OplPcTools/File.h>
#include <OplPcTools/VmcFS.h>

#define INVALID_CLUSTER_PTR static_cast<uint32_t>(-1)
#define NULL_CLUSTER_PTR 0

using namespace OplPcTools;

namespace {

const char * g_vmc_magic = "Sony PS2 Memory Card Format ";
const char * g_vmc_version = "1.2.0.0";

struct VmcSuperblock final
{                                      // OFFSET:  (DEC)  (HEX)
    char magic[28];                    //          0      0x0
    char version[12];                  //          28     0x1C
    int16_t pagesize;                  //          40     0x28
    uint16_t pages_per_cluster;        //          42     0x2A
    uint16_t pages_per_block;          //          44     0x2C
    uint16_t unused;                   //          46     0x2E
    uint32_t clusters_per_card;        //          48     0x30
    uint32_t alloc_offset;             //          52     0x34
    uint32_t alloc_end;                //          56     0x38
    uint32_t rootdir_cluster;          //          60     0x3C
    uint32_t backup_block1;            //          64     0x40
    uint32_t backup_block2;            //          68     0x44
    uint8_t unused2[8];                //          72     0x48
    uint32_t ifc_ptr_list[32];         //          80     0x50
    uint32_t bad_block_list[32];       //          208    0xD0
    uint8_t cardtype;                  //          336    0x150
    uint8_t cardflags;                 //          337    0x151
    uint16_t unused3;                  //          338    0x152
    uint32_t cluster_size;             //          340    0x154
    uint32_t fat_entries_per_cluster;  //          344    0x158
    uint32_t clusters_per_block;       //          348    0x15C
    int32_t cardform;                  //          352    0x160
    uint32_t rootdir_cluster2;         //          356    0x164
    uint32_t unknown1;                 //          360    0x168
    uint32_t unknown2;                 //          364    0x16C
    uint32_t max_allocatable_clusters; //          368    0x170
    uint32_t unknown3;                 //          372    0x174
    uint32_t unknown4;                 //          376    0x178
    int32_t unknown5;                  //          380    0x17C
} __attribute__((packed));             // TOTAL:   384    0x180


struct FSDateTime
{
    uint8_t resv2;
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} __attribute__((packed));


struct FSEntry
{                           // OFFSET:  (DEC)  (HEX)
    uint16_t mode;          //          0      0x0
    uint16_t unused;        //          2      0x2
    uint32_t length;        //          4      0x4
    FSDateTime created;     //          8      0x8
    uint32_t cluster;       //          16     0x10
    uint32_t dir_entry;     //          20     0x14
    FSDateTime modified;    //          24     0x18
    uint32_t attr;          //          32     0x20
    uint32_t unused2[7];    //          36     0x24
    char name[32];          //          64     0x40
    uint8_t unused3[416];   //          96     0x60
} __attribute__((packed));  // TOTAL:   512    0x200


enum FSEntryMode
{
    EM_READ = 0x1,
    EM_WRITE = 0x2,
    EM_EXECUTE = 0x4,
    EM_PROTECTED = 0x8,
    EM_FILE = 0x10,
    EM_DIRECTORY = 0x20,
    EM_POCKETSTATION = 0x800,
    EM_PLAYSTATION = 0x1000,
    EM_HIDDENT = 0x2000,
    EM_EXISTS = 0x8000
};


struct FATEntry
{
    uint32_t cluster: 24;
    uint8_t flag: 8;
} __attribute__((packed));


struct EntryInfo
{
    QString name;
    bool is_directory;
    uint32_t cluster;
    uint32_t length;

    VmcEntryInfo toVmcEntryInfo() const
    {
        VmcEntryInfo vei;
        vei.name = name;
        vei.is_directory = is_directory;
        vei.size = length;
        return vei;
    }
};

class VmcFormatter final
{
public:
    static void format(const QString & _filename, uint8_t _size_mib);

private:
    VmcFormatter(const QString & _filename, uint8_t _size_mib);
    ~VmcFormatter();
    void format();
    void clearFile();
    void initSuperblock();
    void writeSuperblock();
    void writeFAT();
    void writeRootDirectory();

private:
    VmcSuperblock * mp_sb;
    QFile m_file;
    uint8_t m_size_mib;
};

} // namespace

VmcFormatter::VmcFormatter(const QString & _filename, uint8_t _size_mib) :
    mp_sb(new VmcSuperblock { }),
    m_file(_filename),
    m_size_mib(_size_mib)
{
}

VmcFormatter::~VmcFormatter()
{
    delete mp_sb;
}

void VmcFormatter::format(const QString & _filename, uint8_t _size_mib)
{
    if(_size_mib < 8 || _size_mib > 128)
    {
        throw ValidationException(QObject::tr(
            "VMC size must be greater than or equal to 8 Mib and less than or equal to 128 Mib."
        ));
    }
    VmcFormatter(_filename, _size_mib).format();
}

void VmcFormatter::format()
{
    openFile(m_file, QIODevice::WriteOnly | QIODevice::NewOnly);
    clearFile();
    initSuperblock();
    writeSuperblock();
    writeFAT();
}

void VmcFormatter::clearFile()
{
    m_file.seek(0);
    const size_t buffer_size = 1024 * 1024;
    QScopedArrayPointer<char> buffer(new char[buffer_size]);
    std::memset(buffer.data(), -1, buffer_size);
    for(int i = 0; i < m_size_mib; ++i)
    {
        m_file.write(buffer.data(), buffer_size);
    }
}

void VmcFormatter::initSuperblock()
{
    const uint32_t card_size_in_bytes = m_size_mib * 1024 * 1024;
    mp_sb->pagesize = 512;
    mp_sb->pages_per_cluster = 2;
    mp_sb->clusters_per_block = 8;
    mp_sb->cluster_size = mp_sb->pagesize * mp_sb->pages_per_cluster;
    mp_sb->pages_per_block = mp_sb->clusters_per_block * mp_sb->pages_per_cluster;
    mp_sb->clusters_per_card = card_size_in_bytes / mp_sb->cluster_size;
    const uint32_t pointers_per_cluster = mp_sb->cluster_size / sizeof(uint32_t);
    mp_sb->fat_entries_per_cluster = pointers_per_cluster;
    const uint32_t blocks_per_card = mp_sb->clusters_per_card / mp_sb->clusters_per_block;
    mp_sb->backup_block1 = blocks_per_card - 1;
    mp_sb->backup_block2 = blocks_per_card - 2;
    mp_sb->cardtype = 2;
    mp_sb->cardflags = 0x2b;
    mp_sb->cardform = 1;
    mp_sb->rootdir_cluster = mp_sb->rootdir_cluster2 = 0;
    mp_sb->unknown5 = -1;
    std::strcpy(mp_sb->magic, g_vmc_magic);
    std::strcpy(mp_sb->version, g_vmc_version);
    std::memset(mp_sb->bad_block_list, -1, sizeof(VmcSuperblock::bad_block_list));
    const uint32_t available_cluster_count = mp_sb->clusters_per_card - mp_sb->clusters_per_block * 3; // superblock and 2 backups
    const uint32_t fat_list_count = static_cast<uint32_t>(std::ceil(
        static_cast<double>(available_cluster_count) / (pointers_per_cluster + 1 + 1.0d / pointers_per_cluster)
    ));
    const uint32_t ifc_list_count = static_cast<uint32_t>(
        std::ceil(static_cast<double>(fat_list_count) / pointers_per_cluster)
    );
    for(uint32_t i = 0; i < ifc_list_count; ++i)
    {
        mp_sb->ifc_ptr_list[i] = mp_sb->clusters_per_block + i; // IFCs are placed next to the superblock
    }
    mp_sb->alloc_offset = mp_sb->clusters_per_block + fat_list_count + ifc_list_count;
    mp_sb->alloc_end = mp_sb->clusters_per_card - 2 * mp_sb->clusters_per_block; // 2 backup blocks at the end of the card
    mp_sb->max_allocatable_clusters = available_cluster_count - fat_list_count - ifc_list_count;
}

void VmcFormatter::writeSuperblock()
{
    m_file.seek(0);
    const char * buffer = reinterpret_cast<const char *>(mp_sb);
    m_file.write(buffer, sizeof(VmcSuperblock));
}

void VmcFormatter::writeFAT()
{
    const uint32_t fat_entry_count = mp_sb->max_allocatable_clusters;
    const uint32_t pointers_per_cluster = mp_sb->fat_entries_per_cluster;
    const uint32_t fat_cluster_count = static_cast<uint32_t>(std::ceil(static_cast<double>(fat_entry_count) / pointers_per_cluster));
    const uint32_t ifc_cluster_count = static_cast<uint32_t>(std::ceil(static_cast<double>(fat_cluster_count) / pointers_per_cluster));
    const uint32_t ifcs_size_in_bytes = ifc_cluster_count * mp_sb->cluster_size;
    const uint32_t fat_start_cluster_index = mp_sb->clusters_per_block + ifc_cluster_count;
    // IFCs
    //
    QScopedArrayPointer<char> ifc_clusters_raw(new char[ifcs_size_in_bytes]);
    std::memset(ifc_clusters_raw.data(), 0, ifcs_size_in_bytes);
    uint32_t * ifcs = reinterpret_cast<uint32_t *>(ifc_clusters_raw.data());
    for(uint32_t i = 0; i < fat_cluster_count; ++i)
    {
        ifcs[i] = fat_start_cluster_index + i;
    }
    m_file.seek(mp_sb->clusters_per_block * mp_sb->cluster_size);
    m_file.write(ifc_clusters_raw.data(), ifcs_size_in_bytes);
    ifc_clusters_raw.reset();
    // FATs
    //
    QScopedPointer<char> fat_cluster_raw(new char[mp_sb->cluster_size]);
    FATEntry fat_entry;
    fat_entry.flag = 0x7F; // TODO: flags
    fat_entry.cluster = 0xFFFFFF;
    for(uint32_t i = 0; i < mp_sb->fat_entries_per_cluster; ++i)
    {
        memcpy(&fat_cluster_raw.data()[sizeof(FATEntry) * i], &fat_entry, sizeof(FATEntry));
    }
    m_file.seek(fat_start_cluster_index * mp_sb->cluster_size);
    for(uint32_t i = 0; i < fat_entry_count / mp_sb->fat_entries_per_cluster; ++i)
    {
        m_file.write(fat_cluster_raw.data(), mp_sb->cluster_size);
    }
    if(fat_entry_count % mp_sb->fat_entries_per_cluster)
    {
        m_file.write(fat_cluster_raw.data(), (fat_entry_count % mp_sb->fat_entries_per_cluster) * sizeof(FATEntry));
    }
    fat_cluster_raw.reset();
    // Root directory
    //
    writeRootDirectory();
    fat_entry.flag = 0xFF; // TODO: flags
    m_file.seek(fat_start_cluster_index * mp_sb->cluster_size);
    m_file.write(reinterpret_cast<char *>(&fat_entry), sizeof(FATEntry));
}

void VmcFormatter::writeRootDirectory()
{
    QDateTime japanTime = QDateTime::currentDateTimeUtc().toTimeZone(QTimeZone(9 * 60 * 60));
    FSDateTime dt = { };
    dt.year = japanTime.date().year();
    dt.month = japanTime.date().month();
    dt.day  = japanTime.date().day();
    dt.hour = japanTime.time().hour();
    dt.min = japanTime.time().minute();
    dt.sec = japanTime.time().second();
    FSEntry entries[2];
    std::memset(entries, 0, sizeof(entries));
    for(int i = 0; i < 2; ++i)
    {
        entries[i].attr = 0;
        entries[i].mode = EM_READ | EM_WRITE | EM_EXECUTE | EM_DIRECTORY | EM_EXISTS;
        entries[i].cluster = mp_sb->rootdir_cluster;
        entries[i].unused = 0;
        entries[i].length = 2;
        entries[i].created = dt;
        entries[i].dir_entry = 0;
        entries[i].modified = dt;
        entries[i].name[0] = '.';
    }
    entries[0].name[1] = '\0';
    entries[1].name[1] = '.';
    entries[1].name[2] = '\0';
    m_file.seek((mp_sb->alloc_offset + mp_sb->rootdir_cluster) * mp_sb->cluster_size);
    m_file.write(reinterpret_cast<const char *>(entries), sizeof(entries));
}


class VmcFS::Private final
{
public:
    explicit Private(const QString & _filepath);
    ~Private();
    void load();
    void create(uint8_t _size_mib);
    const VmcInfo * info() const;
    QList<VmcEntryInfo> enumerateEntries(const QString & _path);
    void exportEntry(const QString & _vmc_path, const QString & _dest_path);
    QSharedPointer<VmcFile> openFile(const QString & _path);
    int64_t readFile(VmcFile::Private & _file, char * _buffer, uint32_t _max_size);

private:
    void deinit();
    void readSuperblock();
    void readCluster(uint32_t _cluster, bool _is_absolute, char * _buffer, uint32_t _size = 0);
    void read(quint64 _offset, char * _buffer, uint32_t _size);
    void validateSuperblock(const VmcSuperblock & _sb) const;
    void throwNotFormatted() const;
    void readFAT();
    std::optional<EntryInfo> resolvePath(const QString & _path);
    EntryInfo getRootEntry();
    inline EntryInfo map(const FSEntry & _fs_entry) const;
    void enumerateEntries(const EntryInfo & _dir, std::function<bool(const EntryInfo &)> _callback);
    QList<uint32_t> getEntryClusters(const EntryInfo & _entry) const;

private:
    QFile m_file;
    VmcInfo * mp_info;
    FATEntry * mp_fat;
    size_t m_fat_size;
};

struct VmcFile::Private
{
    VmcFS::Private * fs;
    QString name;
    uint32_t size;
    uint32_t position;
    QList<uint32_t> clusters;
};

VmcFS::Private::Private(const QString & _filepath) :
    m_file(_filepath),
    mp_info(nullptr),
    mp_fat(nullptr),
    m_fat_size(0)
{
}

VmcFS::Private::~Private()
{
    deinit();
}

void VmcFS::Private::deinit()
{
    if(m_file.isOpen())
        m_file.close();
    delete mp_info;
    mp_info = nullptr;
    delete [] mp_fat;
    mp_fat = nullptr;
    m_fat_size = 0;
}

const VmcInfo * VmcFS::Private::info() const
{
    return mp_info;
}

void VmcFS::Private::load()
{
    deinit();
    ::openFile(m_file, QIODevice::ReadOnly);
    readSuperblock();
    readFAT();
}

void VmcFS::Private::readSuperblock()
{
    QScopedPointer<VmcSuperblock> sb(new VmcSuperblock);
    read(0, reinterpret_cast<char *>(sb.data()), sizeof(VmcSuperblock));
    validateSuperblock(*sb);
    mp_info = new VmcInfo;
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

void VmcFS::Private::readCluster(uint32_t _cluster, bool _is_absolute, char * _buffer, uint32_t _size /*= 0*/)
{
    qint64 offset = _cluster * mp_info->cluster_size;
    if(!_is_absolute)
        offset += mp_info->alloc_offset * mp_info->cluster_size;
    read(offset, _buffer, _size == 0 ? mp_info->cluster_size : _size);
}

void VmcFS::Private::read(quint64 _offset, char * _buffer, uint32_t _size)
{
    if(!m_file.seek(_offset) || m_file.read(_buffer, _size) != _size)
        throwNotFormatted();
}

void VmcFS::Private::validateSuperblock(const VmcSuperblock & _sb) const
{
    QRegExp version_regex("^1\\.[012]\\.0\\.0$");
    if(
       std::strncmp(g_vmc_magic, _sb.magic, strlen(g_vmc_magic)) != 0 ||
       !version_regex.exactMatch(_sb.version) ||
       _sb.pagesize != 512 ||
       (_sb.cluster_size != 1024 && _sb.cluster_size != 512) ||
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

void VmcFS::Private::throwNotFormatted() const
{
    throw VmcFSException(QObject::tr("The VMC is corrupted or not formatted correctly"));
}

void VmcFS::Private::readFAT()
{
    if(mp_fat)
    {
        delete [] mp_fat;
        mp_fat = nullptr;
    }
    const size_t entries_per_cluster = mp_info->cluster_size / sizeof(uint32_t);
    //
    // Loading IFC
    //
    size_t ifc_ptr_count = 0;
    for(; ifc_ptr_count < sizeof(VmcSuperblock::ifc_ptr_list) / sizeof(uint32_t); ++ifc_ptr_count)
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
    // Loading FAT
    //
    size_t valid_fat_ptr_count = 0;
    for(size_t i = 0; i < fat_ptrs_count; ++i)
    {
        if(fat_ptrs[i] == INVALID_CLUSTER_PTR)
            break;
        ++valid_fat_ptr_count;
    }
    uint32_t fat_size= valid_fat_ptr_count * entries_per_cluster;
    QScopedArrayPointer<FATEntry> fat_tmp(new FATEntry[fat_size]);
    for(size_t i = 0; i < valid_fat_ptr_count; ++i)
    {
        char * address = reinterpret_cast<char *>(fat_tmp.data()) + (mp_info->cluster_size * i);
        readCluster(fat_ptrs[i], true, address);
    }
    mp_fat = fat_tmp.take();
    m_fat_size = fat_size;
}

QList<VmcEntryInfo> VmcFS::Private::enumerateEntries(const QString & _path)
{
    std::optional<EntryInfo> entry = resolvePath(_path);
    QList<VmcEntryInfo> result;
    if(entry.has_value())
    {
        enumerateEntries(*entry, [&](const EntryInfo & next_entry) -> bool {
            if(next_entry.name.compare(".") != 0  && next_entry.name.compare("..") != 0)
                result << next_entry.toVmcEntryInfo();
            return true;
        });
    }
    else
    {
        throw VmcFSException(QObject::tr("Path not found"));
    }
    return result;
}

std::optional<EntryInfo> VmcFS::Private::resolvePath(const QString & _path)
{
    QStringList path_parts = _path.split(VmcFS::path_separator, QString::SkipEmptyParts);
    EntryInfo entry = getRootEntry();
    for(const QString & path_part : path_parts)
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

EntryInfo VmcFS::Private::getRootEntry()
{
    QScopedArrayPointer<char> ptr(new char[sizeof(FSEntry)]);
    readCluster(mp_info->rootdir_cluster, false, ptr.data(), sizeof(FSEntry));
    return map(*reinterpret_cast<FSEntry *>(ptr.data()));
}

EntryInfo VmcFS::Private::map(const FSEntry & _fs_entry) const
{
    EntryInfo info;
    info.is_directory = _fs_entry.mode & EM_DIRECTORY;
    info.cluster = _fs_entry.cluster;
    info.name = QString::fromLatin1(_fs_entry.name, std::min(sizeof(FSEntry::name), std::strlen(_fs_entry.name)));
    info.length = _fs_entry.length;
    return info;
}

void VmcFS::Private::enumerateEntries(const EntryInfo & _dir, std::function<bool(const EntryInfo &)> _callback)
{
    const size_t entry_count_per_cluster = mp_info->cluster_size / sizeof(FSEntry);
    QList<uint32_t> clusters = getEntryClusters(_dir);
    QScopedArrayPointer<FSEntry> entries(new FSEntry[entry_count_per_cluster]);
    uint32_t read_count = 0;
    for(uint32_t cluster : clusters)
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

QList<uint32_t> VmcFS::Private::getEntryClusters(const EntryInfo & _entry) const
{
    if(_entry.cluster > mp_info->max_allocatable_clusters)
        throwNotFormatted();
    QList<uint32_t> result;
    for(uint32_t cluster = _entry.cluster;;) // TODO: check overflow, cycles, etc.
    {
        result.append(cluster);
        FATEntry fat_entry = mp_fat[cluster];
        if(fat_entry.flag == 0xff) // TODO: const, TODO: other flags (EOC, bad sector)
            break;
        cluster = fat_entry.cluster;
    }
    return result;
}

QSharedPointer<VmcFile> VmcFS::Private::openFile(const QString & _path)
{
    std::optional<EntryInfo> entry = resolvePath(_path);
    if(!entry.has_value())
        throw VmcFSException(QObject::tr("File not found"));
    if(entry->is_directory)
        throw VmcFSException(QObject::tr("\"%1\" is not a file").arg(_path));
    VmcFile::Private * pr = new VmcFile::Private;
    pr->clusters = getEntryClusters(*entry);
    pr->fs = this;
    pr->position = 0;
    pr->size = entry->length;
    pr->name = entry->name;
    return QSharedPointer<VmcFile>(new VmcFile(pr));
}

int64_t VmcFS::Private::readFile(VmcFile::Private & _file, char * _buffer, uint32_t _max_size)
{
    const uint32_t start_cluster_index = _file.position / mp_info->cluster_size;
    if(_file.position >= _file.size || start_cluster_index >= _file.clusters.size())
        return -1;
    uint32_t position_in_cluster = _file.position % mp_info->cluster_size;
    uint32_t position_in_file = _file.position;
    uint32_t position_in_buffer = 0;
    QScopedArrayPointer<char> cluster_buff(new char[mp_info->cluster_size]);
    for(int cluster_index = start_cluster_index; cluster_index < _file.clusters.size(); ++cluster_index)
    {
        const uint32_t available_to_read = std::min(
            mp_info->cluster_size - position_in_cluster,
            _file.size - position_in_file
        );
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

VmcFile::VmcFile(Private * _private) :
    mp_private(_private)
{
}

VmcFile::~VmcFile()
{
    delete mp_private;
}

uint32_t VmcFile::size() const
{
    return mp_private->size;
}

const QString & VmcFile::name() const
{
    return mp_private->name;
}

bool VmcFile::seek(uint32_t _pos)
{
    if(_pos == mp_private->position)
        return true;
    if(_pos >= mp_private->size)
        return false;
    mp_private->position = _pos;
    return true;
}

int64_t VmcFile::read(char * _buffer, int64_t _max_size)
{
    return mp_private->fs->readFile(*this->mp_private, _buffer, _max_size);
}

const char VmcFS::path_separator = '/';

VmcFS::VmcFS() { }

VmcFS::~VmcFS()
{
    delete mp_private;
}

const VmcInfo * VmcFS::info() const
{
    return mp_private->info();
}

QString VmcFS::concatPaths(const QString & _base, const QString &  _part)
{
    QStringList parts = _base.split(path_separator, QString::SkipEmptyParts);
    parts.append(_part.split(path_separator, QString::SkipEmptyParts));
    QString path(path_separator);
    path.append(parts.join(path_separator));
    if(_part.endsWith(path_separator))
        path.append(path_separator);
    return path;
}

QSharedPointer<VmcFS> VmcFS::load(const QString & _filepath)
{
    VmcFS::Private * fs_private = new VmcFS::Private(_filepath);
    try
    {
        fs_private->load();
    }
    catch(...)
    {
        delete fs_private;
        throw;
    }
    VmcFS * fs = new VmcFS();
    fs->mp_private = fs_private;
    return QSharedPointer<VmcFS>(fs);
}

void VmcFS::create(const QString & _filepath, uint8_t _size_mib)
{
    VmcFormatter::format(_filepath, _size_mib);
}

QList<VmcEntryInfo> VmcFS::enumerateEntries(const QString & _path)
{
    return mp_private->enumerateEntries(_path);
}

QSharedPointer<VmcFile> VmcFS::openFile(const QString & _path)
{
    return mp_private->openFile(_path);
}
