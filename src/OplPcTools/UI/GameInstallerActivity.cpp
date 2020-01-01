/***********************************************************************************************
 * Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                   *
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

#include <QStyledItemDelegate>
#include <QShortcut>
#include <QFileInfo>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <OplPcTools/Device.h>
#include <OplPcTools/Iso9660DeviceSource.h>
#include <OplPcTools/BinCueDeviceSource.h>
#include <OplPcTools/NrgDeviceSource.h>
#include <OplPcTools/OpticalDriveDeviceSource.h>
#include <OplPcTools/Settings.h>
#include <OplPcTools/UlConfigGameInstaller.h>
#include <OplPcTools/DirectoryGameInstaller.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/ChooseOpticalDiscDialog.h>
#include <OplPcTools/UI/GameRenameDialog.h>
#include <OplPcTools/UI/GameInstallerActivity.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

namespace SettingsKey {
    const char * iso_dir = "ISODirectory";
} // namespace SettingsKey

namespace Column {
enum
{
    Name   = 0,
    Status = 1
};
} // namespace Column

const int g_progressbar_max_value = 1000;
const char * g_iso_ext = ".iso";
const char * g_bin_ext = ".bin";
const char * g_nrg_ext = ".nrg";

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
};

class TaskListItem : public QTreeWidgetItem
{
public:
    TaskListItem(QSharedPointer<Device> _device, QTreeWidget * _widget);
    QVariant data(int _column, int _role) const;
    inline Device & device();
    void rename(const QString & _new_name);
    void setStatus(GameInstallationStatus _status);
    void setError(const QString & _message);
    inline GameInstallationStatus status() const;
    inline const QString & errorMessage() const;
    inline void setProgress(int _progress);
    inline int progress() const;
    inline void setMediaType(MediaType _media_type);
    inline bool isSplittingUpEnabled() const;
    inline void enabelSplittingUp(bool _enable);
    inline bool isRenamingEnabled() const;
    inline void enabelRenaming(bool _enable);
    inline bool isMovingEnabled() const;
    inline void enabelMoving(bool _enable);

private:
    QSharedPointer<Device> m_device_ptr;
    GameInstallationStatus m_status;
    int m_progress;
    QString m_error_message;
    bool m_is_splitting_up_enabled;
    bool m_is_renaming_enabled;
    bool m_is_moving_enabled;
};

class TaskListViewDelegate : public QStyledItemDelegate
{
public:
    TaskListViewDelegate(QTreeWidget * _tree) :
        QStyledItemDelegate(_tree),
        mp_tree(_tree)
    {
    }

    void paint(QPainter * _painter, const QStyleOptionViewItem & _option, const QModelIndex & _index ) const override;

private:
    QTreeWidget * mp_tree;
};

} // namespace

TaskListItem::TaskListItem(QSharedPointer<Device> _device, QTreeWidget * _widget) :
    QTreeWidgetItem(_widget, QTreeWidgetItem::UserType),
    m_device_ptr(_device),
    m_status(GameInstallationStatus::Queued),
    m_progress(0)
{
    const Settings & settings = Settings::instance();
    m_is_splitting_up_enabled = settings.flag(Settings::Flag::SplitUpIso);
    m_is_renaming_enabled = settings.flag(Settings::Flag::RenameIso);
    m_is_moving_enabled = settings.flag(Settings::Flag::MoveIso) && !_device->isReadOnly();
}

QVariant TaskListItem::data(int _column, int _role) const
{
    if(_role != Qt::DisplayRole)
        return QVariant();
    if(_column == Column::Name)
        return m_device_ptr->title();
    switch(m_status)
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
    }
    return QVariant();
}

Device & TaskListItem::device()
{
    return *m_device_ptr;
}

void TaskListItem::rename(const QString & _new_name)
{
    m_device_ptr->setTitle(_new_name);
    emitDataChanged();
}

void TaskListItem::setStatus(GameInstallationStatus _status)
{
    m_status = _status;
    m_error_message = QString();
    emitDataChanged();
}

void TaskListItem::setError(const QString & _message)
{
    m_status = GameInstallationStatus::Error;
    m_error_message = _message;
    emitDataChanged();
}

GameInstallationStatus TaskListItem::status() const
{
    return m_status;
}

const QString & TaskListItem::errorMessage() const
{
    return m_error_message;
}

void TaskListItem::setProgress(int _progress)
{
    if(m_progress != _progress)
    {
        m_progress = _progress;
        emitDataChanged();
    }
}

int TaskListItem::progress() const
{
    return m_progress;
}

void TaskListItem::setMediaType(MediaType _media_type)
{
    m_device_ptr->setMediaType(_media_type);
}

bool TaskListItem::isSplittingUpEnabled() const
{
    return m_is_splitting_up_enabled;
}

void TaskListItem::enabelSplittingUp(bool _enable)
{
    m_is_splitting_up_enabled = _enable;
}

bool TaskListItem::isRenamingEnabled() const
{
    return m_is_renaming_enabled;
}

void TaskListItem::enabelRenaming(bool _enable)
{
    m_is_renaming_enabled = _enable;
}

bool TaskListItem::isMovingEnabled() const
{
    return m_is_moving_enabled;
}

void TaskListItem::enabelMoving(bool _enable)
{
    m_is_moving_enabled = _enable;
}

void TaskListViewDelegate::paint(QPainter * _painter, const QStyleOptionViewItem & _option, const QModelIndex & _index ) const
{
    QStyledItemDelegate::paint(_painter, _option, _index);
    const TaskListItem * item = static_cast<TaskListItem *>(mp_tree->topLevelItem(_index.row()));
    if(_index.column() != Column::Status || item->status() != GameInstallationStatus::Installation)
        return;
    QStyleOptionProgressBar progress_bar_option;
    progress_bar_option.state = QStyle::State_Enabled;
    progress_bar_option.direction = _option.direction;
    progress_bar_option.rect = _option.rect;
    progress_bar_option.fontMetrics = _option.fontMetrics;
    progress_bar_option.minimum = 0;
    progress_bar_option.maximum = g_progressbar_max_value;
    progress_bar_option.textAlignment = Qt::AlignCenter;
    progress_bar_option.textVisible = true;
    progress_bar_option.progress = item->progress();
    progress_bar_option.text = QString::asprintf("%d%%", progress_bar_option.progress / (g_progressbar_max_value / 100));
    QStyleOption progress_indicator_option;
    progress_indicator_option.state = QStyle::State_Enabled;
    progress_indicator_option.direction = _option.direction;
    progress_indicator_option.rect = _option.rect;
    progress_indicator_option.rect.setWidth(progress_indicator_option.rect.width() * progress_bar_option.progress / g_progressbar_max_value);
    progress_indicator_option.fontMetrics = _option.fontMetrics;
    QStyle * style = QApplication::style();
    style->drawPrimitive(QStyle::PE_IndicatorProgressChunk, &progress_indicator_option, _painter);
    style->drawControl(QStyle::CE_ProgressBarLabel, &progress_bar_option, _painter);
}

GameInstallerActivity::GameInstallerActivity(QWidget * _parent /*= nullptr*/) :
    Activity(_parent),
    mp_working_thread(nullptr),
    mp_installer(nullptr),
    m_processing_task_index(0),
    m_is_canceled(false)
{
    setupUi(this);
    setupShortcuts();
    mp_tree_tasks->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mp_tree_tasks->setItemDelegate(new TaskListViewDelegate(mp_tree_tasks));
    mp_btn_cancel->setDisabled(true);
    mp_btn_install->setDisabled(true);
    connect(mp_btn_back, &QPushButton::clicked, this, &GameInstallerActivity::close);
    connect(mp_tree_tasks, &QTreeWidget::itemSelectionChanged, this, &GameInstallerActivity::taskSelectionChanged);
    connect(mp_btn_add_image, &QPushButton::clicked, [this]() { addDiscImage(); });
    connect(mp_btn_add_disc, &QPushButton::clicked, this, &GameInstallerActivity::addDisc);
    connect(mp_btn_remove, &QPushButton::clicked, this, &GameInstallerActivity::removeGame);
    connect(mp_btn_rename, &QPushButton::clicked, this, &GameInstallerActivity::renameGame);
    connect(mp_tree_tasks, &QTreeWidget::doubleClicked, this, &GameInstallerActivity::renameGame);
    connect(mp_radio_mtauto, &QRadioButton::clicked, this, &GameInstallerActivity::mediaTypeChanged);
    connect(mp_radio_mtdvd, &QRadioButton::clicked, this, &GameInstallerActivity::mediaTypeChanged);
    connect(mp_radio_mtcd, &QRadioButton::clicked, this, &GameInstallerActivity::mediaTypeChanged);
    connect(mp_checkbox_move, &QCheckBox::clicked, this, &GameInstallerActivity::moveOptionChanged);
    connect(mp_checkbox_rename, &QCheckBox::clicked, this, &GameInstallerActivity::renameOptionChanged);
    connect(mp_radio_split_up, &QRadioButton::clicked, this, &GameInstallerActivity::splitUpOptionChanged);
    connect(mp_radio_dnot_split_up, &QRadioButton::clicked, this, &GameInstallerActivity::splitUpOptionChanged);
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
    connect(shortcut, &QShortcut::activated, [this]() { addDiscImage(); });
    shortcut = new QShortcut(QKeySequence("Shift+Ins"), this);
    connect(shortcut, &QShortcut::activated, [this]() { addDisc(); });
}

void GameInstallerActivity::close()
{
    if(mp_btn_back->isEnabled())
        deleteLater();
}

QSharedPointer<Intent> GameInstallerActivity::createIntent()
{
    return QSharedPointer<Intent>(new GameInstallerActivityIntent);
}

void GameInstallerActivity::taskSelectionChanged()
{
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    mp_btn_remove->setEnabled(item);
    mp_btn_rename->setEnabled(item);
    if(!item)
    {
        mp_widget_task_details->hide();
        return;
    }
    mp_widget_task_details->show();
    if(item->status() == GameInstallationStatus::Error)
        mp_label_error_message->setText(item->errorMessage());
    else
        mp_label_error_message->clear();
    mp_label_title->setText(item->device().title());
    switch(item->device().mediaType())
    {
    case MediaType::CD:
        mp_radio_mtcd->setChecked(true);
        break;
    case MediaType::DVD:
        mp_radio_mtdvd->setChecked(true);
        break;
    default:
        mp_radio_mtauto->setChecked(true);
        break;
    }
    bool split_up = item->isSplittingUpEnabled();
    mp_radio_split_up->setChecked(split_up);
    mp_radio_dnot_split_up->setChecked(!split_up);
    mp_checkbox_move->setChecked(item->isMovingEnabled());
    mp_checkbox_rename->setChecked(item->isRenamingEnabled());
    mp_checkbox_move->setDisabled(split_up || item->device().isReadOnly());
    mp_checkbox_rename->setDisabled(split_up);
}

void GameInstallerActivity::addDiscImage()
{
    QSettings settings;
    QString filter = tr("All Supported Images (*%1 *%2 *%3);;ISO Images (*%1);;Bin Files (*%2);;Nero Images (*%3)")
            .arg(g_iso_ext)
            .arg(g_bin_ext)
            .arg(g_nrg_ext);
    QString iso_dir = settings.value(SettingsKey::iso_dir).toString();
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select PS2 Disc Image Files"), iso_dir, filter);
    if(files.isEmpty()) return;
    settings.setValue(SettingsKey::iso_dir, QFileInfo(files[0]).absolutePath());
    for(const QString & file : files)
    {
        addDiscImage(file);
    }
}

void GameInstallerActivity::addDiscImage(const QString & _file_path)
{
    QFileInfo file_info(_file_path);
    QString absolute_image_path = file_info.absoluteFilePath();
    QTreeWidgetItem * existingTask = findTaskInList(absolute_image_path);
    if(existingTask)
    {
        mp_tree_tasks->setCurrentItem(existingTask);
        return;
    }
    DeviceSource * source = nullptr;
    if(_file_path.endsWith(g_iso_ext))
        source = new Iso9660DeviceSource(_file_path);
    else if(_file_path.endsWith(g_bin_ext))
        source = new BinCueDeviceSource(_file_path);
    else if(_file_path.endsWith(g_nrg_ext))
        source = new NrgDeviceSource(_file_path);
    QSharedPointer<Device> device(new Device(QSharedPointer<DeviceSource>(source)));
    if(device->init())
    {
        device->setTitle(file_info.completeBaseName());
        TaskListItem * item = new TaskListItem(device, mp_tree_tasks);
        mp_tree_tasks->insertTopLevelItem(mp_tree_tasks->topLevelItemCount(), item);
        mp_tree_tasks->setCurrentItem(item);
        mp_btn_install->setDisabled(false);
    }
    else
    {
        Application::instance().showErrorMessage(tr("Invalid file format"));
    }
}

