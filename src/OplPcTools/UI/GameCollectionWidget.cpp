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

#include <QSettings>
#include <QFileDialog>
#include <QAbstractItemModel>
#include <OplPcTools/UI/GameCollectionWidget.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {
namespace SettingsKey {

const char * ul_dir     = "ULDirectory";
const char * icons_size = "GameListIconSize";


} // namespace SettingsKey
} // namespace

class GameCollectionWidget::GameTreeModel : public QAbstractItemModel
{
public:
    explicit GameTreeModel(Core::GameCollection & _collection, QObject * _parent = nullptr);
    QModelIndex index(int _row, int _column, const QModelIndex & _parent) const override;
    QModelIndex parent(const QModelIndex & _child) const override;
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    const Core::Game * game(const QModelIndex & _index) const;
    void setArtManager(Core::GameArtManager & _manager);

private:
    const QPixmap m_default_icon;
    const Core::GameCollection & mr_collection;
    Core::GameArtManager * mp_art_manager;
};


GameCollectionWidget::GameTreeModel::GameTreeModel(Core::GameCollection & _collection, QObject * _parent /*= nullptr*/) :
    QAbstractItemModel(_parent),
    m_default_icon(QPixmap(":/images/no-icon")),
    mr_collection(_collection),
    mp_art_manager(nullptr)
{
    connect(&_collection, &Core::GameCollection::loaded, this, [this]() {
        beginResetModel();
        endResetModel();
    });
}

QModelIndex GameCollectionWidget::GameTreeModel::index(int _row, int _column, const QModelIndex & _parent) const
{
    Q_UNUSED(_parent)
    return createIndex(_row, _column);
}

QModelIndex GameCollectionWidget::GameTreeModel::parent(const QModelIndex & _child) const
{
    Q_UNUSED(_child)
    return QModelIndex();
}

int GameCollectionWidget::GameTreeModel::rowCount(const QModelIndex & _parent) const
{
    if(_parent.isValid())
        return 0;
    return mr_collection.count();
}

int GameCollectionWidget::GameTreeModel::columnCount(const QModelIndex & _parent) const
{
    Q_UNUSED(_parent)
    return 1;
}

QVariant GameCollectionWidget::GameTreeModel::data(const QModelIndex & _index, int _role) const
{
    switch(_role)
    {
    case Qt::DisplayRole:
        return mr_collection[_index.row()]->title();
    case Qt::DecorationRole:
        if(mp_art_manager)
        {
            QPixmap icon = mp_art_manager->load(mr_collection[_index.row()]->id(), Core::GameArtType::Icon);
            return QIcon(icon.isNull() ? m_default_icon : icon);
        }
        break;
    }
    return QVariant();
}

const Core::Game * GameCollectionWidget::GameTreeModel::game(const QModelIndex & _index) const
{
    return _index.isValid() ? mr_collection[_index.row()] : nullptr;
}

void GameCollectionWidget::GameTreeModel::setArtManager(Core::GameArtManager & _manager)
{
    mp_art_manager = &_manager;
}

