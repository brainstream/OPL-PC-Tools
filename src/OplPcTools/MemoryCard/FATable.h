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

#include <OplPcTools/MemoryCard/FATEntry.h>
#include <OplPcTools/MemoryCard/MCFSException.h>
#include <QList>

namespace OplPcTools {
namespace MemoryCard {

class FATable final
{
    Q_DISABLE_COPY_MOVE(FATable)

private:
    struct Table
    {
        QList<FATEntry> fat;
        uint32_t cluster;
    };

public:
    FATable();
    void append(uint32_t _cluster, const QList<FATEntry> & _fat);
    void reset();
    uint32_t count() const;
    uint32_t allocatedCount() const;
    const FATEntry & operator [] (uint32_t _index) const;
    void setEntry(uint32_t _index, FATEntry _entry);
    std::optional<QList<uint32_t>> findFreeClusters(uint32_t _count) const;
    uint32_t fatCluster(uint32_t _index) const;

private:
    template<typename TableListT>
    static auto findFatEntry(
        TableListT & _tables,
        uint32_t _entry_index,
        uint32_t * _out_entry_index_in_table) -> decltype(&_tables[0]);

private:
    QList<Table> m_tables;
    uint32_t m_cluster_count;
    uint32_t m_allocated_cluster_count;
};

inline FATable::FATable() :
    m_cluster_count(0),
    m_allocated_cluster_count(0)
{
}

inline void FATable::reset()
{
    m_tables.clear();
    m_cluster_count = 0;
    m_allocated_cluster_count = 0;
}

inline uint32_t FATable::count() const
{
    return m_cluster_count;
}

inline uint32_t FATable::allocatedCount() const
{
    return m_allocated_cluster_count;
}

inline const FATEntry & FATable::operator [] (uint32_t _index) const
{
    uint32_t index;
    const Table * table = findFatEntry(m_tables, _index, &index);
    return table->fat[index];
}

inline uint32_t FATable::fatCluster(uint32_t _index) const
{
    uint32_t idx;
    const Table * tbl = findFatEntry(m_tables, _index, &idx);
    return tbl->cluster;
}

template<typename TableListT>
auto FATable::findFatEntry(
    TableListT & _tables,
    uint32_t _entry_index,
    uint32_t * _out_entry_index_in_table) -> decltype(&_tables[0])
{
    qsizetype result_idx = _entry_index;
    for(qsizetype i = 0; i < _tables.count(); ++i)
    {
        auto & table = _tables[i];
        if(result_idx < table.fat.count())
        {
            if(result_idx >= table.fat.count())
                break; // Out of range
            *_out_entry_index_in_table = result_idx;
            return &table;
        }
        result_idx -= table.fat.count();
    }
    throw MemoryCardFileSystemException(QObject::tr("FAT cluster index %1 is out of range").arg(_entry_index));
}

} // namespace MemoryCard
} // namespace OplPcTools
