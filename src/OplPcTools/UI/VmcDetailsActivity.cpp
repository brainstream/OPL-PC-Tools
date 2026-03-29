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
#include <OplPcTools/UI/VmcFileNameDialog.h>
#include <OplPcTools/UI/VmcDetailsActivity.h>
#include <OplPcTools/UI/DisplayUtils.h>
#include <OplPcTools/UI/VmcExporter.h>
#include <OplPcTools/UI/ProgressDialog.h>
#include <OplPcTools/UI/BusySmartThread.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/StringConverter.h>
#include <OplPcTools/Settings.h>
#include <OplPcTools/File.h>
#include <QFileDialog>
#include <QCheckBox>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QShortcut>
#include <QMimeData>
#include <QThreadPool>

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

class VmcProgressDialog : public ProgressDialog
{
public:
    explicit VmcProgressDialog(QWidget * _parent) :
        ProgressDialog(_parent),
        m_tracker(_parent)
    {
        connect(&m_tracker, &MemoryCard::FileTransferProgressTracker::progress, this, &VmcProgressDialog::updateProgress);
    }

    MemoryCard::FileTransferProgressTracker & tracker()
    {
        return m_tracker;
    }

private:
    void updateProgress(qsizetype _total_bytes, qsizetype _done_bytes, qsizetype _delta)
    {
        Q_UNUSED(_total_bytes)
        Q_UNUSED(_done_bytes)
        setProgressValue(progressValue() + static_cast<int>(_delta));
    }

private:
    MemoryCard::FileTransferProgressTracker m_tracker;
};

} // namespace


namespace OplPcTools::UI {

class VmcFileSystemViewModel : public QAbstractItemModel
{
public:
    explicit VmcFileSystemViewModel(const QString & _encoding, QObject * _parent = nullptr);
    void setEncoding(const QString & _encoding);
    void setItems(const QList<MemoryCard::EntryInfo> & _items);
    void clear();
    const MemoryCard::EntryInfo * item(const QModelIndex & _index) const;
    QModelIndex index(int _row, int _column, const QModelIndex & _parent) const override;
    QModelIndex parent(const QModelIndex & _child) const override;
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    void sort(int _column, Qt::SortOrder _order) override;
    const StringConverter & stringConverter() const;
    bool canDropMimeData(
        const QMimeData * _data,
        Qt::DropAction _action,
        int _row,
        int _column,
        const QModelIndex & _parent) const override;

private:
    void unsafeSort();

private:
    enum ColumnIndex
    {
        COL_TTITLE,
        COL_SIZE,
        column_count // It must be the last item
    };

private:
    StringConverter m_string_converter;
    QList<MemoryCard::EntryInfo> m_items;
    std::vector<int> m_item_indices;
    QIcon m_file_icon;
    QIcon m_dir_icon;
    int m_sorted_by_column;
    Qt::SortOrder m_sort_order;
};

} // namespace OplPcTools::UI

VmcFileSystemViewModel::VmcFileSystemViewModel(const QString & _encoding, QObject * _parent /*= nullptr*/) :
    QAbstractItemModel(_parent),
    m_string_converter(_encoding),
    m_file_icon(":/images/file"),
    m_dir_icon(":/images/folder"),
    m_sorted_by_column(COL_TTITLE),
    m_sort_order(Qt::AscendingOrder)
{
}

void VmcFileSystemViewModel::setEncoding(const QString & _encoding)
{
    if(m_string_converter.codecName() == _encoding)
        return;
    beginResetModel();
    m_string_converter = StringConverter(_encoding);
    endResetModel();
}

void VmcFileSystemViewModel::setItems(const QList<MemoryCard::EntryInfo> & _items)
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

const MemoryCard::EntryInfo * VmcFileSystemViewModel::item(const QModelIndex & _index) const
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
    const MemoryCard::EntryInfo * item = this->item(_index);
    if(!item)
        return QVariant();
    switch(_index.column())
    {
    case COL_TTITLE:
        switch(_role)
        {
        case Qt::DisplayRole:
        {
            return QString(m_string_converter.decode(item->name()));
        }
        case Qt::DecorationRole:
            return item->isDirectory() ? m_dir_icon : m_file_icon;
        }
        break;
    case COL_SIZE:
        switch(_role)
        {
        case Qt::DisplayRole:
            if(item->isDirectory())
                return QObject::tr("<directory>");
            return makeBytesDisplayString(item->length());
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
        const MemoryCard::EntryInfo & left = m_items.at(left_index);
        const MemoryCard::EntryInfo & right = m_items.at(right_index);
        if(left.isDirectory() != right.isDirectory())
            return left.isDirectory();
        switch(m_sorted_by_column)
        {
        case COL_TTITLE:
        {
            int result = left.name().compare(right.name(), Qt::CaseInsensitive);
            return m_sort_order == Qt::AscendingOrder ? result < 0 : result > 0;
        }
        case COL_SIZE:
            return m_sort_order == Qt::AscendingOrder ? left.length() < right.length() : right.length() < left.length();
        default:
            return false;
        }
    });
}

