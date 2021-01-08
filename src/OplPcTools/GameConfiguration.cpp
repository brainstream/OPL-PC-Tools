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

#include <QSet>
#include <QDir>
#include <QFile>
#include <OplPcTools/Maybe.h>
#include <OplPcTools/GameConfiguration.h>
#include <OplPcTools/File.h>

using namespace OplPcTools;

namespace {

enum GameMode : quint8
{
    GM_Mode1 = 0b00000001,
    GM_Mode2 = 0b00000010,
    GM_Mode3 = 0b00000100,
    GM_Mode4 = 0b00001000,
    GM_Mode5 = 0b00010000,
    GM_Mode6 = 0b00100000,
    GM_Mode7 = 0b01000000,
    GM_Mode8 = 0b10000000
};

namespace Key {

const QString compatibility("$Compatibility");
const QString enable_gsm("$EnableGSM");
const QString gsm_v_mode("$GSMVMode");
const QString gsm_x_offset("$GSMXOffset");
const QString gsm_y_offset("$GSMYOffset");
const QString gsm_skip_videos("$GSMSkipVideos");
const QString gsm_emulate_field_flipping("$GSMFIELDFix");
const QString gsm_source("$GSMSource");
const QString game_id("$DNAS");
const QString custom_elf("$AltStartup");
const QString vmc_0("$VMC_0");
const QString vmc_1("$VMC_1");
const QString config_source("$ConfigSource");

} // namespace Key

void parse(const QStringRef & _key, const QStringRef & _value, GameConfiguration & _config)
{
    if(Key::compatibility.compare(_key) == 0)
    {
        quint16 modes = _value.toUShort();
        _config.is_mode_1_enabled = modes & GM_Mode1;
        _config.is_mode_2_enabled = modes & GM_Mode2;
        _config.is_mode_3_enabled = modes & GM_Mode3;
        _config.is_mode_4_enabled = modes & GM_Mode4;
        _config.is_mode_5_enabled = modes & GM_Mode5;
        _config.is_mode_6_enabled = modes & GM_Mode6;
        _config.is_mode_7_enabled = modes & GM_Mode7;
        _config.is_mode_8_enabled = modes & GM_Mode8;
    }
    else if(Key::enable_gsm.compare(_key) == 0)
        _config.is_gsm_enabled = _value.toInt() == 1;
    else if(Key::gsm_source.compare(_key) == 0)
        _config.is_global_gsm_enabled = _value.toInt() == 0 ? true : false;
    else if(Key::gsm_v_mode.compare(_key) == 0)
        _config.gsm_video_mode = _value.toInt();
    else if(Key::gsm_x_offset.compare(_key) == 0)
        _config.gsm_x_offset = _value.toInt();
    else if(Key::gsm_y_offset.compare(_key) == 0)
        _config.gsm_y_offset = _value.toInt();
    else if(Key::gsm_skip_videos.compare(_key) == 0)
        _config.is_gsm_skip_fmv_enabled = _value.toInt() == 1;
    else if(Key::gsm_emulate_field_flipping.compare(_key) == 0)
        _config.is_gsm_emulate_field_flipping_enabled = _value.toInt() == 1;
    else if(Key::game_id.compare(_key) == 0)
        _config.game_id = _value.toString();
    else if(Key::custom_elf.compare(_key) == 0)
        _config.custom_elf = _value.toString();
    else if(Key::vmc_0.compare(_key) == 0)
        _config.vmc0 = _value.toString();
    else if(Key::vmc_1.compare(_key) == 0)
        _config.vmc1 = _value.toString();
}

bool write(QFile & _file, const GameConfiguration & _config, const QString & _key)
{
    QString value;
    if(Key::compatibility.compare(_key) == 0)
    {
        quint8 mode = 0;
        if(_config.is_mode_1_enabled) mode |= GM_Mode1;
        if(_config.is_mode_2_enabled) mode |= GM_Mode2;
        if(_config.is_mode_3_enabled) mode |= GM_Mode3;
        if(_config.is_mode_4_enabled) mode |= GM_Mode4;
        if(_config.is_mode_5_enabled) mode |= GM_Mode5;
        if(_config.is_mode_6_enabled) mode |= GM_Mode6;
        if(_config.is_mode_7_enabled) mode |= GM_Mode7;
        if(_config.is_mode_8_enabled) mode |= GM_Mode8;
        value = QString::number(mode);
    }
    else if(Key::enable_gsm.compare(_key) == 0)
        value = _config.is_gsm_enabled ? "1" : "0";
    else if(Key::gsm_source.compare(_key) == 0)
        value = _config.is_global_gsm_enabled ? "0" : "1";
    else if(Key::gsm_v_mode.compare(_key) == 0)
        value = QString::number(static_cast<int>(_config.gsm_video_mode));
    else if(Key::gsm_x_offset.compare(_key) == 0)
        value = QString::number(_config.gsm_x_offset);
    else if(Key::gsm_y_offset.compare(_key) == 0)
        value = QString::number(_config.gsm_y_offset);
    else if(Key::gsm_skip_videos.compare(_key) == 0)
        value = _config.is_gsm_skip_fmv_enabled ? "1" : "0";
    else if(Key::gsm_emulate_field_flipping.compare(_key) == 0)
        value = _config.is_gsm_emulate_field_flipping_enabled ? "1" : "0";
    else if(Key::game_id.compare(_key) == 0)
        value = _config.game_id;
    else if(Key::custom_elf.compare(_key) == 0)
        value = _config.custom_elf;
    else if(Key::vmc_0.compare(_key) == 0)
        value = _config.vmc0;
    else if(Key::vmc_1.compare(_key) == 0)
        value = _config.vmc1;
    else if(Key::config_source.compare(_key) == 0)
        value = "1";
    else
        return false;
    _file.write(QString("%1=%2\n").arg(_key).arg(value).toLatin1());
    return true;
}

Maybe<QPair<QStringRef, QStringRef>> readKeyValue(const QString & _line)
{
    int eq_index = _line.indexOf('=');
    if(eq_index > 0)
        return maybe(qMakePair(_line.leftRef(eq_index), _line.rightRef(_line.length() - eq_index - 1).trimmed()));
    return Maybe<QPair<QStringRef, QStringRef>>();
}

} // namespace

