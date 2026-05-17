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
#include <OplPcTools/GameArtNetworkSource.h>
#include "ui_GameArtDownloaderActivity.h"
#include <QQueue>

namespace OplPcTools {
namespace UI {

struct GameArtDownloaderActivityTask
{
    QString game_id;
    QList<GameArtType> art_types;
};

class GameArtDownloaderActivity : public Activity, private Ui::ArtDownloaderActivity
{
    Q_OBJECT

public:
    GameArtDownloaderActivity(
        GameArtManager & _art_manager,
        const QList<GameArtDownloaderActivityTask> & _tasks,
        QWidget * _parent = nullptr);
    bool onAttach() override;

    static QSharedPointer<Intent> createIntent(
        GameArtManager & _art_manager,
        const QList<GameArtDownloaderActivityTask> & _tasks);

private:
    bool placeNextTask();

private:
    GameArtManager & mr_art_manager;
    GameArtDownloader * mp_downloader;
    QQueue<GameArtDownloaderActivityTask> m_tasks;
    GameArtDownloaderTask * mp_current_task;
};

} // namespace UI
} // namespace OplPcTools
