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

#pragma once

#include "ui_GameConverterActivity.h"
#include <OplPcTools/UI/Activity.h>
#include <OplPcTools/UI/Intent.h>
#include <OplPcTools/UI/LambdaThread.h>

namespace OplPcTools {
namespace UI {

class GameConverterActivity : public Activity, private Ui::GameConverterActivity
{
    Q_OBJECT

private:
    class TaskListModel;

public:
    explicit GameConverterActivity(QWidget * _parent = nullptr);
    static QSharedPointer<Intent> createIntent();

private:
    void onTaskSelectionChanged();
    void onFormatChanged(bool _checked);
    void addGames();
    void removeSelectedTasks();
    void convert();
    void updateOverallProgress();
    void setOverallProgressUnknown();
    bool startNextTask();
    void threadFinished();
    void progress(quint64 _total_bytes, quint64 _processed_bytes);
    void rollbackStarted();
    void rollbackFinished();
    void registrationStarted();
    void registrationFinished();
    void installerError(QString _message);
    void cancel();
    QString canceledErrorMessage() const;
    void setTaskError(qsizetype _index, const QString & _message);

private:
    TaskListModel * mp_model;
    LambdaThread * mp_working_thread;
    qsizetype m_current_task_index;
    bool m_is_canceled;
};

} // namespace UI
} // namespace OplPcTools
