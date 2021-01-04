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

#ifndef __OPLPCTOOLS_GAMECONFIGURATION__
#define __OPLPCTOOLS_GAMECONFIGURATION__

#include <QSharedPointer>
#include <QFile>

namespace OplPcTools {

enum class GameVideoMode: int
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

struct GameVideoModeItem
{
    int index;
    GameVideoMode mode;
};

struct GameVideoMode_v093
{
    GameVideoMode_v093();
    const GameVideoModeItem ntsc;
    const GameVideoModeItem ntsc_non_interlaced;
    const GameVideoModeItem pal;
    const GameVideoModeItem pal_non_interlaced;
    const GameVideoModeItem pal_60hz;
    const GameVideoModeItem pal_60hz_non_interlaced;
    const GameVideoModeItem ps1_ntsc_hdtv_480p_60hz;
    const GameVideoModeItem ps1_pal_hdtv_576p_50hz;
    const GameVideoModeItem hdtv_480p_60hz;
    const GameVideoModeItem hdtv_576p_50hz;
    const GameVideoModeItem hdtv_720p_60hz;
    const GameVideoModeItem hdtv_1080i_60hz;
    const GameVideoModeItem hdtv_1080i_60hz_non_interlaced;
    const GameVideoModeItem hdtv_1080p_60hz;
    const GameVideoModeItem vga_640x480p_60hz;
    const GameVideoModeItem vga_640x480p_72hz;
    const GameVideoModeItem vga_640x480p_75hz;
    const GameVideoModeItem vga_640x480p_85hz;
    const GameVideoModeItem vga_640x480i_60hz;
};

struct GameVideoMode_v100
{
    GameVideoMode_v100();
    const GameVideoModeItem ntsc;
    const GameVideoModeItem ntsc_non_interlaced;
    const GameVideoModeItem pal;
    const GameVideoModeItem pal_non_interlaced;
    const GameVideoModeItem pal_60hz;
    const GameVideoModeItem pal_60hz_non_interlaced;
    const GameVideoModeItem ps1_ntsc_hdtv_480p_60hz;
    const GameVideoModeItem ps1_pal_hdtv_576p_50hz;
    const GameVideoModeItem hdtv_480p_60hz;
    const GameVideoModeItem hdtv_576p_50hz;
    const GameVideoModeItem hdtv_720p_60hz;
    const GameVideoModeItem hdtv_1080i_60hz;
    const GameVideoModeItem hdtv_1080i_60hz_non_interlaced;
    const GameVideoModeItem vga_640x480p_60hz;
    const GameVideoModeItem vga_640x480p_72hz;
    const GameVideoModeItem vga_640x480p_75hz;
    const GameVideoModeItem vga_640x480p_85hz;
    const GameVideoModeItem vga_640x960i_60hz;
    const GameVideoModeItem vga_800x600p_56hz;
    const GameVideoModeItem vga_800x600p_60hz;
    const GameVideoModeItem vga_800x600p_72hz;
    const GameVideoModeItem vga_800x600p_75hz;
    const GameVideoModeItem vga_800x600p_85hz;
    const GameVideoModeItem vga_1024x768p_60hz;
    const GameVideoModeItem vga_1024x768p_70hz;
    const GameVideoModeItem vga_1024x768p_75hz;
    const GameVideoModeItem vga_1024x768p_85hz;
    const GameVideoModeItem vga_1280x1024p_60hz;
    const GameVideoModeItem vga_1280x1024p_75hz;
};

class GameConfiguration final
{
private:
    explicit GameConfiguration(const QString & _filename);

public:
    static QString makeFilename(const QString & _library_path, const QString & _game_id);
    static QSharedPointer<GameConfiguration> load(const QString & _filename);
    void save();

