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

#include <OplPcTools/UI/GameConverterActivity.h>
#include <OplPcTools/UI/ChooseImportGamesDialog.h>
#include <OplPcTools/UI/ProgressBarItemDelegate.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/DirectoryGameInstaller.h>
#include <OplPcTools/UlConfigGameInstaller.h>
#include <OplPcTools/GameInstallationTypeUtils.h>
#include <OplPcTools/Device/DefaultDeviceWriter.h>
#include <OplPcTools/Device/CompressedDeviceWriter.h>
#include <OplPcTools/Exception.h>
#include <QDialog>
#include <QAbstractListModel>
#include <QList>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

constexpr int g_progressbar_max_value = 1000;

class GameConverterActivityIntent : public Intent
{
public:
    Activity *createActivity(QWidget * _parent) override
    {
        return new GameConverterActivity(_parent);
    }

    QString activityClass() const override
    {
        return "GameConverter";
    }
};

enum class ConvertingTaskStatus
{
    Queued,
    Converting,
    Registration,
    Done,
    Error,
    RollingBack
};

struct ConvertingTask
{
    Game game;
    GameInstallationType target_installation_type;
    ConvertingTaskStatus status;
    int progress;
    QSharedPointer<DeviceReader> reader;
    QString error_message;
};

namespace Column {

enum
{
    Title,
    SourceFormat,
    TargetFormat,
    Status,
    __ColumnCount
};

} // namespace Column

} // namespace


class GameConverterActivity::TaskListModel : public QAbstractListModel, public ProgressBarItemDelegateSource
{
public:
    explicit  TaskListModel(QObject * _parent);
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    QVariant headerData(int _section, Qt::Orientation _orientation, int _role) const override;
    void addTasks(const QList<const Game *> & _games);
    qsizetype taskCount() const;
    bool isProgressBarEnabled(const QModelIndex & _index) const override;
    int maxProgressValue(const QModelIndex & _index) const override;
    int currentProgressValue(const QModelIndex & _index) const override;
    const ConvertingTask * task(qsizetype _index) const;
    void setTargetInstallationType(qsizetype _index, GameInstallationType _format);
    void removeTask(qsizetype _index);
    qsizetype taskForNextStart() const;
    void setTaskStatus(qsizetype _index, ConvertingTaskStatus _status, int _progress);
    void setTaskErrorMessage(qsizetype _index, const QString & _message);

private:
    QList<ConvertingTask> m_tasks;
};

inline GameConverterActivity::TaskListModel::TaskListModel(QObject * _parent) :
    QAbstractListModel(_parent)
{
}

int GameConverterActivity::TaskListModel::rowCount(const QModelIndex & _parent) const
{
    if(_parent.isValid())
        return 0;
    return static_cast<int>(m_tasks.count());
}

int GameConverterActivity::TaskListModel::columnCount(const QModelIndex & _parent) const
{
    if(_parent.isValid())
        return 0;
    return Column::__ColumnCount;
}

QVariant GameConverterActivity::TaskListModel::data(const QModelIndex & _index, int _role) const
{
    if(_role != Qt::DisplayRole)
        return {};
    if(_index.row() >= m_tasks.count())
        return {};
    const ConvertingTask & task = m_tasks[_index.row()];
    switch(_index.column())
    {
    case Column::Title:
        return task.game.title();
    case Column::SourceFormat:
        return gameInstallationTypeName(task.game.installationType());
    case Column::TargetFormat:
        return gameInstallationTypeName(task.target_installation_type);
    case Column::Status:
        switch(task.status)
        {
        case ConvertingTaskStatus::Done:
            return QObject::tr("Done");
        case ConvertingTaskStatus::Error:
            return QObject::tr("Error");
        case ConvertingTaskStatus::Queued:
            return QObject::tr("Queued");
        case ConvertingTaskStatus::Registration:
            return QObject::tr("Registration...");
        case ConvertingTaskStatus::RollingBack:
            return QObject::tr("Rolling back...");
        default:
            return QVariant();
        }
    default:
        return {};
    }
}

