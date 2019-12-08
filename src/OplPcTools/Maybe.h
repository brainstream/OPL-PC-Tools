/***********************************************************************************************
 * Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                   *
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

#ifndef __OPLPCTOOLS_MAYBE__
#define __OPLPCTOOLS_MAYBE__

#include <type_traits>

namespace OplPcTools {

template<typename T>
class Maybe final
{
public:
    using ValueType = typename std::enable_if<std::is_copy_constructible<T>::value, T>::type;

    Maybe() :
        m_has_value(false),
        mp_value(nullptr)
    {
    }

    Maybe(std::nullptr_t) :
        Maybe()
    {
    }

    Maybe(const ValueType & _value) :
        m_has_value(true),
        mp_value(new ValueType(_value))
    {
    }

    Maybe(const Maybe & _maybe) :
        m_has_value(_maybe.hasValue()),
        mp_value(nullptr)
    {
        if(m_has_value)
            mp_value = new ValueType(*_maybe.mp_value);
    }

    Maybe(Maybe && _maybe) :
        m_has_value(_maybe.m_has_value),
        mp_value(_maybe.mp_value)
    {
        _maybe.mp_value = nullptr;
        _maybe.m_has_value = false;
    }

    ~Maybe()
    {
        delete mp_value;
    }

    Maybe & operator = (const Maybe & _maybe)
    {
        if(this != &_maybe)
        {
            m_has_value = _maybe.m_has_value;
            mp_value = m_has_value ? new ValueType(*_maybe.mp_value) : nullptr;
        }
        return *this;
    }

    Maybe & operator = (Maybe && _maybe)
    {
        if(this != &_maybe)
        {
            m_has_value = _maybe.m_has_value;
            mp_value = _maybe.mp_value;
            _maybe.mp_value = nullptr;
            _maybe.m_has_value = false;
        }
        return *this;
    }

    Maybe & operator = (const ValueType & _value)
    {
        delete mp_value;
        mp_value = new ValueType(_value);
        m_has_value = true;
        return *this;
    }

    const ValueType * operator -> () const
    {
        return *mp_value;
    }

    ValueType * operator -> ()
    {
        return mp_value;
    }

    const ValueType & operator * () const
    {
        return mp_value;
    }

    ValueType & operator * ()
    {
        return *mp_value;
    }

    const ValueType & value() const
    {
        return *mp_value;
    }

    ValueType & value()
    {
        return *mp_value;
    }

    void reset()
    {
        delete mp_value;
        mp_value = nullptr;
        m_has_value = false;
    }

    bool hasValue() const
    {
        return m_has_value;
    }

private:
    bool m_has_value;
    ValueType * mp_value;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_MAYBE__
