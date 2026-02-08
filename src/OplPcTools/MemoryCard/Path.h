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

#include <QStringList>

namespace OplPcTools {
namespace MemoryCard {

class Path
{
public:
    Path()
    {
    }

    Path(const QByteArray & _path) :
        Path(split(_path))
    {
    }

    Path(const QByteArray & _base_path, const QByteArray _relative_path) :
        Path(split(_base_path.trimmed()) + split(_relative_path.trimmed()))
    {
    }

    explicit Path(const QList<QByteArray> & _parts) :
        m_parts(_parts)
    {
    }

    Path(const Path &) = default;

    Path & operator = (const Path &) = default;

    Path operator + (const QByteArray & _relative_path) const
    {
        return Path(m_parts + split(_relative_path.trimmed()));
    }

    operator QByteArray () const
    {
        return path();
    }

    const QByteArray path() const
    {
        return QByteArray(1, s_path_separator) + m_parts.join(s_path_separator);
    }

    const QList<QByteArray> parts() const
    {
        return m_parts;
    }

    static Path root()
    {
        return Path();
    }

    bool isRoot() const
    {
        return m_parts.empty();
    }

    Path up() const
    {
        return Path(m_parts.mid(0, m_parts.size() - 1));
    }

    QByteArray filename() const
    {
        return m_parts.empty() ? QByteArray() : m_parts.last();
    }

private:
    QList<QByteArray> split(const QByteArray & _path) const
    {
        auto parts = _path.split(s_path_separator);
        // parts.removeAll(QByteArray()); // Cannot be used, Qt 5.14+
        for(qsizetype i = parts.count() - 1; i >=0; --i)
            if(parts.at(i).isEmpty()) parts.removeAt(i);
        return parts;
    }

private:
    static const char s_path_separator = '/';
    QList<QByteArray> m_parts;
};

} // namespace MemoryCard
} // namespace OplPcTools