QTreeWidgetItem * GameInstallerActivity::findTaskInList(const QString & _device_filepath) const
{
    for(int i = mp_tree_tasks->topLevelItemCount() - 1; i >= 0; --i)
    {
        TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(i));
        if(item != nullptr && item->device().filepath() == _device_filepath)
            return item;
    }
    return nullptr;
}

void GameInstallerActivity::dragEnterEvent(QDragEnterEvent * _event)
{
    if(mp_working_thread)
    {
        _event->ignore();
        return;
    }
    for(const QUrl & url : _event->mimeData()->urls())
    {
        QString path = url.path();
        if(path.endsWith(g_iso_ext) || path.endsWith(g_bin_ext) || path.endsWith(g_nrg_ext))
        {
            _event->accept();
            return;
        }
    }
    _event->ignore();
}

void GameInstallerActivity::dropEvent(QDropEvent * _event)
{
    for(const QUrl & url : _event->mimeData()->urls())
    {
        QString path = url.toLocalFile();
        if(path.endsWith(g_iso_ext) || path.endsWith(g_bin_ext) || path.endsWith(g_nrg_ext))
            addDiscImage(path);
    }
}

void GameInstallerActivity::addDisc()
{
    ChooseOpticalDiscDialog dlg(this);
    if(dlg.exec() != QDialog::Accepted)
        return;
    QList<QSharedPointer<Device>> device_list = dlg.devices();
    for(QSharedPointer<Device> & device : device_list)
    {
        QTreeWidgetItem * existingTask = findTaskInList(device->filepath());
        if(existingTask)
        {
            mp_tree_tasks->setCurrentItem(existingTask);
            return;
        }
        TaskListItem * item = new TaskListItem(device, mp_tree_tasks);
        mp_tree_tasks->insertTopLevelItem(mp_tree_tasks->topLevelItemCount(), item);
        mp_tree_tasks->setCurrentItem(item);
        mp_btn_install->setDisabled(false);
    }
}

void GameInstallerActivity::renameGame()
{
    if(mp_working_thread) return;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item || item->status() != GameInstallationStatus::Queued)
        return;
    GameRenameDialog dlg(item->device().title(),
            item->isSplittingUpEnabled() ? GameInstallationType::UlConfig : GameInstallationType::Directory, this);
    if(dlg.exec() == QDialog::Accepted)
    {
        item->rename(dlg.name());
        mp_label_title->setText(dlg.name());
    }
}

void GameInstallerActivity::removeGame()
{
    if(mp_working_thread) return;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(item->status() != GameInstallationStatus::Queued) return;
    delete item;
    if(mp_tree_tasks->topLevelItemCount() == 0)
    {
        mp_btn_install->setDisabled(true);
    }
}

void GameInstallerActivity::mediaTypeChanged(bool _checked)
{
    if(!_checked) return;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item) return;
    MediaType media_type = MediaType::Unknown;
    if(mp_radio_mtcd->isChecked())
        media_type = MediaType::CD;
    else if(mp_radio_mtdvd->isChecked())
        media_type = MediaType::DVD;
    item->setMediaType(media_type);
}

void GameInstallerActivity::splitUpOptionChanged(bool _checked)
{
    if(!_checked) return;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item) return;
    bool split_up = mp_radio_split_up->isChecked();
    mp_checkbox_move->setDisabled(split_up || item->device().isReadOnly());
    mp_checkbox_rename->setDisabled(split_up);
    item->enabelSplittingUp(split_up);
}

