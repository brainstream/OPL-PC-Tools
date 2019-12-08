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

#ifndef __OPLPCTOOLS_FILE__
#define __OPLPCTOOLS_FILE__

#include <QFile>
#include <OplPcTools/Exception.h>

namespace OplPcTools {

inline void openFile(QFile & _file, QIODevice::OpenMode _flags)
{
    if(!_file.open(_flags))
        throw IOException(QObject::tr("Unable to open file \"%1\"").arg(_file.fileName()));
}

inline void renameFile(const QString & _old_filename, const QString & _new_filename)
{
    if(!QFile::rename(_old_filename, _new_filename))
        throw IOException(QObject::tr("Unable to rename file \"%1\" to \"%2\"").arg(_old_filename).arg(_new_filename));
}

} // namespace OplPcTools

#endif // __OPLPCTOOLS_FILE__
