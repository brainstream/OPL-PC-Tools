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
#include <OplPcTools/UI/ChooseOpticalDiscDialog.h>
#include <OplPcTools/UI/GameRenameDialog.h>
#include <OplPcTools/UI/GameInstallerActivity.h>
#include <OplPcTools/UI/ProgressBarItemDelegate.h>
#include <OplPcTools/Device/DeviceReader.h>
#include <OplPcTools/Device/Iso9660DeviceSource.h>
#include <OplPcTools/Device/ZisoDeviceSource.h>
#include <OplPcTools/Device/BinCueDeviceSource.h>
#include <OplPcTools/Device/NrgDeviceSource.h>
#include <OplPcTools/Device/OpticalDriveDeviceSource.h>
#include <OplPcTools/Settings.h>
#include <OplPcTools/UlConfigGameInstaller.h>
#include <OplPcTools/DirectoryGameInstaller.h>
#include <OplPcTools/Device/DefaultDeviceWriter.h>
#include <OplPcTools/Device/CompressedDeviceWriter.h>
#include <OplPcTools/Constants.h>
#include <QShortcut>
#include <QFileInfo>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

namespace Column {
enum
{
    Title,
    SourceFormat,
    TargetFormat,
    Moving,
    Status,

    __ColumnCount
};
} // namespace Column

const int g_progressbar_max_value = 1000;

enum class GameInstallationStatus
{
    Queued,
    Installation,
    Registration,
    Done,
    Error,
    RollingBack
};

class GameInstallerActivityIntent : public Intent
{
public:
    Activity * createActivity(QWidget * _parent) override
    {
        return new GameInstallerActivity(_parent);
    }

    QString activityClass() const override
    {
        return "GameInstaller";
    }
};

struct InstallationTask
{
    QSharedPointer<DeviceReader> device_ptr;
    GameSourceFormat source_format;
    GameInstallationType target_installation_type;
    GameInstallationStatus status;
    int progress;
    QString error_message;
    bool is_moving_requested;

    bool canFileBeMoved() const
    {
        return (
                source_format == GameSourceFormat::Iso9660 &&
                target_installation_type == GameInstallationType::Iso9660)
            || (
                source_format == GameSourceFormat::Ziso &&
                target_installation_type == GameInstallationType::Ziso);
    }
};

} // namespace

class GameInstallerActivity::TaskListModel : public QAbstractListModel, public ProgressBarItemDelegateSource
{
public:
    explicit TaskListModel(QObject * _parent);
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    QVariant headerData(int _section, Qt::Orientation _orientation, int _role) const override;
    void addTask(QSharedPointer<DeviceReader> _device, GameSourceFormat _format);
    qsizetype taskCount() const;
    bool isProgressBarEnabled(const QModelIndex & _index) const override;
    int maxProgressValue(const QModelIndex & _index) const override;
    int currentProgressValue(const QModelIndex & _index) const override;
    const InstallationTask * task(qsizetype _index) const;
    QModelIndex indexByDeviceFilepath(const QString & _device_filepath) const;
    void setTargetInstallationType(qsizetype _index, GameInstallationType _format);
    void setMoveRequest(qsizetype _index, bool _requested);
    void removeTask(qsizetype _index);
    qsizetype taskForNextStart() const;
    void setTaskTitle(qsizetype _index, const QString & _title);
    void setTaskStatus(qsizetype _index, GameInstallationStatus _status, int _progress);
    void setTaskError(qsizetype _index, const QString & _message);

private:
    QList<InstallationTask> m_tasks;
};

inline GameInstallerActivity::TaskListModel::TaskListModel(QObject * _parent) :
    QAbstractListModel(_parent)
{
}

int GameInstallerActivity::TaskListModel::rowCount(const QModelIndex & _parent) const
{
    if(_parent.isValid())
        return 0;
    return m_tasks.count();
}

int GameInstallerActivity::TaskListModel::columnCount(const QModelIndex & _parent) const
{
    if(_parent.isValid())
        return 0;
    return Column::__ColumnCount;
}

