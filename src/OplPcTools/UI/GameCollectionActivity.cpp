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

#include <QShortcut>
#include <QMessageBox>
#include <QFileDialog>
#include <QAbstractItemModel>
#include <OplPcTools/Settings.h>
#include <OplPcTools/GameCollection.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/GameDetailsActivity.h>
#include <OplPcTools/UI/IsoRestorerActivity.h>
#include <OplPcTools/UI/GameCollectionActivity.h>
#include <OplPcTools/UI/GameInstallerActivity.h>
#include <OplPcTools/UI/GameRenameDialog.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

namespace SettingsKey {

const char * ul_dir     = "ULDirectory";
const char * icons_size = "GameListIconSize";

} // namespace SettingsKey

class GameCollectionActivityIntent : public Intent
{
public:
    Activity * createActivity(QWidget * _parent) override
    {
        return new GameCollectionActivity(_parent);
    }
};

} // namespace

class GameCollectionActivity::GameTreeModel : public QAbstractItemModel
{
public:
    explicit GameTreeModel(GameCollection & _collection, QObject * _parent = nullptr);
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
    const GameCollection & mr_collection;
    GameArtManager * mp_art_manager;
    int m_row_count;
};


GameCollectionActivity::GameTreeModel::GameTreeModel(GameCollection & _collection, QObject * _parent /*= nullptr*/) :
    QAbstractItemModel(_parent),
    m_default_icon(QPixmap(":/images/no-icon")),
    mr_collection(_collection),
    mp_art_manager(nullptr),
    m_row_count(_collection.count())
{
    connect(&_collection, &GameCollection::loaded, this, &GameCollectionActivity::GameTreeModel::collectionLoaded);
    connect(&_collection, &GameCollection::gameRenamed, this, &GameCollectionActivity::GameTreeModel::updateRecord);
    connect(&_collection, &GameCollection::gameAdded, this, &GameCollectionActivity::GameTreeModel::gameAdded);
    connect(&_collection, &GameCollection::gameAboutToBeDeleted, this, &GameCollectionActivity::GameTreeModel::gameAboutToBeDeleted);
    connect(&_collection, &GameCollection::gameDeleted, this, &GameCollectionActivity::GameTreeModel::gameDeleted);
}

void GameCollectionActivity::GameTreeModel::collectionLoaded()
{
    beginResetModel();
    m_row_count = mr_collection.count();
    endResetModel();
}

void GameCollectionActivity::GameTreeModel::gameAdded(const QString & _id)
{
    Q_UNUSED(_id);
    beginInsertRows(QModelIndex(), m_row_count, m_row_count);
    ++m_row_count;
    endInsertRows();
}

void GameCollectionActivity::GameTreeModel::gameAboutToBeDeleted(const QString & _id)
{
    for(int i = 0; i < m_row_count; ++i)
    {
        const Game * game = mr_collection[i];
        if(game->id() == _id)
        {
            beginRemoveRows(QModelIndex(), i, i);
            return;
        }
    }
}

void GameCollectionActivity::GameTreeModel::gameDeleted(const QString & _id)
{
    Q_UNUSED(_id);
    m_row_count = mr_collection.count();
    endRemoveRows();
}

void GameCollectionActivity::GameTreeModel::updateRecord(const QString & _id)
{
    int count = mr_collection.count();
    for(int i = 0; i < count; ++i)
    {
        const Game * game = mr_collection[i];
        if(game->id() == _id)
        {
            emit dataChanged(createIndex(i, 0), createIndex(i, 0));
            return;
        }
    }
}

void GameCollectionActivity::GameTreeModel::gameArtChanged(const QString & _game_id, GameArtType _type, const QPixmap * _pixmap)
{
    Q_UNUSED(_pixmap)
    if(_type == GameArtType::Icon)
        updateRecord(_game_id);
}

QModelIndex GameCollectionActivity::GameTreeModel::index(int _row, int _column, const QModelIndex & _parent) const
{
    Q_UNUSED(_parent)
    return createIndex(_row, _column);
}

QModelIndex GameCollectionActivity::GameTreeModel::parent(const QModelIndex & _child) const
{
    Q_UNUSED(_child)
    return QModelIndex();
}

int GameCollectionActivity::GameTreeModel::rowCount(const QModelIndex & _parent) const
{
    if(_parent.isValid())
        return 0;
    return m_row_count;
}

int GameCollectionActivity::GameTreeModel::columnCount(const QModelIndex & _parent) const
{
    Q_UNUSED(_parent)
    return 1;
}

QVariant GameCollectionActivity::GameTreeModel::data(const QModelIndex & _index, int _role) const
{
    switch(_role)
    {
    case Qt::DisplayRole:
        return mr_collection[_index.row()]->title();
    case Qt::DecorationRole:
        if(mp_art_manager)
        {
            QPixmap icon = mp_art_manager->load(mr_collection[_index.row()]->id(), GameArtType::Icon);
            return QIcon(icon.isNull() ? m_default_icon : icon);
        }
        break;
    }
    return QVariant();
}

