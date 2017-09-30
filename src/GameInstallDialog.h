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

#ifndef __OPLPCTOOLS_GAMEINSTALLDIALOG__
#define __OPLPCTOOLS_GAMEINSTALLDIALOG__

#include "ui_GameInstallDialog.h"
#include "LambdaThread.h"
#include "GameInstaller.h"
#include "Device.h"

class GameInstallDialog : public QDialog, private Ui::GameInstallDialog
{
    Q_OBJECT

public:
    explicit GameInstallDialog(GameCollection & _collcection, QWidget * _parent = nullptr);
    ~GameInstallDialog() override;

public slots:
    void reject() override;

protected:
    void closeEvent(QCloseEvent * _event) override;
    void dragEnterEvent(QDragEnterEvent * _event) override;
    void dropEvent(QDropEvent * _event) override;

private slots:
    void addDiscImage();
    void addDisc();
    void splitUpOptionChanged();
    void renameOptionChanged();
    void moveOptionChanged();
    void install();
    void installProgress(quint64 _total_bytes, quint64 _processed_bytes);
    void rollbackStarted();
    void rollbackFinished();
    void registrationStarted();
    void registrationFinished();
    void threadFinished();
    void installerError(QString _message);
    void taskSelectionChanged();
    void mediaTypeChanged(int _index);
    void renameGame();
    void removeGame();

signals:
    void gameInstalled(const QString & _id);

private:
    void addDiscImage(const QString & _file_path);
    void addDevice(QSharedPointer<Device> & _device);
    QTreeWidgetItem * findTaskInList(const QString & _device_filepath) const;
    QString truncateGameName(const QString & _name) const;
    bool startTask();
    void setCurrentProgressBarUnknownStatus(bool _unknown, int _value = 0);
    void setTaskError(const QString & _message, int _index = -1);
    QString canceledErrorMessage() const;

private:
    LambdaThread * mp_work_thread;
    GameInstaller * mp_installer;
    GameCollection & mr_collection;
    int m_processing_task_index;
    bool m_is_canceled;
};

#endif // __OPLPCTOOLS_GAMEINSTALLDIALOG__
