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

#include <OplPcTools/Exception.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/GameInstaller.h>

using namespace OplPcTools;

GameInstaller::GameInstaller(DeviceReader & _device, QObject * _parent /*= nullptr*/) :
    QObject(_parent),
    mr_device(_device),
    m_override(false)
{
}

void GameInstaller::enableOverride()
{
    m_override = true;
}

bool GameInstaller::install()
{
    bool to_override = false;
    const Game * game = Library::instance().games().findGame(mr_device.gameId());
    if(game)
    {
        if(m_override)
            to_override = true;
        else
            throw ValidationException(tr("Game with ID %1 is already installed").arg(mr_device.gameId()));
    }
    bool result = performInstallation();
    if(result && to_override)
    {
        Library::instance().games().deleteGame(*game);
    }
    return result;
}

MediaType GameInstaller::deviceMediaType() const
{
    return mr_device.size() > 681984000 ? MediaType::DVD : MediaType::CD;
}
