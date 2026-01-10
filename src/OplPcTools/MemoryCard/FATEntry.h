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

enum FATEntryFlag : uint8_t
{
    FAT_FREE = 0x7F,
    FAT_EOF = 0xFF,
    FAT_POINTER = 0x80
};

struct __attribute__((packed)) FATEntry
{
    uint32_t cluster: 24;
    FATEntryFlag flag: 8;

    static constexpr FATEntry free()
    {
        return { .cluster = 0xFFFFFF, .flag = FAT_FREE };
    }

    static constexpr FATEntry endOfFile()
    {
        return { .cluster = 0xFFFFFF, .flag = FAT_EOF };
    }

    static constexpr FATEntry pointer(uint32_t _cluster)
    {
        return { .cluster = _cluster, .flag = FAT_POINTER };
    }

    bool isFree() const
    {
        return flag == FAT_FREE;
    }

    bool isEndOfFile() const
    {
        return flag == FAT_EOF;
    }

    bool isPointer() const
    {
        return flag == FAT_POINTER;
    }
};

} // namespace MemoryCard
} // namespace OplPcTools
