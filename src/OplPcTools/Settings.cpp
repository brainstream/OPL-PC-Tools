/***********************************************************************************************
 * Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                   *
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

const char * flagToKey(Settings::Flag _flag)
{
    switch (_flag) {
    case Settings::Flag::ReopenLastSession:
        return "Settings/ReopenLastSession";
    case Settings::Flag::ConfirmGameDeletion:
        return "Settings/ConfirmGameDeletion";
    case Settings::Flag::ConfirmPixmapDeletion:
        return "Settings/ConfirmPixmapDeletion";
    case Settings::Flag::SplitUpIso:
        return "Settings/SplitUpISO";
    case Settings::Flag::MoveIso:
        return "Settings/MoveISO";
    case Settings::Flag::RenameIso:
        return "Settings/RenameISO";
    case Settings::Flag::CheckNewVersion:
        return "Settings/CheckNewVersion";
    case Settings::Flag::ValidateUlCfg:
        return "Settings/ValidateUlCfg";
    default:
        return nullptr;
    }
}

} // namespace


Settings::Settings()
{
    QSettings settings;
    loadFlag(settings, Flag::ReopenLastSession, false);
    loadFlag(settings, Flag::ConfirmGameDeletion, true);
    loadFlag(settings, Flag::ConfirmPixmapDeletion, true);
    loadFlag(settings, Flag::SplitUpIso, true);
    loadFlag(settings, Flag::MoveIso, false);
    loadFlag(settings, Flag::RenameIso, true);
    loadFlag(settings, Flag::CheckNewVersion, true);
    loadFlag(settings, Flag::ValidateUlCfg, true);
}

void Settings::loadFlag(const QSettings & _settings, Flag _flag, bool _default_value)
{
    bool value = _default_value;
    QVariant var = _settings.value(flagToKey(_flag));
    if(var.canConvert(QVariant::Bool))
       value = var.toBool();
    m_flags[_flag] = value;
}

void Settings::setFlag(Flag _flag, bool _value)
{
    QSettings settings;
    settings.setValue(flagToKey(_flag), _value);
    m_flags[_flag] = _value;
}

Settings & Settings::instance()
{
    static Settings * settings = new Settings();
    return *settings;
}