    inline const QString & gameId() const;
    inline void setGameId(const QString & _id);
    inline bool isMode1Enalbed() const;
    inline void setMode1Enabled(bool _enabled);
    inline bool isMode2Enalbed() const;
    inline void setMode2Enabled(bool _enabled);
    inline bool isMode3Enalbed() const;
    inline void setMode3Enabled(bool _enabled);
    inline bool isMode4Enalbed() const;
    inline void setMode4Enabled(bool _enabled);
    inline bool isMode5Enalbed() const;
    inline void setMode5Enabled(bool _enabled);
    inline bool isMode6Enalbed() const;
    inline void setMode6Enabled(bool _enabled);
    inline bool isMode7Enalbed() const;
    inline void setMode7Enabled(bool _enabled);
    inline bool isMode8Enalbed() const;
    inline void setMode8Enabled(bool _enabled);
    inline const QString & customElf() const;
    inline void setCustomElf(const QString & _elf);
    inline const QString & vmc0() const;
    inline void setVmc0(const QString & _vmc);
    inline const QString & vmc1() const;
    inline void setVmc1(const QString & _vmc);
    inline bool isGsmEnabled() const;
    inline void setGsmEnabled(bool _enabled);
    inline bool isGlobalGsmEnabled() const;
    inline void setGlobalGsmEnabled(bool _enabled);
    inline int gsmXOffset() const;
    inline void setXOffset(int _offset);
    inline int gsmYOffset() const;
    inline void setYOffset(int _offset);
    inline int gsmVideoMode() const;
    inline void setVideoMode(int _index);
    inline bool gsmSkipFmv() const;
    inline void setSkipFmv(bool _skip);
    inline bool isEmulationFieldFlippingEnabledd() const;
    inline void setEmulationFieldFlippingEnabledd(bool _enabled);

private:
    void parse(const QStringRef & _key, const QStringRef & _value);
    bool write(QFile & _file, const QString & _key) const;

private:
    const QString m_filename;
    bool m_is_mode_1_enabled;
    bool m_is_mode_2_enabled;
    bool m_is_mode_3_enabled;
    bool m_is_mode_4_enabled;
    bool m_is_mode_5_enabled;
    bool m_is_mode_6_enabled;
    bool m_is_mode_7_enabled;
    bool m_is_mode_8_enabled;
    QString m_game_id;
    QString m_custom_elf;
    QString m_vmc0;
    QString m_vmc1;
    bool m_is_gsm_enabled;
    int m_gsm_x_offset;
    int m_gsm_y_offset;
    int m_gsm_video_mode;
    bool m_gsm_skip_fmv;
    bool m_gsm_emulate_field_flipping;
    bool m_is_global_gsm_enabled;
};

const QString & GameConfiguration::gameId() const
{
    return m_game_id;
}

void GameConfiguration::setGameId(const QString & _id)
{
    m_game_id = _id;
}

bool GameConfiguration::isMode1Enalbed() const
{
    return m_is_mode_1_enabled;
}

void GameConfiguration::setMode1Enabled(bool _enabled)
{
    m_is_mode_1_enabled = _enabled;
}

bool GameConfiguration::isMode2Enalbed() const
{
    return m_is_mode_2_enabled;
}

void GameConfiguration::setMode2Enabled(bool _enabled)
{
    m_is_mode_2_enabled = _enabled;
}

bool GameConfiguration::isMode3Enalbed() const
{
    return m_is_mode_3_enabled;
}

void GameConfiguration::setMode3Enabled(bool _enabled)
{
    m_is_mode_3_enabled = _enabled;
}

bool GameConfiguration::isMode4Enalbed() const
{
    return m_is_mode_4_enabled;
}

void GameConfiguration::setMode4Enabled(bool _enabled)
{
    m_is_mode_4_enabled = _enabled;
}

bool GameConfiguration::isMode5Enalbed() const
{
    return m_is_mode_5_enabled;
}

void GameConfiguration::setMode5Enabled(bool _enabled)
{
    m_is_mode_5_enabled = _enabled;
}

bool GameConfiguration::isMode6Enalbed() const
{
    return m_is_mode_6_enabled;
}

void GameConfiguration::setMode6Enabled(bool _enabled)
{
    m_is_mode_6_enabled = _enabled;
}

bool GameConfiguration::isMode7Enalbed() const
{
    return m_is_mode_7_enabled;
}

void GameConfiguration::setMode7Enabled(bool _enabled)
{
    m_is_mode_7_enabled = _enabled;
}

bool GameConfiguration::isMode8Enalbed() const
{
    return m_is_mode_8_enabled;
}

void GameConfiguration::setMode8Enabled(bool _enabled)
{
    m_is_mode_8_enabled = _enabled;
}

const QString & GameConfiguration::customElf() const
{
    return m_custom_elf;
}

void GameConfiguration::setCustomElf(const QString & _elf)
{
    m_custom_elf = _elf;
}

const QString & GameConfiguration::vmc0() const
{
    return m_vmc0;
}

void GameConfiguration::setVmc0(const QString & _vmc)
{
    m_vmc0 = _vmc;
}

const QString & GameConfiguration::vmc1() const
{
    return m_vmc1;
}

void GameConfiguration::setVmc1(const QString & _vmc)
{
    m_vmc1 = _vmc;
}

bool GameConfiguration::isGsmEnabled() const
{
    return m_is_gsm_enabled;
}

void GameConfiguration::setGsmEnabled(bool _enabled)
{
    m_is_gsm_enabled = _enabled;
}

bool GameConfiguration::isGlobalGsmEnabled() const
{
    return m_is_global_gsm_enabled;
}

void GameConfiguration::setGlobalGsmEnabled(bool _enabled)
{
    m_is_global_gsm_enabled = _enabled;
}

int GameConfiguration::gsmXOffset() const
{
    return m_gsm_x_offset;
}

void GameConfiguration::setXOffset(int _offset)
{
    m_gsm_x_offset = _offset;
}

int GameConfiguration::gsmYOffset() const
{
    return m_gsm_y_offset;
}

void GameConfiguration::setYOffset(int _offset)
{
    m_gsm_y_offset = _offset;
}

int GameConfiguration::gsmVideoMode() const
{
    return m_gsm_video_mode;
}

void GameConfiguration::setVideoMode(int _index)
{
    m_gsm_video_mode = _index;
}

bool GameConfiguration::gsmSkipFmv() const
{
    return m_gsm_skip_fmv;
}

void GameConfiguration::setSkipFmv(bool _skip)
{
    m_gsm_skip_fmv = _skip;
}

bool GameConfiguration::isEmulationFieldFlippingEnabledd() const
{
    return m_gsm_emulate_field_flipping;
}

void GameConfiguration::setEmulationFieldFlippingEnabledd(bool _enabled)
{
    m_gsm_emulate_field_flipping = _enabled;
}

} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMECONFIGURATION__
