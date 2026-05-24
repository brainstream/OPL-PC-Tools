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

#include <OplPcTools/GameCheat.h>
#include <OplPcTools/StandardDirectories.h>
#include <OplPcTools/File.h>
#include <QDir>

using namespace OplPcTools;

QString GameCheat::makeFilename(const QString & _library_path, const QString & _game_id)
{
    QDir dir((_library_path.endsWith(QDir::separator())
        ? _library_path
        : _library_path + QDir::separator()) + StandardDirectories::cht);
    return dir.absoluteFilePath(_game_id + ".cht");
}

QSharedPointer<GameCheat> GameCheat::load(const QString & _filename)
{
    QSharedPointer<GameCheat> cheat(new GameCheat());
    QFile file(_filename);
    if(file.exists())
    {
        openFile(file, QFile::ReadOnly);
        cheat->text = file.readAll();
    }
    return cheat;
}

void GameCheat::save(const GameCheat & _cheat, const QString & _filename)
{
    QFile file(_filename);
    if(_cheat.text.trimmed().isEmpty())
    {
        if(file.exists())
            removeFile(file);
        return;
    }
    QFileInfo(file).absoluteDir().mkpath(".");
    openFile(file, QFile::WriteOnly | QFile::Truncate);
    file.write(_cheat.text.toLatin1());
}
