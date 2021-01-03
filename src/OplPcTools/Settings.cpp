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

#include <OplPcTools/Settings.h>

using namespace OplPcTools;

namespace {
namespace Key {

const QString reopen_last_session("Settings/ReopenLastSession");
const QString confirm_game_deletion("Settings/ConfirmGameDeletion");
const QString confirm_pixmap_deletion("Settings/ConfirmPixmapDeletion");
const QString confirm_vmc_deletion("Settings/ConfirmVmcDeletion");
const QString split_up_iso("Settings/SplitUpISO");
const QString move_iso("Settings/MoveISO");
const QString rename_iso("Settings/RenameISO");
const QString check_new_version("Settings/CheckNewVersion");
const QString validate_ul_cfg("Settings/ValidateUlCfg");
const QString icon_size("Settings/IconSize");
const QString config_version("Settings/ConfigVersion");

} // namespace Key
} // namespace


Settings::Settings()
{
    mp_settings = new QSettings(this);
}

void Settings::flush()
{
    mp_settings->sync();
}

bool Settings::reopenLastSession() const
{
    return mp_settings->value(Key::reopen_last_session, false).toBool();
}

void Settings::setReopenLastSession(bool _value)
{
    mp_settings->setValue(Key::reopen_last_session, _value);
}

bool Settings::confirmGameDeletion() const
{
    return mp_settings->value(Key::confirm_game_deletion, true).toBool();
}

void Settings::setConfirmGameDeletion(bool _value)
{
    mp_settings->setValue(Key::confirm_game_deletion, _value);
}

bool Settings::confirmPixmapDeletion() const
{
    return mp_settings->value(Key::confirm_pixmap_deletion, true).toBool();
}

void Settings::setConfirmPixmapDeletion(bool _value)
{
    mp_settings->setValue(Key::confirm_pixmap_deletion, _value);
}

bool Settings::confirmVmcDeletion() const
{
    return mp_settings->value(Key::confirm_vmc_deletion, true).toBool();
}

void Settings::setConfirmVmcDeletion(bool _value)
{
    mp_settings->setValue(Key::confirm_vmc_deletion, _value);
}

bool Settings::splitUpIso() const
{
    return mp_settings->value(Key::split_up_iso, true).toBool();
}

void Settings::setSplitUpIso(bool _value)
{
    mp_settings->setValue(Key::split_up_iso, _value);
}

bool Settings::moveIso() const
{
    return mp_settings->value(Key::move_iso, false).toBool();
}

void Settings::setMoveIso(bool _value)
{
    mp_settings->setValue(Key::move_iso, _value);
}

bool Settings::renameIso() const
{
    return mp_settings->value(Key::rename_iso, true).toBool();
}

void Settings::setRenameIso(bool _value)
{
    mp_settings->setValue(Key::rename_iso, _value);
}

bool Settings::checkNewVersion() const
{
    return mp_settings->value(Key::check_new_version, true).toBool();
}

void Settings::setCheckNewVersion(bool _value)
{
    mp_settings->setValue(Key::check_new_version, _value);
}

bool Settings::validateUlCfg() const
{
    return mp_settings->value(Key::validate_ul_cfg, true).toBool();
}

void Settings::setValidateUlCfg(bool _value)
{
    mp_settings->setValue(Key::validate_ul_cfg, _value);
}

quint32 Settings::iconSize() const
{
    return mp_settings->value(Key::icon_size, 40).toUInt();
}

void Settings::setIconSize(quint32 _size)
{
    if(iconSize() != _size)
    {
        mp_settings->setValue(Key::icon_size, _size);
        emit iconSizeChanged();
    }
}

int Settings::configVersion() const
{
    return mp_settings->value(Key::config_version, 0).toInt();
}

void Settings::setConfigVersion(int _version)
{
    mp_settings->setValue(Key::config_version, _version);
}

Settings & Settings::instance()
{
    static Settings * settings = new Settings();
    return *settings;
}
