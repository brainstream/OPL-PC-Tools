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

#ifndef __OPLPCTOOLS_SETTINGS__
#define __OPLPCTOOLS_SETTINGS__

#include <QString>
#include <QSettings>

class Settings final
{
private:
    Settings();
    Settings(const Settings &) = delete;
    Settings & operator = (const Settings &) = delete;

public:
    static Settings & instance();

    bool reopenLastSestion() const
    {
        return m_reopen_last_session;
    }

    void setReopenLastSestion(bool _value)
    {
        m_settins.setValue(s_key_reopen_last_session, _value);
        m_reopen_last_session = _value;
    }

    bool confirmGameDeletion() const
    {
        return m_confirm_game_deletion;
    }

    void setConfirmGameDeletion(bool _value)
    {
        m_settins.setValue(s_key_confirm_game_deletion, _value);
        m_confirm_game_deletion = _value;
    }

    bool confirmPixmapDeletion() const
    {
        return m_confirm_pixmap_deletion;
    }

    void setConfirmPixmapDeletion(bool _value)
    {
        m_settins.setValue(s_key_confirm_pixmap_deletion, _value);
        m_confirm_pixmap_deletion = _value;
    }

private:
    bool loadBoolean(const QString & _key, bool _default_value);

private:
    static const QString s_key_reopen_last_session;
    static const QString s_key_confirm_game_deletion;
    static const QString s_key_confirm_pixmap_deletion;
    QSettings m_settins;
    bool m_reopen_last_session;
    bool m_confirm_game_deletion;
    bool m_confirm_pixmap_deletion;
};

#endif // __OPLPCTOOLS_SETTINGS__