GameCollectionWidget::GameCollectionWidget(UIContext & _context, QWidget * _parent /*= nullptr*/) :
    QWidget(_parent),
    mr_context(_context),
    mp_game_art_manager(nullptr),
    mp_model(nullptr),
    mp_proxy_model(nullptr)
{
    setupUi(this);
    m_default_cover = QPixmap(":/images/no-image")
        .scaled(mp_label_cover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    mp_model = new GameTreeModel(_context.collection(), this);
    mp_proxy_model = new QSortFilterProxyModel(this);
    mp_proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mp_proxy_model->setSourceModel(mp_model);
    mp_proxy_model->setDynamicSortFilter(true);
    mp_tree_games->setModel(mp_proxy_model);
    activateCollectionControls(false);
    activateItemControls(false);
    connect(mp_tree_games->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(gameSelected()));
    connect(&mr_context.collection(), SIGNAL(loaded()), this, SLOT(collectionLoaded()));
    connect(this, &GameCollectionWidget::destroyed, this, &GameCollectionWidget::saveSettings);
    connect(mp_edit_filter, SIGNAL(textChanged(QString)), mp_proxy_model, SLOT(setFilterFixedString(QString)));
    applySettings();
}

GameCollectionWidget::~GameCollectionWidget()
{
    delete mp_game_art_manager;
}

void GameCollectionWidget::activateCollectionControls(bool _activate)
{
    mp_btn_install->setEnabled(_activate);
    mp_btn_reload->setEnabled(_activate);
}

void GameCollectionWidget::activateItemControls(bool _activate)
{
    mp_widget_details->setVisible(_activate);
    mp_btn_delete->setEnabled(_activate);
    mp_btn_edit->setEnabled(_activate);
}

void GameCollectionWidget::applySettings()
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

void GameCollectionWidget::saveSettings()
{
    QSettings settings;
    settings.setValue(SettingsKey::icons_size, mp_slider_icons_size->value());
}

void GameCollectionWidget::load()
{
    QSettings settings;
    QString dirpath = settings.value(SettingsKey::ul_dir).toString();
    QString choosen_dirpath = QFileDialog::getExistingDirectory(this, tr("Choose the OPL root directory"), dirpath);
    if(choosen_dirpath.isEmpty()) return;
    if(choosen_dirpath != dirpath)
        settings.setValue(SettingsKey::ul_dir, dirpath);
    load(choosen_dirpath);
}

bool GameCollectionWidget::tryLoadRecentDirectory()
{
    QSettings settings;
    QVariant value = settings.value(SettingsKey::ul_dir);
    if(!value.isValid()) return false;
    QDir dir(value.toString());
    if(!dir.exists()) return false;
    load(dir);
    return true;
}

void GameCollectionWidget::load(const QDir & _directory)
{
    mr_context.collection().load(_directory);
    delete mp_game_art_manager;
    mp_game_art_manager = new Core::GameArtManager(_directory);
    mp_game_art_manager->addCacheType(Core::GameArtType::Icon);
    mp_game_art_manager->addCacheType(Core::GameArtType::Front);
    mp_model->setArtManager(*mp_game_art_manager);
    mp_proxy_model->sort(0, Qt::AscendingOrder);
    if(mr_context.collection().count() > 0)
        mp_tree_games->setCurrentIndex(mp_proxy_model->index(0, 0));
}

void GameCollectionWidget::reload()
{
    load(mr_context.collection().directory());
}

void GameCollectionWidget::collectionLoaded()
{
    mp_label_directory->setText(mr_context.collection().directory());
    activateCollectionControls(true);
    gameSelected();
}

void GameCollectionWidget::changeIconsSize()
{
    int size = mp_slider_icons_size->value() * 16;
    mp_tree_games->setIconSize(QSize(size, size));
}

void GameCollectionWidget::gameSelected()
{
    const Core::Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game)
    {
        mp_label_id->setText(game->id());
        mp_label_title->setText(game->title());
        QPixmap pixmap = mp_game_art_manager->load(game->id(), Core::GameArtType::Front);
        mp_label_cover->setPixmap(pixmap.isNull() ? m_default_cover : pixmap);
        mp_label_type->setText(game->mediaType() == Core::MediaType::CD ? "CD" : "DVD");
        mp_label_parts->setText(QString("%1").arg(game->partCount()));
        mp_label_source->setText(
            game->installationType() == Core::GameInstallationType::UlConfig ? "UL" : tr("Directory"));
        mp_widget_details->show();
    }
    else
    {
        mp_widget_details->hide();
    }
    activateItemControls(game != nullptr);
}

void GameCollectionWidget::showGameDetails()
{
    const Core::Game * game = mp_model->game(mp_proxy_model->mapToSource(mp_tree_games->currentIndex()));
    if(game) mr_context.showGameDetails(game->id(), *mp_game_art_manager);
}