void GameInstallerActivity::renameOptionChanged()
{
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item) return;
    item->enabelRenaming(mp_checkbox_rename->isChecked());
}

void GameInstallerActivity::moveOptionChanged()
{
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item) return;
    item->enabelMoving(mp_checkbox_move->isChecked());
}

void GameInstallerActivity::install()
{
    mp_groupbox_media_type->setDisabled(true);
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
    startTask();
}

void GameInstallerActivity::installProgress(quint64 _total_bytes, quint64 _processed_bytes)
{
    double current_progress = static_cast<double>(_processed_bytes) / _total_bytes;
    double single_task_in_overal_progress = 1.0 / mp_tree_tasks->topLevelItemCount();
    double overall_progress = single_task_in_overal_progress * m_processing_task_index +
            single_task_in_overal_progress * current_progress;
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
        setProgress(current_progress * g_progressbar_max_value);
    mp_progressbar_overall->setValue(overall_progress * g_progressbar_max_value);
}

void GameInstallerActivity::rollbackStarted()
{
    mp_btn_cancel->setDisabled(true);
    setOverallProgressUnknownStatus(true);
    mp_progressbar_overall->setValue(0);
    mp_progressbar_overall->setMaximum(0);
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
            setStatus(GameInstallationStatus::RollingBack);
}

void GameInstallerActivity::rollbackFinished()
{
    setTaskError(canceledErrorMessage());
    setOverallProgressUnknownStatus(false, g_progressbar_max_value);
}

QString GameInstallerActivity::canceledErrorMessage() const
{
    static const QString message = tr("Canceled by user");
    return message;
}

void GameInstallerActivity::setTaskError(const QString & _message, int _index /*= -1*/)
{
    if(_index < 0) _index = m_processing_task_index;
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(_index))->setError(_message);
    if(_index == mp_tree_tasks->currentIndex().row())
        mp_label_error_message->setText(_message);
}

void GameInstallerActivity::registrationStarted()
{
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
        setStatus(GameInstallationStatus::Registration);
    if(m_processing_task_index + 1 == mp_tree_tasks->topLevelItemCount())
        setOverallProgressUnknownStatus(true);
}

void GameInstallerActivity::setOverallProgressUnknownStatus(bool _unknown, int _value /*= 0*/)
{
    mp_progressbar_overall->setMaximum(_unknown ? 0 : g_progressbar_max_value);
    mp_progressbar_overall->setValue(_value);
}

void GameInstallerActivity::registrationFinished()
{
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
        setStatus(GameInstallationStatus::Done);
}

void GameInstallerActivity::threadFinished()
{
    ++m_processing_task_index;
    mp_working_thread = nullptr;
    if(startTask())
    {
        return;
    }
    mp_btn_back->setDisabled(false);
    mp_btn_cancel->setDisabled(true);
    setOverallProgressUnknownStatus(false, g_progressbar_max_value);
    Application::instance().showMessage(tr("Done"), tr("Installation complete"));
}

bool GameInstallerActivity::startTask()
{
    delete mp_installer;
    mp_installer = nullptr;
    if(m_is_canceled)
        return false;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index));
    if(!item)
        return false;
    if(mp_radio_mtcd->isChecked())
        item->setMediaType(MediaType::CD);
    else if(mp_radio_mtdvd->isChecked())
        item->setMediaType(MediaType::DVD);
    else
        item->setMediaType(MediaType::Unknown);
    GameCollection & collection = Application::instance().gameCollection();
    if(item->isSplittingUpEnabled())
    {
        mp_installer = new UlConfigGameInstaller(item->device(), collection, this);
    }
    else
    {
        DirectoryGameInstaller * dir_installer = new DirectoryGameInstaller(item->device(), collection, this);
        dir_installer->setOptionMoveFile(item->isMovingEnabled());
        dir_installer->setOptionRenameFile(item->isRenamingEnabled());
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
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
            setStatus(GameInstallationStatus::Installation);
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
        for(int i = mp_tree_tasks->topLevelItemCount() - 1; i > m_processing_task_index; --i)
        {
            setTaskError(canceledErrorMessage(), i);
        }
    }
}
