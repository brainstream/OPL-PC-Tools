/***********************************************************************************************
 *                                                                                             *
 * This file is part of the qpcopl project, the graphical PC tools for Open PS2 Loader.        *
 *                                                                                             *
 * qpcopl is free software: you can redistribute it and/or modify it under the terms of        *
 * the GNU General Public License as published by the Free Software Foundation,                *
 * either version 3 of the License, or (at your option) any later version.                     *
 *                                                                                             *
 * qpcopl is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;        *
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
#include "GameInstallationTask.h"
#include "ChooseOpticalDiscDialog.h"

namespace {

const int g_progressbar_max_value = 1000;
const char * g_settings_key_iso_dir = "ISODirectory";
const char * g_iso_ext = ".iso";

class TaskListItem : public QTreeWidgetItem
{
public:
    TaskListItem(QSharedPointer<GameInstallationTask> _task, QTreeWidget * _widget);
    inline GameInstallationTask & task();
    QVariant data(int _column, int _role) const;
    void rename(const QString & _new_name);
    void setStatus(GameInstallationStatus _status);
    void setError(const QString & _message);
    inline void setMediaType(MediaType _media_type);

private:
    QSharedPointer<GameInstallationTask> m_task_ptr;
};

TaskListItem::TaskListItem(QSharedPointer<GameInstallationTask> _task, QTreeWidget * _widget) :
    QTreeWidgetItem(_widget, QTreeWidgetItem::UserType),
    m_task_ptr(_task)
{
}

GameInstallationTask & TaskListItem::task()
{
    return *m_task_ptr;
}

QVariant TaskListItem::data(int _column, int _role) const
{
    if(_role != Qt::DisplayRole)
        return QVariant();
    if(_column == 0)
        return m_task_ptr->device().title();
    switch(m_task_ptr->status())
    {
    case GameInstallationStatus::done:
        return QObject::tr("Done");
    case GameInstallationStatus::error:
        return QObject::tr("Error");
    case GameInstallationStatus::installation:
        return QObject::tr("Installation");
    case GameInstallationStatus::queued:
        return QObject::tr("Queued");
    case GameInstallationStatus::registration:
        return QObject::tr("Registration");
    case GameInstallationStatus::rollingBack:
        return QObject::tr("Rolling back");
    default:
        return QString();
    }
}

void TaskListItem::rename(const QString & _new_name)
{
    m_task_ptr->device().setTitle(_new_name);
    emitDataChanged();
}

void TaskListItem::setStatus(GameInstallationStatus _status)
{
    m_task_ptr->setStatus(_status);
    m_task_ptr->setErrorMessage(QString());
    emitDataChanged();
}

void TaskListItem::setError(const QString & _message)
{
    m_task_ptr->setErrorStatus(_message);
    emitDataChanged();
}

void TaskListItem::setMediaType(MediaType _media_type)
{
    m_task_ptr->device().setMediaType(_media_type);
}

} // namespace

GameInstallDialog::GameInstallDialog(GameRepository & _repository, QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    mp_work_thread(nullptr),
    mp_installer(nullptr),
    mr_repository(_repository),
    m_processing_task_index(0),
    m_is_canceled(false)
{
    setupUi(this);
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
        mp_work_thread->quit();
        mp_work_thread->requestInterruption();
        for(int i = mp_tree_tasks->topLevelItemCount() - 1; i > m_processing_task_index; --i)
        {
            setTaskError(canceledErrorMessage(), i);        }
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
        if(url.path().endsWith(g_iso_ext))
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
        if(url.path().endsWith(g_iso_ext))
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
            setStatus(GameInstallationStatus::rollingBack);
}

void GameInstallDialog::setCurrentProgressBarUnknownStatus(bool _unknown, int _value /*= 0*/)
{
    mp_progressbar_current->setMaximum(_unknown ? 0 : g_progressbar_max_value);
    mp_progressbar_current->setValue(_value);
}

void GameInstallDialog::rollbackFinished()
{
    setTaskError(canceledErrorMessage());
    mp_work_thread->quit();
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
            setStatus(GameInstallationStatus::registration);
    setCurrentProgressBarUnknownStatus(true);
}

