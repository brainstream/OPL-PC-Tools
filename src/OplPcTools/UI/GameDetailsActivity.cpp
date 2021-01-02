/***********************************************************************************************
 * Copyright © 2017-2021 Sergey Smolyannikov aka brainstream                                   *
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

#include <QShortcut>
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
    GameDetailsActivityIntent(GameArtManager & _art_manager, const QString & _game_id) :
        mr_art_manager(_art_manager),
        m_game_id(_game_id)
    {
    }

    Activity * createActivity(QWidget * _parent) override
    {
        GameDetailsActivity * widget = new GameDetailsActivity(m_game_id, mr_art_manager, _parent);
        return widget;
    }

    QString activityClass() const override
    {
        return "GameDetails";
    }

private:
    GameArtManager & mr_art_manager;
    const QString m_game_id;
};

} // namespace


GameDetailsActivity::GameDetailsActivity(const QString _game_id, OplPcTools::GameArtManager & _art_manager, QWidget * _parent /*= nullptr*/) :
    Activity(_parent),
    mr_art_manager(_art_manager),
    mp_game(Library::instance().games().findGame(_game_id))
{
    setupUi(this);
    setupShortcuts();
    mp_tabs->setCurrentIndex(0);
    mp_tab_arts->layout()->addWidget(new GameArtsWidget(_game_id, _art_manager, this));
    mp_tab_config->layout()->addWidget(new GameConfigWidget(this));
    connect(mp_btn_close, &QPushButton::clicked, this, &GameDetailsActivity::close);
    connect(mp_label_title, &ClickableLabel::clicked, this, &GameDetailsActivity::renameGame);
    initControls();
}

QSharedPointer<Intent> GameDetailsActivity::createIntent(OplPcTools::GameArtManager & _art_manager, const QString & _game_id)
{
    return QSharedPointer<Intent>(new GameDetailsActivityIntent(_art_manager, _game_id));
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

void GameDetailsActivity::initControls()
{
    if(mp_game == nullptr)
    {
        mp_label_title->clear();
        return;
    }
    mp_label_title->setText(mp_game->title());
}
