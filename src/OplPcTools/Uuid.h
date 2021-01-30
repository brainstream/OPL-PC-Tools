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

#ifndef __OPLPCTOOLS_UUID__
#define __OPLPCTOOLS_UUID__

#include <QUuid>

namespace OplPcTools {

/// This class is similar to QUuid, but without implicit type conversion.
class Uuid final
{
public:
    inline Uuid();
    inline explicit Uuid(const QUuid & _quuid);
    Uuid(const Uuid & _uuid) = default;
    ~Uuid() = default;
    Uuid & operator = (const Uuid &) = default;
    inline Uuid & operator = (const QUuid & _quuid);
    inline bool operator == (const QUuid & _quuid) const;
    inline bool operator == (const Uuid & _uuid) const;
    inline bool operator != (const QUuid & _quuid) const;
    inline bool operator != (const Uuid & _uuid) const;
    inline const QUuid & quuid() const;

    inline static Uuid createUuid();

private:
    QUuid m_quuid;
};

Uuid::Uuid()
{
}

Uuid::Uuid(const QUuid & _quuid) :
    m_quuid(_quuid)
{
}

Uuid & Uuid::operator = (const QUuid & _quuid)
{
    m_quuid = _quuid;
    return *this;
}

bool Uuid::operator == (const QUuid & _quuid) const
{
    return m_quuid == _quuid;
}

bool Uuid::operator == (const Uuid & _uuid) const
{
    return m_quuid == _uuid.m_quuid;
}

bool Uuid::operator != (const QUuid & _quuid) const
{
    return m_quuid != _quuid;
}

bool Uuid::operator != (const Uuid & _uuid) const
{
    return m_quuid != _uuid.m_quuid;
}

const QUuid & Uuid::quuid() const
{
    return m_quuid;
}

Uuid Uuid::createUuid()
{
    return Uuid(QUuid::createUuid());
}

} // namespace OplPcTools

#endif // __OPLPCTOOLS_UUID__
