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

#include <OplPcTools/MemoryCard/DateTime.h>

namespace OplPcTools {
namespace MemoryCard {

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

struct __attribute__((packed)) FSEntry
{                           // OFFSET:  (DEC)  (HEX)
    uint16_t mode;          //          0      0x0
    uint16_t __unused;      //          2      0x2
    uint32_t length;        //          4      0x4
    DateTime created;       //          8      0x8
    uint32_t cluster;       //          16     0x10
    uint32_t dir_entry;     //          20     0x14
    DateTime modified;      //          24     0x18
    uint32_t attr;          //          32     0x20
    uint32_t __unused2[7];  //          36     0x24
    char name[32];          //          64     0x40
    uint8_t __unused3[416]; //          96     0x60
};                          // TOTAL:   512    0x200

} // namespace MemoryCard
} // namespace OplPcTools
