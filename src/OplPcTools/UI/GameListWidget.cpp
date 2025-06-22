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

#include <QLinkedList>
#include <QShortcut>
#include <QMessageBox>
#include <QFileDialog>
#include <QAbstractItemModel>
#include <OplPcTools/Settings.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/GameConfiguration.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/GameDetailsActivity.h>
#include <OplPcTools/UI/IsoRestorerActivity.h>
#include <OplPcTools/UI/GameInstallerActivity.h>
#include <OplPcTools/UI/GameRenameDialog.h>
#include <OplPcTools/UI/BusySmartThread.h>
#include <OplPcTools/UI/GameListWidget.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

class GameListWidget::GameTreeModel : public QAbstractItemModel
{
public:
    explicit GameTreeModel(QObject * _parent = nullptr);
    QModelIndex index(int _row, int _column, const QModelIndex & _parent) const override;
    QModelIndex parent(const QModelIndex & _child) const override;
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    const Game * game(const QModelIndex & _index) const;
    void setArtManager(GameArtManager & _manager);

private:
    void updateUuids();
    void onLibraryLoaded();
    void onGameAdded(const Uuid & _uuid);
    void onGameAboutToBeDeleted(const Uuid & _uuid);
    void onGameDeleted(const Uuid & _uuid);
    void updateRecord(const Uuid & _uuid);
    void updateRecords(const QString & _game_id);
    void onGameArtChanged(const QString & _game_id, GameArtType _type);

private:
    const QPixmap m_default_icon;
    const GameCollection & mr_game_collection;
    GameArtManager * mp_art_manager;
    QVector<Uuid> m_uuids;
};


GameListWidget::GameTreeModel::GameTreeModel(QObject * _parent /*= nullptr*/) :
    QAbstractItemModel(_parent),
    m_default_icon(QPixmap(":/images/no-icon")),
    mr_game_collection(Library::instance().games()),
    mp_art_manager(nullptr)
{
    updateUuids();
    connect(&Library::instance(), &Library::loaded, this, &GameListWidget::GameTreeModel::onLibraryLoaded);
    connect(&mr_game_collection, &GameCollection::gameRenamed, this, &GameListWidget::GameTreeModel::updateRecord);
    connect(&mr_game_collection, &GameCollection::gameAdded, this, &GameListWidget::GameTreeModel::onGameAdded);
    connect(&mr_game_collection, &GameCollection::gameAboutToBeDeleted, this, &GameListWidget::GameTreeModel::onGameAboutToBeDeleted);
}

void GameListWidget::GameTreeModel::updateUuids()
{
    m_uuids.resize(mr_game_collection.count());
    for(int i = m_uuids.size() - 1; i >= 0; --i)
        m_uuids[i] = mr_game_collection[i]->uuid();
}

void GameListWidget::GameTreeModel::onLibraryLoaded()
{
    beginResetModel();
    updateUuids();
    endResetModel();
}

void GameListWidget::GameTreeModel::onGameAdded(const Uuid & _uuid)
{
    int row_number = m_uuids.size();
    beginInsertRows(QModelIndex(), row_number, row_number);
    m_uuids.append(_uuid);
    endInsertRows();
}

void GameListWidget::GameTreeModel::onGameAboutToBeDeleted(const Uuid & _uuid)
{   
    for(int i = m_uuids.size() - 1; i >= 0; --i)
    {
        if(m_uuids[i] == _uuid)
        {
            beginRemoveRows(QModelIndex(), i, i);
            return;
        }
    }
    m_uuids.removeOne(_uuid);
    endRemoveRows();
}

void GameListWidget::GameTreeModel::updateRecord(const Uuid & _uuid)
{
    for(int i = m_uuids.size() - 1; i >= 0; --i)
    {
        if(m_uuids[i] == _uuid)
        {
            emit dataChanged(createIndex(i, 0), createIndex(i, 0));
            return;
        }
    }
}

