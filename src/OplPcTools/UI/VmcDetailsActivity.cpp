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

#include <QStandardItemModel>
#include <OplPcTools/Settings.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/VmcRenameDialog.h>
#include <OplPcTools/UI/VmcDetailsActivity.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

class VmcDetailsActivityIntent : public Intent
{
public:
    explicit VmcDetailsActivityIntent(const Vmc &_vmc) :
        mr_vmc(_vmc)
    {
    }

    Activity * createActivity(QWidget * _parent) override
    {
        return new VmcDetailsActivity(mr_vmc, _parent);
    }

    QString activityClass() const override
    {
        return "VmcDetails";
    }

private:
    const Vmc & mr_vmc;
};


QString makeBytesDisplayString(uint32_t _bytes)
{
    uint32_t order = 0;
    for(; (_bytes >> (order * 10)) > 1024; ++order);
    QStringList result;
    if(order == 0)
        result << QString::number(_bytes);
    else
        result << QString::number(static_cast<double>(_bytes) / (order * 1024), 'f', 1) ;
    switch(order)
    {
    case 0:
        result << QObject::tr("B");
        break;
    case 1:
        result << QObject::tr("KiB");
        break;
    case 2:
        result << QObject::tr("MiB");
        break;
    case 3:
        result << QObject::tr("GiB");
        break;
    }
    return result.join(' ');
}

} // namespace


namespace OplPcTools::UI {

class VmcFileSystemViewModel : public QAbstractItemModel
{
public:
    explicit VmcFileSystemViewModel(QObject * _parent = nullptr);
    void setItems(const QList<VmcEntryInfo> & _items);
    void clear();
    const VmcEntryInfo * item(const QModelIndex & _index) const;
    QModelIndex index(int _row, int _column, const QModelIndex & _parent) const override;
    QModelIndex parent(const QModelIndex & _child) const override;
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    void sort(int _column, Qt::SortOrder _order) override;

private:
    void unsafeSort();

private:
    enum ColumnIndex
    {
        COL_TTITLE,
        COL_SIZE,
        column_count // It should be the last item
    };

private:
    QList<VmcEntryInfo> m_items;
    std::vector<int> m_item_indices;
    QIcon m_file_icon;
    QIcon m_dir_icon;
    int m_sorted_by_column;
    Qt::SortOrder m_sort_order;
};

} // namespace OplPcTools::UI

VmcFileSystemViewModel::VmcFileSystemViewModel(QObject * _parent /*= nullptr*/) :
    QAbstractItemModel(_parent),
    m_file_icon(":/images/file"),
    m_dir_icon(":/images/folder"),
    m_sorted_by_column(COL_TTITLE),
    m_sort_order(Qt::AscendingOrder)
{
}

void VmcFileSystemViewModel::setItems(const QList<VmcEntryInfo> & _items)
{
    beginResetModel();
    m_items = _items;
    m_item_indices.resize(_items.size());
    for(int i = 0; i < m_item_indices.size(); ++i)
        m_item_indices[i] = i;
    unsafeSort();
    endResetModel();
}

void VmcFileSystemViewModel::clear()
{
    beginResetModel();
    m_items.clear();
    m_item_indices.clear();
    endResetModel();
}

const VmcEntryInfo * VmcFileSystemViewModel::item(const QModelIndex & _index) const
{
    if(_index.isValid() & _index.row() < m_items.size() && _index.row() >= 0)
        return &m_items[m_item_indices[_index.row()]];
    return nullptr;
}

QModelIndex VmcFileSystemViewModel::index(int _row, int _column, const QModelIndex & _parent) const
{
    Q_UNUSED(_parent)
    return createIndex(_row, _column);
}

QModelIndex VmcFileSystemViewModel::parent(const QModelIndex & _child) const
{
    return QModelIndex();
}

int VmcFileSystemViewModel::rowCount(const QModelIndex & _parent) const
{
    if(_parent.isValid())
        return 0;
    return m_items.size();
}

int VmcFileSystemViewModel::columnCount(const QModelIndex & _parent) const
{
    return column_count;
}

QVariant VmcFileSystemViewModel::data(const QModelIndex & _index, int _role) const
{
    const VmcEntryInfo * item = this->item(_index);
    if(!item)
        return QVariant();
    switch(_index.column())
    {
    case COL_TTITLE:
        switch(_role)
        {
        case Qt::DisplayRole:
            return item->name;
        case Qt::DecorationRole:
            return item->is_directory ? m_dir_icon : m_file_icon;
        }
        break;
    case COL_SIZE:
        if(_role == Qt::DisplayRole)
        {
            if(item->is_directory)
                return QObject::tr("<directory>");
            return makeBytesDisplayString(item->size);
        }
        break;
    }
    return QVariant();
}

void VmcFileSystemViewModel::sort(int _column, Qt::SortOrder _order)
{
    beginResetModel();
    m_sorted_by_column = _column;
    m_sort_order = _order;
    unsafeSort();
    endResetModel();
}

void VmcFileSystemViewModel::unsafeSort()
{
    std::stable_sort(m_item_indices.begin(), m_item_indices.end(), [&](const int & left_index, const int & right_index) {
        const VmcEntryInfo & left = m_items.at(left_index);
        const VmcEntryInfo & right = m_items.at(right_index);
        if(left.is_directory != right.is_directory)
            return left.is_directory;
        switch(m_sorted_by_column)
        {
        case COL_TTITLE:
        {
            int result = left.name.compare(right.name, Qt::CaseInsensitive);
            return m_sort_order == Qt::AscendingOrder ? result < 0 : result > 0;
        }
        case COL_SIZE:
            return m_sort_order == Qt::AscendingOrder ? left.size < right.size : right.size < left.size;
        default:
            return false;
        }
    });
}

VmcDetailsActivity::VmcDetailsActivity(const Vmc & _vmc, QWidget * _parent /*= nullptr*/) :
    Activity(_parent),
    mr_vmc(_vmc),
    mp_model(nullptr)
{
    setupUi(this);
    hideErrorMessage();
    mp_label_vmc_title->setText(mr_vmc.title());
    loadVmcFS();
    if(m_fs_ptr)
        setupView();
}

void VmcDetailsActivity::showErrorMessage(const QString & _message /*= QString()*/)
{
    mp_label_error_message->setText(_message.isEmpty() ? tr("An unknown error has occurred") : _message);
    mp_widget_error_message->show();
}

void VmcDetailsActivity::hideErrorMessage()
{
    mp_widget_error_message->hide();
}

void VmcDetailsActivity::loadVmcFS()
{
    try
    {
        m_fs_ptr = VmcFS::load(mr_vmc.filepath());
    }
    catch(const Exception &_exception)
    {
        showErrorMessage(_exception.message());
    }
    catch(...)
    {
        showErrorMessage();
    }
}

void VmcDetailsActivity::setupView()
{
    mp_model = new VmcFileSystemViewModel(this);
    navigate(QString::fromLatin1(&VmcFS::path_separator, 1));
    mp_tree_fs->setModel(mp_model);
    QStandardItemModel * header_model = new QStandardItemModel(mp_tree_fs);
    header_model->setHorizontalHeaderLabels({ tr("Name"), tr("Size") });
    mp_tree_fs->header()->setModel(header_model);
    mp_tree_fs->header()->setStretchLastSection(false);
    mp_tree_fs->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mp_tree_fs->sortByColumn(0, Qt::AscendingOrder);
    connect(mp_tree_fs, &QTreeView::doubleClicked, this, &VmcDetailsActivity::onFsListItemDoubleClicked);
    connect(mp_btn_fs_back, &QToolButton::clicked, this, &VmcDetailsActivity::onFsBackButtonClick);
    connect(&Settings::instance(), &Settings::iconSizeChanged, this, &VmcDetailsActivity::setIconSize);
    connect(mp_btn_rename, &QToolButton::clicked, this, &VmcDetailsActivity::renameVmc);
    connect(mp_label_vmc_title, &ClickableLabel::clicked, this, &VmcDetailsActivity::renameVmc);
    setIconSize();
}

void VmcDetailsActivity::setIconSize()
{
    QSize size;
    size.setWidth(Settings::instance().iconSize());
    size.setHeight(size.width());
    mp_tree_fs->setIconSize(size);
}

void VmcDetailsActivity::navigate(const QString & _path)
{
    hideErrorMessage();
    try
    {
        QList<VmcEntryInfo> items = m_fs_ptr->enumerateEntries(_path);
        mp_model->setItems(items);
        mp_edit_fs_path->setText(_path);
        mp_btn_fs_back->setDisabled(_path.isEmpty() || (_path.size() == 1 && _path[0] == VmcFS::path_separator));
    }
    catch(const Exception &_exception)
    {
        showErrorMessage(_exception.message());
    }
    catch(...)
    {
        showErrorMessage();
    }
}

void VmcDetailsActivity::onFsListItemDoubleClicked(const QModelIndex & _index)
{
    const VmcEntryInfo * item = mp_model->item(_index);
    if(item == nullptr)
        return;
    if(item->is_directory)
    {
        QString path = mp_edit_fs_path->text() + item->name + VmcFS::path_separator;
        navigate(path);
    }
}

void VmcDetailsActivity::onFsBackButtonClick()
{
    QStringList parts = mp_edit_fs_path->text().split(VmcFS::path_separator, QString::SkipEmptyParts);
    if(parts.count() == 0)
        return;
    parts.removeLast();
    QString path = QString(VmcFS::path_separator) + parts.join(VmcFS::path_separator);
    if(parts.count() > 0)
        path += VmcFS::path_separator;
    navigate(path);
}

QSharedPointer<Intent> VmcDetailsActivity::createIntent(const Vmc & _vmc)
{
    return QSharedPointer<Intent>(new VmcDetailsActivityIntent(_vmc));
}

void VmcDetailsActivity::renameVmc()
{
    VmcRenameDialog dlg(mr_vmc.title(), this);
    if(dlg.exec() != QDialog::Accepted)
        return;
    try
    {
        Library::instance().vmcs().renameVmc(mr_vmc.uuid(), dlg.name());
        mp_label_vmc_title->setText(dlg.name());
    }
    catch(const Exception & _exception)
    {
        Application::showErrorMessage(_exception.message());
    }
    catch(...)
    {
        Application::showErrorMessage();
    }
}
