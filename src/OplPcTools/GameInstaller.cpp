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

#include <OplPcTools/GameInstaller.h>

using namespace OplPcTools;

GameInstaller::GameInstaller(Device & _device, GameCollection & _collection, QObject * _parent /*= nullptr*/) :
    QObject(_parent),
    mr_device(_device),
    mr_collection(_collection)
{
}

MediaType GameInstaller::deviceMediaType() const
{
    const quint64 iso_size = mr_device.size();
    MediaType type = mr_device.mediaType();
    if(type == MediaType::Unknown)
        type = iso_size > 681984000 ? MediaType::DVD : MediaType::CD;
    return type;
}
