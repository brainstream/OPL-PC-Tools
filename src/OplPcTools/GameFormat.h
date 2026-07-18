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

#ifndef __OPLPCTOOLS_GAMEINSTALLATIONTYPE__
#define __OPLPCTOOLS_GAMEINSTALLATIONTYPE__

#include <OplPcTools/Device/DeviceSource.h>
#include <QSharedPointer>

namespace OplPcTools {

class Game;

enum class GameInstallationType
{
    UlConfig,
    Iso9660,
    Ziso
};

enum class GameSourceFormat
{
    PhysicalDevice,
    Iso9660,
    Ziso,
    Bin,
    NeroImage
};

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

inline QString gameSourceFormatName(GameSourceFormat _type)
{
    switch(_type)
    {
    case GameSourceFormat::PhysicalDevice:
        return "Disc";
    case GameSourceFormat::Ziso:
        return "ZSO";
    case GameSourceFormat::Iso9660:
        return "ISO";
    case GameSourceFormat::Bin:
        return "BIN";
    case GameSourceFormat::NeroImage:
        return "Nero";
    default:
        throw std::runtime_error("Unknown GameSourceFormat");
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

#endif // __OPLPCTOOLS_GAMEINSTALLATIONTYPE__
