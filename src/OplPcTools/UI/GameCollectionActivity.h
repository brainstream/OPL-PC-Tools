/***********************************************************************************************
 * Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                   *
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
 * You should have received a copy of the GNU General Public License along with MailUnit.      *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#ifndef __OPLPCTOOLS_GAMECOLLECTIONACTIVITY__
#define __OPLPCTOOLS_GAMECOLLECTIONACTIVITY__

#include <QSharedPointer>
#include <QDir>
#include <QPixmap>
#include <QWidget>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <OplPcTools/Game.h>
#include <OplPcTools/GameArtManager.h>
#include <OplPcTools/UI/Intent.h>
#include "ui_GameCollectionActivity.h"

namespace OplPcTools {
namespace UI {

class GameCollectionActivity : public Activity, private Ui::GameCollectionActivity
{
    class GameTreeModel;

    Q_OBJECT

public:
    explicit GameCollectionActivity(QWidget * _parent = nullptr);
    bool onAttach() override;
    bool tryLoadRecentDirectory();

    static QSharedPointer<Intent> createIntent();

private:
    void applySettings();
    void saveSettings();
    void activateCollectionControls(bool _activate);
    void activateItemControls(const Game * _selected_game);
    void changeIconsSize();
    void showTreeContextMenu(const QPoint & _point);
    void load();
    void loadDirectory(const QDir & _directory);
    void reload();
    void renameGame();
    void showGameDetails();
    void showGameInstaller();
    void deleteGame();
    void collectionLoaded();
    void gameAdded(const QString & _id);
    void gameRenamed(const QString & _id);
    void gameArtChanged(const QString & _game_id, GameArtType _type, const QPixmap * _pixmap);
    void gameSelected();
    void showIsoRestorer();

private:
    OplPcTools::GameArtManager * mp_game_art_manager;
    GameTreeModel * mp_model;
    QMenu * mp_context_menu;
    QSortFilterProxyModel * mp_proxy_model;
    QPixmap m_default_cover;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMECOLLECTIONACTIVITY__
