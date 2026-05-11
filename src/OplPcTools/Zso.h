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

#include <QByteArray>

namespace OplPcTools {

constexpr quint32 g_zso_magic = 0x4F53495A;
const quint8 g_zso_version_1 = 1;

struct __attribute__((packed)) ZsoHeader
{
    quint32 magic;       // 0
    quint32 header_size; // 4
    quint64 total_bytes; // 8
    quint32 block_size;  // 16
    quint8 version;      // 20
    quint8 align;        // 21
    quint8 reserved[2];  // 22
};

int compressZsoBlock(const QByteArray & _source, QByteArray & _destination);
bool decompressZsoBlock(const QByteArray & _source, QByteArray & _destination);

} // namespace OplPcTools

