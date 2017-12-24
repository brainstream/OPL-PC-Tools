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
#include <QFileDialog>
#include <QAbstractItemModel>
#include <QPixmap>
#include <OplPcTools/UI/GameCollectionWidget.h>
#include "ui_GameCollectionWidget.h"

using namespace OplPcTools::UI;

namespace {

class GameTreeItemModel : public QAbstractItemModel
{
public:
    explicit GameTreeItemModel(OplPcTools::Core::GameCollection & _collection, QObject * _parent = nullptr);
    QModelIndex index(int _row, int _column, const QModelIndex & _parent) const override;
    QModelIndex parent(const QModelIndex & _child) const override;
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    const OplPcTools::Core::Game * game(const QModelIndex & _index) const;

private:
    const OplPcTools::Core::GameCollection & mr_collection;
};

} // namespace

GameTreeItemModel::GameTreeItemModel(OplPcTools::Core::GameCollection & _collection, QObject * _parent /*= nullptr*/) :
    QAbstractItemModel(_parent),
    mr_collection(_collection)
{
    connect(&_collection, SIGNAL(loaded()), this, SIGNAL(modelReset()));
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
        return QString("Item #%1").arg(_index.row());
    case Qt::DecorationRole:
        return QIcon::fromTheme("document-edit");
    default:
        return QVariant();
    }
}

const OplPcTools::Core::Game * GameTreeItemModel::game(const QModelIndex & _index) const
{
    return _index.isValid() ? &mr_collection[_index.row()] : nullptr;
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
    mp_private->default_cover = QPixmap(":/images/no-image")
        .scaled(mp_private->label_cover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    connect(mp_private->tree_games->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(gameSelected()));
    connect(&mp_private->context.collection(), SIGNAL(loaded()), this, SLOT(collectionLoaded()));
}

GameCollectionWidget::~GameCollectionWidget()
{
    delete mp_private;
    qDebug() << "GameCollectionWidget destroyed";
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

void GameCollectionWidget::load()
{
//    QString dirpath = settings.value(g_settings_key_ul_dir).toString();
    QString dirpath = QFileDialog::getExistingDirectory(this, tr("Choose the OPL root directory"));
    if(dirpath.isEmpty()) return;
    mp_private->context.collection().load(dirpath);
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
    const OplPcTools::Core::Game * game = mp_private->tree_model->game(mp_private->tree_games->currentIndex());
    if(game)
    {
        mp_private->label_id->setText(game->id());
        mp_private->label_title->setText(game->title());
        // TODO: if no cover
        mp_private->label_cover->setPixmap(mp_private->default_cover);
        // TODO: mp_private->label_type->setText();
        // TODO: mp_private->label_parts->setText(0);
        // TODO: mp_private->label_source->setText();
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
    const OplPcTools::Core::Game * game = mp_private->tree_model->game(mp_private->tree_games->currentIndex());
    if(game) mp_private->context.showGameDetails(game->id());
}
