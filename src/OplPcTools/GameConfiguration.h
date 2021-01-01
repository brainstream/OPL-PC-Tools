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

enum class GameVideoMode
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
    VGA_640x480p_60Hz,
    VGA_640x480p_72Hz,
    VGA_640x480p_75Hz,
    VGA_640x480p_85Hz,
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

class GameConfiguration final
{
private:
    explicit GameConfiguration(const QString & _filename);

public:
    static QSharedPointer<GameConfiguration> load(const QString & _filename);
    void save();

    inline const QString & gameId() const;
    inline bool isMode1Enalbed() const;
    inline bool isMode2Enalbed() const;
    inline bool isMode3Enalbed() const;
    inline bool isMode4Enalbed() const;
    inline bool isMode5Enalbed() const;
    inline bool isMode6Enalbed() const;
    inline bool isMode7Enalbed() const;
    inline bool isMode8Enalbed() const;
    inline const QString & customElf() const;
    inline const QString & vmc0() const;
    inline const QString & vmc1() const;
    inline bool isGsmEnabled() const;
    inline int gsmXOffset() const;
    inline int gsmYOffset() const;
    inline GameVideoMode gsmVideoMode() const;
    inline bool gsmSkipFmv() const;

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
    GameVideoMode m_gsm_video_mode;
    bool m_gsm_skip_fmv;
};

const QString & GameConfiguration::gameId() const
{
    return m_game_id;
}

bool GameConfiguration::isMode1Enalbed() const
{
    return m_is_mode_1_enabled;
}

bool GameConfiguration::isMode2Enalbed() const
{
    return m_is_mode_2_enabled;
}

bool GameConfiguration::isMode3Enalbed() const
{
    return m_is_mode_3_enabled;
}

bool GameConfiguration::isMode4Enalbed() const
{
    return m_is_mode_4_enabled;
}

bool GameConfiguration::isMode5Enalbed() const
{
    return m_is_mode_5_enabled;
}

bool GameConfiguration::isMode6Enalbed() const
{
    return m_is_mode_6_enabled;
}

bool GameConfiguration::isMode7Enalbed() const
{
    return m_is_mode_7_enabled;
}

bool GameConfiguration::isMode8Enalbed() const
{
    return m_is_mode_8_enabled;
}

const QString & GameConfiguration::customElf() const
{
    return m_custom_elf;
}

const QString & GameConfiguration::vmc0() const
{
    return m_vmc0;
}

const QString & GameConfiguration::vmc1() const
{
    return m_vmc1;
}

bool GameConfiguration::isGsmEnabled() const
{
    return m_is_gsm_enabled;
}

int GameConfiguration::gsmXOffset() const
{
    return m_gsm_x_offset;
}

int GameConfiguration::gsmYOffset() const
{
    return m_gsm_y_offset;
}

GameVideoMode GameConfiguration::gsmVideoMode() const
{
    return m_gsm_video_mode;
}

bool GameConfiguration::gsmSkipFmv() const
{
    return m_gsm_skip_fmv;
}

} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMECONFIGURATION__
