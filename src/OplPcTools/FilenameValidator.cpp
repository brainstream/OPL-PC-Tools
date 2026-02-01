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

#include <OplPcTools/FilenameValidator.h>

using namespace OplPcTools;

FilenameValidator::FilenameValidator(const QString & _forbidden_characters, QObject * _parent /*= nullptr*/) :
    QValidator(_parent),
    m_forbidden_characters(_forbidden_characters),
    m_max_length(0)
{
}

QValidator::State FilenameValidator::validate(QString & _input, int & _pos) const
{
    Q_UNUSED(_pos);
    uint32_t size = static_cast<uint32_t>(_input.size());
    if(size == 0)
        return QValidator::Intermediate;
    if(m_max_length && size > m_max_length)
        return QValidator::Invalid;
    for(uint32_t i = 0; i < size; ++i)
    {
        if(m_forbidden_characters.contains(_input[i]))
            _input[i] = '_';
    }
    return QValidator::Acceptable;
}
