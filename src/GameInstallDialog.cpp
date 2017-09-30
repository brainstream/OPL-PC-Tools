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

#include <QList>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QFileInfo>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include "GameInstallDialog.h"
#include "Game.h"
#include "GameRenameDialog.h"
#include "ChooseOpticalDiscDialog.h"
#include "UlConfigGameInstaller.h"
#include "DirectoryGameInstaller.h"
#include "Settings.h"
#include "OpticalDriveDeviceSource.h"
#include "Iso9660DeviceSource.h"
#include "BinCueDeviceSource.h"

namespace {

const int g_progressbar_max_value = 1000;
const char * g_settings_key_iso_dir = "ISODirectory";
const char * g_iso_ext = ".iso";
const char * g_cue_ext = ".cue";

enum class GameInstallationStatus
{
    Queued,
    Installation,
    Registration,
    Done,
    Error,
    RollingBack
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
    QString m_error_message;
    bool m_is_splitting_up_enabled;
    bool m_is_renaming_enabled;
    bool m_is_moving_enabled;
};

TaskListItem::TaskListItem(QSharedPointer<Device> _device, QTreeWidget * _widget) :
    QTreeWidgetItem(_widget, QTreeWidgetItem::UserType),
    m_device_ptr(_device),
    m_status(GameInstallationStatus::Queued)
{
    const Settings & settings = Settings::instance();
    m_is_splitting_up_enabled = settings.splitUpIso();
    m_is_renaming_enabled = settings.renameIso();
    m_is_moving_enabled = settings.moveIso() && !_device->isReadOnly();
}

QVariant TaskListItem::data(int _column, int _role) const
{
    if(_role != Qt::DisplayRole)
        return QVariant();
    if(_column == 0)
        return m_device_ptr->title();
    switch(m_status)
    {
    case GameInstallationStatus::Done:
        return QObject::tr("Done");
    case GameInstallationStatus::Error:
        return QObject::tr("Error");
    case GameInstallationStatus::Installation:
        return QObject::tr("Installation");
    case GameInstallationStatus::Queued:
        return QObject::tr("Queued");
    case GameInstallationStatus::Registration:
        return QObject::tr("Registration");
    case GameInstallationStatus::RollingBack:
        return QObject::tr("Rolling back");
    default:
        return QString();
    }
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


} // namespace

GameInstallDialog::GameInstallDialog(GameCollection & _collcection, QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    mp_work_thread(nullptr),
    mp_installer(nullptr),
    mr_collection(_collcection),
    m_processing_task_index(0),
    m_is_canceled(false)
{
    setupUi(this);
    QButtonGroup * radiobutton_group = new QButtonGroup(this);
    radiobutton_group->addButton(mp_radiobtn_split_up);
    radiobutton_group->addButton(mp_radiobtn_dnot_split_up);
    mp_widget_task_details->hide();
    mp_progressbar_current->setMaximum(g_progressbar_max_value);
    mp_progressbar_overall->setMaximum(g_progressbar_max_value);
    mp_tree_tasks->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mp_btn_cancel->setDisabled(true);
    mp_btn_install->setDisabled(true);
}

GameInstallDialog::~GameInstallDialog()
{
    delete mp_installer;
}

void GameInstallDialog::reject()
{
    if(mp_work_thread && !m_is_canceled)
    {
        m_is_canceled = true;
        mp_btn_cancel->setDisabled(true);
        mp_work_thread->requestInterruption();
        for(int i = mp_tree_tasks->topLevelItemCount() - 1; i > m_processing_task_index; --i)
        {
            setTaskError(canceledErrorMessage(), i);
        }
    }
}

QString GameInstallDialog::canceledErrorMessage() const
{
    static const QString message = tr("Canceled by user");
    return message;
}

void GameInstallDialog::closeEvent(QCloseEvent * _event)
{
    Q_UNUSED(_event)
}

void GameInstallDialog::dragEnterEvent(QDragEnterEvent * _event)
{
    for(const QUrl & url : _event->mimeData()->urls())
    {
        QString path = url.path();
        if(path.endsWith(g_iso_ext) || path.endsWith(g_cue_ext))
        {
            _event->accept();
            return;
        }
    }
    _event->ignore();
}

void GameInstallDialog::dropEvent(QDropEvent * _event)
{
    for(const QUrl & url : _event->mimeData()->urls())
    {
        QString path = url.path();
        if(path.endsWith(g_iso_ext) || path.endsWith(g_cue_ext))
            addIso(url.path());
    }
}

void GameInstallDialog::installProgress(quint64 _total_bytes, quint64 _processed_bytes)
{
    double current_progress = static_cast<double>(_processed_bytes) / _total_bytes;
    double single_task_in_overal_progress = 1.0 / mp_tree_tasks->topLevelItemCount();
    double overall_progress = single_task_in_overal_progress * m_processing_task_index +
            single_task_in_overal_progress * current_progress;
    mp_progressbar_current->setValue(current_progress * g_progressbar_max_value);
    mp_progressbar_overall->setValue(overall_progress * g_progressbar_max_value);
}

void GameInstallDialog::rollbackStarted()
{
    mp_btn_cancel->setDisabled(true);
    setCurrentProgressBarUnknownStatus(true);
    mp_progressbar_overall->setValue(0);
    mp_progressbar_overall->setMaximum(0);
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
            setStatus(GameInstallationStatus::RollingBack);
}

void GameInstallDialog::setCurrentProgressBarUnknownStatus(bool _unknown, int _value /*= 0*/)
{
    mp_progressbar_current->setMaximum(_unknown ? 0 : g_progressbar_max_value);
    mp_progressbar_current->setValue(_value);
}

void GameInstallDialog::rollbackFinished()
{
    setTaskError(canceledErrorMessage());
    setCurrentProgressBarUnknownStatus(false, g_progressbar_max_value);
    mp_progressbar_overall->setMaximum(g_progressbar_max_value);
    mp_progressbar_overall->setValue(g_progressbar_max_value);
}

void GameInstallDialog::setTaskError(const QString & _message, int _index /*= -1*/)
{
    if(_index < 0) _index = m_processing_task_index;
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(_index))->setError(_message);
    if(_index == mp_tree_tasks->currentIndex().row())
        mp_label_error_message->setText(_message);
}

void GameInstallDialog::registrationStarted()
{
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
            setStatus(GameInstallationStatus::Registration);
    setCurrentProgressBarUnknownStatus(true);
}

void GameInstallDialog::registrationFinished()
{
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
            setStatus(GameInstallationStatus::Done);
    setCurrentProgressBarUnknownStatus(false, g_progressbar_max_value);
    emit gameInstalled(mp_installer->installedGame()->id);
}

void GameInstallDialog::threadFinished()
{
    ++m_processing_task_index;
    mp_work_thread = nullptr;
    if(startTask())
        return;
    mp_btn_ok->setDisabled(false);
    mp_btn_cancel->setDisabled(true);
    setCurrentProgressBarUnknownStatus(false, g_progressbar_max_value);
    mp_progressbar_overall->setValue(g_progressbar_max_value);
    QMessageBox::information(this, QString(), tr("Done!"));
}

bool GameInstallDialog::startTask()
{
    delete mp_installer;
    mp_installer = nullptr;
    if(m_is_canceled)
        return false;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index));
    if(!item)
        return false;
    switch(mp_combo_type->currentIndex())
    {
    case 1:
        item->setMediaType(MediaType::CD);
        break;
    case 2:
        item->setMediaType(MediaType::DVD);
        break;
    default:
        item->setMediaType(MediaType::Unknown);
        break;
    }
    setCurrentProgressBarUnknownStatus(false);
    if(item->isSplittingUpEnabled())
    {
        mp_installer = new UlConfigGameInstaller(item->device(), mr_collection, this);
    }
    else
    {
        DirectoryGameInstaller * dir_installer = new DirectoryGameInstaller(item->device(), mr_collection, this);
        dir_installer->setOptionMoveFile(item->isMovingEnabled());
        dir_installer->setOptionRenameFile(item->isRenamingEnabled());
        mp_installer = dir_installer;
    }
    mp_work_thread = new LambdaThread([this]() {
        mp_installer->install();
    }, this);
    connect(mp_work_thread, &QThread::finished, this, &GameInstallDialog::threadFinished);
    connect(mp_work_thread, &QThread::finished, mp_work_thread, &QThread::deleteLater);
    connect(mp_work_thread, &LambdaThread::exception, this, &GameInstallDialog::installerError);
    connect(mp_installer, &GameInstaller::progress, this, &GameInstallDialog::installProgress);
    connect(mp_installer, &GameInstaller::rollbackStarted, this, &GameInstallDialog::rollbackStarted);
    connect(mp_installer, &GameInstaller::rollbackFinished, this, &GameInstallDialog::rollbackFinished);
    connect(mp_installer, &GameInstaller::registrationStarted, this, &GameInstallDialog::registrationStarted);
    connect(mp_installer, &GameInstaller::registrationFinished, this, &GameInstallDialog::registrationFinished);
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
            setStatus(GameInstallationStatus::Installation);
    mp_work_thread->start(QThread::HighestPriority);
    return true;
}

