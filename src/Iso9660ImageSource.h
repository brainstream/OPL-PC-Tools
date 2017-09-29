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

#ifndef __OPLPCTOOLS_ISO9660IMAGESOURCE__
#define __OPLPCTOOLS_ISO9660IMAGESOURCE__

#include <QFileInfo>
#include "OpticalDriveSource.h"

class Iso9660ImageSource : public OpticalDriveSource
{
public:
    explicit Iso9660ImageSource(const QString & _filepath) :
        OpticalDriveSource(_filepath)
    {
        m_is_readonly = !QFileInfo(_filepath).isWritable();
    }

    bool isReadOnly() const override
    {
        return m_is_readonly;
    }

private:
    bool m_is_readonly;
};

#endif // __OPLPCTOOLS_ISO9660IMAGESOURCE__
