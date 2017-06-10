/***********************************************************************************************
 *                                                                                             *
 * This file is part of the qpcopl project, the graphical PC tools for Open PS2 Loader.        *
 *                                                                                             *
 * qpcopl is free software: you can redistribute it and/or modify it under the terms of        *
 * the GNU General Public License as published by the Free Software Foundation,                *
 * either version 3 of the License, or (at your option) any later version.                     *
 *                                                                                             *
 * qpcopl is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;        *
 * without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  *
 * See the GNU General Public License for more details.                                        *
 *                                                                                             *
 * You should have received a copy of the GNU General Public License along with MailUnit.      *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QListWidgetItem>
#include "MainWindow.h"
#include "Settings.h"
#include "SettingsDialog.h"
#include "GameInstaller.h"
#include "AboutDialog.h"
#include "GameInstallDialog.h"
#include "GameRenameDialog.h"
#include "Exception.h"

namespace {

static const char * g_settings_key_wnd_geometry = "WindowGeometry";
static const char * g_settings_key_ul_dir = "ULDirectory";
static const char * g_settings_key_cover_dir = "PixmapDirectory";

class GameListItem : public QListWidgetItem
{
public:
    explicit GameListItem(const GameRepository & _repository, const QString & _game_id) :
        mr_repository(_repository),
        m_game_id(_game_id)
    {
        mp_game = _repository.game(_game_id);
    }

    QVariant data(int _role) const override;

    void reload()
    {
        mp_game = mr_repository.game(m_game_id);
    }

    const Game & game() const
    {
        return *mp_game;
    }

private:
    const GameRepository & mr_repository;
    const QString m_game_id;
    const Game * mp_game;
};

QVariant GameListItem::data(int _role) const
{
    if(_role == Qt::DisplayRole)
        return mp_game->name;
    return QListWidgetItem::data(_role);
}

} // namespace

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    mp_widget_game_details->setVisible(false);
    mp_label_current_ul_file = new QLabel(mp_statusbar);
    mp_statusbar->addWidget(mp_label_current_ul_file);
    activateFileActions(false);
    activateGameActions(false);
    QSettings settings;
    restoreGeometry(settings.value(g_settings_key_wnd_geometry).toByteArray());
    if(Settings::instance().reopenLastSestion())
    {
        QDir directory(settings.value(g_settings_key_ul_dir).toString());
        if(directory.exists())
            loadUlConfig(directory);
    }
}

void MainWindow::closeEvent(QCloseEvent * _event)
{
    Q_UNUSED(_event)
    QSettings settings;
    settings.setValue(g_settings_key_wnd_geometry, saveGeometry());
}

void MainWindow::about()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::showSettings()
{
    SettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::loadUlConfig()
{
    QSettings settings;
    QString dirpath = settings.value(g_settings_key_ul_dir).toString();
    dirpath = QFileDialog::getExistingDirectory(this, tr("Choose the OPL root dir"), dirpath);
    if(dirpath.isEmpty()) return;
    settings.setValue(g_settings_key_ul_dir, dirpath);
    loadUlConfig(dirpath);
}

void MainWindow::loadUlConfig(const QDir & _directory)
{
    mp_list_games->clear();
    try
    {
        m_game_repository.reloadFromUlConfig(_directory);
        for(const Game & game : m_game_repository.games())
            mp_list_games->addItem(new GameListItem(m_game_repository, game.id));
        mp_label_current_ul_file->setText(m_game_repository.file());
        mp_list_games->setCurrentRow(0);
        activateFileActions(true);
    }
    catch(const Exception & exception)
    {
        mp_label_current_ul_file->clear();
        activateFileActions(false);
        QMessageBox::critical(this, QString(), exception.message());
    }
}

void MainWindow::reloadUlConfig()
{
    loadUlConfig(m_game_repository.directory());
}

void MainWindow::activateFileActions(bool _activate)
{
    mp_action_add_game->setEnabled(_activate);
    mp_action_reload_file->setEnabled(_activate);
}

void MainWindow::activateGameActions(bool _activate)
{
    mp_action_delete_game->setEnabled(_activate);
    mp_action_rename_game->setEnabled(_activate);
}

void MainWindow::addGame()
{
    try
    {
        GameInstallDialog dlg(m_game_repository, this);
        connect(&dlg, &GameInstallDialog::gameInstalled, this, &MainWindow::gameInstalled);
        dlg.exec();
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}

void MainWindow::gameInstalled(const QString & _id)
{
    GameListItem * item = new GameListItem(m_game_repository, _id);
    mp_list_games->addItem(item);
    mp_list_games->setCurrentItem(item);
}

void MainWindow::deleteGame()
{
    GameListItem * item = static_cast<GameListItem *>(mp_list_games->currentItem());
    if(item == nullptr) return;
    if(Settings::instance().confirmGameDeletion() &&
       QMessageBox::question(this, QString(),
       tr("Are you sure you want to delete the game?") + "\r\n" + item->game().name) != QMessageBox::Yes)
    {
        return;
    }
    try
    {
        m_game_repository.deleteGame(item->game().id);
        delete item;
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}

void MainWindow::setCover()
{
    GameListItem * item = static_cast<GameListItem *>(mp_list_games->currentItem());
    if(item == nullptr) return;
    QString filename = getOpenPicturePath(tr("Choose the game cover"));
    if(filename.isEmpty()) return;
    try
    {
        m_game_repository.setGameCover(item->game().id, filename);
        gameSelected(item);
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}

QString MainWindow::getOpenPicturePath(const QString & _title)
{
    QSettings settings;
    QString dirpath = settings.value(g_settings_key_cover_dir).toString();
    if(dirpath.isEmpty())
        dirpath = settings.value(g_settings_key_ul_dir).toString();
    QString filename = QFileDialog::getOpenFileName(this, _title, dirpath, tr("Image Files") + " (*.png *.jpg *.jpeg *.bmp)");
    if(filename.isEmpty()) return filename;
    settings.setValue(g_settings_key_cover_dir, QFileInfo(filename).absoluteDir().absolutePath());
    return filename;
}

void MainWindow::removeCover()
{
    try
    {
        GameListItem * item = static_cast<GameListItem *>(mp_list_games->currentItem());
        if(item == nullptr) return;
        if(Settings::instance().confirmPixmapDeletion() &&
           QMessageBox::question(this, QString(), tr("Are you sure you want to delete the game cover?")) != QMessageBox::Yes)
        {
            return;
        }
        m_game_repository.removeGameCover(item->game().id);
        gameSelected(item);
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}

void MainWindow::setIcon()
{
    GameListItem * item = static_cast<GameListItem *>(mp_list_games->currentItem());
    if(item == nullptr) return;
    QString filename = getOpenPicturePath(tr("Choose the game icon"));
    if(filename.isEmpty()) return;
    try
    {
        m_game_repository.setGameIcon(item->game().id, filename);
        gameSelected(item);
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}

void MainWindow::removeIcon()
{
    try
    {
        GameListItem * item = static_cast<GameListItem *>(mp_list_games->currentItem());
        if(item == nullptr) return;
        if(Settings::instance().confirmPixmapDeletion() &&
           QMessageBox::question(this, QString(), tr("Are you sure you want to delete the game icon?")) != QMessageBox::Yes)
        {
            return;
        }
        m_game_repository.removeGameIcon(item->game().id);
        gameSelected(item);
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}

void MainWindow::gameSelected(QListWidgetItem * _item)
{
    if(_item == nullptr)
    {
        mp_widget_game_details->setVisible(false);
        activateGameActions(false);
        return;
    }
    GameListItem * item = static_cast<GameListItem *>(_item);
    const Game & game = item->game();
    mp_label_game_id->setText(game.id);
    mp_label_game_title->setText(game.name);
    mp_label_game_parts->setText(QString("%1").arg(game.part_count));
    switch(game.media_type)
    {
    case MediaType::cd:
        mp_label_game_type->setText("CD");
        break;
    case MediaType::dvd:
        mp_label_game_type->setText("DVD");
        break;
    default:
        mp_label_game_type->setText(tr("Unknown"));
        break;
    }
    mp_label_cover->setPixmap(game.cover);
    mp_label_icon->setPixmap(game.icon);
    mp_widget_game_details->setVisible(true);
    activateGameActions(true);
}

void MainWindow::renameGame()
{
    GameListItem * item = static_cast<GameListItem *>(mp_list_games->currentItem());
    if(item == nullptr) return;
    const Game & game = item->game();
    try
    {
        GameRenameDialog dlg(game.name, this);
        if(dlg.exec() == QDialog::Accepted)
        {
            QString new_name = dlg.name();
            m_game_repository.renameGame(game.id, new_name);
            item->reload();
            mp_list_games->update(mp_list_games->currentIndex());
            gameSelected(item);
        }
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}
