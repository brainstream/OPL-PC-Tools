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

#pragma once

#include <OplPcTools/GameInstaller.h>
#include <OplPcTools/Device/DeviceWriter.h>

namespace OplPcTools {

class DirectoryGameInstaller : public GameInstaller
{
    Q_OBJECT

public:
    DirectoryGameInstaller(
        DeviceReader & _reader,
        std::unique_ptr<DeviceWriter> && _writer,
        QObject * _parent = nullptr);
    ~DirectoryGameInstaller() override;
    inline quint8 options() const;
    inline void setOptionMoveFile(bool _value);
    inline bool isOptionMoveFileSet() const;
    inline void setOptionRenameFile(bool _value);
    inline bool isOptionRenameFileSet() const;
    inline const Game * installedGame() const override;

protected:
    bool performInstallation() override;

private:
    void rollback(const QString & _dest);
    void registerGame();

private:
    bool m_move_file;
    bool m_rename_file;
    Game * mp_game;
    std::unique_ptr<DeviceWriter> m_writer_ptr;
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
