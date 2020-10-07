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

#include <QPixmap>
#include <QAbstractItemModel>
#include <QShortcut>
#include <OplPcTools/Settings.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/VmcListWidget.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

class VmcTreeModel final : public QAbstractItemModel
{
public:
    VmcTreeModel();
    QModelIndex index(int _row, int _column, const QModelIndex & _parent) const override;
    QModelIndex parent(const QModelIndex & _child) const override;
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    QVariant headerData(int _section, Qt::Orientation _orientation, int _role) const override;

private:
    void onLibraryLoaded();

private:
    QPixmap m_icon;
    VmcManager & mr_vmcs;
};

} // namespace

VmcTreeModel::VmcTreeModel():
    m_icon(":/images/vmc"),
    mr_vmcs(Application::instance().library().vmcs())
{
    connect(&Application::instance().library(), &Library::loaded, this, &VmcTreeModel::onLibraryLoaded);
}

void VmcTreeModel::onLibraryLoaded()
{
    beginResetModel();
    endResetModel();
}

QModelIndex VmcTreeModel::index(int _row, int _column, const QModelIndex & _parent) const
{
    Q_UNUSED(_parent);
    return createIndex(_row, _column);
}

QModelIndex VmcTreeModel::parent(const QModelIndex & _child) const
{
    Q_UNUSED(_child);
    return QModelIndex();
}

int VmcTreeModel::rowCount(const QModelIndex & _parent) const
{
    return _parent.isValid() ? 0 : mr_vmcs.count();
}

int VmcTreeModel::columnCount(const QModelIndex & _parent) const
{
    Q_UNUSED(_parent);
    return 2;
}

QVariant VmcTreeModel::data(const QModelIndex & _index, int _role) const
{
    const Vmc * vmc = mr_vmcs[_index.row()];
    if(_index.column() == 0)
    {
        switch (_role)
        {
        case Qt::DisplayRole:
            return vmc->title();
        case Qt::DecorationRole:
            return QIcon(m_icon);
        }
    }
    else if(_role == Qt::DisplayRole)
    {
        switch (vmc->size())
        {
        case VmcSize::_8M:
            return QString(QObject::tr("8 MiB"));
        case VmcSize::_16M:
            return QString(QObject::tr("16 MiB"));
        case VmcSize::_32M:
            return QString(QObject::tr("32 MiB"));
        case VmcSize::_64M:
            return QString(QObject::tr("64 MiB"));
        case VmcSize::_128M:
            return QString(QObject::tr("128 MiB"));
        case VmcSize::_256M:
            return QString(QObject::tr("256 MiB"));
        }
    }
    return QVariant();
}

QVariant VmcTreeModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    Q_UNUSED(_orientation);
    if(_role == Qt::DisplayRole)
    {
        switch (_section)
        {
        case 0:
            return QObject::tr("Name");
        case 1:
            return QObject::tr("Size");
        }
    }
    return QVariant();
}


VmcListWidget::VmcListWidget(QWidget * _parent /*= nullptr*/):
    QWidget(_parent)
{
    setupUi(this);
    setupShortcuts();
    mp_btn_create_vmc->setDefaultAction(mp_action_create_vmc);
    mp_btn_delete_vmc->setDefaultAction(mp_action_delete_vmc);
    mp_btn_rename_vmc->setDefaultAction(mp_action_rename_vmc);
    VmcTreeModel * model = new VmcTreeModel();
    mp_proxy_model = new QSortFilterProxyModel(this);
    mp_proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mp_proxy_model->setSourceModel(model);
    mp_proxy_model->setDynamicSortFilter(true);
    mp_tree_vmcs->setModel(mp_proxy_model);
    mp_tree_vmcs->header()->setStretchLastSection(false);
    mp_tree_vmcs->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    connect(&Settings::instance(), SIGNAL(iconThemeChanged()), this, SLOT(update()));
    connect(mp_edit_filter, &QLineEdit::textChanged, mp_proxy_model, &QSortFilterProxyModel::setFilterFixedString);
    connect(&Settings::instance(), &Settings::iconSizeChanged, this, &VmcListWidget::setIconSize);
}

void VmcListWidget::setupShortcuts()
{
    QShortcut * shortcut = new QShortcut(QKeySequence("Back"), this);
    connect(shortcut, &QShortcut::activated, this, &VmcListWidget::close);
    shortcut = new QShortcut(QKeySequence("Esc"), this);
    connect(shortcut, &QShortcut::activated, this, &VmcListWidget::close);
    shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    mp_edit_filter->setPlaceholderText(QString("%1 (%2)")
        .arg(mp_edit_filter->placeholderText())
        .arg(shortcut->key().toString()));
    connect(shortcut, &QShortcut::activated, [this]() { mp_edit_filter->setFocus(); });
    setIconSize();
}

void VmcListWidget::setIconSize()
{
    QSize size;
    size.setWidth(Settings::instance().iconSize());
    size.setHeight(size.width());
    mp_tree_vmcs->setIconSize(size);
}
