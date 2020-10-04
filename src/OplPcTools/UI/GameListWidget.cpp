/***********************************************************************************************
 * Copyright © 2017-2020 Sergey Smolyannikov aka brainstream                                   *
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
#include <QFileDialog>
#include <QAbstractItemModel>
#include <OplPcTools/Settings.h>
#include <OplPcTools/GameManager.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/GameDetailsActivity.h>
#include <OplPcTools/UI/IsoRestorerActivity.h>
#include <OplPcTools/UI/GameInstallerActivity.h>
#include <OplPcTools/UI/GameRenameDialog.h>
#include <OplPcTools/UI/GameListWidget.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

class GameListWidget::GameTreeModel : public QAbstractItemModel
{
public:
    explicit GameTreeModel(GameManager & _manager, QObject * _parent = nullptr);
    QModelIndex index(int _row, int _column, const QModelIndex & _parent) const override;
    QModelIndex parent(const QModelIndex & _child) const override;
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    const Game * game(const QModelIndex & _index) const;
    void setArtManager(GameArtManager & _manager);

private:
    void collectionLoaded();
    void gameAdded(const QString & _id);
    void gameAboutToBeDeleted(const QString & _id);
    void gameDeleted(const QString & _id);
    void updateRecord(const QString & _id);
    void gameArtChanged(const QString & _game_id, GameArtType _type, const QPixmap * _pixmap);

private:
    const QPixmap m_default_icon;
    const GameManager & mr_manager;
    GameArtManager * mp_art_manager;
    int m_row_count;
};


GameListWidget::GameTreeModel::GameTreeModel(GameManager & _manager, QObject * _parent /*= nullptr*/) :
    QAbstractItemModel(_parent),
    m_default_icon(QPixmap(":/images/no-icon")),
    mr_manager(_manager),
    mp_art_manager(nullptr),
    m_row_count(_manager.count())
{
    connect(&_manager, &GameManager::loaded, this, &GameListWidget::GameTreeModel::collectionLoaded);
    connect(&_manager, &GameManager::gameRenamed, this, &GameListWidget::GameTreeModel::updateRecord);
    connect(&_manager, &GameManager::gameAdded, this, &GameListWidget::GameTreeModel::gameAdded);
    connect(&_manager, &GameManager::gameAboutToBeDeleted, this, &GameListWidget::GameTreeModel::gameAboutToBeDeleted);
    connect(&_manager, &GameManager::gameDeleted, this, &GameListWidget::GameTreeModel::gameDeleted);
}

void GameListWidget::GameTreeModel::collectionLoaded()
{
    beginResetModel();
    m_row_count = mr_manager.count();
    endResetModel();
}

void GameListWidget::GameTreeModel::gameAdded(const QString & _id)
{
    Q_UNUSED(_id);
    beginInsertRows(QModelIndex(), m_row_count, m_row_count);
    ++m_row_count;
    endInsertRows();
}

void GameListWidget::GameTreeModel::gameAboutToBeDeleted(const QString & _id)
{
    for(int i = 0; i < m_row_count; ++i)
    {
        const Game * game = mr_manager[i];
        if(game->id() == _id)
        {
            beginRemoveRows(QModelIndex(), i, i);
            return;
        }
    }
}

void GameListWidget::GameTreeModel::gameDeleted(const QString & _id)
{
    Q_UNUSED(_id);
    m_row_count = mr_manager.count();
    endRemoveRows();
}

void GameListWidget::GameTreeModel::updateRecord(const QString & _id)
{
    int count = mr_manager.count();
    for(int i = 0; i < count; ++i)
    {
        const Game * game = mr_manager[i];
        if(game->id() == _id)
        {
            emit dataChanged(createIndex(i, 0), createIndex(i, 0));
            return;
        }
    }
}

void GameListWidget::GameTreeModel::gameArtChanged(const QString & _game_id, GameArtType _type, const QPixmap * _pixmap)
{
    Q_UNUSED(_pixmap)
    if(_type == GameArtType::Icon)
        updateRecord(_game_id);
}

QModelIndex GameListWidget::GameTreeModel::index(int _row, int _column, const QModelIndex & _parent) const
{
    Q_UNUSED(_parent)
    return createIndex(_row, _column);
}

QModelIndex GameListWidget::GameTreeModel::parent(const QModelIndex & _child) const
{
    Q_UNUSED(_child)
    return QModelIndex();
}

int GameListWidget::GameTreeModel::rowCount(const QModelIndex & _parent) const
{
    if(_parent.isValid())
        return 0;
    return m_row_count;
}

int GameListWidget::GameTreeModel::columnCount(const QModelIndex & _parent) const
{
    Q_UNUSED(_parent)
    return 1;
}

QVariant GameListWidget::GameTreeModel::data(const QModelIndex & _index, int _role) const
{
    switch(_role)
    {
    case Qt::DisplayRole:
        return mr_manager[_index.row()]->title();
    case Qt::DecorationRole:
        if(mp_art_manager)
        {
            QPixmap icon = mp_art_manager->load(mr_manager[_index.row()]->id(), GameArtType::Icon);
            return QIcon(icon.isNull() ? m_default_icon : icon);
        }
        break;
    }
    return QVariant();
}

const Game * GameListWidget::GameTreeModel::game(const QModelIndex & _index) const
{
    return _index.isValid() ? mr_manager[_index.row()] : nullptr;
}

void GameListWidget::GameTreeModel::setArtManager(GameArtManager & _manager)
{
    if(mp_art_manager)
        disconnect(mp_art_manager, &GameArtManager::artChanged, this, &GameTreeModel::gameArtChanged);
    mp_art_manager = &_manager;
    connect(mp_art_manager, &GameArtManager::artChanged, this, &GameTreeModel::gameArtChanged);
}

GameListWidget::GameListWidget(QWidget * _parent /*= nullptr*/) :
    QWidget(_parent),
    mp_game_manager(nullptr),
    mp_game_art_manager(nullptr),
    mp_model(nullptr),
    mp_context_menu(nullptr),
    mp_proxy_model(nullptr)
{
    setupUi(this);
    mp_game_manager = &Application::instance().library().games();
    QShortcut * filter_shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    mp_edit_filter->setPlaceholderText(QString("%1 (%2)")
        .arg(mp_edit_filter->placeholderText())
        .arg(filter_shortcut->key().toString()));
    m_default_cover = QPixmap(":/images/no-image")
        .scaled(mp_label_cover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    mp_model = new GameTreeModel(*mp_game_manager, this);
    mp_proxy_model = new QSortFilterProxyModel(this);
    mp_proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mp_proxy_model->setSourceModel(mp_model);
    mp_proxy_model->setDynamicSortFilter(true);
    mp_tree_games->setModel(mp_proxy_model);
    mp_btn_rename->setDefaultAction(mp_action_rename);
    mp_btn_edit->setDefaultAction(mp_action_edit);
    mp_btn_delete->setDefaultAction(mp_action_delete);
    mp_btn_install->setDefaultAction(mp_action_install);
    mp_btn_resore_iso->setDefaultAction(mp_action_restore_iso);
    mp_context_menu = new QMenu(this);
    mp_context_menu->addAction(mp_action_rename);
    mp_context_menu->addAction(mp_action_edit);
    mp_context_menu->addAction(mp_action_restore_iso);
    mp_context_menu->addAction(mp_action_delete);
    mp_context_menu->addSeparator();
    mp_context_menu->addAction(mp_action_install);
    mp_tree_games->setContextMenuPolicy(Qt::CustomContextMenu);
    activateCollectionControls(false);
    activateItemControls(nullptr);
    connect(&Application::instance().library(), &Library::loaded, this, &GameListWidget::load);
    connect(filter_shortcut, &QShortcut::activated, [this]() { mp_edit_filter->setFocus(); });
    connect(mp_action_edit, &QAction::triggered, this, &GameListWidget::showGameDetails);
    connect(mp_action_rename, &QAction::triggered, this, &GameListWidget::renameGame);
    connect(mp_action_delete, &QAction::triggered, this, &GameListWidget::deleteGame);
    connect(mp_action_install, &QAction::triggered, this, &GameListWidget::showGameInstaller);
    connect(mp_action_restore_iso, &QAction::triggered, this, &GameListWidget::showIsoRestorer);
    connect(mp_tree_games, &QTreeView::doubleClicked, [this](const QModelIndex &) { showGameDetails(); });
    connect(mp_tree_games, &QTreeView::customContextMenuRequested, this, &GameListWidget::showTreeContextMenu);
    connect(mp_tree_games->selectionModel(), &QItemSelectionModel::selectionChanged, [this](QItemSelection, QItemSelection) { gameSelected(); });
    connect(mp_game_manager, &GameManager::loaded, this, &GameListWidget::collectionLoaded);
    connect(mp_game_manager, &GameManager::gameAdded, this, &GameListWidget::gameAdded);
    connect(mp_game_manager, &GameManager::gameRenamed, this, &GameListWidget::gameRenamed);
    connect(mp_edit_filter, &QLineEdit::textChanged, mp_proxy_model, &QSortFilterProxyModel::setFilterFixedString);
}

void GameListWidget::showTreeContextMenu(const QPoint & _point)
{
    if(mp_game_manager->isLoaded())
        mp_context_menu->exec(mp_tree_games->mapToGlobal(_point));
}

void GameListWidget::activateCollectionControls(bool _activate)
{
    mp_action_install->setEnabled(_activate);
}

void GameListWidget::activateItemControls(const Game * _selected_game)
{
    mp_widget_details->setVisible(_selected_game);
    mp_action_delete->setEnabled(_selected_game);
    mp_action_edit->setEnabled(_selected_game);
    mp_action_rename->setEnabled(_selected_game);
    mp_action_restore_iso->setEnabled(_selected_game && _selected_game->installationType() == GameInstallationType::UlConfig);
}

bool GameListWidget::tryLoadRecentDirectory()
{
//    QSettings settings;
//    QVariant value = settings.value(SettingsKey::ul_dir);
//    if(!value.isValid()) return false;
//    QDir dir(value.toString());
//    if(!dir.exists()) return false;
//    loadDirectory(dir);
    return true;
}

void GameListWidget::load()
{
    try
    {
        const QDir & directory = Application::instance().library().directory();
        delete mp_game_art_manager;
        mp_game_art_manager = new GameArtManager(directory, this);
        connect(mp_game_art_manager, &GameArtManager::artChanged, this, &GameListWidget::gameArtChanged);
        mp_game_art_manager->addCacheType(GameArtType::Icon);
        mp_game_art_manager->addCacheType(GameArtType::Front);
        mp_model->setArtManager(*mp_game_art_manager);
        mp_proxy_model->sort(0, Qt::AscendingOrder);
        if(mp_game_manager->count() > 0)
            mp_tree_games->setCurrentIndex(mp_proxy_model->index(0, 0));
    }
    catch(const Exception & exception)
    {
        Application::instance().showErrorMessage(exception.message());
    }
    catch(...)
    {
        Application::instance().showErrorMessage();
    }
}

void GameListWidget::collectionLoaded()
{
    activateCollectionControls(true);
    gameSelected();
}

void GameListWidget::gameAdded(const QString & _id)
{
    Q_UNUSED(_id)
    QModelIndex index = mp_tree_games->currentIndex();
    if(!index.isValid())
        index = mp_proxy_model->index(0, 0);
    mp_tree_games->setCurrentIndex(index);
}

void GameListWidget::gameRenamed(const QString & _id)
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game && game->id() == _id)
        gameSelected();
}

void GameListWidget::gameArtChanged(const QString & _game_id, GameArtType _type, const QPixmap * _pixmap)
{
    if(_type != GameArtType::Front)
        return;
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game && game->id() == _game_id)
        mp_label_cover->setPixmap(_pixmap ? *_pixmap : m_default_cover);
}

void GameListWidget::gameSelected()
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game)
    {
        mp_label_id->setText(game->id());
        mp_label_title->setText(game->title());
        QPixmap pixmap = mp_game_art_manager->load(game->id(), GameArtType::Front);
        mp_label_cover->setPixmap(pixmap.isNull() ? m_default_cover : pixmap);
        mp_label_type->setText(game->mediaType() == MediaType::CD ? "CD" : "DVD");
        mp_label_parts->setText(QString("%1").arg(game->partCount()));
        mp_label_source->setText(
            game->installationType() == GameInstallationType::UlConfig ? "UL" : tr("Directory"));
        mp_widget_details->show();
    }
    else
    {
        mp_widget_details->hide();
    }
    activateItemControls(game);
}

void GameListWidget::renameGame()
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game)
    {
        GameRenameDialog dlg(game->title(), game->installationType(), this);
        if(dlg.exec() == QDialog::Accepted)
        {
            try
            {
                mp_game_manager->renameGame(*game, dlg.name());
            }
            catch(const Exception & exception)
            {
                Application::instance().showErrorMessage(exception.message());
            }
            catch(...)
            {
                Application::instance().showErrorMessage();
            }
        }
    }
}

void GameListWidget::showGameDetails()
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game)
    {
        QSharedPointer<Intent> intent = GameDetailsActivity::createIntent(*mp_game_manager, *mp_game_art_manager, game->id());
        Application::instance().pushActivity(*intent);
    }
}

void GameListWidget::showIsoRestorer()
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game && game->installationType() == GameInstallationType::UlConfig)
    {
        QSharedPointer<Intent> intent = IsoRestorerActivity::createIntent(*mp_game_manager, game->id());
        Application::instance().pushActivity(*intent);
    }
}

void GameListWidget::showGameInstaller()
{
    QSharedPointer<Intent> intent = GameInstallerActivity::createIntent(*mp_game_manager);
    Application::instance().pushActivity(*intent);
}

void GameListWidget::deleteGame()
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(!game) return;
    const QString id = game->id();
    Settings & settings = Settings::instance();
    if(settings.flag(Settings::Flag::ConfirmGameDeletion))
    {
        QCheckBox * checkbox = new QCheckBox(tr("Don't show again"));
        QMessageBox message_box(QMessageBox::Question, tr("Remove Game"),
                    tr("The %1 will be deleted.\nContinue?").arg(game->title()),
                    QMessageBox::Yes | QMessageBox::No);
        message_box.setDefaultButton(QMessageBox::Yes);
        message_box.setCheckBox(checkbox);
        if(message_box.exec() != QMessageBox::Yes)
            return;
        if(checkbox->isChecked())
            settings.setFlag(Settings::Flag::ConfirmGameDeletion, false);
    }
    try
    {
        mp_game_manager->deleteGame(*game);
        mp_game_art_manager->clearArts(id);
    }
    catch(Exception & exception)
    {
        Application::instance().showErrorMessage(exception.message());
    }
    catch(...)
    {
        Application::instance().showErrorMessage();
    }
}