GameConfiguration::GameConfiguration() :
    is_mode_1_enabled(false),
    is_mode_2_enabled(false),
    is_mode_3_enabled(false),
    is_mode_4_enabled(false),
    is_mode_5_enabled(false),
    is_mode_6_enabled(false),
    is_mode_7_enabled(false),
    is_mode_8_enabled(false),
    is_gsm_enabled(false),
    gsm_x_offset(0),
    gsm_y_offset(0),
    gsm_video_mode(-1),
    is_gsm_skip_fmv_enabled(false),
    is_global_gsm_enabled(false),
    is_gsm_emulate_field_flipping_enabled(false)
{
}

QString GameConfiguration::makeFilename(const QString & _library_path, const QString & _game_id)
{
    QDir dir((_library_path.endsWith(QDir::separator()) ? _library_path : _library_path + QDir::separator()) + "CFG");
    return dir.absoluteFilePath(_game_id + ".cfg");
}

QSharedPointer<GameConfiguration> GameConfiguration::load(const QString & _filename)
{
    QSharedPointer<GameConfiguration> config(new GameConfiguration());
    QFile file(_filename);
    if(file.exists())
    {
        openFile(file, QFile::ReadOnly | QFile::Text);
        for(;;)
        {
            QByteArray bytes = file.readLine();
            if(bytes.isNull())
                break;
            auto kv = readKeyValue(QString::fromLatin1(bytes));
            if(kv.hasValue())
                parse(kv->first, kv->second, *config);
        }
    }
    return config;
}



void GameConfiguration::save(const GameConfiguration & _config, const QString & _filename)
{
    QFile src_file(_filename);
    QFile tmp_file(_filename + ".tmp");
    src_file.open(QFile::ReadOnly | QFile::Text);
    openFile(tmp_file, QFile::WriteOnly | QFile::Truncate | QFile::Text);
    QSet<QString> written_keys;
    for(;;)
    {
        const QByteArray bytes = src_file.readLine();
        if(bytes.isNull())
            break;
        auto kv = readKeyValue(QString::fromLatin1(bytes));
        if(kv.hasValue())
        {
            const QString key = kv->first.toString();
            if(write(tmp_file, _config, key))
            {
                written_keys.insert(key);
                continue;
            }
        }
        tmp_file.write(bytes);
    }
    src_file.close();
    auto write_if_not_written = [&](const QString & key) {
        if(!written_keys.contains(key))
            write(tmp_file, _config, key);
    };
    write_if_not_written(Key::compatibility);
    write_if_not_written(Key::enable_gsm);
    write_if_not_written(Key::gsm_source);
    write_if_not_written(Key::gsm_v_mode);
    write_if_not_written(Key::gsm_x_offset);
    write_if_not_written(Key::gsm_y_offset);
    write_if_not_written(Key::gsm_skip_videos);
    write_if_not_written(Key::gsm_emulate_field_flipping);
    write_if_not_written(Key::game_id);
    write_if_not_written(Key::custom_elf);
    write_if_not_written(Key::vmc_0);
    write_if_not_written(Key::vmc_1);
    write_if_not_written(Key::config_source);
    tmp_file.close();
    src_file.remove();
    tmp_file.rename(src_file.fileName());
}



QVector<VideoMode> GameConfiguration::supportedVideoModes(GameConfigurationVersion _version)
{
    switch(_version)
    {
    case GameConfigurationVersion::OPLv093:
        return QVector<VideoMode> {
            VideoMode::NTSC,
            VideoMode::NTSC_Non_Interlaced,
            VideoMode::PAL,
            VideoMode::PAL_Non_Interlaced,
            VideoMode::PAL_60Hz,
            VideoMode::PAL_60Hz_Non_Interlaced,
            VideoMode::PS1_NTSC_HDTV_480p_60Hz,
            VideoMode::PS1_PAL_HDTV_576p_50Hz,
            VideoMode::HDTV_480p_60Hz,
            VideoMode::HDTV_576p_50Hz,
            VideoMode::HDTV_720p_60Hz,
            VideoMode::HDTV_1080i_60Hz,
            VideoMode::HDTV_1080i_60Hz_Non_Interlaced,
            VideoMode::HDTV_1080p_60Hz,
            VideoMode::VGA_640x480p_60Hz,
            VideoMode::VGA_640x480p_72Hz,
            VideoMode::VGA_640x480p_75Hz,
            VideoMode::VGA_640x480p_85Hz,
            VideoMode::VGA_640x480i_60Hz
        };
    case GameConfigurationVersion::OPLv100:
        return QVector<VideoMode> {
            VideoMode::NTSC,
            VideoMode::NTSC_Non_Interlaced,
            VideoMode::PAL,
            VideoMode::PAL_Non_Interlaced,
            VideoMode::PAL_60Hz,
            VideoMode::PAL_60Hz_Non_Interlaced,
            VideoMode::PS1_NTSC_HDTV_480p_60Hz,
            VideoMode::PS1_PAL_HDTV_576p_50Hz,
            VideoMode::HDTV_480p_60Hz,
            VideoMode::HDTV_576p_50Hz,
            VideoMode::HDTV_720p_60Hz,
            VideoMode::HDTV_1080i_60Hz,
            VideoMode::HDTV_1080i_60Hz_Non_Interlaced,
            VideoMode::VGA_640x480p_60Hz,
            VideoMode::VGA_640x480p_72Hz,
            VideoMode::VGA_640x480p_75Hz,
            VideoMode::VGA_640x480p_85Hz,
            VideoMode::VGA_640x960i_60Hz,
            VideoMode::VGA_800x600p_56Hz,
            VideoMode::VGA_800x600p_60Hz,
            VideoMode::VGA_800x600p_72Hz,
            VideoMode::VGA_800x600p_75Hz,
            VideoMode::VGA_800x600p_85Hz,
            VideoMode::VGA_1024x768p_60Hz,
            VideoMode::VGA_1024x768p_70Hz,
            VideoMode::VGA_1024x768p_75Hz,
            VideoMode::VGA_1024x768p_85Hz,
            VideoMode::VGA_1280x1024p_60Hz,
            VideoMode::VGA_1280x1024p_75Hz
        };
    default:
        return QVector<VideoMode>();
    }
}