QVariant GameInstallerActivity::TaskListModel::data(const QModelIndex & _index, int _role) const
{
    if(_role != Qt::DisplayRole)
        return {};
    if(_index.row() >= m_tasks.count())
        return {};
    const InstallationTask & task = m_tasks[_index.row()];
    switch(_index.column())
    {
    case Column::Title:
        return task.device_ptr->title();
    case Column::Status:
        switch(task.status)
        {
        case GameInstallationStatus::Done:
            return QObject::tr("Done");
        case GameInstallationStatus::Error:
            return QObject::tr("Error");
        case GameInstallationStatus::Queued:
            return QObject::tr("Queued");
        case GameInstallationStatus::Registration:
            return QObject::tr("Registration...");
        case GameInstallationStatus::RollingBack:
            return QObject::tr("Rolling back...");
        default:
            return {};
        }
    case Column::SourceFormat:
        return gameSourceFormatName(task.source_format);
    case Column::TargetFormat:
        return gameInstallationTypeName(task.target_installation_type);
    case Column::Moving:
        if(task.is_moving_requested)
            return QObject::tr(task.canFileBeMoved() ? "Yes" : "N/A");
        else
            return QObject::tr("No");
    default:
        return {};
    }
}

QVariant GameInstallerActivity::TaskListModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    Q_UNUSED(_orientation)

    if(_role != Qt::DisplayRole)
        return {};

    switch(_section)
    {
    case Column::Title:
        return tr("Title");
    case Column::SourceFormat:
        return tr("Source");
    case Column::TargetFormat:
        return tr("Target");
    case Column::Moving:
            return tr("Move");
    case Column::Status:
        return tr("Status");
    default:
        return {};
    }
}

void GameInstallerActivity::TaskListModel::addTask(QSharedPointer<DeviceReader> _device, GameSourceFormat _format)
{
    const int row = static_cast<int>(m_tasks.count());
    beginInsertRows({}, row, row);
    const Settings & settings = Settings::instance();
    InstallationTask task
    {
        .device_ptr = _device,
        .source_format = _format,
        .target_installation_type = GameInstallationType::Iso9660,
        .status = GameInstallationStatus::Queued,
        .progress = 0,
        .error_message = {},
        .is_moving_requested = settings.moveIso()
    };
    if(settings.splitUpIso())
        task.target_installation_type = GameInstallationType::UlConfig;
    else if(settings.compressIso())
        task.target_installation_type = GameInstallationType::Ziso;
    m_tasks.append(task);
    endInsertRows();
}

inline qsizetype GameInstallerActivity::TaskListModel::taskCount() const
{
    return m_tasks.count();
}

bool GameInstallerActivity::TaskListModel::isProgressBarEnabled(const QModelIndex & _index) const
{
    const InstallationTask * t = task(_index.row());
    return t && t->status == GameInstallationStatus::Installation;
}

int GameInstallerActivity::TaskListModel::maxProgressValue(const QModelIndex & _index) const
{
    Q_UNUSED(_index)
    return g_progressbar_max_value;
}

int GameInstallerActivity::TaskListModel::currentProgressValue(const QModelIndex & _index) const
{
    const InstallationTask * t = task(_index.row());
    return t ? t->progress : -1;
}

inline const InstallationTask * GameInstallerActivity::TaskListModel::task(qsizetype _index) const
{
    return _index >= 0 && _index < m_tasks.count() ? &m_tasks[_index] : nullptr;
}

QModelIndex GameInstallerActivity::TaskListModel::indexByDeviceFilepath(const QString & _device_filepath) const
{
    for(int i = 0; i < m_tasks.count(); ++i)
    {
        if(m_tasks[i].device_ptr->filepath() == _device_filepath)
            return index(i);
    }
    return {};
}

void GameInstallerActivity::TaskListModel::setTargetInstallationType(qsizetype _index, GameInstallationType _format)
{
    if(_index < m_tasks.count() && m_tasks[_index].target_installation_type != _format)
    {
        m_tasks[_index].target_installation_type = _format;
        emit dataChanged(index(_index, 0), index(_index, Column::__ColumnCount - 1));
    }
}

void GameInstallerActivity::TaskListModel::setMoveRequest(qsizetype _index, bool _requested)
{
    if(_index < m_tasks.count() && m_tasks[_index].is_moving_requested != _requested)
    {
        m_tasks[_index].is_moving_requested = _requested;
        QModelIndex idx = index(_index, Column::Moving);
        emit dataChanged(idx, idx);
    }
}

void GameInstallerActivity::TaskListModel::removeTask(qsizetype _index)
{
    if(_index < m_tasks.count())
    {
        beginRemoveRows({}, _index, _index);
        m_tasks.removeAt(_index);
        endRemoveRows();
    }
}

