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

#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QAbstractItemModel>
#include <QPixmap>
#include <OplPcTools/UI/GameCollectionWidget.h>
#include "ui_GameCollectionWidget.h"

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

namespace SettingsKey {

const char * ul_dir     = "ULDirectory";
const char * icons_size = "GameListIconSize";

} // namespace SettingsKey

class GameTreeItemModel : public QAbstractItemModel
{
public:
    explicit GameTreeItemModel(Core::GameCollection & _collection, QObject * _parent = nullptr);
    QModelIndex index(int _row, int _column, const QModelIndex & _parent) const override;
    QModelIndex parent(const QModelIndex & _child) const override;
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    const Core::Game * game(const QModelIndex & _index) const;

private:
    const Core::GameCollection & mr_collection;
};

} // namespace

GameTreeItemModel::GameTreeItemModel(Core::GameCollection & _collection, QObject * _parent /*= nullptr*/) :
    QAbstractItemModel(_parent),
    mr_collection(_collection)
{
//    connect(&_collection, SIGNAL(loaded()), this, SIGNAL(modelReset()));

    connect(&_collection, &Core::GameCollection::loaded, this, [this]() {
        beginResetModel();
        qDebug() << "Resetting the tree model";
        endResetModel();
    });
}

QModelIndex GameTreeItemModel::index(int _row, int _column, const QModelIndex & _parent) const
{
    Q_UNUSED(_parent)
    return createIndex(_row, _column);
}

QModelIndex GameTreeItemModel::parent(const QModelIndex & _child) const
{
    Q_UNUSED(_child)
    return QModelIndex();
}

int GameTreeItemModel::rowCount(const QModelIndex & _parent) const
{
    if(_parent.isValid())
        return 0;
    return mr_collection.count();
}

int GameTreeItemModel::columnCount(const QModelIndex & _parent) const
{
    Q_UNUSED(_parent)
    return 1;
}

QVariant GameTreeItemModel::data(const QModelIndex & _index, int _role) const
{
    switch(_role)
    {
    case Qt::DisplayRole:
        return mr_collection[_index.row()]->title();
    case Qt::DecorationRole:
        return QIcon::fromTheme("document-edit");
    default:
        return QVariant();
    }
}

const Core::Game * GameTreeItemModel::game(const QModelIndex & _index) const
{
    return _index.isValid() ? mr_collection[_index.row()] : nullptr;
}

struct GameCollectionWidget::Private : public Ui::GameCollectionWidget
{
    explicit Private(UIContext & _context) :
        context(_context),
        tree_model(nullptr)
    {
    }

    UIContext & context;
    GameTreeItemModel * tree_model;
    QPixmap default_cover;
};

GameCollectionWidget::GameCollectionWidget(UIContext & _context, QWidget * _parent /*= nullptr*/) :
    QWidget(_parent),
    mp_private(new Private(_context))
{
    mp_private->setupUi(this);
    activateCollectionControls(false);
    activateItemControls(false);
    mp_private->tree_model = new GameTreeItemModel(_context.collection(), this);
    mp_private->tree_games->setModel(mp_private->tree_model);
    // TODO: sorting
    mp_private->default_cover = QPixmap(":/images/no-image")
        .scaled(mp_private->label_cover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    connect(mp_private->tree_games->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(gameSelected()));
    connect(&mp_private->context.collection(), SIGNAL(loaded()), this, SLOT(collectionLoaded()));
    connect(this, &GameCollectionWidget::destroyed, this, &GameCollectionWidget::saveSettings);
    applySettings();
}

GameCollectionWidget::~GameCollectionWidget()
{
    delete mp_private;
}

void GameCollectionWidget::activateCollectionControls(bool _activate)
{
    mp_private->btn_install->setEnabled(_activate);
    mp_private->btn_reload->setEnabled(_activate);
}

void GameCollectionWidget::activateItemControls(bool _activate)
{
    mp_private->widget_details->setVisible(_activate);
    mp_private->btn_delete->setEnabled(_activate);
    mp_private->btn_edit->setEnabled(_activate);
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
    mp_private->slider_icons_size->setValue(icons_size);
    icons_size *= 16;
    mp_private->tree_games->setIconSize(QSize(icons_size, icons_size));
}

void GameCollectionWidget::saveSettings()
{
    QSettings settings;
    settings.setValue(SettingsKey::icons_size, mp_private->slider_icons_size->value());
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
    mp_private->context.collection().load(_directory);
}

void GameCollectionWidget::reload()
{
    mp_private->context.collection().load(mp_private->context.collection().directory());
}

void GameCollectionWidget::collectionLoaded()
{
    mp_private->label_directory->setText(mp_private->context.collection().directory());
    activateCollectionControls(true);
    gameSelected();
}

void GameCollectionWidget::changeIconsSize()
{
    int size = mp_private->slider_icons_size->value() * 16;
    mp_private->tree_games->setIconSize(QSize(size, size));
}

void GameCollectionWidget::gameSelected()
{
    const Core::Game * game = mp_private->tree_model->game(mp_private->tree_games->currentIndex());
    if(game)
    {
        mp_private->label_id->setText(game->id());
        mp_private->label_title->setText(game->title());
        // TODO: if no cover
        mp_private->label_cover->setPixmap(mp_private->default_cover);
        mp_private->label_type->setText(game->mediaType() == Core::MediaType::CD ? "CD" : "DVD");
        mp_private->label_parts->setText(QString("%1").arg(game->partCount()));
        mp_private->label_source->setText(
            game->installationType() == Core::GameInstallationType::UlConfig ? "UL" : tr("Directory"));
        mp_private->widget_details->show();
    }
    else
    {
        mp_private->widget_details->hide();
    }
    activateItemControls(game != nullptr);
}

void GameCollectionWidget::showGameDetails()
{
    const Core::Game * game = mp_private->tree_model->game(mp_private->tree_games->currentIndex());
    if(game) mp_private->context.showGameDetails(game->id());
}
