/***********************************************************************************************
 * Copyright © 2017-2021 Sergey Smolyannikov aka brainstream                                   *
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

#include <OplPcTools/Exception.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/GameInstaller.h>

using namespace OplPcTools;

GameInstaller::GameInstaller(Device & _device, QObject * _parent /*= nullptr*/) :
    QObject(_parent),
    mr_device(_device)
{
}

bool GameInstaller::install()
{
    if(Library::instance().games().contains(mr_device.gameId()))
    {
        throw ValidationException(tr("Game with ID %1 is already installed").arg(mr_device.gameId()));
    }
    return performInstallation();
}

MediaType GameInstaller::deviceMediaType() const
{
    const quint64 iso_size = mr_device.size();
    MediaType type = mr_device.mediaType();
    if(type == MediaType::Unknown)
        type = iso_size > 681984000 ? MediaType::DVD : MediaType::CD;
    return type;
}
