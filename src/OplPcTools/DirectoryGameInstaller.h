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

#ifndef __OPLPCTOOLS_DIRECTORYGAMEINSTALLER__
#define __OPLPCTOOLS_DIRECTORYGAMEINSTALLER__

#include <OplPcTools/GameInstaller.h>

namespace OplPcTools {

class DirectoryGameInstaller : public GameInstaller
{
    Q_OBJECT

public:
    DirectoryGameInstaller(Device & _device, GameCollection & _collection, QObject * _parent = nullptr);
    ~DirectoryGameInstaller() override;
    inline quint8 options() const;
    inline void setOptionMoveFile(bool _value);
    inline bool isOptionMoveFileSet() const;
    inline void setOptionRenameFile(bool _value);
    inline bool isOptionRenameFileSet() const;
    bool install() override;
    inline const Game * installedGame() const override;

private:
    bool copyDeviceTo(const QString & _dest);
    void rollback(const QString & _dest);
    void registerGame();

private:
    bool m_move_file;
    bool m_rename_file;
    Game * mp_game;
};

void DirectoryGameInstaller::setOptionMoveFile(bool _value)
{
    m_move_file = _value;
}

bool DirectoryGameInstaller::isOptionMoveFileSet() const
{
    return m_move_file;
}

void DirectoryGameInstaller::setOptionRenameFile(bool _value)
{
    m_rename_file = _value;
}

bool DirectoryGameInstaller::isOptionRenameFileSet() const
{
    return m_rename_file;
}

const Game * DirectoryGameInstaller::installedGame() const
{
    return mp_game;
}

} // namespace OplPcTools

#endif // __OPLPCTOOLS_DIRECTORYGAMEINSTALLER__