qsizetype GameInstallerActivity::TaskListModel::taskForNextStart() const
{
    for(qsizetype i = 0; i < m_tasks.count(); ++i)
    {
        if(m_tasks[i].status == GameInstallationStatus::Queued)
            return i;
    }
    return -1;
}

void GameInstallerActivity::TaskListModel::setTaskTitle(qsizetype _index, const QString & _title)
{
    if(_index < 0 || _index >= m_tasks.count())
        return;
    m_tasks[_index].device_ptr->setTitle(_title);
    QModelIndex model_index = index(_index, Column::Title);
    emit dataChanged(model_index, model_index);
}

void GameInstallerActivity::TaskListModel::setTaskStatus(qsizetype _index, GameInstallationStatus _status, int _progress)
{
    if(_index < 0 || _index >= m_tasks.count())
        return;
    m_tasks[_index].status = _status;
    if(_progress >= 0)
        m_tasks[_index].progress = _progress;
    QModelIndex model_index = index(_index, Column::Status);
    emit dataChanged(model_index, model_index);
}

void GameInstallerActivity::TaskListModel::setTaskError(qsizetype _index, const QString & _message)
{
    if(_index >= 0 && m_tasks.count() > _index)
    {
        m_tasks[_index].error_message = _message;
        m_tasks[_index].status = GameInstallationStatus::Error;
        QModelIndex model_index = index(_index, Column::Status);
        emit dataChanged(model_index, model_index);
    }
}

GameInstallerActivity::GameInstallerActivity(QWidget * _parent /*= nullptr*/) :
    Activity(_parent),
    mp_model(nullptr),
    mp_working_thread(nullptr),
    mp_installer(nullptr),
    m_processing_task_index(-1),
    m_is_canceled(false)
{
    mp_model = new TaskListModel(this);
    setupUi(this);
    setupShortcuts();

    mp_tree_tasks->setModel(mp_model);
    mp_tree_tasks->setItemDelegateForColumn(Column::Status, new ProgressBarItemDelegate(*mp_model, this));
    mp_tree_tasks->header()->setSectionResizeMode(Column::Title, QHeaderView::Stretch);
    mp_tree_tasks->header()->setSectionResizeMode(Column::SourceFormat, QHeaderView::ResizeToContents);
    mp_tree_tasks->header()->setSectionResizeMode(Column::TargetFormat, QHeaderView::ResizeToContents);
    mp_tree_tasks->header()->setSectionResizeMode(Column::Moving, QHeaderView::ResizeToContents);
    mp_tree_tasks->header()->setSectionResizeMode(Column::Status, QHeaderView::Fixed);
    mp_btn_cancel->setDisabled(true);
    mp_btn_install->setDisabled(true);
    mp_btn_remove->setDisabled(true);
    mp_btn_rename->setDisabled(true);
    connect(mp_btn_back, &QPushButton::clicked, this, &GameInstallerActivity::close);
    connect(mp_tree_tasks->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &GameInstallerActivity::taskSelectionChanged);
    connect(mp_btn_add_image, &QPushButton::clicked, this, [this]() { addDiscImage(); });
    connect(mp_btn_add_disc, &QPushButton::clicked, this, &GameInstallerActivity::addDisc);
    connect(mp_btn_remove, &QPushButton::clicked, this, &GameInstallerActivity::removeGame);
    connect(mp_btn_rename, &QPushButton::clicked, this, &GameInstallerActivity::renameGame);
    connect(mp_tree_tasks, &QTreeWidget::doubleClicked, this, &GameInstallerActivity::renameGame);
    connect(mp_checkbox_move, &QCheckBox::clicked, this, &GameInstallerActivity::moveOptionChanged);
    connect(mp_radio_target_ul, &QRadioButton::clicked, this, &GameInstallerActivity::targetOptionChanged);
    connect(mp_radio_target_iso, &QRadioButton::clicked, this, &GameInstallerActivity::targetOptionChanged);
    connect(mp_radio_target_zso, &QRadioButton::clicked, this, &GameInstallerActivity::targetOptionChanged);
    connect(mp_btn_install, &QPushButton::clicked, this, &GameInstallerActivity::install);
    connect(mp_btn_cancel, &QPushButton::clicked, this, &GameInstallerActivity::cancel);
    taskSelectionChanged();
}

