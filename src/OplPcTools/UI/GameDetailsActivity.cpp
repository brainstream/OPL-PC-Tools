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

#include <QShortcut>
#include <QMessageBox>
#include <OplPcTools/Exception.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/GameRenameDialog.h>
#include <OplPcTools/UI/GameArtsWidget.h>
#include <OplPcTools/UI/GameConfigWidget.h>
#include <OplPcTools/UI/GameDetailsActivity.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

class GameDetailsActivityIntent : public Intent
{
public:
    GameDetailsActivityIntent(GameArtManager & _art_manager, const Uuid & _game_uuid) :
        mr_art_manager(_art_manager),
        m_game_uuid(_game_uuid)
    {
    }

    Activity * createActivity(QWidget * _parent) override
    {
        GameDetailsActivity * widget = new GameDetailsActivity(m_game_uuid, mr_art_manager, _parent);
        return widget;
    }

    QString activityClass() const override
    {
        return "GameDetails";
    }

private:
    GameArtManager & mr_art_manager;
    const Uuid m_game_uuid;
};

} // namespace


GameDetailsActivity::GameDetailsActivity(const Uuid _game_uuid, GameArtManager & _art_manager, QWidget * _parent /*= nullptr*/) :
    Activity(_parent),
    mr_art_manager(_art_manager),
    mp_game(Library::instance().games().findGame(_game_uuid))
{
    setupUi(this);
    setupShortcuts();
    mp_tabs->setCurrentIndex(0);
    if(mp_game)
    {
        GameArtsWidget * game_art_widget = new GameArtsWidget(mp_game->id(), _art_manager, this);
        mp_tab_arts->layout()->addWidget(game_art_widget);
        mp_tab_config->layout()->addWidget( new GameConfigWidget(*mp_game, this));
        mp_label_title->setText(mp_game->title());
        connect(mp_btn_download_all_art, &QPushButton::clicked, game_art_widget, &GameArtsWidget::downloadAllGameArts);
    }
    connect(mp_btn_close, &QPushButton::clicked, this, &GameDetailsActivity::close);
    connect(mp_label_title, &ClickableLabel::clicked, this, &GameDetailsActivity::renameGame);
    connect(mp_btn_rename_game, &QPushButton::clicked, this, &GameDetailsActivity::renameGame);
}

QSharedPointer<Intent> GameDetailsActivity::createIntent(GameArtManager & _art_manager, const Uuid & _game_uuid)
{
    return QSharedPointer<Intent>(new GameDetailsActivityIntent(_art_manager, _game_uuid));
}

void GameDetailsActivity::setupShortcuts()
{
    QShortcut * shortcut = new QShortcut(QKeySequence("Back"), this);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsActivity::close);
    shortcut = new QShortcut(QKeySequence("Esc"), this);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsActivity::close);
    shortcut = new QShortcut(QKeySequence("F2"), this);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsActivity::renameGame);
}

void GameDetailsActivity::renameGame()
{
    if(mp_game == nullptr) return;
    GameRenameDialog dlg(mp_game->title(), mp_game->installationType(), this);
    if(dlg.exec() != QDialog::Accepted)
        return;
    try
    {
        Library::instance().games().renameGame(*mp_game, dlg.name());
        mp_label_title->setText(dlg.name());
    }
    catch(const Exception & exception)
    {
        Application::showErrorMessage(exception.message());
    }
    catch(...)
    {
        Application::showErrorMessage();
    }
}