inline const StringConverter & VmcFileSystemViewModel::stringConverter() const
{
    return m_string_converter;
}

inline bool VmcFileSystemViewModel::canDropMimeData(
    const QMimeData * _data,
    Qt::DropAction _action,
    int _row,
    int _column,
    const QModelIndex & _parent) const
{
    Q_UNUSED( _data)
    Q_UNUSED(_action)
    Q_UNUSED(_row)
    Q_UNUSED(_column)
    Q_UNUSED(_parent)
    return _data->hasUrls();
}

VmcDetailsActivity::VmcDetailsActivity(const Vmc & _vmc, QWidget * _parent /*= nullptr*/) :
    Activity(_parent),
    mr_vmc(_vmc),
    mp_vmc_fs(nullptr),
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
    if(mp_vmc_fs)
    {
        mp_model = new VmcFileSystemViewModel(encoding, this);
        navigate(MemoryCard::Path::root());
        mp_tree_fs->setModel(mp_model);
        QStandardItemModel * header_model = new QStandardItemModel(mp_tree_fs);
        header_model->setHorizontalHeaderLabels({ tr("Title"), tr("Size") });
        header_model->horizontalHeaderItem(1)->setTextAlignment(Qt::Alignment(Qt::AlignRight));
        mp_tree_fs->header()->setModel(header_model);
        mp_tree_fs->header()->setStretchLastSection(false);
        mp_tree_fs->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        mp_tree_fs->sortByColumn(0, Qt::AscendingOrder);
        updateSpaceInfo();
        addAction(mp_action_create_directory);
        addAction(mp_action_rename_entry);
        addAction(mp_action_delete);
        addAction(mp_action_download);
        addAction(mp_action_upload_files);
        addAction(mp_action_upload_directory);
        connect(mp_vmc_fs, &MemoryCard::FileSystem::changed, this, &VmcDetailsActivity::onFsChanged);
        connect(mp_tree_fs, &QTreeView::activated, this, &VmcDetailsActivity::onFsListItemActivated);
        connect(mp_tree_fs, &QTreeView::customContextMenuRequested, this, &VmcDetailsActivity::showTreeContextMenu);
        connect(mp_tree_fs, &VmcFileSystemTreeView::filesDropped, this, &VmcDetailsActivity::uploadDroppedData);
        connect(mp_btn_fs_back, &QToolButton::clicked, this, &VmcDetailsActivity::onFsBackButtonClick);
        connect(&Settings::instance(), &Settings::iconSizeChanged, this, &VmcDetailsActivity::setIconSize);
        connect(mp_btn_rename, &QToolButton::clicked, this, &VmcDetailsActivity::renameVmc);
        connect(mp_label_vmc_title, &ClickableLabel::clicked, this, &VmcDetailsActivity::renameVmc);
        connect(mp_combobox_charset, &QComboBox::currentTextChanged, this, &VmcDetailsActivity::onEncodingChanged);
        connect(mp_action_create_directory, &QAction::triggered, this, &VmcDetailsActivity::createDirectory);
        connect(mp_action_rename_entry, &QAction::triggered, this, &VmcDetailsActivity::renameEntry);
        connect(mp_action_upload_files, &QAction::triggered, this, &VmcDetailsActivity::uploadFiles);
        connect(mp_action_upload_directory, &QAction::triggered, this, &VmcDetailsActivity::uploadDirectory);
        connect(mp_action_download, &QAction::triggered, this, &VmcDetailsActivity::download);
        connect(mp_action_delete, &QAction::triggered, this, &VmcDetailsActivity::deleteEntry);

        setIconSize();
    }
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

