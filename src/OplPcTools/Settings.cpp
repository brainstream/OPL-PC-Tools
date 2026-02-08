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

#include <OplPcTools/Settings.h>
#include <OplPcTools/StringConverter.h>
#include <QUrl>

using namespace OplPcTools;

namespace {
namespace Key {

const char reopen_last_session[] = "Settings/ReopenLastSession";
const char confirm_game_deletion[] = "Settings/ConfirmGameDeletion";
const char confirm_pixmap_deletion[] = "Settings/ConfirmPixmapDeletion";
const char confirm_vmc_deletion[] = "Settings/ConfirmVmcDeletion";
const char split_up_iso[] = "Settings/SplitUpISO";
const char move_iso[] = "Settings/MoveISO";
const char rename_iso[] = "Settings/RenameISO";
const char check_new_version[] = "Settings/CheckNewVersion";
const char validate_ul_cfg[] = "Settings/ValidateUlCfg";
const char icon_size[] = "Settings/IconSize";
const char vmc_fs_charset[] = "Settings/VmcCharset";

const char ul_dir[] = "ULDirectory";
const char game_cover_dir[] = "PixmapDirectory";
const char game_import_dir[] = "ImportDirectory";
const char iso_source_dir[] = "ISODirectory";
const char iso_recover_dir[] = "ISORecoverDirectory";
const char vmc_export_dir[] = "VmcExportDir";

const char wnd_geometry[] = "WindowGeometry";

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

void Settings::setPath(Directory _directory, const QString & _path)
{
    switch (_directory)
    {
    case Directory::UL:
        mp_settings->setValue(Key::ul_dir, _path);
        return;
    case Directory::GameCover:
        mp_settings->setValue(Key::game_cover_dir, _path);
        return;
    case Directory::GameImport:
        mp_settings->setValue(Key::game_import_dir, _path);
        return;
    case Directory::IsoSource:
        mp_settings->setValue(Key::iso_source_dir, _path);
        return;
    case Directory::IsoRecover:
        mp_settings->setValue(Key::iso_recover_dir, _path);
        return;
    case Directory::VmcExport:
        mp_settings->setValue(Key::vmc_export_dir, _path);
        return;
    }
}

const QString Settings::path(Directory _directory)
{
    switch (_directory)
    {
    case Directory::UL:
        return mp_settings->value(Key::ul_dir).toString();
    case Directory::GameCover:
        return mp_settings->value(Key::game_cover_dir).toString();
    case Directory::GameImport:
        return mp_settings->value(Key::game_import_dir).toString();
    case Directory::IsoSource:
        return mp_settings->value(Key::iso_source_dir).toString();
    case Directory::IsoRecover:
        return mp_settings->value(Key::iso_recover_dir).toString();
    case Directory::VmcExport:
        return mp_settings->value(Key::vmc_export_dir).toString();
    default:
        return QString();
    }
}

void Settings::setWindowGeometry(const QByteArray & _geometry)
{
    mp_settings->setValue(Key::wnd_geometry, _geometry);
}

QByteArray Settings::windowGeometry() const
{
    return mp_settings->value(Key::wnd_geometry, false).toByteArray();
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

void Settings::setDefaultVmcFsCharset(const QString & _encoding)
{
    mp_settings->setValue(Key::vmc_fs_charset, _encoding);
}

QString Settings::defaultVmcFsCharset() const
{
    QString encoding = mp_settings->value(Key::vmc_fs_charset).toString();
    if(encoding.isEmpty())
        encoding = TextEncoding::latin1();
    return encoding;
}

Settings & Settings::instance()
{
    static Settings * settings = new Settings();
    return *settings;
}
