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

#include <QStringList>

#ifndef __OPLPCTOOLS_VMCPATH__
#define __OPLPCTOOLS_VMCPATH__

namespace OplPcTools {

class VmcPath
{
    Q_DISABLE_COPY(VmcPath)

public:
    VmcPath()
    {
    }

    VmcPath(const QByteArray & _path) :
        VmcPath(split(_path))
    {
    }

    VmcPath(const QByteArray & _base_path, const QByteArray _relative_path) :
        VmcPath(split(_base_path.trimmed()) + split(_relative_path.trimmed()))
    {
    }

    explicit VmcPath(const QList<QByteArray> & _parts) :
        m_parts(_parts)
    {
    }

    VmcPath operator + (const QByteArray & _relative_path) const
    {
        return VmcPath(m_parts + split(_relative_path.trimmed()));
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

    static VmcPath root()
    {
        return VmcPath();
    }

    bool isRoot() const
    {
        return m_parts.empty();
    }

    VmcPath up() const
    {
        return VmcPath(m_parts.mid(0, m_parts.size() - 1));
    }

private:
    QList<QByteArray> split(const QByteArray & _path) const
    {
        auto parts = _path.split(s_path_separator);
        // parts.removeAll(QByteArray()); // Qt 5.14+
        for(qsizetype i = parts.count() - 1; i >=0; --i)
            if(parts.at(i).isEmpty()) parts.removeAt(i);
        return parts;
    }

private:
    static const char s_path_separator = '/';
    const QList<QByteArray> m_parts;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMCPATH__
