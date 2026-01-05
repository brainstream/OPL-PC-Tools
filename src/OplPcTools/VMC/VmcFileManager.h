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

#include <OplPcTools/VMC/VmcDriver.h>

namespace OplPcTools {

class VmcFileManager final
{
    Q_DISABLE_COPY_MOVE(VmcFileManager)

private:
    struct FSTree;
    struct FSTreeNode;

private:
    explicit VmcFileManager(QSharedPointer<VmcDriver> && _fs);
    static VmcFileManager::FSTree * loadTree(VmcDriver & _fs);
    static uint32_t loadDirectory(VmcDriver & _fs, const VmcPath & _path, FSTreeNode & _node);

public:
    ~VmcFileManager();
    static QSharedPointer<VmcFileManager> load(const QString & _filepath);
    QList<VmcEntryInfo> enumerateEntries(const VmcPath & _path) const;

private:
    QSharedPointer<VmcDriver> m_driver_ptr;
    FSTree * mp_tree;
};

} // namespace OplPcTools
