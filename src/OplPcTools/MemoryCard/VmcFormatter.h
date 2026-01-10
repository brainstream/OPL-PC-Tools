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

#include <OplPcTools/MemoryCard/Superblock.h>
#include <QFile>

namespace OplPcTools {
namespace MemoryCard {

class VmcFormatter final
{
    Q_DISABLE_COPY_MOVE(VmcFormatter)

public:
    static void format(const QString & _filename, uint32_t _size_mib);

private:
    VmcFormatter(const QString & _filename, uint32_t _size_mib);
    ~VmcFormatter();
    void format();
    void clearFile();
    void initSuperblock();
    void writeSuperblock();
    void writeFAT();
    void writeRootDirectory();

private:
    Superblock * mp_sb;
    QFile m_file;
    uint32_t m_size_mib;
};

} // namespace MemoryCard
} // namespace OplPcTools