void GameInstallerActivity::setupShortcuts()
{
    QShortcut * shortcut = new QShortcut(QKeySequence("Back"), this);
    connect(shortcut, &QShortcut::activated, this, &GameInstallerActivity::close);
    shortcut = new QShortcut(QKeySequence("Esc"), this);
    connect(shortcut, &QShortcut::activated, this, &GameInstallerActivity::close);
    shortcut = new QShortcut(QKeySequence("F2"), this);
    connect(shortcut, &QShortcut::activated, this, &GameInstallerActivity::renameGame);
    shortcut = new QShortcut(QKeySequence("Del"), this);
    connect(shortcut, &QShortcut::activated, this, &GameInstallerActivity::removeGame);
    shortcut = new QShortcut(QKeySequence("Ins"), this);
    connect(shortcut, &QShortcut::activated, this, [this]() { addDiscImage(); });
    shortcut = new QShortcut(QKeySequence("Shift+Ins"), this);
    connect(shortcut, &QShortcut::activated, this, [this]() { addDisc(); });
}

void GameInstallerActivity::close()
{
    if(mp_btn_back->isEnabled())
        deleteLater();
}

QSharedPointer<Intent> GameInstallerActivity::createIntent()
{
    return QSharedPointer<Intent>(new GameInstallerActivityIntent());
}

void GameInstallerActivity::taskSelectionChanged()
{
    QModelIndexList selected_rows = mp_tree_tasks->selectionModel()->selectedRows();
    if(selected_rows.empty())
    {
        mp_widget_task_details->hide();
        mp_label_details_placeholder->show();
        mp_btn_remove->setDisabled(true);
        mp_btn_rename->setDisabled(true);
        return;
    }

    if(!isStarted())
    {
        mp_btn_remove->setEnabled(true);
        mp_btn_rename->setEnabled(true);
    }

    mp_label_details_placeholder->hide();
    mp_widget_task_details->show();
    const InstallationTask * first_task = mp_model->task(selected_rows[0].row());

    if(selected_rows.count() == 1)
    {
        if(first_task->status == GameInstallationStatus::Error)
            mp_label_error_message->setText(first_task->error_message);
        else
            mp_label_error_message->clear();
        mp_label_title->setText(first_task->device_ptr->title());
    }
    else
    {
        mp_label_title->setText(tr("[Multiple games selected]"));
        mp_label_error_message->clear();
    }

    bool are_all_targets_identical = true;
    bool are_all_move_options_identical = true;
    {
        const GameInstallationType target = first_task->target_installation_type;
        const bool is_moving_requested = first_task->is_moving_requested;
        for(int i = 1; i < selected_rows.count(); ++i)
        {
            const InstallationTask * task = mp_model->task(selected_rows[i].row());
            if(target != task->target_installation_type)
                are_all_targets_identical = false;
            if(is_moving_requested != task->is_moving_requested)
                are_all_move_options_identical = false;
            if(!are_all_targets_identical && !are_all_move_options_identical)
                break;
        }
    }

    if(are_all_targets_identical)
    {
        mp_radio_target_ul->setChecked(first_task->target_installation_type == GameInstallationType::UlConfig);
        mp_radio_target_iso->setChecked(first_task->target_installation_type == GameInstallationType::Iso9660);
        mp_radio_target_zso->setChecked(first_task->target_installation_type == GameInstallationType::Ziso);
    }
    else
    {
        foreach(QRadioButton * rb, mp_groupbox_options->findChildren<QRadioButton *>())
        {
            rb->setAutoExclusive(false);
            rb->setChecked(false);
            rb->setAutoExclusive(true);
        }
    }

    if(are_all_move_options_identical)
        mp_checkbox_move->setChecked(first_task->is_moving_requested);
    else
        mp_checkbox_move->setCheckState(Qt::PartiallyChecked);
}

bool GameInstallerActivity::isStarted() const
{
    return m_processing_task_index >= 0;
}

