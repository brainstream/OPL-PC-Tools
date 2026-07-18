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

#include <OplPcTools/Device/UlDeviceSource.h>
#include <OplPcTools/Device/ZisoDeviceSource.h>
#include <OplPcTools/Device/Iso9660DeviceSource.h>
#include <OplPcTools/DirectoryGameStorage.h>
#include <OplPcTools/Library.h>

using namespace OplPcTools;

GameDeviceSourceFactory::GameDeviceSourceFactory(const Game & _game) :
    mp_game(&_game)
{
}

QSharedPointer<DeviceSource> GameDeviceSourceFactory::produceForUlConfig() const
{
    return QSharedPointer<DeviceSource>(new UlDeviceSource(*mp_game));
}

QSharedPointer<DeviceSource> GameDeviceSourceFactory::produceForIso9660() const
{
    std::optional<DirectoryGameStorage::FindIsoResult> result =
        Iso9660GameStorage::findIsoFile(*mp_game, Library::instance().directory());
    if(!result.has_value())
        return nullptr;
    return QSharedPointer<DeviceSource>(new Iso9660DeviceSource(result->path));
}

QSharedPointer<DeviceSource> GameDeviceSourceFactory::produceForZiso() const
{
    std::optional<DirectoryGameStorage::FindIsoResult> result =
        ZisoGameStorage::findIsoFile(*mp_game, Library::instance().directory());
    if(!result.has_value())
        return nullptr;
    return QSharedPointer<DeviceSource>(new ZisoDeviceSource(result->path));
}
