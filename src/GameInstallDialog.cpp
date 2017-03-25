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

namespace {

static const char * g_settings_key_iso_dir = "isodir";

enum class InstallationStatus
{
    queued,
    installation,
    registration,
    done,
    error,
    rollingBack
};

struct GameInstallTask
{
    QString iso_path;
    QString game_name;
    QString error_message;
    InstallationStatus status;
};

class GameTasksTableModel : public QAbstractTableModel
{
public:
    explicit inline GameTasksTableModel(QObject * _parent = nullptr);
    int rowCount(const QModelIndex & _parent) const override;
    inline int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    QVariant headerData(int _section, Qt::Orientation _orientation, int _role) const override;
    void addTask(const QString & _iso_path);
    const GameInstallTask * task(int _index) const;
    void setTaskStatus(int _index, InstallationStatus _status);
    void setTaskError(int _index, const QString & _message);

private:
    QString statusToString(InstallationStatus _status) const;
    QString truncateGameName(const QString & _name);
    QString dispayText(const QModelIndex & _index) const;
    QString tipText(const QModelIndex & _index) const;

private:
    QList<GameInstallTask> m_tasks;
    static const int s_column_name = 0;
    static const int s_column_status = 1;
};

GameTasksTableModel::GameTasksTableModel(QObject * _parent /*= nullptr*/) :
    QAbstractTableModel(_parent)
{
}

int GameTasksTableModel::rowCount(const QModelIndex & _parent) const
{
    Q_UNUSED(_parent)
    return m_tasks.size();
}

int GameTasksTableModel::columnCount(const QModelIndex & _parent) const
{
    Q_UNUSED(_parent)
    return 2;
}

QVariant GameTasksTableModel::data(const QModelIndex & _index, int _role) const
{
    switch(_role)
    {
    case Qt::DisplayRole:
        return dispayText(_index);
    case Qt::ToolTipRole:
        return tipText(_index);
    default:
        return QVariant();
    }
}

QString GameTasksTableModel::dispayText(const QModelIndex & _index) const
{
    switch(_index.column())
    {
    case s_column_name:
        return m_tasks[_index.row()].game_name;
    case s_column_status:
        return statusToString(m_tasks[_index.row()].status);
    default:
        return QString();
    }
}

QString GameTasksTableModel::tipText(const QModelIndex & _index) const
{
    if(_index.column() == s_column_status)
        return m_tasks[_index.row()].error_message;
    return QString();
}

QVariant GameTasksTableModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    if(_orientation == Qt::Vertical || _role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(_section, _orientation, _role);
    switch(_section)
    {
    case s_column_name:
        return tr("Title");
    case s_column_status:
        return tr("Status");
    default:
        return QVariant();
    }
}

QString GameTasksTableModel::statusToString(InstallationStatus _status) const
{
    switch(_status)
    {
    case InstallationStatus::done:
        return tr("Done");
    case InstallationStatus::error:
        return tr("Error");
    case InstallationStatus::installation:
        return tr("Installation");
    case InstallationStatus::queued:
        return tr("Queued");
    case InstallationStatus::registration:
        return tr("Registration");
    case InstallationStatus::rollingBack:
        return tr("Rolling back");
    default:
        return QString();
    }
}

void GameTasksTableModel::addTask(const QString & _iso_path)
{
    int row_count = m_tasks.size();
    QModelIndex parent_index = index(row_count, 0);
    beginInsertRows(parent_index, row_count, row_count);
    GameInstallTask task;
    task.iso_path = _iso_path;
    task.game_name = truncateGameName(QFileInfo(_iso_path).completeBaseName());
    task.status = InstallationStatus::queued;
    m_tasks.append(task);
    endInsertRows();
    emit dataChanged(parent_index, index(m_tasks.size(), columnCount(QModelIndex())));
}

QString GameTasksTableModel::truncateGameName(const QString & _name)
{
    const QByteArray utf8 = _name.toUtf8();
    if(utf8.size() <= Game::max_game_name_length)
        return _name;
    QString result = QString::fromUtf8(utf8.constData(), Game::max_game_name_length);
    for(int i = result.size() - 1; result[i] != _name[i]; --i)
        result.truncate(i);
    return result;
}

const GameInstallTask * GameTasksTableModel::task(int _index) const
{
    return _index >= m_tasks.size() ? nullptr : &m_tasks[_index];
}

void GameTasksTableModel::setTaskStatus(int _index, InstallationStatus _status)
{
    if(_index < m_tasks.size())
    {
        m_tasks[_index].status = _status;
        QModelIndex cell_index = index(_index, s_column_status);
        emit dataChanged(cell_index, cell_index);
    }
}

void GameTasksTableModel::setTaskError(int _index, const QString & _message)
{
    if(_index < m_tasks.size())
    {
        GameInstallTask & task = m_tasks[_index];
        task.status = InstallationStatus::error;
        task.error_message = _message;
        QModelIndex cell_index = index(_index, s_column_status);
        emit dataChanged(cell_index, cell_index);
    }
}

const int g_progressbar_max_value = 1000;

} // namespace