void GameInstallDialog::addIso() // TODO: rename to addDiscImage
{
    QSettings settings;
    QString iso_dir = settings.value(g_settings_key_iso_dir).toString();
    QString filter = tr("All Supported Images (*%1 *%2);;ISO Images (*%1);;CUE Sheets (*%2)").arg(g_iso_ext).arg(g_cue_ext);
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select PS2 Disc Image Files"), iso_dir, filter);
    if(files.isEmpty()) return;
    settings.setValue(g_settings_key_iso_dir, QFileInfo(files[0]).absolutePath());
    for(const QString & file : files)
    {
        addIso(file);
    }
}

void GameInstallDialog::addIso(const QString & _file_path) // TODO: rename to addDiscImage
{
    QFileInfo file_info(_file_path);
    QString absolute_iso_path = file_info.absoluteFilePath();
    QTreeWidgetItem * existingTask = findTaskInList(absolute_iso_path); // TODO: need to be rewritten :'(
    if(existingTask)
    {
        mp_tree_tasks->setCurrentItem(existingTask);
        return;
    }
    DeviceSource * source = nullptr;
    if(_file_path.endsWith(g_iso_ext))
        source = new Iso9660DeviceSource(_file_path);
    else
        source = new BinCueDeviceSource(_file_path);
    QSharedPointer<Device> device(new Device(QSharedPointer<DeviceSource>(source)));
    if(device->init())
    {
        device->setTitle(truncateGameName(file_info.completeBaseName()));
        TaskListItem * item = new TaskListItem(device, mp_tree_tasks);
        mp_tree_tasks->insertTopLevelItem(mp_tree_tasks->topLevelItemCount(), item);
        mp_tree_tasks->setCurrentItem(item);
        mp_btn_install->setDisabled(false);
    }
    else
    {
        QMessageBox::critical(this, QString(), tr("Invalid ISO file"));
    }
}

