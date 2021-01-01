/***********************************************************************************************
 * Copyright © 2017-2021 Sergey Smolyannikov aka brainstream                                   *
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
 * You should have received a copy of the GNU General Public License along with MailUnit.      *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#ifndef __OPLPCTOOLS_VMC__
#define __OPLPCTOOLS_VMC__

#include <QUuid>
#include <QString>

namespace OplPcTools {

enum class VmcSize
{
    _8M   = 8,
    _16M  = 16,
    _32M  = 32,
    _64M  = 64,
    _128M = 128,
    _256M = 256
};

class Vmc final
{
public:
    inline Vmc(const QString & _filepath, const QString & _title, VmcSize _size);
    Vmc(const Vmc &) = default;
    ~Vmc() = default;
    inline const QUuid & uuid() const;
    inline const QString & filepath() const;
    inline void setFilepath(const QString & _filepath);
    inline const QString & title() const;
    inline void setTitle(const QString & _title);
    inline VmcSize size() const;

private:
    QUuid m_uuid;
    QString m_filepath;
    QString m_title;
    VmcSize m_size;
};

Vmc::Vmc(const QString & _filepath, const QString & _title, VmcSize _size) :
    m_uuid(QUuid::createUuid()),
    m_filepath(_filepath),
    m_title(_title),
    m_size(_size)
{
}

const QUuid & Vmc::uuid() const
{
    return m_uuid;
}

const QString & Vmc::filepath() const
{
    return m_filepath;
}

void Vmc::setFilepath(const QString & _filepath)
{
    m_filepath = _filepath;
}

const QString & Vmc::title() const
{
    return m_title;
}

void Vmc::setTitle(const QString & _title)
{
    m_title = _title;
}

VmcSize Vmc::size() const
{
    return m_size;
}

} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMC__
