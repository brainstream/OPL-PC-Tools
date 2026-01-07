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

#include <OplPcTools/VmcFileManager.h>
#include <OplPcTools/MCFS/VmcFormatter.h>

#define INVALID_CLUSTER_PTR static_cast<uint32_t>(-1)
#define NULL_CLUSTER_PTR 0

using namespace OplPcTools;
using namespace OplPcTools::MCFS;

struct FSTreeNode
{
    QByteArray name;
    size_t size;
    bool is_directory;
    QList<FSTreeNode *> children;
};

struct FSTree
{
    FSTreeNode * root;
};

struct VmcFileManager::FSTreeNode : VmcFsEntryInfo
{
    QList<QSharedPointer<FSTreeNode>> children;
};

struct VmcFileManager::FSTree : VmcFileManager::FSTreeNode
{
    uint32_t used_bytes;
};

VmcFileManager::VmcFileManager(FileSystemDriver * _private) :
    mp_driver(_private),
    mp_tree(loadTree())
{
}

VmcFileManager::~VmcFileManager()
{
    delete mp_driver;
    delete mp_tree;
}

VmcFileManager::FSTree * VmcFileManager::loadTree()
{
    FSTree * root = new FSTree;
    root->name = QByteArray("/");
    root->size = 0;
    root->is_directory = true;
    root->children = QList<QSharedPointer<FSTreeNode>>();
    root->used_bytes = loadDirectory(VmcPath::root(), *root);
    return root;
}

uint32_t VmcFileManager::loadDirectory(const VmcPath & _path, FSTreeNode & _node)
{
    uint32_t total_size = 0;
    foreach(const VmcFsEntryInfo & entry, mp_driver->enumerateEntries(_path))
    {
        FSTreeNode * child_node = new FSTreeNode;
        child_node->name = entry.name;
        child_node->size = entry.size;
        child_node->is_directory = entry.is_directory;
        child_node->children = QList<QSharedPointer<FSTreeNode>>();
        _node.children.append(QSharedPointer<FSTreeNode>(child_node));
        if(entry.is_directory)
        {
            total_size += loadDirectory(VmcPath(_path, entry.name), *child_node);
        }
        else
        {
            total_size += entry.size;
        }
    }
    return total_size;
}

const VmcFsInfo * VmcFileManager::fileSystemInfo() const
{
    return mp_driver->info();
}

QSharedPointer<VmcFileManager> VmcFileManager::load(const QString & _filepath)
{
    FileSystemDriver * driver = new FileSystemDriver(_filepath);
    try
    {
        driver->load();
    }
    catch(...)
    {
        delete driver;
        throw;
    }
    return QSharedPointer<VmcFileManager>(new VmcFileManager(driver));
}

void VmcFileManager::createVmc(const QString & _filepath, uint32_t _size_mib)
{
    VmcFormatter::format(_filepath, _size_mib);
}

QList<VmcFsEntryInfo> VmcFileManager::enumerateEntries(const VmcPath & _path) const
{
    const FSTreeNode * dir = mp_tree;
    foreach(const QByteArray & part, _path.parts())
    {
        bool is_found = false;
        foreach(const QSharedPointer<FSTreeNode> & child, dir->children)
        {
            if(child->name == part)
            {
                is_found = child->is_directory;
                dir = child.data();
                break;
            }
        }
        if(!is_found)
        {
            throw MCFSException(QObject::tr("Path not found"));
        }
    }
    QList<VmcFsEntryInfo> result;
    result.reserve(dir->size);
    foreach(const QSharedPointer<FSTreeNode> & entry, dir->children)
        result.append(*entry);
    return result;
}

QSharedPointer<VmcFile> VmcFileManager::openFile(const VmcPath & _path)
{
    return mp_driver->openFile(_path);
}

void VmcFileManager::writeFile(const VmcPath & _path, const QByteArray & _data)
{
    mp_driver->writeFile(_path, _data);
}

uint32_t VmcFileManager::totalUsedBytes() const
{
    return mp_tree->used_bytes;
}

uint32_t VmcFileManager::totalFreeBytes() const
{
    const VmcFsInfo * info = mp_driver->info();
    const uint32_t allocable_space = (info->alloc_end - info->alloc_offset) * info->cluster_size;
    return allocable_space - totalUsedBytes();
}
