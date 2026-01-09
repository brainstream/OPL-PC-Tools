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

#pragma once

#include <QDateTime>

namespace OplPcTools {
namespace MCFS {

struct __attribute__((packed)) FSDateTime
{
    uint8_t __reserved;
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;

    static FSDateTime now()
    {
        const int japan_timezone_offset_seconds = 9 * 60 * 60;
        const QDateTime japan_now = QDateTime::currentDateTime().toOffsetFromUtc(japan_timezone_offset_seconds);
        return FSDateTime
        {
            .__reserved = 0,
            .sec = static_cast<uint8_t>(japan_now.time().second()),
            .min = static_cast<uint8_t>(japan_now.time().minute()),
            .hour = static_cast<uint8_t>(japan_now.time().hour()),
            .day = static_cast<uint8_t>(japan_now.date().day()),
            .month = static_cast<uint8_t>(japan_now.date().month()),
            .year = static_cast<uint16_t>(japan_now.date().year())
        };
    }
};

} // namespace MCFS
} // namespace OplPcTools
