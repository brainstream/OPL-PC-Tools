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

#ifndef __OPLPCTOOLS_LIBRARY__
#define __OPLPCTOOLS_LIBRARY__

#include <QObject>
#include <OplPcTools/GameCollection.h>
#include <OplPcTools/VmcCollection.h>
#include <OplPcTools/GameArtManager.h>

namespace OplPcTools {

class Library final : public QObject
{
    Q_OBJECT

private:
    Library();

public:
    inline const QString & directory() const;
    void load(const QDir & _directory);
    inline bool isLoaded() const;
    inline GameCollection & games() const;
    inline VmcCollection & vmcs() const;

public:
    static Library & instance();

signals:
    void loading();
    void loaded();

private:
    static Library * sp_instance;
    QString m_directory;
    GameCollection * mp_games;
    VmcCollection * mp_vmcs;
};

const QString & Library::directory() const
{
    return m_directory;
}

inline bool Library::isLoaded() const
{
    return !m_directory.isEmpty();
}

GameCollection & Library::games() const
{
    return *mp_games;
}

VmcCollection & Library::vmcs() const
{
    return *mp_vmcs;
}

} // namespace OplPcTools

#endif // __OPLPCTOOLS_LIBRARY__
