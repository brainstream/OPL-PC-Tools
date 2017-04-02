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
#include "GameInstallDialog.h"
#include "Game.h"
#include "GameRenameDialog.h"

namespace {

const int g_progressbar_max_value = 1000;
const char * g_settings_key_iso_dir = "isodir";
static const QString g_canceled_message = QObject::tr("Canceled by user");

enum class InstallationStatus
{
    queued,
    installation,
    registration,
    done,
    error,
    rollingBack
};

struct InstallationTask
{
    QString iso_path;
    QString game_name;
    QString error_message;
    InstallationStatus status;
};

class TaskListItem : public QTreeWidgetItem
{
public:
    TaskListItem(const QString & _iso_path, QTreeWidget * _widget);
    QVariant data(int _column, int _role) const;
    inline const InstallationTask & task() const;
    void rename(const QString & _new_name);
    void setStatus(InstallationStatus _status);
    void setError(const QString & _message);

private:
    QString truncateGameName(const QString & _name);

private:
    InstallationTask m_task;
};

TaskListItem::TaskListItem(const QString & _iso_path, QTreeWidget * _widget) :
    QTreeWidgetItem(_widget, QTreeWidgetItem::UserType)
{
    QFileInfo iso_info(_iso_path);
    m_task.iso_path = iso_info.absoluteFilePath();
    m_task.game_name = truncateGameName(iso_info.completeBaseName());
    m_task.status = InstallationStatus::queued;
}

QString TaskListItem::truncateGameName(const QString & _name)
{
    const QByteArray utf8 = _name.toUtf8();
    if(utf8.size() <= Game::max_game_name_length)
        return _name;
    QString result = QString::fromUtf8(utf8.constData(), Game::max_game_name_length);
    for(int i = result.size() - 1; result[i] != _name[i]; --i)
        result.truncate(i);
    return result;
}

QVariant TaskListItem::data(int _column, int _role) const
{
    if(_role != Qt::DisplayRole) return QVariant();
    if(_column == 0) return m_task.game_name;
    switch(m_task.status)
    {
    case InstallationStatus::done:
        return QObject::tr("Done");
    case InstallationStatus::error:
        return QObject::tr("Error");
    case InstallationStatus::installation:
        return QObject::tr("Installation");
    case InstallationStatus::queued:
        return QObject::tr("Queued");
    case InstallationStatus::registration:
        return QObject::tr("Registration");
    case InstallationStatus::rollingBack:
        return QObject::tr("Rolling back");
    default:
        return QString();
    }
}

const InstallationTask & TaskListItem::task() const
{
    return m_task;
}

void TaskListItem::rename(const QString & _new_name)
{
    m_task.game_name = _new_name;
    emitDataChanged();
}

void TaskListItem::setStatus(InstallationStatus _status)
{
    m_task.status = _status;
    m_task.error_message.clear();
    emitDataChanged();
}

void TaskListItem::setError(const QString & _message)
{
    m_task.status = InstallationStatus::error;
    m_task.error_message = _message;
    emitDataChanged();
}

} // namespace

GameInstallDialog::GameInstallDialog(UlConfig & _config, QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    mp_work_thread(nullptr),
    mp_installer(nullptr),
    mp_source(nullptr),
    mr_config(_config),
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
    delete mp_source;
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
            static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(i))->setError(g_canceled_message);
        }
    }
}

void GameInstallDialog::closeEvent(QCloseEvent * _event)
{
    Q_UNUSED(_event)
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
            setStatus(InstallationStatus::rollingBack);
}

void GameInstallDialog::setCurrentProgressBarUnknownStatus(bool _unknown, int _value /*= 0*/)
{
    mp_progressbar_current->setMaximum(_unknown ? 0 : g_progressbar_max_value);
    mp_progressbar_current->setValue(_value);
}

void GameInstallDialog::rollbackFinished()
{
    setTaskError(g_canceled_message);
    mp_work_thread->quit();
    setCurrentProgressBarUnknownStatus(false, g_progressbar_max_value);
    mp_progressbar_overall->setMaximum(g_progressbar_max_value);
    mp_progressbar_overall->setValue(g_progressbar_max_value);
}

void GameInstallDialog::setTaskError(const QString & _message, int _index /*= -1*/)
{
    if(_index < 0) _index = m_processing_task_index;
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->setError(_message);
    if(_index == mp_tree_tasks->currentIndex().row())
        mp_label_error_message->setText(_message);
}

void GameInstallDialog::registrationStarted()
{
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
            setStatus(InstallationStatus::registration);
    setCurrentProgressBarUnknownStatus(true);
}

void GameInstallDialog::registrationFinished()
{
    static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index))->
            setStatus(InstallationStatus::done);
    setCurrentProgressBarUnknownStatus(false, g_progressbar_max_value);
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
}

bool GameInstallDialog::startTask()
{
    delete mp_installer;
    delete mp_source;
    mp_installer = nullptr;
    mp_source = nullptr;
    if(m_is_canceled)
        return false;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(m_processing_task_index));
    if(!item) return false;
    const InstallationTask & task = item->task();
    setCurrentProgressBarUnknownStatus(false);
    mp_source = new Iso9660GameInstallerSource(task.iso_path);
    switch(mp_combo_type->currentIndex())
    {
    case 1:
        mp_source->setType(MediaType::cd);
        break;
    case 2:
        mp_source->setType(MediaType::dvd);
        break;
    }
    mp_installer = new GameInstaller(*mp_source, mr_config, this);
    mp_installer->setGameName(task.game_name);
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
            setStatus(InstallationStatus::installation);
    mp_work_thread->start(QThread::HighestPriority);
    return true;
}

void GameInstallDialog::addTask()
{
    QSettings settings;
    QString iso_dir = settings.value(g_settings_key_iso_dir).toString();
    QStringList iso_files = QFileDialog::getOpenFileNames(this, tr("Select the PS2 ISO files"), iso_dir, tr("ISO files (*.iso)"));
    if(iso_files.isEmpty()) return;
    settings.setValue(g_settings_key_iso_dir, QFileInfo(iso_files[0]).absolutePath());
    for(const QString & file : iso_files)
    {
        TaskListItem * item = new TaskListItem(file, mp_tree_tasks);
        mp_tree_tasks->insertTopLevelItem(mp_tree_tasks->topLevelItemCount(), item);
        mp_tree_tasks->setCurrentItem(item);
    }
    mp_btn_install->setDisabled(false);
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
    const InstallationTask & task = item->task();
    if(task.status == InstallationStatus::error)
        mp_label_error_message->setText(task.error_message);
    else
        mp_label_error_message->clear();
    mp_label_title->setText(task.game_name);
}

void GameInstallDialog::renameGame()
{
    if(mp_work_thread) return;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item || item->task().status != InstallationStatus::queued)
        return;
    GameRenameDialog dlg(item->task().game_name, this);
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
    if(item->task().status != InstallationStatus::queued) return;
    delete item;
}

void GameInstallDialog::installerError(QString _message)
{
    setCurrentProgressBarUnknownStatus(false, g_progressbar_max_value);
    setTaskError(_message);
}

void GameInstallDialog::install()
{
    mp_btn_add->setDisabled(true);
    mp_btn_install->setDisabled(true);
    mp_btn_cancel->setDisabled(false);
    mp_btn_ok->setDisabled(true);
    mp_combo_type->setDisabled(true);
    mp_btn_remove->hide();
    mp_btn_rename->hide();
    m_processing_task_index = 0;
    startTask();
}