GameInstallDialog::GameInstallDialog(UlConfig & _config, QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    mp_work_thread(nullptr),
    mp_installer(nullptr),
    mp_source(nullptr),
    mr_config(_config),
    m_processing_task_index(0)
{
    setupUi(this);
    mp_progressbar_current->setMaximum(g_progressbar_max_value);
    mp_progressbar_overall->setMaximum(g_progressbar_max_value);
    mp_table_tasks->setModel(new GameTasksTableModel(this));
    mp_table_tasks->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    mp_table_tasks->setColumnWidth(1, 150);
    mp_table_tasks->setColumnWidth(2, 150);
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
    mp_btn_cancel->setDisabled(true);
    mp_work_thread->quit();
    mp_work_thread->requestInterruption();
}

void GameInstallDialog::closeEvent(QCloseEvent * _event)
{
    Q_UNUSED(_event)
}

void GameInstallDialog::installProgress(quint64 _total_bytes, quint64 _processed_bytes)
{
    double current_progress = static_cast<double>(_processed_bytes) / _total_bytes;
    double single_task_in_overal_progress = 1.0 / mp_table_tasks->model()->rowCount();
    double overall_progress = single_task_in_overal_progress * m_processing_task_index +
            single_task_in_overal_progress * current_progress;
    mp_progressbar_current->setValue(current_progress * g_progressbar_max_value);
    mp_progressbar_overall->setValue(overall_progress * g_progressbar_max_value);
}

void GameInstallDialog::rollbackStarted()
{
    mp_btn_cancel->setDisabled(true);
    setCurrentProgressBarUnknownStatus(true);
    static_cast<GameTasksTableModel *>(mp_table_tasks->model())->
            setTaskStatus(m_processing_task_index, InstallationStatus::rollingBack);
}

void GameInstallDialog::setCurrentProgressBarUnknownStatus(bool _unknown, int _value /*= 0*/)
{
    mp_progressbar_current->setMaximum(_unknown ? 0 : g_progressbar_max_value);
    mp_progressbar_current->setValue(_value);
}

void GameInstallDialog::rollbackFinished()
{
    static_cast<GameTasksTableModel *>(mp_table_tasks->model())->
            setTaskError(m_processing_task_index, tr("Canceled by user"));
    setCurrentProgressBarUnknownStatus(false, g_progressbar_max_value);
    mp_work_thread->quit();
}

void GameInstallDialog::registrationStarted()
{
    static_cast<GameTasksTableModel *>(mp_table_tasks->model())->
            setTaskStatus(m_processing_task_index, InstallationStatus::registration);
    setCurrentProgressBarUnknownStatus(true);
}

void GameInstallDialog::registrationFinished()
{
    static_cast<GameTasksTableModel *>(mp_table_tasks->model())->
            setTaskStatus(m_processing_task_index, InstallationStatus::done);
    setCurrentProgressBarUnknownStatus(false, g_progressbar_max_value);
    mp_work_thread->quit();
}

void GameInstallDialog::threadFinished()
{
    ++m_processing_task_index;
    if(startTask()) {
        return;
    }
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
    const GameInstallTask * task = static_cast<GameTasksTableModel *>(mp_table_tasks->model())->task(m_processing_task_index);
    if(task == nullptr)
        return false;
    setCurrentProgressBarUnknownStatus(false);
    mp_source = new Iso9660GameInstallerSource(task->iso_path);
    mp_installer = new GameInstaller(*mp_source, mr_config, this);
    mp_installer->setGameName(task->game_name);
    mp_work_thread = new GameInstallThread(*mp_installer);
    connect(mp_work_thread, &QThread::finished, this, &GameInstallDialog::threadFinished);
    connect(mp_work_thread, &QThread::finished, mp_work_thread, &QThread::deleteLater);
    connect(mp_work_thread, &GameInstallThread::exception, this, &GameInstallDialog::installerError);
    connect(mp_installer, &GameInstaller::progress, this, &GameInstallDialog::installProgress);
    connect(mp_installer, &GameInstaller::rollbackStarted, this, &GameInstallDialog::rollbackStarted);
    connect(mp_installer, &GameInstaller::rollbackFinished, this, &GameInstallDialog::rollbackFinished);
    connect(mp_installer, &GameInstaller::registrationStarted, this, &GameInstallDialog::registrationStarted);
    connect(mp_installer, &GameInstaller::registrationFinished, this, &GameInstallDialog::registrationFinished);
    static_cast<GameTasksTableModel *>(mp_table_tasks->model())->
            setTaskStatus(m_processing_task_index, InstallationStatus::installation);
    mp_work_thread->start(QThread::HighestPriority);
    return true;
}

void GameInstallDialog::addTask()
{
    QSettings settings;
    QString iso_dir = settings.value(g_settings_key_iso_dir).toString();
    QString iso_file = QFileDialog::getOpenFileName(this, tr("Select the PS2 ISO file"), iso_dir, tr("ISO files (*.iso)"));
    if(iso_file.isEmpty()) return;
    settings.setValue(g_settings_key_iso_dir, QFileInfo(iso_file).absolutePath());
    static_cast<GameTasksTableModel *>(mp_table_tasks->model())->addTask(iso_file);
    mp_btn_install->setDisabled(false);
}

void GameInstallDialog::installerError(QString _message)
{
    setCurrentProgressBarUnknownStatus(false, g_progressbar_max_value);
    static_cast<GameTasksTableModel *>(mp_table_tasks->model())->setTaskError(m_processing_task_index, _message);
}

void GameInstallDialog::install()
{
    mp_btn_add->setDisabled(true);
    mp_btn_install->setDisabled(true);
    mp_btn_cancel->setDisabled(false);
    mp_btn_ok->setDisabled(true);
    m_processing_task_index = 0;
    startTask();
}
