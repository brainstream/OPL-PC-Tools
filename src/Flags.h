/***********************************************************************************************
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

#ifndef __OPLPCTOOLS_FLAGS__
#define __OPLPCTOOLS_FLAGS__

#include <QMap>

class Flag final
{
public:
    Flag() :
        m_value(false)
    {
    }

    Flag(bool _value) :
        m_value(_value)
    {
    }

    operator bool () const
    {
        return m_value;
    }

    Flag & operator = (const Flag & _flag)
    {
        if(this != &_flag)
            m_value = _flag.m_value;
        return *this;
    }

    Flag & operator = (bool _value)
    {
        m_value = _value;
        return *this;
    }

    bool operator == (bool _value) const
    {
        return _value == m_value;
    }

    bool operator != (const bool _value) const
    {
        return _value != m_value;
    }

private:
    bool m_value;
};

template<typename Key>
class Flags final
{
public:
    bool operator [](const Key & _key) const
    {
        return m_map[_key];
    }

    Flag & operator [](const Key & _key)
    {
        return m_map[_key];
    }

private:
    QMap<Key, Flag> m_map;
};

#endif // __OPLPCTOOLS_FLAGS__
