/***********************************************************************************************
 * Copyright © 2017-2021 Sergey Smolyannikov aka brainstream                                   *
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
#include <QCheckBox>
#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>
#include <OplPcTools/Settings.h>
#include <OplPcTools/Exception.h>
#include <OplPcTools/File.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/VmcRenameDialog.h>
#include <OplPcTools/UI/VmcCreateDialog.h>
#include <OplPcTools/UI/VmcListWidget.h>
#include <OplPcTools/UI/VmcDetailsActivity.h>
#include <OplPcTools/UI/VmcPropertiesDialog.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

namespace SettingsKey {

const QString export_dir("VmcExportDir");

} // namespace SettingsKey

class VmcExporter final
{
public:
    explicit VmcExporter(const Vmc & _vmc);
    void exportTo(const QString _dir);

private:
    void exportDirectory(VmcFS & _fs, const QString & _vmc_dir, const QString & _dest_directory);
    void exportFile(VmcFS & _fs, const QString & _vmc_file, const QString & _dest_directory);

private:
    const QString m_vmc_file;
};

} // namespace

VmcExporter::VmcExporter(const Vmc & _vmc) :
    m_vmc_file(_vmc.filepath())
{
}

void VmcExporter::exportTo(const QString _dir)
{
    QSharedPointer<VmcFS> fs = VmcFS::load(m_vmc_file);
    exportDirectory(*fs, "/", _dir);
}

void VmcExporter::exportDirectory(VmcFS & _fs, const QString & _vmc_dir, const QString & _dest_directory)
{
    QDir dir(_dest_directory);
    if(!dir.exists() && !QDir().mkpath(dir.path()))
        throw Exception(QObject::tr("Unable to create directory \"%1\"").arg(dir.path()));
    QList<VmcEntryInfo> entries = _fs.enumerateEntries(_vmc_dir);
    for(const VmcEntryInfo & entry : entries)
    {
        QString next_vmc_entry = VmcFS::concatPaths(_vmc_dir, entry.name);
        if(entry.is_directory)
        {
            QString next_directory = dir.absoluteFilePath(entry.name);
            exportDirectory(_fs, next_vmc_entry, next_directory);
        }
        else
            exportFile(_fs, next_vmc_entry, _dest_directory);
    }
}

void VmcExporter::exportFile(VmcFS & _fs, const QString & _vmc_file, const QString & _dest_directory)
{
    QSharedPointer<VmcFile> file = _fs.openFile(_vmc_file);
    if(!file)
        throw Exception(QObject::tr("Unable to open VMC file \"%1\"").arg(_vmc_file));

    char * buffer = new char[file->size()];
    int64_t size = file->read(buffer, file->size());
    if(size > 0)
    {
        QFile out(QDir(_dest_directory).absoluteFilePath(file->name()));
        openFile(out, QIODevice::Truncate | QIODevice::WriteOnly);
        out.write(buffer, size);
    }
    delete [] buffer;
}

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
    const Vmc * vmc(const QModelIndex & _index);

private:
    void onLibraryLoaded();
    void onVmcAdded(const QUuid & _uuid);
    void onVmcAboutToBeDeleted(const QUuid & _uuid);
    void onVmcDeleted(const QUuid & _uuid);
    void updateRecord(const QUuid & _uuid);

private:
    QPixmap m_icon;
};

VmcListWidget::VmcTreeModel::VmcTreeModel(QObject * _parent):
    QAbstractItemModel(_parent),
    m_icon(":/images/vmc")
{
    VmcCollection * vmc_manager = &Library::instance().vmcs();
    connect(&Library::instance(), &Library::loaded, this, &VmcTreeModel::onLibraryLoaded);
    connect(vmc_manager, &VmcCollection::vmcAdded, this, &VmcTreeModel::onVmcAdded);
    connect(vmc_manager, &VmcCollection::vmcRenamed, this, &VmcTreeModel::updateRecord);
    connect(vmc_manager, &VmcCollection::vmcAboutToBeDeleted, this, &VmcTreeModel::onVmcAboutToBeDeleted);
    connect(vmc_manager, &VmcCollection::vmcDeleted, this, &VmcTreeModel::onVmcDeleted);
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
    VmcCollection & vmc_man = Library::instance().vmcs();
    int count = vmc_man.count();
    for(int i = 0; i < count; ++i)
    {
        const Vmc * vmc = vmc_man[i];
        if(vmc->uuid() == _uuid)
        {
            return createIndex(i, 0);
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
    return _parent.isValid() ? 0 : Library::instance().vmcs().count();
}

int VmcListWidget::VmcTreeModel::columnCount(const QModelIndex & _parent) const
{
    Q_UNUSED(_parent);
    return 2;
}

QVariant VmcListWidget::VmcTreeModel::data(const QModelIndex & _index, int _role) const
{
    const Vmc * vmc = Library::instance().vmcs()[_index.row()];
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
        }
    }
    else if(_role == Qt::TextAlignmentRole)
    {
        return QVariant(Qt::AlignRight | Qt::AlignCenter);
    }
    return QVariant();
}

const Vmc * VmcListWidget::VmcTreeModel::vmc(const QModelIndex & _index)
{
    VmcCollection & vmc_man = Library::instance().vmcs();
    if(_index.isValid() && _index.row() < vmc_man.count())
        return vmc_man[_index.row()];
    return nullptr;
}

void VmcListWidget::VmcTreeModel::onVmcAdded(const QUuid & _uuid)
{
    Q_UNUSED(_uuid);
    int count = Library::instance().vmcs().count() - 1;
    beginInsertRows(QModelIndex(), count, count);
    endInsertRows();
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
    mp_btn_export->setDefaultAction(mp_action_export);
    mp_model = new VmcTreeModel(this);
    mp_proxy_model = new QSortFilterProxyModel(this);
    mp_proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mp_proxy_model->setSortCaseSensitivity(Qt::CaseInsensitive);
    mp_proxy_model->setSourceModel(mp_model);
    mp_proxy_model->setDynamicSortFilter(true);
    mp_tree_vmcs->setModel(mp_proxy_model);
    mp_tree_vmcs->header()->setStretchLastSection(false);
    mp_tree_vmcs->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mp_context_menu = new QMenu(this);
    mp_context_menu->addAction(mp_action_export);
    mp_context_menu->addAction(mp_action_rename_vmc);
    mp_context_menu->addAction(mp_action_delete_vmc);
    mp_context_menu->addAction(mp_action_properties);
    mp_context_menu->addSeparator();
    mp_context_menu->addAction(mp_action_create_vmc);
    activateItemControls(nullptr);
    connect(mp_edit_filter, &QLineEdit::textChanged, mp_proxy_model, &QSortFilterProxyModel::setFilterFixedString);
    connect(&Settings::instance(), &Settings::iconSizeChanged, this, &VmcListWidget::setIconSize);
    connect(mp_action_rename_vmc, &QAction::triggered, this, &VmcListWidget::renameVmc);
    connect(mp_tree_vmcs->selectionModel(), &QItemSelectionModel::selectionChanged,
        [this](QItemSelection, QItemSelection) { onVmcSelected(); });
    connect(mp_action_delete_vmc, &QAction::triggered, this, &VmcListWidget::deleteVmc);
    connect(mp_action_create_vmc, &QAction::triggered, this, &VmcListWidget::createVmc);
    connect(mp_action_properties, &QAction::triggered, this, &VmcListWidget::showVmcProperties);
    connect(mp_action_export, &QAction::triggered, this, &VmcListWidget::exportFiles);
    connect(mp_tree_vmcs, &QTreeView::customContextMenuRequested, this, &VmcListWidget::showTreeContextMenu);
    connect(mp_tree_vmcs, &QTreeView::doubleClicked, this, &VmcListWidget::onTreeViewDoubleClicked);
    if(Library::instance().vmcs().count() > 0)
        mp_tree_vmcs->setCurrentIndex(mp_proxy_model->index(0, 0));
    mp_proxy_model->sort(0, Qt::AscendingOrder);
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
                Library::instance().vmcs().renameVmc(vmc->uuid(), dlg.name());
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
    mp_action_properties->setDisabled(disabled);
    mp_action_export->setDisabled(disabled);
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
        Library::instance().vmcs().deleteVmc(vmc->uuid());
    }
    catch(Exception & exception)
    {
        Application::showErrorMessage(exception.message());
    }
    catch(...)
    {
        Application::showErrorMessage();
    }
}

void VmcListWidget::showVmcProperties()
{
    const Vmc * vmc = mp_model->vmc(mp_proxy_model->mapToSource(mp_tree_vmcs->currentIndex()));
    if(vmc)
    {
        VmcPropertiesDialog dlg(*vmc, this);
        dlg.exec();
    }
}

void VmcListWidget::createVmc()
{
    VmcCreateDialog dlg(this);
    dlg.exec();
}

void VmcListWidget::showTreeContextMenu(const QPoint & _point)
{
    if(Library::instance().vmcs().isLoaded())
        mp_context_menu->exec(mp_tree_vmcs->mapToGlobal(_point));
}

void VmcListWidget::onTreeViewDoubleClicked(const QModelIndex & _index)
{
    const Vmc * vmc = mp_model->vmc(mp_proxy_model->mapToSource(_index));
    if(vmc)
    {
        auto intent = VmcDetailsActivity::createIntent(*vmc);
        Application::pushActivity(*intent);
    }
}

void VmcListWidget::exportFiles()
{
    const Vmc * vmc = mp_model->vmc(mp_proxy_model->mapToSource(mp_tree_vmcs->currentIndex()));
    if(!vmc)
    {
        return;
    }
    QSettings settings;
    QString directory = settings.value(SettingsKey::export_dir).toString();
    if(directory.isEmpty())
        directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    directory = QFileDialog::getExistingDirectory(this, tr("Select directory"), directory);
    if(directory.isEmpty())
        return;
    settings.setValue(SettingsKey::export_dir, directory);
    try
    {
        VmcExporter exporter(*vmc);
        exporter.exportTo(directory);
    }
    catch(const Exception & _exception)
    {
        Application::showErrorMessage(_exception.message());
    }
}
