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

#include <OplPcTools/FilenameValidator.h>
#include <OplPcTools/File.h>

namespace OplPcTools {

bool isFilenameValid(const QString & _filename)
{
    for(const QChar & chr : _filename)
    {
        if(FilenameValidator::s_disallowed_characters.contains(chr))
            return false;
    }
    return true;
}

void validateFilename(const QString & _filename)
{
    if(!isFilenameValid(_filename))
        throw ValidationException(QString("%1: %2")
            .arg(QObject::tr("Name must not contain following symbols")
            .arg(FilenameValidator::s_disallowed_characters)
        ));
}

} // namespace OplPcTools
