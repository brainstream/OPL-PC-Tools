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

#include <OplPcTools/VMC/VmcFileManager.h>

using namespace OplPcTools;

struct VmcFileManager::FSTreeNode : VmcEntryInfo
{
    QList<QSharedPointer<FSTreeNode>> children;
};

struct VmcFileManager::FSTree : VmcFileManager::FSTreeNode
{
    uint32_t used_size;
};

QSharedPointer<VmcFileManager> VmcFileManager::load(const QString & _filepath)
{
    QSharedPointer<VmcDriver> driver = VmcDriver::load(_filepath);
    if(!driver) return nullptr;
    return QSharedPointer<VmcFileManager>(new VmcFileManager(std::move(driver)));
}

VmcFileManager::VmcFileManager(QSharedPointer<VmcDriver> && _fs) :
    m_driver_ptr(std::move(_fs)),
    mp_tree(loadTree(*m_driver_ptr))
{
}

VmcFileManager::~VmcFileManager()
{
    delete mp_tree;
}

VmcFileManager::FSTree* VmcFileManager::loadTree(VmcDriver & _fs)
{
    FSTree * root = new FSTree;
    root->name = QByteArray("/");
    root->size = 0;
    root->is_directory = true;
    root->children = QList<QSharedPointer<FSTreeNode>>();
    root->used_size = loadDirectory(_fs, VmcPath::root(), *root);
    return root;
}

uint32_t VmcFileManager::loadDirectory(VmcDriver & _fs, const VmcPath & _path, FSTreeNode & _node)
{
    uint32_t total_size = 0;
    foreach(const VmcEntryInfo & entry, _fs.enumerateEntries(_path))
    {
        FSTreeNode * child_node = new FSTreeNode;
        child_node->name = entry.name;
        child_node->size = entry.size;
        child_node->is_directory = entry.is_directory;
        child_node->children = QList<QSharedPointer<FSTreeNode>>();
        _node.children.append(QSharedPointer<FSTreeNode>(child_node));
        if(entry.is_directory)
        {
            total_size += loadDirectory(_fs, VmcPath(_path, entry.name), *child_node);
        }
        else
        {
            total_size += entry.size;
        }
    }
    return total_size;
}

QList<VmcEntryInfo> VmcFileManager::enumerateEntries(const VmcPath & _path) const
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
            throw VmcFSException(QObject::tr("Path not found"));
    }
    QList<VmcEntryInfo> result;
    result.reserve(dir->size);
    foreach(const QSharedPointer<FSTreeNode> & entry, dir->children)
        result.append(*entry);
    return result;
}