void GameInstallerActivity::addDiscImage()
{
    Settings & settings = Settings::instance();
    QString filter = tr(
        "All Supported Images (%1 %2 %3 %4);;"
        "ISO Images (%1);;"
        "Compressed ISO Images (%2);;"
        "Bin Files (%3);;"
        "Nero Images (%4)")
        .arg(g_filename_pattern_iso, g_filename_pattern_zso, g_filename_pattern_bin, g_filename_pattern_nrg);
    QString iso_dir = settings.path(Settings::Directory::IsoSource);
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select PS2 Disc Image Files"), iso_dir, filter);
    if(files.isEmpty()) return;
    settings.setPath(Settings::Directory::IsoSource, QFileInfo(files[0]).absolutePath());
    foreach(const QString & file, files)
    {
        addDiscImage(file);
    }
}

void GameInstallerActivity::addDiscImage(const QString & _file_path)
{
    QFileInfo file_info(_file_path);
    QString absolute_image_path = file_info.absoluteFilePath();
    QModelIndex existing_index = mp_model->indexByDeviceFilepath(absolute_image_path);
    if(existing_index.isValid())
    {
        mp_tree_tasks->setCurrentIndex(existing_index);
        return;
    }
    QSharedPointer<DeviceSource> source;
    GameSourceFormat format;
    if(_file_path.endsWith(g_file_ext_iso, Qt::CaseInsensitive))
    {
        source.reset(new Iso9660DeviceSource(_file_path));
        format = GameSourceFormat::Iso9660;
    }
    else if(_file_path.endsWith(g_file_ext_zso, Qt::CaseInsensitive))
    {
        source.reset(new ZisoDeviceSource(_file_path));
        format = GameSourceFormat::Ziso;
    }
    else if(_file_path.endsWith(g_file_ext_bin, Qt::CaseInsensitive))
    {
        source.reset(new BinCueDeviceSource(_file_path));
        format = GameSourceFormat::Bin;
    }
    else if(_file_path.endsWith(g_file_ext_nrg, Qt::CaseInsensitive))
    {
        source.reset(new NrgDeviceSource(_file_path));
        format = GameSourceFormat::NeroImage;
    }
    else
    {
        Application::showErrorMessage(tr("Unknown file format"));
        return;
    }
    QSharedPointer<DeviceReader> device(new DeviceReader(source));
    if(device->open())
    {
        device->setTitle(file_info.completeBaseName());
        mp_model->addTask(device, format);
        mp_tree_tasks->setCurrentIndex(mp_model->index(mp_model->taskCount() - 1));
        mp_btn_install->setDisabled(false);
    }
    else
    {
        Application::showErrorMessage(tr("Invalid file format"));
    }
}

void GameInstallerActivity::dragEnterEvent(QDragEnterEvent * _event)
{
    if(isStarted())
    {
        _event->ignore();
        return;
    }
    foreach(const QUrl & url, _event->mimeData()->urls())
    {
        QString path = url.path();
        if(isSourceFileSupported(path))
        {
            _event->accept();
            return;
        }
    }
    _event->ignore();
}

bool GameInstallerActivity::isSourceFileSupported(const QString & _file_path)
{
    return _file_path.endsWith(g_file_ext_iso, Qt::CaseInsensitive) ||
           _file_path.endsWith(g_file_ext_zso, Qt::CaseInsensitive) ||
           _file_path.endsWith(g_file_ext_bin, Qt::CaseInsensitive) ||
           _file_path.endsWith(g_file_ext_nrg, Qt::CaseInsensitive);
}

void GameInstallerActivity::dropEvent(QDropEvent * _event)
{
    foreach(const QUrl & url, _event->mimeData()->urls())
    {
        QString path = url.toLocalFile();
        if(isSourceFileSupported(path))
            addDiscImage(path);
    }
}

void GameInstallerActivity::addDisc()
{
    ChooseOpticalDiscDialog dlg(this);
    if(dlg.exec() != QDialog::Accepted)
        return;
    QList<QSharedPointer<DeviceReader>> device_list = dlg.devices();
    for(QSharedPointer<DeviceReader> & device : device_list)
    {
        QModelIndex existing_index = mp_model->indexByDeviceFilepath(device->filepath());
        if(existing_index.isValid())
        {
            mp_tree_tasks->setCurrentIndex(existing_index);
            return;
        }
        mp_model->addTask(device, GameSourceFormat::PhysicalDevice);
        mp_tree_tasks->setCurrentIndex(mp_model->index(mp_model->taskCount() - 1));
        mp_btn_install->setDisabled(false);
    }
}