void GameInstallDialog::registrationFinished()
{
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
            setStatus(GameInstallationStatus::done);
    setCurrentProgressBarUnknownStatus(false, g_progressbar_max_value);
    emit gameInstalled(mp_installer->installedGame()->id);
    mp_work_thread->quit();
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
    if(!item) return false;
    switch(mp_combo_type->currentIndex())
    {
    case 1:
        item->setMediaType(MediaType::cd);
        break;
    case 2:
        item->setMediaType(MediaType::dvd);
        break;
    default:
        item->setMediaType(MediaType::unknown);
        break;
    }
    GameInstallationTask & task = item->task();
    setCurrentProgressBarUnknownStatus(false);
    mp_installer = new GameInstaller(task.device(), mr_repository, this);
    mp_work_thread = new GameInstallThread(*mp_installer);
    connect(mp_work_thread, &QThread::finished, this, &GameInstallDialog::threadFinished);
    connect(mp_work_thread, &QThread::finished, mp_work_thread, &QThread::deleteLater);
    connect(mp_work_thread, &GameInstallThread::exception, this, &GameInstallDialog::installerError);
    connect(mp_installer, &GameInstaller::progress, this, &GameInstallDialog::installProgress);
    connect(mp_installer, &GameInstaller::rollbackStarted, this, &GameInstallDialog::rollbackStarted);
    connect(mp_installer, &GameInstaller::rollbackFinished, this, &GameInstallDialog::rollbackFinished);
    connect(mp_installer, &GameInstaller::registrationStarted, this, &GameInstallDialog::registrationStarted);
    connect(mp_installer, &GameInstaller::registrationFinished, this, &GameInstallDialog::registrationFinished);
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
            setStatus(GameInstallationStatus::installation);
    mp_work_thread->start(QThread::HighestPriority);
    return true;
}

void GameInstallDialog::addIso()
{
    QSettings settings;
    QString iso_dir = settings.value(g_settings_key_iso_dir).toString();
    QString filter = tr("PS2 Disc Images") + QString(" (*%1)").arg(g_iso_ext);
    QStringList iso_files = QFileDialog::getOpenFileNames(this, tr("Select PS2 Disc Image Files"), iso_dir, filter);
    if(iso_files.isEmpty()) return;
    settings.setValue(g_settings_key_iso_dir, QFileInfo(iso_files[0]).absolutePath());
    for(const QString & file : iso_files)
    {
        addIso(file);
    }
}

void GameInstallDialog::addIso(const QString & _iso_path)
{
    QFileInfo iso_info(_iso_path);
    QString absolute_iso_path = iso_info.absoluteFilePath();
    QTreeWidgetItem * existingTask = findTaskInList(absolute_iso_path);
    if(existingTask)
    {
        mp_tree_tasks->setCurrentItem(existingTask);
        return;
    }
    QSharedPointer<Device> device(new Iso9660Image(_iso_path));
    if(device->init())
    {
        device->setTitle(truncateGameName(iso_info.completeBaseName()));
        QSharedPointer<GameInstallationTask> task(new GameInstallationTask(device));
        TaskListItem * item = new TaskListItem(task, mp_tree_tasks);
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
        const GameInstallationTask * task = dynamic_cast<const GameInstallationTask *>(&item->task());
        if(task != nullptr && task->device().filepath() == _device_filepath)
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
        QSharedPointer<GameInstallationTask> task(new GameInstallationTask(device));
        TaskListItem * item = new TaskListItem(task, mp_tree_tasks);
        mp_tree_tasks->insertTopLevelItem(mp_tree_tasks->topLevelItemCount(), item);
        mp_tree_tasks->setCurrentItem(item);
        mp_btn_install->setDisabled(false);
    }
}

QString GameInstallDialog::truncateGameName(const QString & _name) const
{
    const QByteArray utf8 = _name.toUtf8();
    if(utf8.size() <= MAX_GAME_NAME_LENGTH)
        return _name;
    QString result = QString::fromUtf8(utf8.constData(), MAX_GAME_NAME_LENGTH);
    for(int i = result.size() - 1; result[i] != _name[i]; --i)
        result.truncate(i);
    return result;
}

void GameInstallDialog::taskSelectionChanged()
{
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item)
    {
        mp_widget_task_details->hide();
        return;
    }
    else
    {
        mp_widget_task_details->show();
    }
    const GameInstallationTask & task = item->task();
    if(task.status() == GameInstallationStatus::error)
        mp_label_error_message->setText(task.errorMessage());
    else
        mp_label_error_message->clear();
    mp_label_title->setText(task.device().title());
    mp_combo_type->setCurrentIndex(static_cast<int>(task.device().mediaType()));
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
    if(!item || item->task().status() != GameInstallationStatus::queued)
        return;
    GameRenameDialog dlg(item->task().device().title(), this);
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
    if(item->task().status() != GameInstallationStatus::queued) return;
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
    mp_btn_remove->hide();
    mp_btn_rename->hide();
    m_processing_task_index = 0;
    startTask();
}