QVariant GameConverterActivity::TaskListModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    Q_UNUSED(_orientation)

    if(_role != Qt::DisplayRole)
        return {};
    switch(_section)
    {
    case Column::Title:
        return tr("Title");
    case Column::SourceFormat:
        return tr("Source format");
    case Column::TargetFormat:
        return tr("Target format");
    case Column::Status:
        return tr("Status");
    default:
        return {};
    }
}

void GameConverterActivity::TaskListModel::addTasks(const QList<const Game *> & _games)
{
    QStringList errors;
    const int row = static_cast<int>(m_tasks.count());
    beginInsertRows({}, row, row + _games.count());
    foreach(const Game * game, _games)
    {
        QSharedPointer<DeviceSource> source = GameDeviceSourceFactory(*game).produce(game->installationType());
        QSharedPointer<DeviceReader> reader(new DeviceReader(source));
        if(!reader->open())
        {
            errors << tr("Game data \"%1\" is corrupted").arg(game->title());
            continue;
        }
        reader->setMediaType(game->mediaType());
        reader->setTitle(game->title());
        m_tasks.append(
            ConvertingTask
            {
                .game = *game,
                .target_installation_type = GameInstallationType::Iso9660,
                .status = ConvertingTaskStatus::Queued,
                .progress = 0,
                .reader = reader,
                .error_message = {}
            });
    }
    endInsertRows();
    if(!errors.empty())
    {
        Application::showErrorMessage(errors.join("\n"));
    }
}

inline qsizetype GameConverterActivity::TaskListModel::taskCount() const
{
    return m_tasks.count();
}

bool GameConverterActivity::TaskListModel::isProgressBarEnabled(const QModelIndex & _index) const
{
    const ConvertingTask * t = task(_index.row());
    return t && t->status == ConvertingTaskStatus::Converting;
}

int GameConverterActivity::TaskListModel::maxProgressValue(const QModelIndex & _index) const
{
    Q_UNUSED(_index)
    return g_progressbar_max_value;
}

int GameConverterActivity::TaskListModel::currentProgressValue(const QModelIndex & _index) const
{
    const ConvertingTask * t = task(_index.row());
    return t ? t->progress : -1;
}

const ConvertingTask * GameConverterActivity::TaskListModel::task(qsizetype _index) const
{
    return _index >= 0 && _index < m_tasks.count() ? &m_tasks[_index] : nullptr;
}

void GameConverterActivity::TaskListModel::setTargetInstallationType(qsizetype _index, GameInstallationType _format)
{
    if(_index < m_tasks.count())
    {
        m_tasks[_index].target_installation_type = _format;
        QModelIndex idx = index(_index, Column::TargetFormat);
        emit dataChanged(idx, idx);
    }
}

void GameConverterActivity::TaskListModel::removeTask(qsizetype _index)
{
    if(_index < m_tasks.count())
    {
        beginRemoveRows({}, _index, _index);
        m_tasks.removeAt(_index);
        endRemoveRows();
    }
}

qsizetype GameConverterActivity::TaskListModel::taskForNextStart() const
{
    for(qsizetype i = 0; i < m_tasks.count(); ++i)
    {
        if(m_tasks[i].status == ConvertingTaskStatus::Queued)
            return i;
    }
    return -1;
}

void GameConverterActivity::TaskListModel::setTaskStatus(qsizetype _index, ConvertingTaskStatus _status, int _progress)
{
    if(_index < 0 || _index >= m_tasks.count())
        return;
    m_tasks[_index].status = _status;
    if(_progress >= 0)
        m_tasks[_index].progress = _progress;
    QModelIndex model_index = index(_index, Column::Status);
    emit dataChanged(model_index, model_index);
}

void GameConverterActivity::TaskListModel::setTaskErrorMessage(qsizetype _index, const QString & _message)
{
    if(_index >= 0 && m_tasks.count() > _index)
        m_tasks[_index].error_message = _message;
}

