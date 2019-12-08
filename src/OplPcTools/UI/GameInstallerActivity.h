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

#ifndef __OPLPCTOOLS_GAMEINSTALLERACTIVITY__
#define __OPLPCTOOLS_GAMEINSTALLERACTIVITY__

#include <QWidget>
#include <QTreeWidgetItem>
#include <OplPcTools/GameInstaller.h>
#include <OplPcTools/UI/LambdaThread.h>
#include <OplPcTools/UI/Intent.h>
#include "ui_GameInstallerActivity.h"

namespace OplPcTools {
namespace UI {

class GameInstallerActivity : public Activity, private Ui::GameInstallerActivity
{
    Q_OBJECT

public:
    explicit GameInstallerActivity(QWidget * _parent = nullptr);
    static QSharedPointer<Intent> createIntent();

protected:
    void dragEnterEvent(QDragEnterEvent * _event) override;
    void dropEvent(QDropEvent * _event) override;

private:
    void setupShortcuts();
    void close();
    void taskSelectionChanged();
    void addDiscImage();
    void addDiscImage(const QString & _file_path);
    void addDisc();
    QTreeWidgetItem * findTaskInList(const QString & _device_filepath) const;
    void renameGame();
    void removeGame();
    void mediaTypeChanged(bool _checked);
    void splitUpOptionChanged(bool _checked);
    void renameOptionChanged();
    void moveOptionChanged();
    void install();
    bool startTask();
    void installProgress(quint64 _total_bytes, quint64 _processed_bytes);
    void rollbackStarted();
    void rollbackFinished();
    void registrationStarted();
    void registrationFinished();
    void threadFinished();
    void installerError(QString _message);
    void setTaskError(const QString & _message, int _index = -1);
    QString canceledErrorMessage() const;
    void setOverallProgressUnknownStatus(bool _unknown, int _value = 0);
    void cancel();

private:
    LambdaThread * mp_working_thread;
    GameInstaller * mp_installer;
    quint32 m_processing_task_index;
    bool m_is_canceled;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMEINSTALLERACTIVITY__
