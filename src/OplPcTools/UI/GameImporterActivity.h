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

#include "ui_GameImporterActivity.h"
#include <OplPcTools/UI/Activity.h>
#include <OplPcTools/UI/Intent.h>
#include <OplPcTools/UlConfigGameStorage.h>
#include <OplPcTools/GameArtManager.h>

namespace OplPcTools::UI {

class GameImporterActivity : public Activity, private Ui::GameImporterActivity
{
    Q_OBJECT

private:
    class WorkerThread;

public:
    explicit GameImporterActivity(GameArtManager & _art_manager, QWidget * _parent = nullptr);
    bool onAttach() override;
    static QSharedPointer<Intent> createIntent(GameArtManager & _art_manager);

private:
    void setBusyUIState(bool _is_busy);

private slots:
    void onThreadFinished();

private:
    GameArtManager & mr_art_manager;
    WorkerThread * mp_thread;
};

} // namespace OplPcTools::UI
