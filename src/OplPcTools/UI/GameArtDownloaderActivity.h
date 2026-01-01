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

#include <OplPcTools/UI/Activity.h>
#include <OplPcTools/UI/Intent.h>
#include <OplPcTools/GameArtManager.h>
#include "ui_GameArtDownloaderActivity.h"

namespace OplPcTools {
namespace UI {

class GameArtDownloaderActivity : public Activity, private Ui::ArtDownloaderActivity
{
    Q_OBJECT

public:
    GameArtDownloaderActivity(
        GameArtManager & _art_manager,
        const QString & _game_id,
        const QList<GameArtType> & _art_types,
        QWidget * _parent = nullptr);

    static QSharedPointer<Intent> createIntent(
        GameArtManager & _art_manager,
        const QString & _game_id,
        const QList<GameArtType> & _art_types);
};

} // namespace UI
} // namespace OplPcTools
