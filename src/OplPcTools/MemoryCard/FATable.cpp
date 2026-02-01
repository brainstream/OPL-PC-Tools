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

#include <OplPcTools/MemoryCard/FATable.h>

using namespace OplPcTools::MemoryCard;

void FATable::append(uint32_t _cluster, const QList<FATEntry> & _fat)
{
    m_cluster_count += _fat.count();
    m_tables.append(Table { .fat = _fat, .cluster = _cluster });
    foreach(const FATEntry & entry, _fat)
    {
        if(!entry.isFree())
            ++m_allocated_cluster_count;
    }
}

void FATable::setEntry(uint32_t _index, FATEntry _entry)
{
    uint32_t index;
    Table * table = findFatEntry(m_tables, _index, &index);
    table->fat[index] = _entry;

    if(!_entry.isFree() && table->fat[index].isFree())
        --m_allocated_cluster_count;
    else if(_entry.isFree() && !table->fat[index].isFree())
        ++m_allocated_cluster_count;
}

std::optional<QList<uint32_t>> FATable::findFreeClusters(uint32_t _count) const
{
    if(_count == 0)
        return std::nullopt;
    QList<uint32_t> result;
    result.reserve(_count);
    uint32_t cluster = 0;
    foreach(const Table & table, m_tables)
    {
        foreach(const FATEntry & entry, table.fat)
        {
            if(entry.isFree())
            {
                result.append(cluster);
                if(result.count() == static_cast<qsizetype>(_count))
                    return result;
            }
            ++cluster;
        }
    }
    return std::nullopt;
}
