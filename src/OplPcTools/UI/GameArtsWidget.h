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

#ifndef __OPLPCTOOLS_GAMEARTSWIDGET__
#define __OPLPCTOOLS_GAMEARTSWIDGET__

#include <functional>
#include <QWidget>
#include <QMenu>
#include <OplPcTools/GameArtManager.h>
#include "ui_GameArtsWidget.h"

namespace OplPcTools {
namespace UI {

class GameArtsWidget : public QWidget, private Ui::GameArtsWidget
{
    Q_OBJECT

public:
    GameArtsWidget(const QString & _game_id, GameArtManager & _art_manager, QWidget * _parent = nullptr);
    void downloadAllGameArts();

private:
    void setupShortcuts();
    void initGameArts();
    void addArtListItem(GameArtType _type, const QString & _text);
    void downloadGameArts(const QList<GameArtType> & _types);
    void startBusySmartThread(std::function<void()> _lambda);

private slots:
    void showItemContextMenu(const QPoint & _point);
    void changeGameArt();
    void downloadGameArt();
    void deleteGameArt();
    void onArtChanged(const QString & _game_id, OplPcTools::GameArtType _type);


private:
    GameArtManager & mr_art_manager;
    const QString m_game_id;
    QMenu * mp_item_context_menu;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMEARTSWIDGET__
