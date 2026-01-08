/***********************************************************************************************
 * Copyright © 2017-2026 Sergey Smolyannikov aka brainstream                                   *
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

#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/VmcRenameDialog.h>
#include <OplPcTools/UI/VmcDetailsActivity.h>
#include <OplPcTools/UI/DisplayUtils.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/TextEncoding.h>
#include <OplPcTools/Settings.h>
#include <QStandardItemModel>
#include <QShortcut>

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

} // namespace


namespace OplPcTools::UI {

class VmcFileSystemViewModel : public QAbstractItemModel
{
public:
    explicit VmcFileSystemViewModel(const QString & _encoding, QObject * _parent = nullptr);
    void setEncoding(const QString & _encoding);
    void setItems(const QList<MCFS::EntryInfo> & _items);
    void clear();
    const MCFS::EntryInfo * item(const QModelIndex & _index) const;
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
    TextDecoder m_string_decoder;
    QList<MCFS::EntryInfo> m_items;
    std::vector<int> m_item_indices;
    QIcon m_file_icon;
    QIcon m_dir_icon;
    int m_sorted_by_column;
    Qt::SortOrder m_sort_order;
};

} // namespace OplPcTools::UI

VmcFileSystemViewModel::VmcFileSystemViewModel(const QString & _encoding, QObject * _parent /*= nullptr*/) :
    QAbstractItemModel(_parent),
    m_string_decoder(_encoding),
    m_file_icon(":/images/file"),
    m_dir_icon(":/images/folder"),
    m_sorted_by_column(COL_TTITLE),
    m_sort_order(Qt::AscendingOrder)
{
}

void VmcFileSystemViewModel::setEncoding(const QString & _encoding)
{
    if(m_string_decoder.codecName() == _encoding)
        return;
    beginResetModel();
    m_string_decoder = TextDecoder(_encoding);
    endResetModel();
}

void VmcFileSystemViewModel::setItems(const QList<MCFS::EntryInfo> & _items)
{
    beginResetModel();
    m_items = _items;
    m_item_indices.resize(_items.size());
    for(size_t i = 0; i < m_item_indices.size(); ++i)
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

const MCFS::EntryInfo * VmcFileSystemViewModel::item(const QModelIndex & _index) const
{
    if(_index.isValid() && _index.row() < m_items.size() && _index.row() >= 0)
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
    Q_UNUSED(_child)
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
    Q_UNUSED(_parent)
    return column_count;
}

QVariant VmcFileSystemViewModel::data(const QModelIndex & _index, int _role) const
{
    const MCFS::EntryInfo * item = this->item(_index);
    if(!item)
        return QVariant();
    switch(_index.column())
    {
    case COL_TTITLE:
        switch(_role)
        {
        case Qt::DisplayRole:
        {
            return QString(m_string_decoder.decode(item->name));
        }
        case Qt::DecorationRole:
            return item->is_directory ? m_dir_icon : m_file_icon;
        }
        break;
    case COL_SIZE:
        switch(_role)
        {
        case Qt::DisplayRole:
            if(item->is_directory)
                return QObject::tr("<directory>");
            return makeBytesDisplayString(item->length);
        case Qt::TextAlignmentRole:
            return static_cast<int>(Qt::AlignRight) | static_cast<int>(Qt::AlignVCenter);
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
        const MCFS::EntryInfo & left = m_items.at(left_index);
        const MCFS::EntryInfo & right = m_items.at(right_index);
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
            return m_sort_order == Qt::AscendingOrder ? left.length < right.length : right.length < left.length;
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
    setupShortcuts();
    hideErrorMessage();
    mp_label_vmc_title->setText(mr_vmc.title());
    loadFileManager();
    QString encoding = getFsEncoding();
    {
        QStringList codecs = TextEncoding::availableCodecs();
        codecs.sort(Qt::CaseInsensitive);
        if(!codecs.contains(encoding))
            encoding = TextEncoding::latin1();
        mp_combobox_charset->addItems(codecs);
        mp_combobox_charset->setCurrentText(encoding);
    }
    if(m_vmc_driver_ptr)
    {
        mp_model = new VmcFileSystemViewModel(encoding, this);
        navigate(MCFS::Path::root());
        mp_tree_fs->setModel(mp_model);
        QStandardItemModel * header_model = new QStandardItemModel(mp_tree_fs);
        header_model->setHorizontalHeaderLabels({ tr("Title"), tr("Size") });
        header_model->horizontalHeaderItem(1)->setTextAlignment(Qt::Alignment(Qt::AlignRight));
        mp_tree_fs->header()->setModel(header_model);
        mp_tree_fs->header()->setStretchLastSection(false);
        mp_tree_fs->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        mp_tree_fs->sortByColumn(0, Qt::AscendingOrder);
        {
            const uint32_t total_free_bytes = m_vmc_driver_ptr->totalFreeBytes();
            const uint32_t total_used_bytes = m_vmc_driver_ptr->totalUsedBytes();
            const uint32_t total_bytes = total_used_bytes + total_free_bytes;
            mp_progress_bar_free->setMinimum(0);
            mp_progress_bar_free->setMaximum(total_bytes);
            mp_progress_bar_free->setValue(total_used_bytes);
            mp_progress_bar_free->setFormat(tr("Free %1 of %2").arg(
                makeBytesDisplayString(total_free_bytes),
                makeBytesDisplayString(total_bytes)));
        }
        connect(mp_tree_fs, &QTreeView::activated, this, &VmcDetailsActivity::onFsListItemActivated);
        connect(mp_btn_fs_back, &QToolButton::clicked, this, &VmcDetailsActivity::onFsBackButtonClick);
        connect(&Settings::instance(), &Settings::iconSizeChanged, this, &VmcDetailsActivity::setIconSize);
        connect(mp_btn_rename, &QToolButton::clicked, this, &VmcDetailsActivity::renameVmc);
        connect(mp_label_vmc_title, &ClickableLabel::clicked, this, &VmcDetailsActivity::renameVmc);
        connect(mp_combobox_charset, &QComboBox::currentTextChanged, this, &VmcDetailsActivity::onEncodingChanged);
        setIconSize();
    }



    // QPushButton * create_dir_btn = new QPushButton("Create Directory", this);
    // mp_layout_3->addWidget(create_dir_btn);
    // connect(create_dir_btn, &QPushButton::clicked, this, [this]() {
    //     m_vmc_driver_ptr->createDirectory(MCFS::Path("/TEST_DATA"));
    // });

    // QPushButton * create_file_btn = new QPushButton("Create File", this);
    // mp_layout_3->addWidget(create_file_btn);
    // connect(create_file_btn, &QPushButton::clicked, this, [this]() {
    //     m_vmc_driver_ptr->writeFile(MCFS::Path("/TEST_DATA/test.txt"), QString("Hello, World").toLatin1());
    // });




}

void VmcDetailsActivity::setupShortcuts()
{
    QShortcut * shortcut = new QShortcut(QKeySequence("Back"), this);
    connect(shortcut, &QShortcut::activated, this, &VmcDetailsActivity::close);
    shortcut = new QShortcut(QKeySequence("Esc"), this);
    connect(shortcut, &QShortcut::activated, this, &VmcDetailsActivity::close);
    shortcut = new QShortcut(QKeySequence(static_cast<int>(Qt::ALT) | static_cast<int>(Qt::Key_Left)), this);
    connect(shortcut, &QShortcut::activated, mp_btn_fs_back, &QToolButton::click);
    shortcut = new QShortcut(QKeySequence(Qt::Key_PageUp), this);
    connect(shortcut, &QShortcut::activated, mp_btn_fs_back, &QToolButton::click);
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

void VmcDetailsActivity::loadFileManager()
{
    try
    {
        QSharedPointer<MCFS::FileSystemDriver> driver(new MCFS::FileSystemDriver(mr_vmc.filepath()));
        driver->load();
        m_vmc_driver_ptr = std::move(driver);
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

QString VmcDetailsActivity::getFsEncoding() const
{
    QString cahrset = Library::instance().config().vmcFsCharset(mr_vmc);
    if(cahrset.isEmpty())
        cahrset = Settings::instance().defaultVmcFsCharset();
    return cahrset;
}

void VmcDetailsActivity::setIconSize()
{
    QSize size;
    size.setWidth(Settings::instance().iconSize());
    size.setHeight(size.width());
    mp_tree_fs->setIconSize(size);
}

void VmcDetailsActivity::navigate(const MCFS::Path & _path)
{
    hideErrorMessage();
    try
    {
        QList<MCFS::EntryInfo> items = m_vmc_driver_ptr->enumerateEntries(_path);
        mp_model->setItems(items);
        mp_edit_fs_path->setText(


            QString::fromLatin1(_path) // FIXME: encoder!


        );
        mp_btn_fs_back->setDisabled(_path.isRoot());
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

void VmcDetailsActivity::onFsListItemActivated(const QModelIndex & _index)
{
    const MCFS::EntryInfo * item = mp_model->item(_index);
    if(item == nullptr)
        return;
    if(item->is_directory)
    {
        navigate(MCFS::Path(


            mp_edit_fs_path->text().toLatin1(), // FIXME: decoder!
            item->name

        ));
    }
}

void VmcDetailsActivity::onFsBackButtonClick()
{
    MCFS::Path path = MCFS::Path(


        mp_edit_fs_path->text().toLatin1() // FIXME: decoder!


    );
    if(!path.isRoot())
        navigate(path.up());
}

void VmcDetailsActivity::onEncodingChanged()
{
    QString encoding = mp_combobox_charset->currentText();
    mp_model->setEncoding(encoding);
    Library::instance().config().setVmcFsCharset(mr_vmc, encoding);
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
