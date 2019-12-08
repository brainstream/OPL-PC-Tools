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

#ifndef __OPLPCTOOLS_ISO9660DEVICESOURCE__
#define __OPLPCTOOLS_ISO9660DEVICESOURCE__

#include <QFileInfo>
#include <OplPcTools/OpticalDriveDeviceSource.h>

namespace OplPcTools {

class Iso9660DeviceSource : public OpticalDriveDeviceSource
{
public:
    explicit Iso9660DeviceSource(const QString & _filepath) :
        OpticalDriveDeviceSource(_filepath)
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

} // namespace OplPcTools

#endif // __OPLPCTOOLS_ISO9660DEVICESOURCE__