QSharedPointer<Intent> GameConverterActivity::createIntent()
{
    return QSharedPointer<Intent>(new GameConverterActivityIntent());
}

GameConverterActivity::GameConverterActivity(QWidget * _parent) :
    Activity(_parent),
    m_current_task_index(-1),
    m_is_canceled(false)
{
    mp_model = new GameConverterActivity::TaskListModel(this);
    setupUi(this);
    mp_tree_tasks->setModel(mp_model);
    mp_tree_tasks->setItemDelegateForColumn(Column::Status, new ProgressBarItemDelegate(*mp_model, this));
    mp_tree_tasks->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mp_tree_tasks->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    mp_tree_tasks->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    mp_tree_tasks->header()->setSectionResizeMode(3, QHeaderView::Fixed);
    mp_widget_details->setVisible(false);
    mp_label_details_placeholder->setVisible(true);
    mp_progress_bar->setRange(0, g_progressbar_max_value);
    mp_progress_bar->setValue(0);
    mp_btn_cancel->setEnabled(false);
    mp_btn_convert->setEnabled(false);
    connect(mp_btn_back, &QPushButton::clicked, this, &QObject::deleteLater);
    connect(mp_btn_convert, &QPushButton::clicked, this, &GameConverterActivity::convert);
    connect(mp_btn_cancel, &QPushButton::clicked, this, &GameConverterActivity::cancel);
    connect(mp_btn_remove, &QPushButton::clicked, this, &GameConverterActivity::removeSelectedTasks);
    connect(mp_btn_add, &QPushButton::clicked, this, &GameConverterActivity::addGames);
    connect(mp_tree_tasks->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &GameConverterActivity::onTaskSelectionChanged);
    connect(mp_radio_target_ul, &QRadioButton::clicked, this, &GameConverterActivity::onFormatChanged);
    connect(mp_radio_target_iso, &QRadioButton::clicked, this, &GameConverterActivity::onFormatChanged);
    connect(mp_radio_target_zso, &QRadioButton::clicked, this, &GameConverterActivity::onFormatChanged);
}

void GameConverterActivity::onTaskSelectionChanged()
{
    QModelIndexList selected_rows = mp_tree_tasks->selectionModel()->selectedRows();
    if(selected_rows.empty())
    {
        mp_widget_details->setVisible(false);
        mp_label_details_placeholder->setVisible(true);
        return;
    }
    mp_widget_details->setVisible(true);
    mp_label_details_placeholder->setVisible(false);
    if(selected_rows.count() == 1)
    {
        const ConvertingTask * task = mp_model->task(selected_rows[0].row());
        mp_label_game_title->setText(task->game.title());
        mp_label_error_message->setText(task->error_message);
    }
    else
    {
        mp_label_game_title->setText(tr("[Multiple games selected]"));
        mp_label_error_message->setText({});
    }

    GameInstallationType shared_target_installation_type =
            mp_model->task(selected_rows[0].row())->target_installation_type;
    bool have_shared_format = true;
    for(qsizetype i = 1; i < selected_rows.size(); ++i)
    {
        const ConvertingTask * task = mp_model->task(selected_rows[i].row());
        if(shared_target_installation_type != task->target_installation_type)
        {
            have_shared_format = false;
            break;
        }
    }

    foreach(QRadioButton * rb, mp_group_box_target->findChildren<QRadioButton *>())
    {
        rb->setAutoExclusive(false);
        rb->setChecked(false);
        rb->setAutoExclusive(true);
    }
    if(have_shared_format)
    {
        switch(shared_target_installation_type)
        {
        case GameInstallationType::UlConfig:
            mp_radio_target_ul->setChecked(true);
            break;
        case GameInstallationType::Iso9660:
            mp_radio_target_iso->setChecked(true);
            break;
        case GameInstallationType::Ziso:
            mp_radio_target_zso->setChecked(true);
            break;
        default:
            break;
        }
    }
}

