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
#include "UlConfig.h"
#include "Game.h"
#include "GameInstaller.h"
#include "Iso9660GameInstallerSource.h"
#include "GameInstallDialog.h"
#include "GameRenameDialog.h"
#include "Exception.h"

namespace {

static const char * g_settings_key_wnd_geometry = "wndgeom";
static const char * g_settings_key_ul_dir = "uldir";

class GameListItem : public QListWidgetItem
{
public:
    explicit GameListItem(const UlConfigRecord & _config_record) :
        m_config_record(_config_record)
    {
    }

    QVariant data(int _role) const;

    UlConfigRecord & configRecord()
    {
        return m_config_record;
    }

private:
    UlConfigRecord m_config_record;
};

QVariant GameListItem::data(int _role) const
{
    if(_role == Qt::DisplayRole)
        return m_config_record.name;
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
}

void MainWindow::closeEvent(QCloseEvent * _event)
{
    Q_UNUSED(_event)
    QSettings settings;
    settings.setValue(g_settings_key_wnd_geometry, saveGeometry());
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
        m_config_ptr = UlConfig::load(_directory);
        for(const auto & record : m_config_ptr->records())
            mp_list_games->addItem(new GameListItem(record));
        mp_label_current_ul_file->setText(m_config_ptr->file());
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
    loadUlConfig(m_config_ptr->directory());
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
        GameInstallDialog dlg(*m_config_ptr, this);
        if(dlg.exec() == QDialog::Accepted)
            reloadUlConfig();
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}

void MainWindow::deleteGame()
{
    GameListItem * item = static_cast<GameListItem *>(mp_list_games->currentItem());
    if(item == nullptr) return;
    UlConfigRecord & config_record = item->configRecord();
    try
    {
        Game(*m_config_ptr, config_record.image).remove();
        delete item;
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
    const UlConfigRecord & record = item->configRecord();
    mp_label_game_id->setText(record.image);
    mp_label_game_title->setText(record.name);
    mp_label_game_parts->setText(QString("%1").arg(record.parts));
    switch(record.type)
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
    mp_widget_game_details->setVisible(true);
    activateGameActions(true);
}

void MainWindow::renameGame()
{
    GameListItem * item = static_cast<GameListItem *>(mp_list_games->currentItem());
    if(item == nullptr) return;
    UlConfigRecord & config_record = item->configRecord();
    try
    {
        GameRenameDialog dlg(config_record.name, this);
        if(dlg.exec() == QDialog::Accepted)
        {
            QString new_name = dlg.name();
            Game(*m_config_ptr, config_record.image).rename(new_name);
            config_record.name = new_name;
            mp_list_games->update(mp_list_games->currentIndex());
            gameSelected(item);
        }
    }
    catch(const Exception & exception)
    {
        QMessageBox::critical(this, QString(), exception.message());
    }
}
