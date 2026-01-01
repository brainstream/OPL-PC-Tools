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

#ifndef __OPLPCTOOLS_GAMEVIDEOMODE__
#define __OPLPCTOOLS_GAMEVIDEOMODE__

namespace OplPcTools {

enum class VideoMode: int
{
    NTSC,
    NTSC_Non_Interlaced,
    PAL,
    PAL_Non_Interlaced,
    PAL_60Hz,
    PAL_60Hz_Non_Interlaced,
    PS1_NTSC_HDTV_480p_60Hz,
    PS1_PAL_HDTV_576p_50Hz,
    HDTV_480p_60Hz,
    HDTV_576p_50Hz,
    HDTV_720p_60Hz,
    HDTV_1080i_60Hz,
    HDTV_1080i_60Hz_Non_Interlaced,
    HDTV_1080p_60Hz,
    VGA_640x480p_60Hz,
    VGA_640x480p_72Hz,
    VGA_640x480p_75Hz,
    VGA_640x480p_85Hz,
    VGA_640x480i_60Hz,
    VGA_640x960i_60Hz,
    VGA_800x600p_56Hz,
    VGA_800x600p_60Hz,
    VGA_800x600p_72Hz,
    VGA_800x600p_75Hz,
    VGA_800x600p_85Hz,
    VGA_1024x768p_60Hz,
    VGA_1024x768p_70Hz,
    VGA_1024x768p_75Hz,
    VGA_1024x768p_85Hz,
    VGA_1280x1024p_60Hz,
    VGA_1280x1024p_75Hz
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMEVIDEOMODE__
