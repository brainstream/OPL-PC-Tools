/***********************************************************************************************
 * Copyright © 2017-2025 Sergey Smolyannikov aka brainstream                                   *
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

#include <OplPcTools/Settings.h>
#include <OplPcTools/TextEncoding.h>
#include <QUrl>

using namespace OplPcTools;

namespace {
namespace Key {

static const char reopen_last_session[] = "Settings/ReopenLastSession";
static const char confirm_game_deletion[] = "Settings/ConfirmGameDeletion";
static const char confirm_pixmap_deletion[] = "Settings/ConfirmPixmapDeletion";
static const char confirm_vmc_deletion[] = "Settings/ConfirmVmcDeletion";
static const char split_up_iso[] = "Settings/SplitUpISO";
static const char move_iso[] = "Settings/MoveISO";
static const char rename_iso[] = "Settings/RenameISO";
static const char check_new_version[] = "Settings/CheckNewVersion";
static const char validate_ul_cfg[] = "Settings/ValidateUlCfg";
static const char icon_size[] = "Settings/IconSize";
static const char vmc_fs_encoding[] = "Settings/VmcFsEncoding";

static const char vmc_by_path_section[] = "Vmc";
static const char vmc_by_path_fs_encoding[] = "FsEncoding";

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

void Settings::setDefaultVmcFsEncoding(const QString & _encoding)
{
    mp_settings->setValue(Key::vmc_fs_encoding, _encoding);
}

QString Settings::defaultVmcFsEncoding() const
{
    QString encoding = mp_settings->value(Key::vmc_fs_encoding).toString();
    if(encoding.isEmpty())
        encoding = TextEncoding::latin1();
    return encoding;
}

void Settings::setVmcFsEncodingForPath(const QString & _path, const QString & _encoding)
{
    QString encodedPath = QUrl::toPercentEncoding(_path);
    mp_settings->setValue(
        QString("%1_%2/%3").arg(Key::vmc_by_path_section, encodedPath, Key::vmc_by_path_fs_encoding),
        _encoding
    );
}

QString Settings::vmcFsEncodingForPath(const QString & _path) const
{
    QString encodedPath = QUrl::toPercentEncoding(_path);
    return mp_settings->value(
        QString("%1_%2/%3").arg(Key::vmc_by_path_section, encodedPath, Key::vmc_by_path_fs_encoding)
    ).toString();
}

Settings & Settings::instance()
{
    static Settings * settings = new Settings();
    return *settings;
}
