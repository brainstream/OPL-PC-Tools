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

#ifndef __OPLPCTOOLS_GAMEDETAILSACTIVITY__
#define __OPLPCTOOLS_GAMEDETAILSACTIVITY__

#include <QSharedPointer>
#include <OplPcTools/Game.h>
#include <OplPcTools/GameArtManager.h>
#include <OplPcTools/UI/Intent.h>
#include "ui_GameDetailsActivity.h"

namespace OplPcTools {
namespace UI {

class GameDetailsActivity : public Activity, private Ui::GameDetailsActivity
{
    Q_OBJECT

public:
    explicit GameDetailsActivity(const Uuid _game_uuid, GameArtManager & _art_manager, QWidget * _parent = nullptr);
    static QSharedPointer<Intent> createIntent(GameArtManager & _art_manager, const Uuid & _game_uuid);

private:
    void setupShortcuts();
    void renameGame();

private:
    GameArtManager & mr_art_manager;
    const Game * mp_game;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMEDETAILSACTIVITY__
