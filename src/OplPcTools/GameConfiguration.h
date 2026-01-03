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

#ifndef __OPLPCTOOLS_GAMECONFIGURATION__
#define __OPLPCTOOLS_GAMECONFIGURATION__

#include <QSharedPointer>
#include <QVector>
#include <OplPcTools/VideoMode.h>

namespace OplPcTools {

enum class GameConfigurationVersion: int
{
    OPLv093 = 0,
    OPLv100 = 1,

    Latest = OPLv100
};

struct GameConfiguration
{
    static QString makeFilename(const QString & _library_path, const QString & _game_id);
    static QSharedPointer<GameConfiguration> load(const QString & _filename);
    static void save(const GameConfiguration & _config, const QString & _filename);
    static QVector<VideoMode> supportedVideoModes(GameConfigurationVersion _version);

    GameConfiguration();
    bool is_mode_1_enabled;
    bool is_mode_2_enabled;
    bool is_mode_3_enabled;
    bool is_mode_4_enabled;
    bool is_mode_5_enabled;
    bool is_mode_6_enabled;
    bool is_mode_7_enabled;
    bool is_mode_8_enabled;
    QString game_id;
    QString custom_elf;
    QString vmc0;
    QString vmc1;
    bool is_gsm_enabled;
    int gsm_x_offset;
    int gsm_y_offset;
    int gsm_video_mode;
    bool is_gsm_skip_fmv_enabled;
    bool is_gsm_emulate_field_flipping_enabled;
    bool is_global_gsm_enabled;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMECONFIGURATION__
