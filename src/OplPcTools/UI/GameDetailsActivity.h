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

#ifndef __OPLPCTOOLS_GAMEDETAILSACTIVITY__
#define __OPLPCTOOLS_GAMEDETAILSACTIVITY__

#include <QSharedPointer>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <OplPcTools/UI/Intent.h>
#include <OplPcTools/Game.h>
#include <OplPcTools/GameArtManager.h>
#include "ui_GameDetailsActivity.h"

namespace OplPcTools {
namespace UI {

class GameDetailsActivity : public Activity, private Ui::GameDetailsActivity
{
    Q_OBJECT

public:
    explicit GameDetailsActivity(OplPcTools::GameArtManager & _art_manager, QWidget * _parent = nullptr);
    void setGameId(const QString & _id);

    static QSharedPointer<Intent> createIntent(OplPcTools::GameArtManager & _art_manager, const QString & _game_id);

private:
    void setupShortcuts();
    void initGameControls();
    void addArtListItem(GameArtType _type, const QString & _text);
    void clearGameControls();

private slots:
    void renameGame();
    void showItemContextMenu(const QPoint & _point);
    void changeGameArt();
    void deleteGameArt();

private:
    OplPcTools::GameArtManager & mr_art_manager;
    const OplPcTools::Game * mp_game;
    QMenu * mp_item_context_menu;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMEDETAILSACTIVITY__
