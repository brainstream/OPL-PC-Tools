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

#ifndef __OPLPCTOOLS_EXCEPTION__
#define __OPLPCTOOLS_EXCEPTION__

#include <QObject>
#include <QException>
#include <QMetaType>

namespace OplPcTools {

class Exception : public QException
{
public:
    explicit Exception(const QString & _message) :
        m_message(_message)
    {
    }

    const char * what() const noexcept override
    {
        return nullptr;
    }

    const QString & message() const noexcept
    {
        return m_message;
    }

private:
    QString m_message;
};

#define DECLARE_EXCEPTION(type_name)                      \
    class type_name : public OplPcTools::Exception        \
    {                                                     \
    public:                                               \
        explicit type_name(const QString & _message) :    \
            Exception(_message)                           \
        {                                                 \
        }                                                 \
    };

DECLARE_EXCEPTION(ValidationException)

DECLARE_EXCEPTION(IOException)

} // namespace OplPcTools

#endif // __OPLPCTOOLS_EXCEPTION__