void GameInstallerActivity::renameGame()
{
    if(isStarted()) return;


    QModelIndex index = mp_tree_tasks->currentIndex();
    if(!index.isValid())
        return;

    const InstallationTask * task = mp_model->task(index.row());
    if(!task || task->status != GameInstallationStatus::Queued)
        return;
    GameRenameDialog dlg(task->device_ptr->title(), task->target_installation_type, this);
    if(dlg.exec() == QDialog::Accepted)
    {
        mp_model->setTaskTitle(index.row(), dlg.name());
        if(mp_tree_tasks->selectionModel()->selectedRows().count() == 1)
            mp_label_title->setText(dlg.name());
    }
}

void GameInstallerActivity::removeGame()
{
    if(isStarted())
        return;
    QModelIndexList selected_indices = mp_tree_tasks->selectionModel()->selectedRows();
    if(selected_indices.empty())
        return;

    QList<int> rows;
    rows.reserve(selected_indices.count());
    foreach(const QModelIndex & idx, selected_indices)
        rows.append(idx.row());
    std::sort(rows.rbegin(), rows.rend());
    foreach(int row, rows)
        mp_model->removeTask(row);
    if(mp_model->taskCount() == 0)
    {
        mp_btn_install->setDisabled(true);
    }

    int row_to_select = rows.last();
    if(row_to_select >= mp_model->taskCount())
        row_to_select = mp_model->taskCount() - 1;
    if(row_to_select >= 0)
        mp_tree_tasks->setCurrentIndex(mp_model->index(row_to_select));
}

void GameInstallerActivity::targetOptionChanged(bool _checked)
{
    if(!_checked)
        return;

    GameInstallationType installation_type;
    if(mp_radio_target_iso->isChecked())
        installation_type = GameInstallationType::Iso9660;
    else if(mp_radio_target_zso->isChecked())
        installation_type = GameInstallationType::Ziso;
    else
        installation_type = GameInstallationType::UlConfig;

    foreach(const QModelIndex & idx, mp_tree_tasks->selectionModel()->selectedRows())
        mp_model->setTargetInstallationType(idx.row(), installation_type);
}

void GameInstallerActivity::moveOptionChanged()
{
    if(mp_checkbox_move->isTristate())
        mp_checkbox_move->setTristate(false);
    foreach(const QModelIndex & idx, mp_tree_tasks->selectionModel()->selectedRows())
        mp_model->setMoveRequest(idx.row(), mp_checkbox_move->isChecked());
}

void GameInstallerActivity::install()
{
    mp_groupbox_options->setDisabled(true);
    mp_btn_add_image->setDisabled(true);
    mp_btn_add_disc->setDisabled(true);
    mp_btn_install->setDisabled(true);
    mp_btn_back->setDisabled(true);
    mp_btn_remove->setDisabled(true);
    mp_btn_rename->setDisabled(true);
    mp_btn_cancel->setDisabled(false);
    m_processing_task_index = 0;
    mp_progressbar_overall->setMaximum(g_progressbar_max_value);
    startNextTask();
}

void GameInstallerActivity::installProgress(quint64 _total_bytes, quint64 _processed_bytes)
{
    const InstallationTask * task = mp_model->task(m_processing_task_index);
    if(!task || task->status != GameInstallationStatus::Installation)
        return;

    double current_progress = static_cast<double>(_processed_bytes) / _total_bytes;
    double single_task_in_overal_progress = 1.0 / mp_model->taskCount();
    double overall_progress = single_task_in_overal_progress * m_processing_task_index +
            single_task_in_overal_progress * current_progress;
    mp_model->setTaskStatus(
        m_processing_task_index,
        GameInstallationStatus::Installation,
        current_progress * g_progressbar_max_value);
    mp_progressbar_overall->setValue(overall_progress * g_progressbar_max_value);
}

void GameInstallerActivity::rollbackStarted()
{
    mp_btn_cancel->setDisabled(true);
    setOverallProgressUnknownStatus(true);
    mp_progressbar_overall->setValue(0);
    mp_progressbar_overall->setMaximum(0);
    mp_model->setTaskStatus(m_processing_task_index, GameInstallationStatus::RollingBack, g_progressbar_max_value);
}

void GameInstallerActivity::rollbackFinished()
{
    mp_model->setTaskStatus(m_processing_task_index, GameInstallationStatus::Error, g_progressbar_max_value);
    setOverallProgressUnknownStatus(false, g_progressbar_max_value);
}

QString GameInstallerActivity::canceledErrorMessage() const
{
    static const QString message = tr("Canceled by user");
    return message;
}

