/***********************************************************************************************
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

#ifndef __OPLPCTOOLS_GAMEDETAILSWIDGET__
#define __OPLPCTOOLS_GAMEDETAILSWIDGET__

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <OplPcTools/Core/GameArtManager.h>
#include <OplPcTools/UI/UIContext.h>
#include "ui_GameDetailsWidget.h"

namespace OplPcTools {
namespace UI {

class GameDetailsWidget : public QWidget, private Ui::GameDetailsWidget
{
    Q_OBJECT

public:
    GameDetailsWidget(UIContext & _context, OplPcTools::Core::GameArtManager & _art_manager, QWidget * _parent = nullptr);
    void setGameId(const QString & _id);

private:
    void setupShortcuts();
    void initGameControls();
    void addArtListItem(Core::GameArtType _type, const QString & _text);
    void clearGameControls();

private slots:
    void renameGame();
    void showItemContextMenu(const QPoint & _point);
    void changeGameArt();
    void removeGameArt();

private:
    UIContext & mr_context;
    OplPcTools::Core::GameArtManager & mr_art_manager;
    const OplPcTools::Core::Game * mp_game;
    QMenu * mp_item_context_menu;
    QAction * mp_action_change_art;
    QAction * mp_action_remove_art;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMEDETAILSWIDGET__
