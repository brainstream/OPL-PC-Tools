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

#include <OplPcTools/Device/DeviceSource.h>
#include <OplPcTools/GameInstallationType.h>
#include <OplPcTools/Game.h>
#include <QSharedPointer>

namespace OplPcTools {

inline QString gameInstallationTypeName(GameInstallationType _type)
{
    switch(_type)
    {
    case GameInstallationType::UlConfig:
        return "UL";
    case GameInstallationType::Ziso:
        return "ZSO";
    case GameInstallationType::Iso9660:
        return "ISO";
    default:
        throw std::runtime_error("Unknown GameInstallationType");
    }
}

template<typename T>
class GameInstallationTypeUtilityFactory
{
public:
    virtual ~GameInstallationTypeUtilityFactory() { }

    T produce(GameInstallationType _type)
    {
        switch(_type)
        {
        case GameInstallationType::UlConfig:
            return produceForUlConfig();
        case GameInstallationType::Iso9660:
            return produceForIso9660();
        case GameInstallationType::Ziso:
            return produceForZiso();
        default:
            throw std::runtime_error("Unknown GameInstallationType");
        }
    }

protected:
    virtual T produceForUlConfig() const = 0;
    virtual T produceForIso9660() const = 0;
    virtual T produceForZiso() const = 0;
};

class GameDeviceSourceFactory final : public GameInstallationTypeUtilityFactory<QSharedPointer<DeviceSource>>
{
public:
    explicit GameDeviceSourceFactory(const Game & _game);

protected:
    QSharedPointer<DeviceSource> produceForUlConfig() const override;
    QSharedPointer<DeviceSource> produceForIso9660() const override;
    QSharedPointer<DeviceSource> produceForZiso() const override;

private:
    const Game * mp_game;
};

} // namespace OplPcTools
