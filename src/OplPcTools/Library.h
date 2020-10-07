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

#ifndef __OPLPCTOOLS_LIBRARY__
#define __OPLPCTOOLS_LIBRARY__

#include <QObject>
#include <OplPcTools/GameManager.h>
#include <OplPcTools/GameArtManager.h>
#include <OplPcTools/VmcManager.h>

namespace OplPcTools {

class Library final : public QObject
{
    Q_OBJECT

public:
    explicit Library(QObject * _parent = nullptr);
    inline const QString & directory() const;
    void load(const QDir & _directory);
    inline bool isLoaded() const;
    inline GameManager & games() const;
    inline VmcManager & vmcs() const;

signals:
    void loading();
    void loaded();

private:
    static Library * sp_instance;
    QString m_directory;
    GameManager * mp_games;
    VmcManager * mp_vmcs;
};

const QString & Library::directory() const
{
    return m_directory;
}

inline bool Library::isLoaded() const
{
    return !m_directory.isEmpty();
}

GameManager & Library::games() const
{
    return *mp_games;
}

VmcManager & Library::vmcs() const
{
    return *mp_vmcs;
}

} // namespace OplPcTools

#endif // __OPLPCTOOLS_LIBRARY__
