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

#include "GameInstaller.h"

GameInstaller::GameInstaller(Device & _device, GameCollection & _collection, QObject * _parent /*= nullptr*/) :
    QObject(_parent),
    mr_device(_device),
    mr_collection(_collection),
    mp_installed_game(nullptr)
{
}

GameInstaller::~GameInstaller()
{
    delete mp_installed_game;
}

bool GameInstaller::install()
{
    delete mp_installed_game;
    mp_installed_game = new Game();
    try
    {
        return performInstallation();
    }
    catch(...)
    {
        delete mp_installed_game;
        mp_installed_game = nullptr;
        throw;
    }
}
