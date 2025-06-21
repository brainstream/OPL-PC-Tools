/***********************************************************************************************
 * Copyright © 2017-2025 Sergey Smolyannikov aka brainstream                                   *
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

#include <OplPcTools/Library.h>

using namespace OplPcTools;

Library::Library() :
    QObject(nullptr)
{
    mp_games = new GameCollection(this);
    mp_vmcs = new VmcCollection(this);
}

void Library::load(const QDir & _directory)
{
    emit loading();
    mp_games->load(_directory);
    mp_vmcs->load(_directory);
    m_directory = _directory.absolutePath();
    emit loaded();
}

Library & Library::instance()
{
    static Library * library = new Library();
    return *library;
}
