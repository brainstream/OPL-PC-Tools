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

#ifndef __OPLPCTOOLS_SETTINGS__
#define __OPLPCTOOLS_SETTINGS__

#include <QString>
#include <QSettings>
#include <OplPcTools/GameInstallationType.h>

namespace OplPcTools {

class Settings final
{
    Q_DISABLE_COPY(Settings)

public:
    enum class Flag
    {
        ReopenLastSession,
        ConfirmGameDeletion,
        ConfirmPixmapDeletion,
        SplitUpIso,
        MoveIso,
        RenameIso,
        CheckNewVersion,
        ValidateUlCfg
    };

public:
    static Settings & instance();
    inline bool flag(Flag _flag) const;
    void setFlag(Flag _flag, bool _value);

private:
    Settings();
    void loadFlag(const QSettings & _settings, Flag _flag, bool _default_value);

private:
    QMap<Flag, bool> m_flags;
};

bool Settings::flag(Flag _flag) const
{
    return m_flags[_flag];
}

} // namespace OplPcTools

#endif // __OPLPCTOOLS_SETTINGS__
