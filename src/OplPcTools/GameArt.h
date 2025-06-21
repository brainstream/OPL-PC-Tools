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

#pragma once

#include <QString>
#include <QSize>
#include <QMap>

namespace OplPcTools {

enum class GameArtType
{
    Icon         = 0x1,
    Front        = 0x2,
    Back         = 0x4,
    Spine        = 0x8,
    Screenshot1  = 0x10,
    Screenshot2  = 0x20,
    Background   = 0x40,
    Logo         = 0x80
};

inline uint qHash(GameArtType _game_art_type, uint _seed = 0) noexcept
{
    return ::qHash(static_cast<int>(_game_art_type), _seed);
}

struct GameArtProperties
{
    QString suffix;
    QString name;
    QSize size;
};

QMap<GameArtType, GameArtProperties> makeGameArtProperies();

} // namespace OplPcTools
