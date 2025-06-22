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

#include <OplPcTools/UI/GameArtDownloaderActivity.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/GameArtNetworkSource.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

class ArtDownloaderIntent : public Intent
{
public:
    ArtDownloaderIntent(
        GameArtManager & _art_manager,
        const QString & _game_id,
        const QList<GameArtType> & _art_types
    ) :
        mr_art_manager(_art_manager),
        m_game_id(_game_id),
        m_art_types(_art_types)
    {
    }

    Activity * createActivity(QWidget * _parent) override
    {
        return new GameArtDownloaderActivity(mr_art_manager, m_game_id, m_art_types, _parent);
    }

    QString activityClass() const override
    {
        return "GameArtDownloader";
    }

private:
    GameArtManager & mr_art_manager;
    const QString m_game_id;
    const QList<GameArtType> m_art_types;
};

} // namespace name

GameArtDownloaderActivity::GameArtDownloaderActivity(
    GameArtManager & _art_manager,
    const QString & _game_id,
    const QList<GameArtType> & _art_types,
    QWidget * _parent /*= nullptr*/
) :
    Activity(_parent)
{
    setupUi(this);
    if(_art_types.empty())
    {
        deleteLater();
        return;
    }
    mp_progress_bar->setMinimum(0);
    mp_progress_bar->setMaximum(_art_types.count());
    mp_progress_bar->setValue(0);
    GameArtDownloader * downloader = new GameArtDownloader(this);
    connect(downloader, &GameArtDownloader::taskComplete, this, [=](quint32, const QStringList & __errors) {
       downloader->deleteLater();
       if(!__errors.isEmpty())
           Application::showErrorMessage(__errors.join("\n\n"));
       deleteLater();
    });
    connect(downloader, &GameArtDownloader::downloadComplete, this, [this, &_art_manager, _game_id](
        const OplPcTools::GameArtNetworkTask & __task,
        const OplPcTools::GameArtNetworkSource & __source
    ) {
        _art_manager.setArt(_game_id, __task.art_type, __source);
        mp_progress_bar->setValue(mp_progress_bar->value() + 1);
    });
    GameArtDownloaderTask * task = downloader->downloadArts(_game_id, _art_types);
    connect(mp_button_box, &QDialogButtonBox::rejected, task, &GameArtDownloaderTask::cancel);
}

QSharedPointer<Intent> GameArtDownloaderActivity::createIntent(
    GameArtManager & _art_manager,
    const QString & _game_id,
    const QList<GameArtType> & _art_types)
{
    return QSharedPointer<Intent>(new ArtDownloaderIntent(_art_manager, _game_id, _art_types));
}