void GameConverterActivity::addGames()
{
    GameCollection & game_collection = Library::instance().games();
    ChooseImportGamesDialog dlg(game_collection, this);
    if(dlg.exec() != QDialog::Accepted)
        return;
    QList<const Game *> games;
    foreach(const Uuid game_id, dlg.selectedGameIds())
    {
        const Game * game = game_collection.findGame(game_id);
        if(game)
        {
            games.append(game);
        }
    }
    if(!games.empty())
    {
        mp_model->addTasks(games);
        mp_btn_convert->setEnabled(true);
    }
}

void GameConverterActivity::onFormatChanged(bool _checked)
{
    if(!_checked)
        return;

    QModelIndexList selected_rows = mp_tree_tasks->selectionModel()->selectedRows();
    if(selected_rows.empty())
        return;

    GameInstallationType installation_type;
    if(mp_radio_target_ul->isChecked())
        installation_type = GameInstallationType::UlConfig;
    else if(mp_radio_target_iso->isChecked())
        installation_type = GameInstallationType::Iso9660;
    else if(mp_radio_target_zso->isChecked())
        installation_type = GameInstallationType::Ziso;
    else
        return;

    foreach(const QModelIndex & idx, selected_rows)
        mp_model->setTargetInstallationType(idx.row(), installation_type);
}

void GameConverterActivity::removeSelectedTasks()
{
    std::vector<qsizetype> idx_list;
    QModelIndexList selected_rows = mp_tree_tasks->selectionModel()->selectedRows();
    idx_list.reserve(selected_rows.count());
    foreach(const QModelIndex & idx, mp_tree_tasks->selectionModel()->selectedRows())
        idx_list.push_back(idx.row());
    std::sort(idx_list.begin(), idx_list.end());
    for(auto it = idx_list.rbegin(); it != idx_list.rend(); ++it)
        mp_model->removeTask(*it);
    if(mp_model->taskCount() == 0)
        mp_btn_convert->setEnabled(false);
}

void GameConverterActivity::convert()
{
    mp_btn_add->setEnabled(false);
    mp_btn_remove->setEnabled(false);
    mp_btn_convert->setEnabled(false);
    mp_btn_cancel->setEnabled(true);
    mp_btn_back->setEnabled(false);
    mp_group_box_target->setEnabled(false);
    mp_tree_tasks->setFocus();
    startNextTask();
}

bool GameConverterActivity::startNextTask()
{
    m_current_task_index = mp_model->taskForNextStart();
    const ConvertingTask * task = mp_model->task(m_current_task_index);
    if(!task) return false;

    if(task->game.installationType() == task->target_installation_type)
    {
        mp_model->setTaskStatus(m_current_task_index, ConvertingTaskStatus::Done, g_progressbar_max_value);
        return true;
    }
    mp_model->setTaskStatus(m_current_task_index, ConvertingTaskStatus::Converting, 0);
    updateOverallProgress();

    GameInstaller * installer = nullptr;

    if(task->target_installation_type == GameInstallationType::UlConfig)
    {
        installer = new UlConfigGameInstaller(*task->reader);
    }
    else
    {
        DeviceWriter * writer = task->target_installation_type == GameInstallationType::Ziso
            ? static_cast<DeviceWriter *>(new CompressedDeviceWriter()) // FIXME: .iso is creating
            : static_cast<DeviceWriter *>(new DefaultDeviceWriter());
        installer =  new DirectoryGameInstaller(*task->reader, std::unique_ptr<DeviceWriter>(writer));
    }
    installer->enableOverride();

    // FIXME: Option: "Add game ID to filename" when converting to ISO or ZISO

    mp_working_thread = new LambdaThread([installer]() {
        installer->install();
    }, this);
    connect(mp_working_thread, &QThread::finished, this, &GameConverterActivity::threadFinished);
    connect(mp_working_thread, &QThread::finished, mp_working_thread, &QThread::deleteLater);
    connect(mp_working_thread, &LambdaThread::exception, this, &GameConverterActivity::installerError);
    connect(installer, &GameInstaller::progress, this, &GameConverterActivity::progress);
    connect(installer, &GameInstaller::rollbackStarted, this, &GameConverterActivity::rollbackStarted);
    connect(installer, &GameInstaller::rollbackFinished, this, &GameConverterActivity::rollbackFinished);
    connect(installer, &GameInstaller::registrationStarted, this, &GameConverterActivity::registrationStarted);
    connect(installer, &GameInstaller::registrationFinished, this, &GameConverterActivity::registrationFinished);
    mp_working_thread->start(QThread::HighestPriority);

    return true;
}

