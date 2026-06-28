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
#include <OplPcTools/Library.h>
#include <QDialog>
#include <QAbstractListModel>
#include <QList>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

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

enum class TargetFormat
{
    None,
    Ul,
    Iso,
    Zso
};

QString targetFormatToString(TargetFormat _format)
{
    switch(_format)
    {
    case TargetFormat::Ul:
        return g_game_format_ul;
    case TargetFormat::Iso:
        return g_game_format_iso;
    case TargetFormat::Zso:
        return g_game_format_zso;
    default:
        return {};
    }
}

struct ConvertingTask
{
    Game game;
    TargetFormat target_format;
};

} // namespace

class GameConverterActivity::TaskListModel : public QAbstractListModel
{
private:
    enum
    {
        ColumnIndex_Title,
        ColumnIndex_SourceFormat,
        ColumnIndex_TargetFormat,
        ColumnCount
    };

public:
    explicit  TaskListModel(QObject * _parent);
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    QVariant headerData(int _section, Qt::Orientation _orientation, int _role) const override;
    void addTasks(const QList<const Game *> & _games);
    const ConvertingTask * task(qsizetype _index) const;
    void setTargetFormat(qsizetype _index, TargetFormat _format);
    void removeTask(qsizetype _index);

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
    return ColumnCount;
}

QVariant GameConverterActivity::TaskListModel::data(const QModelIndex & _index, int _role) const
{
    if(_role != Qt::DisplayRole)
        return {};
    if(_index.row() >= m_tasks.count())
        return {};
    const auto & [game, target_format] = m_tasks[_index.row()];
    switch(_index.column())
    {
    case ColumnIndex_Title:
        return game.title();
    case ColumnIndex_SourceFormat:
        return game.formatName();
    case ColumnIndex_TargetFormat:
        return targetFormatToString(target_format);
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
    case ColumnIndex_Title:
        return tr("Title");
    case ColumnIndex_SourceFormat:
        return tr("Source format");
    case ColumnIndex_TargetFormat:
        return tr("Target format");
    default:
        return {};
    }
}

void GameConverterActivity::TaskListModel::addTasks(const QList<const Game *> & _games)
{
    const int row = static_cast<int>(m_tasks.count());
    beginInsertRows({}, row, row + _games.count());
    foreach(const Game * game, _games)
        m_tasks.append(ConvertingTask { .game = *game, .target_format = TargetFormat::Iso});
    endInsertRows();
}

const ConvertingTask * GameConverterActivity::TaskListModel::task(qsizetype _index) const
{
    return _index < m_tasks.count() ? &m_tasks[_index] : nullptr;
}

void GameConverterActivity::TaskListModel::setTargetFormat(qsizetype _index, TargetFormat _format)
{
    if(_index < m_tasks.count())
    {
        m_tasks[_index].target_format = _format;
        QModelIndex idx = index(_index, ColumnIndex_TargetFormat);
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

QSharedPointer<Intent> GameConverterActivity::createIntent()
{
    return QSharedPointer<Intent>(new GameConverterActivityIntent());
}

GameConverterActivity::GameConverterActivity(QWidget * _parent) :
    Activity(_parent)
{
    mp_model = new GameConverterActivity::TaskListModel(this);
    setupUi(this);
    QPushButton * btn_convert = mp_button_box->button(QDialogButtonBox::Apply);
    btn_convert->setText(tr("Convert"));
    btn_convert->setIcon(QIcon(":/images/start"));
    mp_tree_tasks->setModel(mp_model);
    mp_tree_tasks->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mp_tree_tasks->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    mp_tree_tasks->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    mp_tree_tasks->header()->setSectionResizeMode(3, QHeaderView::Fixed);
    mp_widget_details->setVisible(false);
    mp_label_details_placeholder->setVisible(true);
    connect(mp_btn_back, &QPushButton::clicked, this, &QObject::deleteLater);
    connect(btn_convert, &QPushButton::clicked, this, &GameConverterActivity::convert);
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
        mp_label_game_title->setText(mp_model->task(selected_rows[0].row())->game.title());
    else
        mp_label_game_title->setText(tr("[Multiple games selected]"));

    TargetFormat shared_target_format = mp_model->task(selected_rows[0].row())->target_format;
    bool have_shared_format = true;
    for(qsizetype i = 1; i < selected_rows.size(); ++i)
    {
        const ConvertingTask * task = mp_model->task(selected_rows[i].row());
        if(shared_target_format != task->target_format)
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
        switch(shared_target_format)
        {
        case TargetFormat::Ul:
            mp_radio_target_ul->setChecked(true);
            break;
        case TargetFormat::Iso:
            mp_radio_target_iso->setChecked(true);
            break;
        case TargetFormat::Zso:
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
        mp_model->addTasks(games);
}

void GameConverterActivity::onFormatChanged(bool _checked)
{
    if(!_checked)
        return;

    QModelIndexList selected_rows = mp_tree_tasks->selectionModel()->selectedRows();
    if(selected_rows.empty())
        return;

    TargetFormat format;
    if(mp_radio_target_ul->isChecked())
        format = TargetFormat::Ul;
    else if(mp_radio_target_iso->isChecked())
        format = TargetFormat::Iso;
    else if(mp_radio_target_zso->isChecked())
        format = TargetFormat::Zso;
    else
        return;

    foreach(const QModelIndex & idx, selected_rows)
        mp_model->setTargetFormat(idx.row(), format);
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
}

void GameConverterActivity::convert()
{

}
