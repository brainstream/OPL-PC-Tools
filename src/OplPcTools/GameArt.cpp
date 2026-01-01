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

#include <OplPcTools/GameArt.h>
#include <QObject>

using namespace OplPcTools;

QMap<GameArtType, GameArtProperties> OplPcTools::makeGameArtProperies()
{
    return QMap<GameArtType, GameArtProperties>
    {
        {
            GameArtType::Icon,
            {
                .suffix = "_ICO",
                .name = QObject::tr("Icon"),
                .size = QSize(64, 64)
            }
        },
        {
            GameArtType::Front,
            {
                .suffix = "_COV",
                .name = QObject::tr("Front Cover"),
                .size = QSize(140, 200)
            }
        },
        {
            GameArtType::Back,
            {
                .suffix = "_COV2",
                .name = QObject::tr("Back Cover"),
                .size = QSize(242, 344)
            }
        },
        {
            GameArtType::Spine,
            {
                .suffix = "_LAB",
                .name = QObject::tr("Spine Cover"),
                .size = QSize(18, 240)
            }
        },
        {
            GameArtType::Screenshot1,
            {
                .suffix = "_SCR",
                .name = QObject::tr("Screenshot #1"),
                .size = QSize(250, 188)
            }
        },
        {
            GameArtType::Screenshot2,
            {
                .suffix = "_SCR2",
                .name = QObject::tr("Screenshot #2"),
                .size = QSize(250, 188)
            }
        },
        {
            GameArtType::Background,
            {
                .suffix = "_BG",
                .name = QObject::tr("Background"),
                .size = QSize(640, 480)
            }
        },
        {
            GameArtType::Logo,
            {
                .suffix = "_LGO",
                .name = QObject::tr("Logo"),
                .size = QSize(300, 125)
            }
        }
    };
}
