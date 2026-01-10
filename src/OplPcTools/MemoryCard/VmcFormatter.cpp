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

#include <OplPcTools/MemoryCard/VmcFormatter.h>
#include <OplPcTools/MemoryCard/FATEntry.h>
#include <OplPcTools/MemoryCard/FSEntry.h>
#include <OplPcTools/File.h>

using namespace OplPcTools::MemoryCard;

namespace {

constexpr uint32_t g_min_size_mib = 8;
constexpr uint32_t g_max_size_mib = 512;

} // namespace

VmcFormatter::VmcFormatter(const QString & _filename, uint32_t _size_mib) :
    mp_sb(new Superblock { }),
    m_file(_filename),
    m_size_mib(_size_mib)
{
}

VmcFormatter::~VmcFormatter()
{
    delete mp_sb;
}

void VmcFormatter::format(const QString & _filename, uint32_t _size_mib)
{
    if(_size_mib < g_min_size_mib || _size_mib > g_max_size_mib)
    {
        throw ValidationException(
            QObject::tr("VMC size must be greater than or equal to %1 Mib and less than or equal to %2 Mib")
                .arg(g_min_size_mib)
                .arg(g_max_size_mib)
            );
    }
    VmcFormatter(_filename, _size_mib).format();
}

void VmcFormatter::format()
{
    openFile(m_file, QIODevice::WriteOnly | QIODevice::Truncate);
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
    for(uint32_t i = 0; i < m_size_mib; ++i)
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
    mp_sb->__unknown5 = -1;
    std::memcpy(mp_sb->magic, g_mcfs_magic, sizeof(mp_sb->magic));
    std::strncpy(mp_sb->version, g_mcfs_version, sizeof(mp_sb->version));
    std::memset(mp_sb->bad_block_list, -1, sizeof(Superblock::bad_block_list));
    const uint32_t available_cluster_count = mp_sb->clusters_per_card - mp_sb->clusters_per_block * 3; // superblock and 2 backups
    const uint32_t fat_list_count = static_cast<uint32_t>(std::ceil(
        static_cast<double>(available_cluster_count) / (pointers_per_cluster + 1 + double(1.0) / pointers_per_cluster)));
    const uint32_t ifc_list_count = static_cast<uint32_t>(
        std::ceil(static_cast<double>(fat_list_count) / pointers_per_cluster));
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
    m_file.write(buffer, sizeof(Superblock));
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
    std::unique_ptr<char[]> fat_cluster_raw(new char[mp_sb->cluster_size]);
    FATEntry fat_entry = FATEntry::free();
    for(uint32_t i = 0; i < mp_sb->fat_entries_per_cluster; ++i)
    {
        memcpy(&fat_cluster_raw.get()[sizeof(FATEntry) * i], &fat_entry, sizeof(FATEntry));
    }
    m_file.seek(fat_start_cluster_index * mp_sb->cluster_size);
    for(uint32_t i = 0; i < fat_entry_count / mp_sb->fat_entries_per_cluster; ++i)
    {
        m_file.write(fat_cluster_raw.get(), mp_sb->cluster_size);
    }
    if(fat_entry_count % mp_sb->fat_entries_per_cluster)
    {
        m_file.write(fat_cluster_raw.get(), (fat_entry_count % mp_sb->fat_entries_per_cluster) * sizeof(FATEntry));
    }
    fat_cluster_raw.reset();
    // Root directory
    //
    writeRootDirectory();
    fat_entry = FATEntry::endOfFile();
    m_file.seek(fat_start_cluster_index * mp_sb->cluster_size);
    m_file.write(reinterpret_cast<char *>(&fat_entry), sizeof(FATEntry));
}

void VmcFormatter::writeRootDirectory()
{
    const DateTime now = DateTime::now();
    FSEntry entries[2];
    std::memset(entries, 0, sizeof(entries));
    for(int i = 0; i < 2; ++i)
    {
        entries[i].attr = 0;
        entries[i].mode = EM_READ | EM_WRITE | EM_EXECUTE | EM_DIRECTORY | EM_EXISTS;
        entries[i].cluster = mp_sb->rootdir_cluster;
        entries[i].__unused = 0;
        entries[i].length = 2;
        entries[i].created = now;
        entries[i].dir_entry = 0;
        entries[i].modified = now;
        entries[i].name[0] = '.';
    }
    entries[0].name[1] = '\0';
    entries[1].name[1] = '.';
    entries[1].name[2] = '\0';
    m_file.seek((mp_sb->alloc_offset + mp_sb->rootdir_cluster) * mp_sb->cluster_size);
    m_file.write(reinterpret_cast<const char *>(entries), sizeof(entries));
}