void GameListWidget::GameTreeModel::updateRecords(const QString & _game_id)
{
    QLinkedList<Uuid> uuids;
    for(int i = mr_game_collection.count() - 1; i >= 0; --i)
    {
        const Game * game = mr_game_collection[i];
        if(game && game->id() == _game_id)
            uuids.append(game->uuid());
    }
    for(int i = m_uuids.size() - 1; i >= 0; --i)
    {
        if(uuids.contains(m_uuids[i]))
            emit dataChanged(createIndex(i, 0), createIndex(i, 0));
    }
}

void GameListWidget::GameTreeModel::onGameArtChanged(const QString & _game_id, GameArtType _type)
{
    if(_type == GameArtType::Icon)
        updateRecords(_game_id);
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
    return m_uuids.size();
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
        return mr_game_collection.findGame(m_uuids[_index.row()])->title();
    case Qt::DecorationRole:
        if(mp_art_manager)
        {
            const Game * game = mr_game_collection.findGame(m_uuids[_index.row()]);
            QPixmap icon = mp_art_manager->load(game->id(), GameArtType::Icon);
            return QIcon(icon.isNull() ? m_default_icon : icon);
        }
        break;
    }
    return QVariant();
}

const Game * GameListWidget::GameTreeModel::game(const QModelIndex & _index) const
{
    return _index.isValid() ? mr_game_collection.findGame(m_uuids[_index.row()]) : nullptr;
}

void GameListWidget::GameTreeModel::setArtManager(GameArtManager & _manager)
{
    mp_art_manager = &_manager;
    connect(mp_art_manager, &GameArtManager::artChanged, this, &GameTreeModel::onGameArtChanged);
}

GameListWidget::GameListWidget(QWidget * _parent /*= nullptr*/) :
    QWidget(_parent),
    mp_game_art_manager(nullptr),
    mp_model(nullptr),
    mp_context_menu(nullptr),
    mp_proxy_model(nullptr)
{
    setupUi(this);
    setupShortcuts();
    m_default_cover = QPixmap(":/images/no-image")
        .scaled(mp_label_cover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    mp_model = new GameTreeModel(this);
    mp_proxy_model = new QSortFilterProxyModel(this);
    mp_proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mp_proxy_model->setSortCaseSensitivity(Qt::CaseInsensitive);
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
    activateCollectionControls(false);
    activateItemControls(nullptr);
    connect(&Settings::instance(), &Settings::iconSizeChanged, this, &GameListWidget::setIconSize);
    connect(&Library::instance(), &Library::loaded, this, &GameListWidget::onLibraryLoaded);
    connect(&Library::instance().games(), &GameCollection::gameAdded, this, &GameListWidget::onGameAdded);
    connect(&Library::instance().games(), &GameCollection::gameRenamed, this, &GameListWidget::onGameRenamed);
    connect(mp_action_edit, &QAction::triggered, this, &GameListWidget::showGameDetails);
    connect(mp_action_rename, &QAction::triggered, this, &GameListWidget::renameGame);
    connect(mp_action_delete, &QAction::triggered, this, &GameListWidget::deleteGame);
    connect(mp_action_install, &QAction::triggered, this, &GameListWidget::showGameInstaller);
    connect(mp_action_restore_iso, &QAction::triggered, this, &GameListWidget::showIsoRestorer);
    connect(mp_tree_games, &QTreeView::activated, [this](const QModelIndex &) { showGameDetails(); });
    connect(mp_tree_games, &QTreeView::customContextMenuRequested, this, &GameListWidget::showTreeContextMenu);
    connect(mp_tree_games->selectionModel(), &QItemSelectionModel::selectionChanged, [this](QItemSelection, QItemSelection) { onGameSelected(); });
    connect(mp_edit_filter, &QLineEdit::textChanged, mp_proxy_model, &QSortFilterProxyModel::setFilterFixedString);
    setIconSize();
    onLibraryLoaded();
}

void GameListWidget::setupShortcuts()
{
    QShortcut * filter_shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    mp_edit_filter->setPlaceholderText(QString("%1 (%2)")
        .arg(mp_edit_filter->placeholderText())
        .arg(filter_shortcut->key().toString()));
    connect(filter_shortcut, &QShortcut::activated, [this]() { mp_edit_filter->setFocus(); });
}

void GameListWidget::setIconSize()
{
    QSize size;
    size.setWidth(Settings::instance().iconSize());
    size.setHeight(size.width());
    mp_tree_games->setIconSize(size);
}

void GameListWidget::showTreeContextMenu(const QPoint & _point)
{
    if(Library::instance().games().isLoaded())
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

void GameListWidget::onLibraryLoaded()
{
    try
    {
        const QDir directory (Library::instance().directory());
        delete mp_game_art_manager;
        mp_game_art_manager = new GameArtManager(directory, this);
        connect(mp_game_art_manager, &GameArtManager::artChanged, this, &GameListWidget::onGameArtChanged);
        mp_game_art_manager->addCacheType(GameArtType::Icon);
        mp_game_art_manager->addCacheType(GameArtType::Front);
        mp_model->setArtManager(*mp_game_art_manager);
        mp_proxy_model->sort(0, Qt::AscendingOrder);
        if(Library::instance().games().count() > 0)
            mp_tree_games->setCurrentIndex(mp_proxy_model->index(0, 0));
        activateCollectionControls(true);
        onGameSelected();
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

void GameListWidget::onGameAdded(const Uuid & _uuid)
{
    Q_UNUSED(_uuid)
    QModelIndex index = mp_tree_games->currentIndex();
    if(!index.isValid())
        index = mp_proxy_model->index(0, 0);
    mp_tree_games->setCurrentIndex(index);
}

void GameListWidget::onGameRenamed(const Uuid & _uuid)
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game && game->uuid() == _uuid)
        onGameSelected();
}

void GameListWidget::onGameArtChanged(const QString & _game_id, GameArtType _type)
{
    if(_type != GameArtType::Front)
        return;
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game && game->id() == _game_id)
    {
        QPixmap pixmap = mp_game_art_manager->load(_game_id, GameArtType::Front);
        mp_label_cover->setPixmap(pixmap.isNull() ? m_default_cover : pixmap);
    }
}

void GameListWidget::onGameSelected()
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
                Library::instance().games().renameGame(*game, dlg.name());
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
    }
}

