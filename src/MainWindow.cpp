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

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTreeWidgetItem>
#include "MainWindow.h"
#include "Settings.h"
#include "SettingsDialog.h"
#include "GameInstaller.h"
#include "AboutDialog.h"
#include "GameInstallDialog.h"
#include "GameRenameDialog.h"
#include "IsoRecoverDialog.h"
#include "Exception.h"

namespace {

static const char * g_settings_key_wnd_geometry = "WindowGeometry";
static const char * g_settings_key_ul_dir = "ULDirectory";
static const char * g_settings_key_iso_recover_dir = "ISORecoverDirectory";
static const char * g_settings_key_cover_dir = "PixmapDirectory";

class GameTreeItem : public QTreeWidgetItem
{
public:
    GameTreeItem(const GameCollection & _collection, const QString & _game_id) :
        mr_collection(_collection),
        m_game_id(_game_id)
    {
        mp_game = _collection.game(_game_id);
    }

    QVariant data(int _column, int _role) const override;

    void reload()
    {
        mp_game = mr_collection.game(m_game_id);
    }

    const Game & game() const
    {
        return *mp_game;
    }

private:
    const GameCollection & mr_collection;
    const QString m_game_id;
    const Game * mp_game;
};

QVariant GameTreeItem::data(int _column, int _role) const
{
    if(_role == Qt::DisplayRole)
    {
        switch(_column)
        {
        case 0:
            return mp_game->title;
        case 1:
            return mp_game->id;
        case 2:
            switch(mp_game->media_type)
            {
            case MediaType::CD:
                return "CD";
            case MediaType::DVD:
                return "DVD";
            default:
                return QObject::tr("Unknown");
            }
        case 3:
            return mp_game->part_count;
        default:
            break;
        }
    }
    return QTreeWidgetItem::data(_column, _role);
}

} // namespace

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    mp_tree_games->header()->setStretchLastSection(false);
    mp_tree_games->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mp_widget_game_details->setVisible(false);
    mp_label_current_root = new QLabel(mp_statusbar);
    mp_statusbar->addWidget(mp_label_current_root);
    activateFileActions(false);
    activateGameActions(nullptr);
    QSettings settings;
    restoreGeometry(settings.value(g_settings_key_wnd_geometry).toByteArray());
    if(Settings::instance().reopenLastSestion())
    {
        QDir directory(settings.value(g_settings_key_ul_dir).toString());
        if(directory.exists())
            loadGameCollection(directory);
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

void MainWindow::loadGameCollection()
{
    QSettings settings;
    QString dirpath = settings.value(g_settings_key_ul_dir).toString();
    dirpath = QFileDialog::getExistingDirectory(this, tr("Choose the OPL root dir"), dirpath);
    if(dirpath.isEmpty()) return;
    settings.setValue(g_settings_key_ul_dir, dirpath);
    loadGameCollection(dirpath);
}

void MainWindow::loadGameCollection(const QDir & _directory)
{
    mp_tree_games->clear();
    mp_label_cover->clear();
    mp_label_icon->clear();
    try
    {
        m_game_collection.reload(_directory);
        for(const Game & game : m_game_collection.games())
            mp_tree_games->addTopLevelItem(new GameTreeItem(m_game_collection, game.id));
        mp_label_current_root->setText(m_game_collection.directory());
        mp_tree_games->setCurrentItem(mp_tree_games->topLevelItem(0));
        activateFileActions(true);
    }
    catch(const Exception & exception)
    {
        mp_label_current_root->clear();
        activateFileActions(false);
        QMessageBox::critical(this, QString(), exception.message());
    }
}

void MainWindow::reloadUlConfig()
{
    loadGameCollection(m_game_collection.directory());
}

void MainWindow::activateFileActions(bool _activate)
{
    mp_action_add_game->setEnabled(_activate);
    mp_action_reload_file->setEnabled(_activate);
}

void MainWindow::activateGameActions(const Game * _selected_game)
{
    mp_action_delete_game->setEnabled(_selected_game != nullptr);
    mp_action_rename_game->setEnabled(_selected_game != nullptr);
    mp_action_to_iso->setEnabled(_selected_game != nullptr &&
        _selected_game->installation_type != GameInstallationType::Directory);
}

void MainWindow::addGame()
{
    try
    {
        GameInstallDialog dlg(m_game_collection, this);
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
    GameTreeItem * item = new GameTreeItem(m_game_collection, _id);
    mp_tree_games->addTopLevelItem(item);
    mp_tree_games->setCurrentItem(item);
}

void MainWindow::gameToIso()
{
    GameTreeItem * item = static_cast<GameTreeItem *>(mp_tree_games->currentItem());
    if(item == nullptr) return;
    const Game & game = item->game();
    QSettings settings;
    QString iso_dir = settings.value(g_settings_key_iso_recover_dir).toString();
    QString iso_filename;
    if(iso_dir.isEmpty())
        iso_filename = game.title + ".iso";
    else
        iso_filename = QDir(iso_dir).absoluteFilePath(game.title + ".iso");
    iso_filename = QFileDialog::getSaveFileName(this, tr("Choose an ISO image filename to save"),
        iso_filename, tr("ISO Image") + " (*.iso)");
    if(iso_filename.isEmpty())
        return;
    settings.setValue(g_settings_key_iso_recover_dir, QFileInfo(iso_filename).absoluteDir().absolutePath());
    IsoRecoverDialog dlg(game, settings.value(g_settings_key_ul_dir).toString(), iso_filename, this);
    dlg.exec();
}

void MainWindow::deleteGame()
{
    GameTreeItem * item = static_cast<GameTreeItem *>(mp_tree_games->currentItem());
    if(item == nullptr) return;
    if(Settings::instance().confirmGameDeletion() &&
       QMessageBox::question(this, QString(),
       tr("Are you sure you want to delete the game?") + "\r\n" + item->game().title) != QMessageBox::Yes)
    {
        return;
    }
    try
    {
        m_game_collection.deleteGame(item->game().id);
        delete item;
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}

void MainWindow::setCover()
{
    GameTreeItem * item = static_cast<GameTreeItem *>(mp_tree_games->currentItem());
    if(item == nullptr) return;
    QString filename = getOpenPicturePath(tr("Choose the game cover"));
    if(filename.isEmpty()) return;
    try
    {
        m_game_collection.setGameCover(item->game().id, filename);
        gameSelectionChanged();
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
        GameTreeItem * item = static_cast<GameTreeItem *>(mp_tree_games->currentItem());
        if(item == nullptr) return;
        if(Settings::instance().confirmPixmapDeletion() &&
           QMessageBox::question(this, QString(), tr("Are you sure you want to delete the game cover?")) != QMessageBox::Yes)
        {
            return;
        }
        m_game_collection.removeGameCover(item->game().id);
        gameSelectionChanged();
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}

void MainWindow::setIcon()
{
    GameTreeItem * item = static_cast<GameTreeItem *>(mp_tree_games->currentItem());
    if(item == nullptr) return;
    QString filename = getOpenPicturePath(tr("Choose the game icon"));
    if(filename.isEmpty()) return;
    try
    {
        m_game_collection.setGameIcon(item->game().id, filename);
        gameSelectionChanged();
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
        GameTreeItem * item = static_cast<GameTreeItem *>(mp_tree_games->currentItem());
        if(item == nullptr) return;
        if(Settings::instance().confirmPixmapDeletion() &&
           QMessageBox::question(this, QString(), tr("Are you sure you want to delete the game icon?")) != QMessageBox::Yes)
        {
            return;
        }
        m_game_collection.removeGameIcon(item->game().id);
        gameSelectionChanged();
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}

void MainWindow::gameSelectionChanged()
{
    GameTreeItem * item = static_cast<GameTreeItem *>(mp_tree_games->currentItem());
    if(item == nullptr)
    {
        mp_widget_game_details->setVisible(false);
        activateGameActions(nullptr);
        return;
    }
    const Game & game = item->game();
    mp_label_cover->setPixmap(game.cover);
    mp_label_icon->setPixmap(game.icon);
    mp_widget_game_details->setVisible(true);
    activateGameActions(&game);
}

void MainWindow::renameGame()
{
    GameTreeItem * item = static_cast<GameTreeItem *>(mp_tree_games->currentItem());
    if(item == nullptr) return;
    const Game & game = item->game();
    try
    {
        GameRenameDialog dlg(game.title, game.installation_type, this);
        if(dlg.exec() == QDialog::Accepted)
        {
            QString new_name = dlg.name();
            m_game_collection.renameGame(game.id, new_name);
            item->reload();
            mp_tree_games->update(mp_tree_games->currentIndex());
            gameSelectionChanged();
        }
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}
