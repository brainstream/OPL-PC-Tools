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

#include <OplPcTools/UI/GameArtDownloaderActivity.h>
#include <OplPcTools/UI/Application.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

class ArtDownloaderIntent : public Intent
{
public:
    ArtDownloaderIntent(
        GameArtManager & _art_manager,
        const QList<GameArtDownloaderActivityTask> & _tasks
    ) :
        mr_art_manager(_art_manager),
        m_tasks(_tasks)
    {
    }

    Activity * createActivity(QWidget * _parent) override
    {
        return new GameArtDownloaderActivity(mr_art_manager, m_tasks, _parent);
    }

    QString activityClass() const override
    {
        return "GameArtDownloader";
    }

private:
    GameArtManager & mr_art_manager;
    const QList<GameArtDownloaderActivityTask> m_tasks;
};

} // namespace name

GameArtDownloaderActivity::GameArtDownloaderActivity(
    GameArtManager & _art_manager,
    const QList<GameArtDownloaderActivityTask> & _tasks,
    QWidget * _parent /*= nullptr*/
) :
    Activity(_parent),
    mr_art_manager(_art_manager),
    mp_downloader(new GameArtDownloader(this)),
    mp_current_task(nullptr)
{
    setupUi(this);

    qsizetype total_art_count = 0;
    foreach(const GameArtDownloaderActivityTask & t, _tasks)
    {
        if(t.art_types.empty())
            continue;
        m_tasks.enqueue(t);
        total_art_count += t.art_types.count();
    }
    if(m_tasks.empty())
    {
        deleteLater();
        return;
    }

    mp_progress_bar->setMinimum(0);
    mp_progress_bar->setMaximum(total_art_count);
    mp_progress_bar->setValue(0);

    connect(mp_downloader, &GameArtDownloader::taskComplete, this, [this](quint32, const QStringList & __errors) {
        mp_current_task = nullptr;
        if(!__errors.isEmpty())
            Application::showErrorMessage(__errors.join("\n\n"));
        if(!placeNextTask())
        {
            mp_downloader->deleteLater();
            close();
        }
    });
    connect(mp_downloader, &GameArtDownloader::downloadComplete, this, [this](
        const OplPcTools::GameArtNetworkTask & __task,
        const OplPcTools::GameArtNetworkSource & __source
    ) {
        mr_art_manager.setArt(__task.game_id, __task.art_type, __source);
        mp_progress_bar->setValue(mp_progress_bar->value() + 1);
    });
    connect(mp_button_box, &QDialogButtonBox::rejected, this, [this]() {
        if(mp_current_task)
        {
            m_tasks.clear();
            mp_current_task->cancel();
        }
    });
}

bool GameArtDownloaderActivity::onAttach()
{
    if(m_tasks.empty())
        return false;
    return placeNextTask();
}

bool GameArtDownloaderActivity::placeNextTask()
{
    if(m_tasks.empty())
        return false;
    GameArtDownloaderActivityTask task_src = m_tasks.dequeue();
    mp_current_task = mp_downloader->downloadArts(task_src.game_id, task_src.art_types);
    return true;
}

QSharedPointer<Intent> GameArtDownloaderActivity::createIntent(
    GameArtManager & _art_manager,
    const QList<GameArtDownloaderActivityTask> & _tasks)
{
    return QSharedPointer<Intent>(new ArtDownloaderIntent(_art_manager, _tasks));
}
