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

#include <cstdint>

namespace OplPcTools {
namespace MemoryCard {

struct __attribute__((packed)) Superblock final
{                                      // OFFSET:  (DEC)  (HEX)
    char magic[28];                    //          0      0x0
    char version[12];                  //          28     0x1C
    int16_t pagesize;                  //          40     0x28
    uint16_t pages_per_cluster;        //          42     0x2A
    uint16_t pages_per_block;          //          44     0x2C
    uint16_t __unused;                 //          46     0x2E
    uint32_t clusters_per_card;        //          48     0x30
    uint32_t alloc_offset;             //          52     0x34
    uint32_t alloc_end;                //          56     0x38
    uint32_t rootdir_cluster;          //          60     0x3C
    uint32_t backup_block1;            //          64     0x40
    uint32_t backup_block2;            //          68     0x44
    uint8_t __unused2[8];              //          72     0x48
    uint32_t ifc_ptr_list[32];         //          80     0x50
    uint32_t bad_block_list[32];       //          208    0xD0
    uint8_t cardtype;                  //          336    0x150
    uint8_t cardflags;                 //          337    0x151
    uint16_t __unused3;                //          338    0x152
    uint32_t cluster_size;             //          340    0x154
    uint32_t fat_entries_per_cluster;  //          344    0x158
    uint32_t clusters_per_block;       //          348    0x15C
    int32_t cardform;                  //          352    0x160
    uint32_t rootdir_cluster2;         //          356    0x164
    uint32_t __unknown1;               //          360    0x168
    uint32_t __unknown2;               //          364    0x16C
    uint32_t max_allocatable_clusters; //          368    0x170
    uint32_t __unknown3;               //          372    0x174
    uint32_t __unknown4;               //          376    0x178
    int32_t __unknown5;                //          380    0x17C
};                                     // TOTAL:   384    0x180

constexpr char g_mcfs_magic[] = "Sony PS2 Memory Card Format ";
constexpr char g_mcfs_version[] = "1.2.0.0";

} // namespace MemoryCard
} // namespace OplPcTools
