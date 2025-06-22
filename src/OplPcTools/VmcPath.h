/***********************************************************************************************
 * Copyright © 2017-2025 Sergey Smolyannikov aka brainstream                                   *
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
    Q_DISABLE_COPY_MOVE(VmcPath)

public:
    VmcPath()
    {
    }

    VmcPath(const QString & _path) :
        VmcPath(split(_path))
    {
    }

    VmcPath(const QString & _base_path, const QString _relative_path) :
        VmcPath(split(_base_path) + split(_relative_path))
    {
    }

    explicit VmcPath(const QStringList & _parts) :
        m_parts(_parts)
    {
    }

    VmcPath operator + (const QString & _relative_path) const
    {
        return VmcPath(m_parts + split(_relative_path));
    }

    operator QString () const
    {
        return path();
    }

    const QString path() const
    {
        return QString(s_path_separator) + m_parts.join(s_path_separator);
    }

    const QStringList parts() const
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
    QStringList split(const QString & _path) const
    {
        return _path.split(s_path_separator, Qt::SkipEmptyParts);
    }


private:
    static const char s_path_separator = '/';
    const QStringList m_parts;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMCPATH__
