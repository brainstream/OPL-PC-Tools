/***********************************************************************************************
 * Copyright © 2017-2020 Sergey Smolyannikov aka brainstream                                   *
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

#include <QSettings>

namespace OplPcTools {

class Settings  final : public QObject
{
    Q_OBJECT

public:
    static Settings & instance();

public:
    void flush();
    bool reopenLastSession() const;
    void setReopenLastSession(bool _value);
    bool confirmGameDeletion() const;
    void setConfirmGameDeletion(bool _value);
    bool confirmPixmapDeletion() const;
    void setConfirmPixmapDeletion(bool _value);
    bool splitUpIso() const;
    void setSplitUpIso(bool _value);
    bool moveIso() const;
    void setMoveIso(bool _value);
    bool renameIso() const;
    void setRenameIso(bool _value);
    bool checkNewVersion() const;
    void setCheckNewVersion(bool _value);
    bool validateUlCfg() const;
    void setValidateUlCfg(bool _value);
    QString iconTheme() const;
    void setIconTheme(const QString & _theme);
    quint32 iconSize() const;
    void setIconSize(quint32 _size);

signals:
    void iconSizeChanged();

private:
    Settings();
    bool loadFlag(const QString & _key, bool _default_value);

private:
    QSettings * mp_settings;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_SETTINGS__
