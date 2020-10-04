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

#include <OplPcTools/Library.h>

using namespace OplPcTools;

Library::Library(const QDir & _directory) :
    m_directory(_directory),
    mp_games(nullptr),
    mp_game_arts(nullptr),
    mp_vmcs(nullptr)
{
}

bool Library::load(const QDir & _directory)
{
//    if(mp_games)
//    {
//        mp_games->deleteLater();
//        mp_games = nullptr;
//    }
//    if(mp_game_arts)
//    {
//        mp_game_arts->deleteLater();
//        mp_game_arts = nullptr;
//    }
//    if(mp_vmcs)
//    {
//        mp_vmcs->deleteLater();
//        mp_vmcs = nullptr;
//    }
//    GameManager * games = new GameManager(m_directory, this);
//    VmcManager * vmcs = new VmcManager(m_directory, this);
//    if(!games->load() || !vmcs->load())
//    {
//        delete games;
//        delete vmcs;
//        return false;
//    }
//    mp_games = games;
//    mp_game_arts = new GameArtManager(m_directory, this);
//    mp_game_arts->addCacheType(GameArtType::Icon);
//    mp_game_arts->addCacheType(GameArtType::Front);
//    mp_vmcs = vmcs;
    return true;
}


