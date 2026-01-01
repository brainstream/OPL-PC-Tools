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

#ifndef __OPLPCTOOLS_GAMELISTWIDGET__
#define __OPLPCTOOLS_GAMELISTWIDGET__

#include <functional>
#include <QSharedPointer>
#include <QDir>
#include <QPixmap>
#include <QWidget>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <OplPcTools/Game.h>
#include <OplPcTools/GameArtManager.h>
#include "ui_GameListWidget.h"

namespace OplPcTools {
namespace UI {

class GameListWidget : public QWidget, private Ui::GameListWidget
{
    class GameTreeModel;

    Q_OBJECT

public:
    explicit GameListWidget(QWidget * _parent = nullptr);

private:
    void setupShortcuts();
    void setIconSize();
    void activateCollectionControls(bool _activate);
    void activateItemControls(const Game * _selected_game);
    void showTreeContextMenu(const QPoint & _point);
    void onLibraryLoaded();
    void renameGame();
    void showGameDetails();
    void showGameImporter();
    void showGameInstaller();
    void deleteGame();
    void startBusyThread(std::function<void()> _lambda);
    void onGameAdded(const Uuid & _uuid);
    void onGameRenamed(const Uuid & _uuid);
    void onGameArtChanged(const QString & _game_id, GameArtType _type);
    void onGameSelected();
    void showIsoRestorer();

private:
    GameArtManager * mp_game_art_manager;
    GameTreeModel * mp_model;
    QMenu * mp_context_menu;
    QSortFilterProxyModel * mp_proxy_model;
    QPixmap m_default_cover;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMELISTWIDGET__