void GameListWidget::showGameDetails()
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game)
    {
        QSharedPointer<Intent> intent = GameDetailsActivity::createIntent(*mp_game_art_manager, game->uuid());
        Application::pushActivity(*intent);
    }
}

void GameListWidget::showIsoRestorer()
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game && game->installationType() == GameInstallationType::UlConfig)
    {
        QSharedPointer<Intent> intent = IsoRestorerActivity::createIntent(game->uuid());
        Application::pushActivity(*intent);
    }
}

void GameListWidget::showGameInstaller()
{
    QSharedPointer<Intent> intent = GameInstallerActivity::createIntent();
    Application::pushActivity(*intent);
}

void GameListWidget::deleteGame()
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(!game) return;
    Settings & settings = Settings::instance();
    if(settings.confirmGameDeletion())
    {
        QCheckBox * checkbox = new QCheckBox(tr("Do not ask again"));
        QMessageBox message_box(QMessageBox::Question, tr("Delete Game"),
            QString("%1\n%2")
                .arg(tr("Are you sure you want to delete this game?"))
                .arg(game->title()),
            QMessageBox::Yes | QMessageBox::No);
        message_box.setDefaultButton(QMessageBox::Yes);
        message_box.setCheckBox(checkbox);
        if(message_box.exec() != QMessageBox::Yes)
            return;
        if(checkbox->isChecked())
            settings.setConfirmGameDeletion(false);
    }
    startBusyThread([this, game]() {
        GameCollection & game_collection = Library::instance().games();
        const QString game_id = game->id();
        game_collection.deleteGame(*game);
        if(!game_collection.contains(game_id))
        {
            mp_game_art_manager->clearArts(game_id);
            QFile config(GameConfiguration::makeFilename(Library::instance().directory(), game_id));
            if(config.exists())
                config.remove();
        }
    });
}

void GameListWidget::startBusyThread(std::function<void()> _lambda)
{
    BusySmartThread * thread = new BusySmartThread(_lambda, this);
    connect(thread, &BusySmartThread::finished, thread, &BusySmartThread::deleteLater);
    connect(thread, &BusySmartThread::exception, [](const QString message) {
        Application::showErrorMessage(message);
    });
    thread->start();
}