void GameConverterActivity::threadFinished()
{
    if(!m_is_canceled)
    {
        if(startNextTask())
            return;
    }
    mp_btn_cancel->setEnabled(false);
    mp_btn_back->setEnabled(true);
    Application::showMessage(tr("Done"), tr("Converting complete"));
}

void GameConverterActivity::progress(quint64 _total_bytes, quint64 _processed_bytes)
{
    const ConvertingTask * task = mp_model->task(m_current_task_index);
    if(!task || task->status != ConvertingTaskStatus::Converting)
        return;

    const float current_progress = static_cast<float>(_processed_bytes) / _total_bytes;
    mp_model->setTaskStatus(
        m_current_task_index,
        ConvertingTaskStatus::Converting,
        static_cast<int>(current_progress * g_progressbar_max_value));
    updateOverallProgress();
}

void GameConverterActivity::updateOverallProgress()
{
    if(mp_progress_bar->maximum() == 0)
        mp_progress_bar->setMaximum(g_progressbar_max_value);
    int task_progress_sum = 0;
    for(qsizetype i = 0; i < mp_model->taskCount(); ++i)
        task_progress_sum += mp_model->task(i)->progress;
    const float progress = static_cast<float>(task_progress_sum) / mp_model->taskCount();
    mp_progress_bar->setValue(static_cast<int>(progress));
}

void GameConverterActivity::setOverallProgressUnknown()
{
    mp_progress_bar->setRange(0, 0);
}

void GameConverterActivity::installerError(QString _message)
{
    setTaskError(m_current_task_index, _message);
}

void GameConverterActivity::cancel()
{
    if(mp_working_thread && !m_is_canceled)
    {
        m_is_canceled = true;
        mp_btn_cancel->setDisabled(true);
        for(qsizetype i = m_current_task_index; i < mp_model->taskCount(); ++i)
            setTaskError(i, canceledErrorMessage());
        mp_working_thread->requestInterruption();
    }
}

void GameConverterActivity::setTaskError(qsizetype _index, const QString & _message)
{
    mp_model->setTaskStatus(_index, ConvertingTaskStatus::Error, g_progressbar_max_value);
    updateOverallProgress();
    mp_model->setTaskErrorMessage(_index, _message);
    if(_index == mp_tree_tasks->currentIndex().row())
        mp_label_error_message->setText(_message);
}

QString GameConverterActivity::canceledErrorMessage() const
{
    static const QString message = tr("Canceled by user");
    return message;
}

void GameConverterActivity::rollbackStarted()
{
    mp_btn_cancel->setDisabled(true);
    setOverallProgressUnknown();
}

void GameConverterActivity::rollbackFinished()
{
    mp_model->setTaskStatus(m_current_task_index, ConvertingTaskStatus::Error, g_progressbar_max_value);
    updateOverallProgress();
}

void GameConverterActivity::registrationStarted()
{
    mp_model->setTaskStatus(m_current_task_index, ConvertingTaskStatus::Registration, g_progressbar_max_value);
    if(m_current_task_index + 1 == mp_model->taskCount())
        setOverallProgressUnknown();
}

void GameConverterActivity::registrationFinished()
{
    mp_model->setTaskStatus(m_current_task_index, ConvertingTaskStatus::Done, g_progressbar_max_value);
    updateOverallProgress();
}
