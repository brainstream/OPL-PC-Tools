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

#include <OplPcTools/FilenameValidator.h>

using namespace OplPcTools;

const QString FilenameValidator::s_disallowed_characters("<>:\"/\\|?*");

FilenameValidator::FilenameValidator(QObject * _parent /*= nullptr*/) :
    QValidator(_parent)
{
}

QValidator::State FilenameValidator::validate(QString & _input, int & _pos) const
{
    Q_UNUSED(_pos);
    int size = _input.size();
    for(int i = 0; i < size; ++i)
    {
        if(s_disallowed_characters.contains(_input[i]))
            _input[i] = '_';
    }
    return QValidator::Acceptable;
}
