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

#include "ui_GameCheatWidget.h"
#include <OplPcTools/Game.h>
#include <OplPcTools/GameCheatManager.h>

namespace OplPcTools {
namespace UI {

class GameCheatWidget : public QWidget, private Ui::GameCheatWidget
{
    Q_OBJECT

public:
    explicit GameCheatWidget(const Game & _game, QWidget * _parent = nullptr);
    ~GameCheatWidget() override;

private:
    void startSmartThread(std::function<void()> _action, std::function<void()> _finished);
    std::optional<QString> loadCheat();
    void saveCheat();
    void deleteCheat();
    void download();

private:
    GameCheatManager * mp_cheat_manager;
    const QString m_game_id;
};

} // namespace UI
} // namespace OplPcTools