void VmcDetailsActivity::showErrorMessage(const QString & _message, const QByteArray & _path /*= QByteArray()*/)
{
    if(qApp->thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "showErrorMessage", Q_ARG(QString, _message), Q_ARG(QByteArray, _path));
    }
    else
    {
        QString message = _message.isEmpty() ? tr("An unknown error has occurred") : _message;
        if(!_path.isEmpty())
            message += QString(" (%1)").arg(decodePath(_path));
        mp_label_error_message->setText(message);
        mp_widget_error_message->show();
    }
}

void VmcDetailsActivity::hideErrorMessage()
{
    mp_widget_error_message->hide();
}

void VmcDetailsActivity::loadFileManager()
{
    handleErrors([this]
    {
        std::unique_ptr<MemoryCard::FileSystem> fs(new MemoryCard::FileSystem(mr_vmc.filepath(), this));
        fs->load();
        if(mp_vmc_fs)
            delete mp_vmc_fs;
        mp_vmc_fs = fs.release();
    });
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

void VmcDetailsActivity::navigate(const MemoryCard::Path & _path)
{
    hideErrorMessage();
    handleErrors([this, &_path]
    {
        QList<MemoryCard::EntryInfo> items = mp_vmc_fs->enumerateEntries(_path);
        mp_model->setItems(items);
        mp_edit_fs_path->setText(decodePath(_path));
        mp_btn_fs_back->setDisabled(_path.isRoot());
    });
}

QByteArray VmcDetailsActivity::encodePath(const QString & _path) const
{
    return mp_model->stringConverter().encode(_path);
}

QString VmcDetailsActivity::decodePath(const QByteArray & _path) const
{
    return mp_model->stringConverter().decode(_path);
}

void VmcDetailsActivity::updateSpaceInfo()
{
    const uint32_t total_free_bytes = mp_vmc_fs->totalFreeBytes();
    const uint32_t total_used_bytes = mp_vmc_fs->totalUsedBytes();
    const uint32_t total_bytes = total_used_bytes + total_free_bytes;
    mp_progress_bar_free->setMinimum(0);
    mp_progress_bar_free->setMaximum(total_bytes);
    mp_progress_bar_free->setValue(total_used_bytes);
    mp_progress_bar_free->setFormat(tr("Free %1 of %2").arg(
        makeBytesDisplayString(total_free_bytes),
        makeBytesDisplayString(total_bytes)));
}

void VmcDetailsActivity::onFsChanged()
{
    const MemoryCard::Path vmc_current_dir = encodePath(mp_edit_fs_path->text());
    mp_model->setItems(mp_vmc_fs->enumerateEntries(vmc_current_dir));
    updateSpaceInfo();
}

void VmcDetailsActivity::onFsListItemActivated(const QModelIndex & _index)
{
    const MemoryCard::EntryInfo * item = mp_model->item(_index);
    if(item == nullptr)
        return;
    if(item->isDirectory())
        navigate(MemoryCard::Path(encodePath(mp_edit_fs_path->text()), item->name()));
}

void VmcDetailsActivity::onFsBackButtonClick()
{
    MemoryCard::Path path = MemoryCard::Path(encodePath(mp_edit_fs_path->text()));
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
    handleErrors([this]
    {
        VmcRenameDialog dlg(mr_vmc.title(), this);
        if(dlg.exec() != QDialog::Accepted)
            return;
        Library::instance().vmcs().renameVmc(mr_vmc.uuid(), dlg.name());
        mp_label_vmc_title->setText(dlg.name());
    });
}

void VmcDetailsActivity::showTreeContextMenu(const QPoint & _point)
{
    QMenu menu;
    if(mp_tree_fs->currentIndex().isValid())
    {
        menu.addAction(mp_action_rename_entry);
        menu.addAction(mp_action_download);
        menu.addAction(mp_action_delete);
        menu.addSeparator();
    }
    menu.addAction(mp_action_create_directory);
    QMenu * upload_menu = menu.addMenu(tr("Upload"));
    upload_menu->setIcon(QIcon(":/images/upload"));
    upload_menu->addAction(mp_action_upload_files);
    upload_menu->addAction(mp_action_upload_directory);
    menu.exec(mp_tree_fs->mapToGlobal(_point + QPoint(0, mp_tree_fs->header()->height())));
}

void VmcDetailsActivity::createDirectory()
{
    VmcFileNameDialog dlg(this);
    dlg.setTitle(true);
    if(dlg.exec() != QDialog::Accepted)
        return;
    const MemoryCard::Path vmc_current_dir = encodePath(mp_edit_fs_path->text());
    mp_vmc_fs->createDirectory(vmc_current_dir + encodePath(dlg.currentFilename()));
}

void VmcDetailsActivity::renameEntry()
{
    const MemoryCard::EntryInfo * entry = mp_model->item(mp_tree_fs->currentIndex());
    if(!entry)
        return;

    VmcFileNameDialog dlg(this);
    dlg.setTitle(entry->isDirectory());
    dlg.setCurrentFilename(decodePath(entry->name()));
    if(dlg.exec() != QDialog::Accepted)
        return;

    const MemoryCard::Path vmc_current_dir(encodePath(mp_edit_fs_path->text()));
    mp_vmc_fs->rename(vmc_current_dir + encodePath(entry->name()), encodePath(dlg.currentFilename()));
}

void VmcDetailsActivity::uploadDroppedData(const QMimeData & _data)
{
    handleErrors([this, &_data]
    {
        MemoryCard::Path vmc_destination_dir(encodePath(mp_edit_fs_path->text()));
        foreach(const QUrl & url, _data.urls())
        {
            QFileInfo fi(url.path());
            if(!fi.exists())
                continue;
            else if(fi.isDir())
                uploadDirectoryImpl(fi.absoluteFilePath(), vmc_destination_dir, nullptr);
            else if(fi.isFile())
                uploadFileImpl(fi.absoluteFilePath(), vmc_destination_dir, nullptr);
        }
    });
}

void VmcDetailsActivity::uploadFiles()
{
    QStringList filenames = QFileDialog::getOpenFileNames(
        this,
        QString(),
        Settings::instance().path(Settings::Directory::VmcImport));
    if(!filenames.empty())
        Settings::instance().setPath(Settings::Directory::VmcImport, QFileInfo(filenames.first()).absolutePath());

    VmcProgressDialog * progress_dialog = new VmcProgressDialog(this);
    {
        progress_dialog->disableCancelation(true);
        progress_dialog->setWindowTitle(tr("Uploading files"));
        qint64 total_bytes = std::accumulate(
            filenames.begin(),
            filenames.end(),
            qint64(0),
            [](qint64 __size, const QString & __fn) { return __size + QFileInfo(__fn).size(); });
        progress_dialog->setProgressRange(0, static_cast<int>(total_bytes));
    }

    std::function<void()> lambda = [this, filenames, progress_dialog]()
    {
        handleErrors([this, filenames, progress_dialog]
        {
            const MemoryCard::Path vmc_current_dir(encodePath(mp_edit_fs_path->text()));
            foreach(const QString & filename, filenames)
            {
                progress_dialog->setProgressLabelText(QFileInfo(filename).fileName());
                uploadFileImpl(filename, vmc_current_dir, &progress_dialog->tracker());
            }
        });
    };
    BusySmartThread * thread = new BusySmartThread(lambda, progress_dialog, this);
    connect(thread, &BusySmartThread::finished, thread, &QObject::deleteLater);
    connect(thread, &BusySmartThread::exception, [](const QString & message) {
        Application::showErrorMessage(message);
    });
    thread->start();
}

void VmcDetailsActivity::uploadDirectoryImpl(
    const QString & _directory_path,
    const MemoryCard::Path & _dest_dir,
    MemoryCard::FileTransferProgressTracker * _tracker)
{
    QDir directory(_directory_path);
    if(!directory.exists())
        return;

    const MemoryCard::Path vmc_dir(_dest_dir, encodePath(directory.dirName()));
    mp_vmc_fs->createDirectory(vmc_dir);

    foreach(
        const QFileInfo & entry,
        directory.entryInfoList(QDir::NoDotAndDotDot | QDir::CaseSensitive | QDir::Dirs | QDir::Files))
    {
        if(entry.isFile())
            uploadFileImpl(entry.absoluteFilePath(), vmc_dir, _tracker);
        else if(entry.isDir())
            uploadDirectoryImpl(entry.absoluteFilePath(), vmc_dir, _tracker);
    }
}

void VmcDetailsActivity::uploadFileImpl(
    const QString & _file_path,
    const MemoryCard::Path & _dest_dir,
    MemoryCard::FileTransferProgressTracker * _tracker)
{
    QFile file(_file_path);
    openFile(file, QIODevice::ReadOnly);
    QByteArray content = file.readAll();
    mp_vmc_fs->writeFile(_dest_dir + encodePath(QFileInfo(file).fileName()), content, _tracker);
}

void VmcDetailsActivity::uploadDirectory()
{
    handleErrors([this]
    {
        const QString directory_path = QFileDialog::getExistingDirectory(
            this,
            QString(),
            Settings::instance().path(Settings::Directory::VmcImport));
        if(directory_path.isEmpty())
            return;
        Settings::instance().setPath(Settings::Directory::VmcImport, directory_path);
        const MemoryCard::Path vmc_current_dir(encodePath(mp_edit_fs_path->text()));
        uploadDirectoryImpl(directory_path, vmc_current_dir, nullptr);
    });
}

void VmcDetailsActivity::handleErrors(std::function<void()> _lambda)
{
    try
    {
        _lambda();
    }
    catch(const MemoryCard::MemoryCardFileException & exception)
    {
        showErrorMessage(exception.message(), exception.path());
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

void VmcDetailsActivity::download()
{
    QList<MemoryCard::Path> paths;
    QString directory_path;
    handleErrors([this, &paths, &directory_path]
    {
        const MemoryCard::Path vmc_current_dir(encodePath(mp_edit_fs_path->text()));
        Settings & settings = Settings::instance();
        directory_path = QFileDialog::getExistingDirectory(
            this, QString(), settings.path(Settings::Directory::VmcExport));
        if(directory_path.isEmpty())
            return;
        settings.setPath(Settings::Directory::VmcExport,  directory_path);
        QModelIndexList selection = mp_tree_fs->selectionModel()->selectedRows();
        if(selection.empty())
        {
            int count = mp_model->rowCount(mp_tree_fs->rootIndex());
            for(int i = 0; i < count; ++i)
                selection.append(mp_model->index(i, 0, mp_tree_fs->rootIndex()));
        }
        paths.reserve((selection.count()));
        foreach(const QModelIndex & index, selection)
        {
            const MemoryCard::EntryInfo * entry = mp_model->item(index);
            if(!entry) continue;
            paths.append(MemoryCard::Path(vmc_current_dir, entry->name()));
        }
    });

    if(paths.empty())
        return;

    std::function<void()> lambda = [this, paths, directory_path]
    {
        VmcExporter * exportert = new VmcExporter(mr_vmc, mp_model->stringConverter(), paths, directory_path);
        handleErrors([exportert] { exportert->run(); });
        delete exportert;
    };
    BusySmartThread * thread = new BusySmartThread(lambda, nullptr, this);
    // When the progress dialog appears, the question dialog is behind it and can't get focus for clicking the button.
    // Therefore, we display the progress dialog immediately, without delay.
    thread->setSpinnerDisplayTimeout(0);
    connect(thread, &BusySmartThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void VmcDetailsActivity::deleteEntry()
{
    handleErrors([this]
    {
        const MemoryCard::Path vmc_current_dir(encodePath(mp_edit_fs_path->text()));
        const QModelIndexList selection = mp_tree_fs->selectionModel()->selectedRows();

        Settings & settings = Settings::instance();
        QList<QByteArray> entries;
        entries.reserve(selection.count());
        foreach(const QModelIndex & index, selection)
        {
            const MemoryCard::EntryInfo * entry = mp_model->item(index);
            if(entry) entries.append(entry->name());
        }

        if(settings.confirmVmcFileDeletion())
        {
            QString items;
            int display_items_count = 0;
            foreach(const QByteArray & entry, entries)
            {
                if(display_items_count == 9)
                {
                    items += QString("\n...");
                    break;
                }
                items += QString("\n") + mp_model->stringConverter().decode(entry);
                ++display_items_count;
            }
            QString message =
                display_items_count == 1 ?
                tr("Are you sure you want to delete this file?") :
                tr("Are you sure you want to delete these files?");
            QCheckBox * checkbox = new QCheckBox(tr("Do not ask again"));
            QMessageBox message_box(QMessageBox::Question, tr("Delete VMC files"),
                                    QString("%1\n%2\n").arg(message, items),
                                    QMessageBox::Yes | QMessageBox::No);
            message_box.setDefaultButton(QMessageBox::Yes);
            message_box.setCheckBox(checkbox);
            if(message_box.exec() != QMessageBox::Yes)
                return;
            if(checkbox->isChecked())
                settings.setConfirmVmcFileDeletion(false);
        }

        foreach(const QByteArray & entry, entries)
            mp_vmc_fs->remove(vmc_current_dir + entry);
    });
}
