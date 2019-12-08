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

#ifndef __OPLPCTOOLS_BIGENDIAN__
#define __OPLPCTOOLS_BIGENDIAN__

#include <QtGlobal>

namespace OplPcTools {

template<typename IntT>
struct BigEndian
{
    quint8 data[sizeof(IntT)];

    IntT toIntLE() const
    {
        IntT value = 0;
        for(short i = 0; i < sizeof(IntT); ++i)
            value = (value << 8) | data[i];
        return value;
    }
} __attribute__((packed));

} // namespace OplPcTools

#endif // __OPLPCTOOLS_BIGENDIAN__
