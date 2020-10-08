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
#include <QMessageBox>
#include <QCheckBox>
#include <OplPcTools/Settings.h>
#include <OplPcTools/Exception.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/VmcListWidget.h>
#include <OplPcTools/UI/VmcRenameDialog.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

class VmcListWidget::VmcTreeModel final : public QAbstractItemModel
{
public:
    explicit VmcTreeModel(QObject * _parent);
    QModelIndex index(int _row, int _column, const QModelIndex & _parent) const override;
    QModelIndex index(const QUuid & _uuid) const;
    QModelIndex parent(const QModelIndex & _child) const override;
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    QVariant headerData(int _section, Qt::Orientation _orientation, int _role) const override;
    const Vmc * vmc(const QModelIndex & _index);

private:
    void onLibraryLoaded();
    void onVmcAboutToBeDeleted(const QUuid & _uuid);
    void onVmcDeleted(const QUuid & _uuid);
    void updateRecord(const QUuid & _uuid);

private:
    QPixmap m_icon;
    VmcManager & mr_vmcs;
};

VmcListWidget::VmcTreeModel::VmcTreeModel(QObject * _parent):
    QAbstractItemModel(_parent),
    m_icon(":/images/vmc"),
    mr_vmcs(Application::instance().library().vmcs())
{
    connect(&Application::instance().library(), &Library::loaded, this, &VmcTreeModel::onLibraryLoaded);
    connect(&mr_vmcs, &VmcManager::vmcRenamed, this, &VmcTreeModel::updateRecord);
    connect(&mr_vmcs, &VmcManager::vmcAboutToBeDeleted, this, &VmcTreeModel::onVmcAboutToBeDeleted);
    connect(&mr_vmcs, &VmcManager::vmcDeleted, this, &VmcTreeModel::onVmcDeleted);
}

void VmcListWidget::VmcTreeModel::onLibraryLoaded()
{
    beginResetModel();
    endResetModel();
}

QModelIndex VmcListWidget::VmcTreeModel::index(int _row, int _column, const QModelIndex & _parent) const
{
    Q_UNUSED(_parent);
    return createIndex(_row, _column);
}

QModelIndex VmcListWidget::VmcTreeModel::index(const QUuid & _uuid) const
{
    int count = mr_vmcs.count();
    for(int i = 0; i < count; ++i)
    {
        const Vmc * vmc = mr_vmcs[i];
        if(vmc->uuid() == _uuid)
        {
            return createIndex(i, 0), createIndex(i, 1);
        }
    }
    return QModelIndex();
}

QModelIndex VmcListWidget::VmcTreeModel::parent(const QModelIndex & _child) const
{
    Q_UNUSED(_child);
    return QModelIndex();
}

int VmcListWidget::VmcTreeModel::rowCount(const QModelIndex & _parent) const
{
    return _parent.isValid() ? 0 : mr_vmcs.count();
}

int VmcListWidget::VmcTreeModel::columnCount(const QModelIndex & _parent) const
{
    Q_UNUSED(_parent);
    return 2;
}

QVariant VmcListWidget::VmcTreeModel::data(const QModelIndex & _index, int _role) const
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

QVariant VmcListWidget::VmcTreeModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
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

const Vmc * VmcListWidget::VmcTreeModel::vmc(const QModelIndex & _index)
{
    if(_index.isValid() && _index.row() < mr_vmcs.count())
        return mr_vmcs[_index.row()];
    return nullptr;
}

void VmcListWidget::VmcTreeModel::updateRecord(const QUuid & _uuid)
{
    QModelIndex start_index = index(_uuid);
    if(start_index.isValid())
    {
        QModelIndex end_index = createIndex(start_index.row(), 1);
        emit dataChanged(start_index, end_index);
    }
}

void VmcListWidget::VmcTreeModel::onVmcAboutToBeDeleted(const QUuid & _uuid)
{
    QModelIndex start_index = index(_uuid);
    if(start_index.isValid())
        beginRemoveRows(QModelIndex(), start_index.row(), start_index.row());
}

void VmcListWidget::VmcTreeModel::onVmcDeleted(const QUuid & _uuid)
{
    Q_UNUSED(_uuid);
    endRemoveRows();
}

