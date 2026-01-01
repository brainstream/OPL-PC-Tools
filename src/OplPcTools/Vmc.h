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

#ifndef __OPLPCTOOLS_VMC__
#define __OPLPCTOOLS_VMC__

#include <QString>
#include <OplPcTools/Uuid.h>

namespace OplPcTools {

class Vmc final
{
public:
    inline Vmc(const QString & _filepath, const QString & _title, uint32_t _size_mib);
    Vmc(const Vmc &) = default;
    ~Vmc() = default;
    inline const Uuid & uuid() const;
    inline const QString & filepath() const;
    inline void setFilepath(const QString & _filepath);
    inline const QString & title() const;
    inline void setTitle(const QString & _title);
    inline uint32_t size() const;

private:
    const Uuid m_uuid;
    QString m_filepath;
    QString m_title;
    uint32_t m_size_mib;
};

Vmc::Vmc(const QString & _filepath, const QString & _title, uint32_t _size_mib) :
    m_uuid(Uuid::createUuid()),
    m_filepath(_filepath),
    m_title(_title),
    m_size_mib(_size_mib)
{
}

const Uuid & Vmc::uuid() const
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

uint32_t Vmc::size() const
{
    return m_size_mib;
}

} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMC__