void GameInstallerActivity::setTaskError(const QString & _message, int _index)
{
    if(_index < 0) _index = m_processing_task_index;
    mp_model->setTaskError(_index, _message);
    if(_index == mp_tree_tasks->currentIndex().row() && mp_tree_tasks->selectionModel()->selectedRows().count() == 1)
        mp_label_error_message->setText(_message);
}

void GameInstallerActivity::registrationStarted()
{
    mp_model->setTaskStatus(
        m_processing_task_index,
        GameInstallationStatus::Registration,
        g_progressbar_max_value);
    if(m_processing_task_index + 1 == static_cast<quint32>(mp_model->taskCount()))
        setOverallProgressUnknownStatus(true);
}

void GameInstallerActivity::setOverallProgressUnknownStatus(bool _unknown, int _value /*= 0*/)
{
    mp_progressbar_overall->setMaximum(_unknown ? 0 : g_progressbar_max_value);
    mp_progressbar_overall->setValue(_value);
}

void GameInstallerActivity::registrationFinished()
{
    mp_model->setTaskStatus(
        m_processing_task_index,
        GameInstallationStatus::Done,
        g_progressbar_max_value);
}

void GameInstallerActivity::threadFinished()
{
    mp_working_thread = nullptr;
    if(startNextTask())
    {
        return;
    }
    mp_btn_back->setDisabled(false);
    mp_btn_cancel->setDisabled(true);
    setOverallProgressUnknownStatus(false, g_progressbar_max_value);
    Application::showMessage(tr("Done"), tr("Installation complete"));
}

bool GameInstallerActivity::startNextTask()
{
    delete mp_installer;
    mp_installer = nullptr;
    if(m_is_canceled)
        return false;

    m_processing_task_index = mp_model->taskForNextStart();
    if(m_processing_task_index < 0)
        return false;
    const InstallationTask * task = mp_model->task(m_processing_task_index);
    if(!task)
        return false;

    if(task->target_installation_type == GameInstallationType::UlConfig)
    {
        mp_installer = new UlConfigGameInstaller(*task->device_ptr, this);
    }
    else
    {
        DeviceWriter * writer = task->target_installation_type == GameInstallationType::Ziso
            ? static_cast<DeviceWriter *>(new CompressedDeviceWriter())
            : static_cast<DeviceWriter *>(new DefaultDeviceWriter());
        DirectoryGameInstaller * dir_installer = new DirectoryGameInstaller(
            *task->device_ptr,
            std::unique_ptr<DeviceWriter>(writer),
            this);
        dir_installer->setOptionMoveFile(task->is_moving_requested && task->canFileBeMoved());
        mp_installer = dir_installer;
    }
    mp_working_thread = new LambdaThread([this]() {
        mp_installer->install();
    }, this);
    connect(mp_working_thread, &QThread::finished, this, &GameInstallerActivity::threadFinished);
    connect(mp_working_thread, &QThread::finished, mp_working_thread, &QThread::deleteLater);
    connect(mp_working_thread, &LambdaThread::exception, this, &GameInstallerActivity::installerError);
    connect(mp_installer, &GameInstaller::progress, this, &GameInstallerActivity::installProgress);
    connect(mp_installer, &GameInstaller::rollbackStarted, this, &GameInstallerActivity::rollbackStarted);
    connect(mp_installer, &GameInstaller::rollbackFinished, this, &GameInstallerActivity::rollbackFinished);
    connect(mp_installer, &GameInstaller::registrationStarted, this, &GameInstallerActivity::registrationStarted);
    connect(mp_installer, &GameInstaller::registrationFinished, this, &GameInstallerActivity::registrationFinished);
    mp_model->setTaskStatus(m_processing_task_index, GameInstallationStatus::Installation, 0);
    mp_working_thread->start(QThread::HighestPriority);
    return true;
}

void GameInstallerActivity::installerError(QString _message)
{
    setTaskError(_message);
}

void GameInstallerActivity::cancel()
{
    if(mp_working_thread && !m_is_canceled)
    {
        m_is_canceled = true;
        mp_btn_cancel->setDisabled(true);
        mp_working_thread->requestInterruption();
        for(qsizetype i = m_processing_task_index; i < mp_model->taskCount(); ++i)
            setTaskError(canceledErrorMessage(), i);
    }
}
