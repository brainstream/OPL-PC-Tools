/***********************************************************************************************
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

#include "Settings.h"

const QString Settings::s_key_reopen_last_session("Settings/ReopenLastSession");
const QString Settings::s_key_confirm_game_deletion("Settings/ConfirmGameDeletion");
const QString Settings::s_key_confirm_pixmap_deletion("Settings/ConfirmPixmapDeletion");

Settings::Settings()
{
    m_reopen_last_session = loadBoolean(s_key_reopen_last_session, false);
    m_confirm_game_deletion = loadBoolean(s_key_confirm_game_deletion, true);
    m_confirm_pixmap_deletion = loadBoolean(s_key_confirm_pixmap_deletion, true);
}

bool Settings::loadBoolean(const QString & _key, bool _default_value)
{
    QVariant value = m_settins.value(_key);
    if(value.isNull() || !value.isValid())
        return _default_value;
    return value.toBool();
}

Settings & Settings::instance()
{
    static Settings * settings = new Settings();
    return *settings;
}