VmcListWidget::VmcListWidget(QWidget * _parent /*= nullptr*/):
    QWidget(_parent)
{
    setupUi(this);
    setupShortcuts();
    mp_btn_create_vmc->setDefaultAction(mp_action_create_vmc);
    mp_btn_delete_vmc->setDefaultAction(mp_action_delete_vmc);
    mp_btn_rename_vmc->setDefaultAction(mp_action_rename_vmc);
    mp_model = new VmcTreeModel(this);
    mp_proxy_model = new QSortFilterProxyModel(this);
    mp_proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mp_proxy_model->setSourceModel(mp_model);
    mp_proxy_model->setDynamicSortFilter(true);
    mp_tree_vmcs->setModel(mp_proxy_model);
    mp_tree_vmcs->header()->setStretchLastSection(false);
    mp_tree_vmcs->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mp_context_menu = new QMenu(this);
    mp_context_menu->addAction(mp_action_rename_vmc);
    mp_context_menu->addAction(mp_action_delete_vmc);
    mp_context_menu->addSeparator();
    mp_context_menu->addAction(mp_action_create_vmc);
    mp_tree_vmcs->setContextMenuPolicy(Qt::CustomContextMenu);
    activateItemControls(nullptr);
    connect(&Settings::instance(), SIGNAL(iconThemeChanged()), this, SLOT(update()));
    connect(mp_edit_filter, &QLineEdit::textChanged, mp_proxy_model, &QSortFilterProxyModel::setFilterFixedString);
    connect(&Settings::instance(), &Settings::iconSizeChanged, this, &VmcListWidget::setIconSize);
    connect(mp_action_rename_vmc, &QAction::triggered, this, &VmcListWidget::renameVmc);
    connect(mp_tree_vmcs->selectionModel(), &QItemSelectionModel::selectionChanged,
        [this](QItemSelection, QItemSelection) { onVmcSelected(); });
    connect(mp_action_delete_vmc, &QAction::triggered, this, &VmcListWidget::deleteVmc);
    connect(mp_tree_vmcs, &QTreeView::customContextMenuRequested, this, &VmcListWidget::showTreeContextMenu);
    if(Application::instance().library().vmcs().count() > 0)
        mp_tree_vmcs->setCurrentIndex(mp_proxy_model->index(0, 0));
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

void VmcListWidget::renameVmc()
{
    const Vmc * vmc = mp_model->vmc(mp_proxy_model->mapToSource(mp_tree_vmcs->currentIndex()));
    if(vmc)
    {
        VmcRenameDialog dlg(vmc->title(), this);
        if(dlg.exec() == QDialog::Accepted)
        {
            try
            {
                Application::instance().library().vmcs().renameVmc(vmc->uuid(), dlg.name());
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

void VmcListWidget::onVmcSelected()
{
    const Vmc * vmc = mp_model->vmc(mp_proxy_model->mapToSource(mp_tree_vmcs->currentIndex()));
    activateItemControls(vmc);
}

void VmcListWidget::activateItemControls(const Vmc * _vmc)
{
    bool disabled = _vmc == nullptr;
    mp_action_rename_vmc->setDisabled(disabled);
    mp_action_delete_vmc->setDisabled(disabled);
}

void VmcListWidget::deleteVmc()
{
    const Vmc * vmc = mp_model->vmc(mp_proxy_model->mapToSource(mp_tree_vmcs->currentIndex()));
    if(!vmc) return;
    Settings & settings = Settings::instance();
    if(settings.confirmVmcDeletion())
    {
        QCheckBox * checkbox = new QCheckBox(tr("Do not ask again"));
        QMessageBox message_box(QMessageBox::Question, tr("Delete VMC"),
            QString("%1\n%2")
                .arg(tr("Are you sure you want to delete this VMC?"))
                .arg(vmc->title()),
            QMessageBox::Yes | QMessageBox::No);
        message_box.setDefaultButton(QMessageBox::Yes);
        message_box.setCheckBox(checkbox);
        if(message_box.exec() != QMessageBox::Yes)
            return;
        if(checkbox->isChecked())
            settings.setConfirmGameDeletion(false);
    }
    try
    {
        Application::instance().library().vmcs().deleteVmc(vmc->uuid());
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

void VmcListWidget::showTreeContextMenu(const QPoint & _point)
{
    if(Application::instance().library().vmcs().isLoaded())
        mp_context_menu->exec(mp_tree_vmcs->mapToGlobal(_point));
}
