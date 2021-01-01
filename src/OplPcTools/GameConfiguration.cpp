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
#include <OplPcTools/Maybe.h>
#include <OplPcTools/GameConfiguration.h>

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
const QString game_id("$DNAS");
const QString custom_elf("$AltStartup");
const QString vmc_0("$VMC_0");
const QString vmc_1("$VMC_1");
const QString config_source("$ConfigSource");

} // namespace Key

Maybe<QPair<QStringRef, QStringRef>> readKeyValue(const QString & _line)
{
    int eq_index = _line.indexOf('=');
    if(eq_index > 0)
        return maybe(qMakePair(_line.leftRef(eq_index), _line.rightRef(_line.length() - eq_index - 1).trimmed()));
    return Maybe<QPair<QStringRef, QStringRef>>();
}

} // namespace

GameConfiguration::GameConfiguration(const QString & _filename) :
    m_filename(_filename),
    m_is_mode_1_enabled(false),
    m_is_mode_2_enabled(false),
    m_is_mode_3_enabled(false),
    m_is_mode_4_enabled(false),
    m_is_mode_5_enabled(false),
    m_is_mode_6_enabled(false),
    m_is_mode_7_enabled(false),
    m_is_mode_8_enabled(false),
    m_is_gsm_enabled(false),
    m_gsm_x_offset(0),
    m_gsm_y_offset(0),
    m_gsm_video_mode(GameVideoMode::NTSC),
    m_gsm_skip_fmv(false)
{
}

QSharedPointer<GameConfiguration> GameConfiguration::load(const QString & _filename)
{
    GameConfiguration * config = new GameConfiguration(_filename);
    QFile file(_filename);
    file.open(QFile::ReadOnly | QFile::Text); // TODO: openFile

    for(;;)
    {
        QByteArray bytes = file.readLine();
        if(bytes.isNull())
            break;
        auto kv = readKeyValue(QString::fromLatin1(bytes));
        if(kv.hasValue())
            config->parse(kv->first, kv->second);
    }
    return QSharedPointer<GameConfiguration>(config);
}

void GameConfiguration::parse(const QStringRef & _key, const QStringRef & _value)
{
    if(Key::compatibility.compare(_key) == 0)
    {
        quint16 modes = _value.toUShort();
        m_is_mode_1_enabled = modes & GM_Mode1;
        m_is_mode_2_enabled = modes & GM_Mode2;
        m_is_mode_3_enabled = modes & GM_Mode3;
        m_is_mode_4_enabled = modes & GM_Mode4;
        m_is_mode_5_enabled = modes & GM_Mode5;
        m_is_mode_6_enabled = modes & GM_Mode6;
        m_is_mode_7_enabled = modes & GM_Mode7;
        m_is_mode_8_enabled = modes & GM_Mode8;
    }
    else if(Key::enable_gsm.compare(_key) == 0)
        m_is_gsm_enabled = _value.toInt() == 1;
    else if(Key::gsm_v_mode.compare(_key) == 0)
        m_gsm_video_mode = static_cast<GameVideoMode>(_value.toInt());
    else if(Key::gsm_x_offset.compare(_key) == 0)
        m_gsm_x_offset = _value.toInt();
    else if(Key::gsm_y_offset.compare(_key) == 0)
        m_gsm_y_offset = _value.toInt();
    else if(Key::gsm_skip_videos.compare(_key) == 0)
        m_gsm_skip_fmv = _value.toInt() == 1;
    else if(Key::game_id.compare(_key) == 0)
        m_game_id = _value.toString();
    else if(Key::custom_elf.compare(_key) == 0)
        m_custom_elf = _value.toString();
    else if(Key::vmc_0.compare(_key) == 0)
        m_vmc0 = _value.toString();
    else if(Key::vmc_1.compare(_key) == 0)
        m_vmc1 = _value.toString();
}

void GameConfiguration::save()
{
    QFile src_file(m_filename);
    QFile tmp_file(m_filename + ".tmp");
    src_file.open(QFile::ReadOnly | QFile::Text); // TODO: openFile
    tmp_file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text); // TODO: openFile
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
            if(write(tmp_file, key))
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
            write(tmp_file, key);
    };
    write_if_not_written(Key::compatibility);
    write_if_not_written(Key::enable_gsm);
    write_if_not_written(Key::gsm_v_mode);
    write_if_not_written(Key::gsm_x_offset);
    write_if_not_written(Key::gsm_y_offset);
    write_if_not_written(Key::gsm_skip_videos);
    write_if_not_written(Key::game_id);
    write_if_not_written(Key::custom_elf);
    write_if_not_written(Key::vmc_0);
    write_if_not_written(Key::vmc_1);
    write_if_not_written(Key::config_source);
    tmp_file.close();
    src_file.remove();
    tmp_file.rename(src_file.fileName());
}

bool GameConfiguration::write(QFile & _file, const QString & _key) const
{
    QString value;
    if(Key::compatibility.compare(_key) == 0)
    {
        quint8 mode = 0;
        if(m_is_mode_1_enabled) mode |= GM_Mode1;
        if(m_is_mode_2_enabled) mode |= GM_Mode2;
        if(m_is_mode_3_enabled) mode |= GM_Mode3;
        if(m_is_mode_4_enabled) mode |= GM_Mode4;
        if(m_is_mode_5_enabled) mode |= GM_Mode5;
        if(m_is_mode_6_enabled) mode |= GM_Mode6;
        if(m_is_mode_7_enabled) mode |= GM_Mode7;
        if(m_is_mode_8_enabled) mode |= GM_Mode8;
        value = QString::number(mode);
    }
    else if(Key::enable_gsm.compare(_key) == 0)
        value = m_is_gsm_enabled ? "1" : "0";
    else if(Key::gsm_v_mode.compare(_key) == 0)
        value = QString::number(static_cast<int>(m_gsm_video_mode));
    else if(Key::gsm_x_offset.compare(_key) == 0)
        value = QString::number(m_gsm_x_offset);
    else if(Key::gsm_y_offset.compare(_key) == 0)
        value = QString::number(m_gsm_y_offset);
    else if(Key::gsm_skip_videos.compare(_key) == 0)
        value = m_gsm_skip_fmv ? "1" : "0";
    else if(Key::game_id.compare(_key) == 0)
        value = m_game_id;
    else if(Key::custom_elf.compare(_key) == 0)
        value = m_custom_elf;
    else if(Key::vmc_0.compare(_key) == 0)
        value = m_vmc0;
    else if(Key::vmc_1.compare(_key) == 0)
        value = m_vmc1;
    else if(Key::config_source.compare(_key) == 0)
        value = "1";
    else
        return false;
    _file.write(QString("%1=%2\n").arg(_key).arg(value).toLatin1());
    return true;
}

