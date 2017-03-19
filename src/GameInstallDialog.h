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

#ifndef __QPCOPL_GAMEINSTALLDIALOG__
#define __QPCOPL_GAMEINSTALLDIALOG__

#include <QThread>
#include "GameInstaller.h"
#include "ui_GameInstallDialog.h"

class GameInstallDialog : public QDialog, private Ui::GameInstallDialog
{
    Q_OBJECT

public:
    explicit GameInstallDialog(GameInstaller & _installer, QWidget * _parent = nullptr);
    ~GameInstallDialog() override;

public slots:
    int exec() override;
    void reject() override;

protected:
    void closeEvent(QCloseEvent * _event) override;

private slots:
    void installProgress(quint64 _total_bytes, quint64 _processed_bytes);
    void rollbackStarted();
    void rollbackFinished();
    void threadFinished();

private:
    void connectInstaller();
    void disconnectInstaller();

private:
    GameInstaller * mp_installer;
    QThread * mp_work_thread;
};

#endif // __QPCOPL_GAMEINSTALLDIALOG__