const Game * GameCollectionActivity::GameTreeModel::game(const QModelIndex & _index) const
{
    return _index.isValid() ? mr_collection[_index.row()] : nullptr;
}

void GameCollectionActivity::GameTreeModel::setArtManager(GameArtManager & _manager)
{
    if(mp_art_manager)
        disconnect(mp_art_manager, &GameArtManager::artChanged, this, &GameTreeModel::gameArtChanged);
    mp_art_manager = &_manager;
    connect(mp_art_manager, &GameArtManager::artChanged, this, &GameTreeModel::gameArtChanged);
}

GameCollectionActivity::GameCollectionActivity(QWidget * _parent /*= nullptr*/) :
    Activity(_parent),
    mp_game_art_manager(nullptr),
    mp_model(nullptr),
    mp_context_menu(nullptr),
    mp_proxy_model(nullptr)
{
    setupUi(this);
    QShortcut * filter_shortcat = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    mp_edit_filter->setPlaceholderText(QString("%1 (%2)")
        .arg(mp_edit_filter->placeholderText())
        .arg(filter_shortcat->key().toString()));
    m_default_cover = QPixmap(":/images/no-image")
        .scaled(mp_label_cover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    GameCollection & game_collection = Application::instance().gameCollection();
    mp_model = new GameTreeModel(game_collection, this);
    mp_proxy_model = new QSortFilterProxyModel(this);
    mp_proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mp_proxy_model->setSourceModel(mp_model);
    mp_proxy_model->setDynamicSortFilter(true);
    mp_tree_games->setModel(mp_proxy_model);
    mp_btn_load->setDefaultAction(mp_action_load);
    mp_btn_reload->setDefaultAction(mp_action_reload);
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
    mp_context_menu->addAction(mp_action_reload);
    mp_tree_games->setContextMenuPolicy(Qt::CustomContextMenu);
    activateCollectionControls(false);
    activateItemControls(nullptr);
    connect(filter_shortcat, &QShortcut::activated, [this]() { mp_edit_filter->setFocus(); });
    connect(mp_slider_icons_size, &QSlider::valueChanged, [this](int) { changeIconsSize(); });
    connect(mp_action_load, &QAction::triggered, this, &GameCollectionActivity::load);
    connect(mp_action_reload, &QAction::triggered, this, &GameCollectionActivity::reload);
    connect(mp_action_edit, &QAction::triggered, this, &GameCollectionActivity::showGameDetails);
    connect(mp_action_rename, &QAction::triggered, this, &GameCollectionActivity::renameGame);
    connect(mp_action_delete, &QAction::triggered, this, &GameCollectionActivity::deleteGame);
    connect(mp_action_install, &QAction::triggered, this, &GameCollectionActivity::showGameInstaller);
    connect(mp_action_restore_iso, &QAction::triggered, this, &GameCollectionActivity::showIsoRestorer);
    connect(mp_tree_games, &QTreeView::doubleClicked, [this](const QModelIndex &) { showGameDetails(); });
    connect(mp_tree_games, &QTreeView::customContextMenuRequested, this, &GameCollectionActivity::showTreeContextMenu);
    connect(mp_tree_games->selectionModel(), &QItemSelectionModel::selectionChanged, [this](QItemSelection, QItemSelection) { gameSelected(); });
    connect(&game_collection, &GameCollection::loaded, this, &GameCollectionActivity::collectionLoaded);
    connect(&game_collection, &GameCollection::gameAdded, this, &GameCollectionActivity::gameAdded);
    connect(&game_collection, &GameCollection::gameRenamed, this, &GameCollectionActivity::gameRenamed);
    connect(this, &GameCollectionActivity::destroyed, this, &GameCollectionActivity::saveSettings);
    connect(mp_edit_filter, &QLineEdit::textChanged, mp_proxy_model, &QSortFilterProxyModel::setFilterFixedString);
    applySettings();
}

QSharedPointer<Intent> GameCollectionActivity::createIntent()
{
    return QSharedPointer<Intent>(new GameCollectionActivityIntent);
}

bool GameCollectionActivity::onAttach()
{
    if(Settings::instance().flag(Settings::Flag::ReopenLastSession))
        tryLoadRecentDirectory();
    return true;
}

void GameCollectionActivity::changeIconsSize()
{
    int size = mp_slider_icons_size->value() * 16;
    mp_tree_games->setIconSize(QSize(size, size));
}

void GameCollectionActivity::showTreeContextMenu(const QPoint & _point)
{
    if(Application::instance().gameCollection().isLoaded())
        mp_context_menu->exec(mp_tree_games->mapToGlobal(_point));
}

void GameCollectionActivity::activateCollectionControls(bool _activate)
{
    mp_action_install->setEnabled(_activate);
    mp_action_reload->setEnabled(_activate);
}

void GameCollectionActivity::activateItemControls(const Game * _selected_game)
{
    mp_widget_details->setVisible(_selected_game);
    mp_action_delete->setEnabled(_selected_game);
    mp_action_edit->setEnabled(_selected_game);
    mp_action_rename->setEnabled(_selected_game);
    mp_action_restore_iso->setEnabled(_selected_game && _selected_game->installationType() == GameInstallationType::UlConfig);
}

void GameCollectionActivity::applySettings()
{
    QSettings settings;
    QVariant icons_size_value = settings.value(SettingsKey::icons_size);
    int icons_size = 3;
    if(!icons_size_value.isNull() && icons_size_value.canConvert(QVariant::Int))
    {
        icons_size = icons_size_value.toInt();
        if(icons_size > 4) icons_size = 4;
        else if(icons_size < 1) icons_size = 1;
    }
    mp_slider_icons_size->setValue(icons_size);
    icons_size *= 16;
    mp_tree_games->setIconSize(QSize(icons_size, icons_size));
}

void GameCollectionActivity::saveSettings()
{
    QSettings settings;
    settings.setValue(SettingsKey::icons_size, mp_slider_icons_size->value());
}

void GameCollectionActivity::load()
{
    QSettings settings;
    QString dirpath = settings.value(SettingsKey::ul_dir).toString();
    QString choosen_dirpath = QFileDialog::getExistingDirectory(this, tr("Choose the OPL root directory"), dirpath);
    if(choosen_dirpath.isEmpty()) return;
    if(choosen_dirpath != dirpath)
        settings.setValue(SettingsKey::ul_dir, choosen_dirpath);
    loadDirectory(choosen_dirpath);
}

bool GameCollectionActivity::tryLoadRecentDirectory()
{
    QSettings settings;
    QVariant value = settings.value(SettingsKey::ul_dir);
    if(!value.isValid()) return false;
    QDir dir(value.toString());
    if(!dir.exists()) return false;
    loadDirectory(dir);
    return true;
}

void GameCollectionActivity::loadDirectory(const QDir & _directory)
{
    try
    {
        GameCollection & game_collection = Application::instance().gameCollection();
        game_collection.load(_directory);
        delete mp_game_art_manager;
        mp_game_art_manager = new GameArtManager(_directory, this);
        connect(mp_game_art_manager, &GameArtManager::artChanged, this, &GameCollectionActivity::gameArtChanged);
        mp_game_art_manager->addCacheType(GameArtType::Icon);
        mp_game_art_manager->addCacheType(GameArtType::Front);
        mp_model->setArtManager(*mp_game_art_manager);
        mp_proxy_model->sort(0, Qt::AscendingOrder);
        if(game_collection.count() > 0)
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

void GameCollectionActivity::reload()
{
    loadDirectory(Application::instance().gameCollection().directory());
}

void GameCollectionActivity::collectionLoaded()
{
    mp_label_directory->setText(Application::instance().gameCollection().directory());
    activateCollectionControls(true);
    gameSelected();
}

void GameCollectionActivity::gameAdded(const QString & _id)
{
    Q_UNUSED(_id)
    QModelIndex index = mp_tree_games->currentIndex();
    if(!index.isValid())
        index = mp_proxy_model->index(0, 0);
    mp_tree_games->setCurrentIndex(index);
}

void GameCollectionActivity::gameRenamed(const QString & _id)
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game && game->id() == _id)
        gameSelected();
}

void GameCollectionActivity::gameArtChanged(const QString & _game_id, GameArtType _type, const QPixmap * _pixmap)
{
    if(_type != GameArtType::Front)
        return;
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game && game->id() == _game_id)
        mp_label_cover->setPixmap(_pixmap ? *_pixmap : m_default_cover);
}

void GameCollectionActivity::gameSelected()
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

void GameCollectionActivity::renameGame()
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game)
    {
        GameRenameDialog dlg(game->title(), game->installationType(), this);
        if(dlg.exec() == QDialog::Accepted)
        {
            try
            {
                Application::instance().gameCollection().renameGame(*game, dlg.name());
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

void GameCollectionActivity::showGameDetails()
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game)
    {
        QSharedPointer<Intent> intent = GameDetailsActivity::createIntent(*mp_game_art_manager, game->id());
        Application::instance().pushActivity(*intent);
    }
}

void GameCollectionActivity::showIsoRestorer()
{
    const Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game && game->installationType() == GameInstallationType::UlConfig)
    {
        QSharedPointer<Intent> intent = IsoRestorerActivity::createIntent(game->id());
        Application::instance().pushActivity(*intent);
    }
}

void GameCollectionActivity::showGameInstaller()
{
    QSharedPointer<Intent> intent = GameInstallerActivity::createIntent();
    Application::instance().pushActivity(*intent);
}

void GameCollectionActivity::deleteGame()
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
        GameCollection & collection = Application::instance().gameCollection();
        collection.deleteGame(*game);
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
