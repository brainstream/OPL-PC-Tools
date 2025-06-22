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
        mp_tab_arts->layout()->addWidget(new GameArtsWidget(mp_game->id(), _art_manager, this));
        mp_tab_config->layout()->addWidget(new GameConfigWidget(*mp_game, this));
        mp_label_title->setText(mp_game->title());
    }
    connect(mp_btn_close, &QPushButton::clicked, this, &GameDetailsActivity::close);
    connect(mp_label_title, &ClickableLabel::clicked, this, &GameDetailsActivity::renameGame);
    connect(mp_btn_download_all_art, &QPushButton::clicked, this, &GameDetailsActivity::downloadAllArtwork);
    connect(mp_btn_rename_game, &QPushButton::clicked, this, &GameDetailsActivity::renameGame);
    connect(&mr_art_manager, &GameArtManager::allDownloadsCompleted, this, &GameDetailsActivity::onAllDownloadsCompleted);
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

void GameDetailsActivity::downloadAllArtwork()
{
    if(mp_game == nullptr) return;
    
    // Show confirmation dialog
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Download artwork"));
    
    QPushButton *downloadAllBtn = msgBox.addButton(tr("Replace existing"), QMessageBox::ActionRole);
    QPushButton *downloadMissingBtn = msgBox.addButton(tr("Missing only"), QMessageBox::ActionRole);
    QPushButton *cancelBtn = msgBox.addButton(QMessageBox::Cancel);
    
    msgBox.setDefaultButton(downloadMissingBtn);
    
    msgBox.exec();
    
    if(msgBox.clickedButton() == downloadAllBtn)
    {
        // Download all artwork (replace existing)
        mr_art_manager.downloadAllArt(mp_game->id(), false);
    }
    else if(msgBox.clickedButton() == downloadMissingBtn)
    {
        // Download only missing artwork
        mr_art_manager.downloadAllArt(mp_game->id(), true);
    }
    // If cancel was clicked, do nothing
}

void GameDetailsActivity::onAllDownloadsCompleted(const QString & _game_id, int _successful, int _total)
{
    if(mp_game == nullptr || _game_id != mp_game->id()) return;
    
    QString message;
    if(_total == 0)
    {
        message = tr("All artwork already exists for this game.");
    }
    else if(_successful == _total)
    {
        message = tr("Successfully downloaded all %1 artwork images.").arg(_successful);
    }
    else if(_successful > 0)
    {
        message = tr("Downloaded %1 of %2 artwork images. Some images were not found at archive.org.").arg(_successful).arg(_total);
    }
    else
    {
        message = tr("No artwork was found for this game at archive.org.");
    }
    
    Application::showMessage(tr("Download Complete"), message);
}