QTreeWidgetItem * GameInstallDialog::findTaskInList(const QString & _device_filepath) const
{
    for(int i = mp_tree_tasks->topLevelItemCount() - 1; i >= 0; --i)
    {
        TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(i));
        if(item != nullptr && item->device().filepath() == _device_filepath)
            return item;
    }
    return nullptr;
}

void GameInstallDialog::addDisc()
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

QString GameInstallDialog::truncateGameName(const QString & _name) const
{
    const QByteArray utf8 = _name.toUtf8();
    if(utf8.size() <= g_max_game_name_length)
        return _name;
    QString result = QString::fromUtf8(utf8.constData(), g_max_game_name_length);
    for(int i = result.size() - 1; result[i] != _name[i]; --i)
        result.truncate(i);
    return result;
}

void GameInstallDialog::splitUpOptionChanged()
{
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item) return;
    bool split_up = mp_radiobtn_split_up->isChecked();
    mp_checkbox_move->setDisabled(split_up || item->device().isReadOnly());
    mp_checkbox_rename->setDisabled(split_up);
    item->enabelSplittingUp(split_up);
}

void GameInstallDialog::renameOptionChanged()
{
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item) return;
    item->enabelRenaming(mp_checkbox_rename->isChecked());
}

void GameInstallDialog::moveOptionChanged()
{
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item) return;
    item->enabelMoving(mp_checkbox_move->isChecked());
}

void GameInstallDialog::taskSelectionChanged()
{
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
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
    mp_combo_type->setCurrentIndex(static_cast<int>(item->device().mediaType()));
    bool split_up = item->isSplittingUpEnabled();
    mp_radiobtn_split_up->setChecked(split_up);
    mp_checkbox_move->setChecked(item->isMovingEnabled());
    mp_checkbox_rename->setChecked(item->isRenamingEnabled());
    mp_checkbox_move->setDisabled(split_up || item->device().isReadOnly());
    mp_checkbox_rename->setDisabled(split_up);
}

void GameInstallDialog::mediaTypeChanged(int _index)
{
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item) return;
    item->setMediaType(static_cast<MediaType>(_index));
}

void GameInstallDialog::renameGame()
{
    if(mp_work_thread) return;
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

void GameInstallDialog::removeGame()
{
    if(mp_work_thread) return;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(item->status() != GameInstallationStatus::Queued) return;
    delete item;
    if(mp_tree_tasks->topLevelItemCount() == 0)
    {
        mp_btn_install->setDisabled(true);
    }
}

void GameInstallDialog::installerError(QString _message)
{
    setCurrentProgressBarUnknownStatus(false, g_progressbar_max_value);
    setTaskError(_message);
}

void GameInstallDialog::install()
{
    mp_btn_add_iso->setDisabled(true);
    mp_btn_add_disc->setDisabled(true);
    mp_btn_install->setDisabled(true);
    mp_btn_cancel->setDisabled(false);
    mp_btn_ok->setDisabled(true);
    mp_combo_type->setDisabled(true);
    mp_btn_remove->hide();
    mp_btn_rename->hide();
    m_processing_task_index = 0;
    startTask();
}
