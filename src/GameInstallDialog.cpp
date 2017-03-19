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

#include <QDebug>
#include <QCloseEvent>
#include "GameInstallDialog.h"

namespace {

class WorkThread : public QThread
{
public:
    explicit WorkThread(GameInstaller & _installer, QObject * _parent = nullptr) :
        QThread(_parent),
        mr_installer(_installer)
    {
    }

    void run() override;

private:
    GameInstaller & mr_installer;
};

void WorkThread::run()
{
    mr_installer.install();
}

} // namespace

GameInstallDialog::GameInstallDialog(GameInstaller & _installer, QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    mp_installer(&_installer)
{
    setupUi(this);
    mp_progressbar->setMaximum(1000);
    mp_work_thread = new WorkThread(_installer);
    connect(mp_work_thread, &QThread::finished, this, &GameInstallDialog::threadFinished);
    connect(mp_work_thread, &QThread::finished, mp_work_thread, &QThread::deleteLater);
    connect(mp_installer, &GameInstaller::progress, this, &GameInstallDialog::installProgress);
    connect(mp_installer, &GameInstaller::rollbackStarted, this, &GameInstallDialog::rollbackStarted);
}

int GameInstallDialog::exec()
{
    mp_label_info->setText(tr("Game installation..."));
    mp_buttonbox->setDisabled(false);
    mp_work_thread->start(QThread::HighestPriority);
    return QDialog::exec();
}

void GameInstallDialog::reject()
{
    mp_buttonbox->setDisabled(true);
    mp_label_info->setText(tr("Preparation to abort..."));
    mp_work_thread->quit();
    mp_work_thread->requestInterruption();
}

void GameInstallDialog::closeEvent(QCloseEvent * _event)
{
    Q_UNUSED(_event)
}

void GameInstallDialog::installProgress(quint64 _total_bytes, quint64 _processed_bytes)
{
    mp_progressbar->setValue((static_cast<double>(_processed_bytes) / _total_bytes) * 1000);
    if(_total_bytes == _processed_bytes)
    {
        mp_work_thread->quit();
        mp_label_info->setText(tr("Almost done. Wait a few seconds."));
        mp_progressbar->setMaximum(0);
    }
}

void GameInstallDialog::rollbackStarted()
{
    mp_buttonbox->setDisabled(true);
    mp_label_info->setText(tr("Rolling back changes..."));
    mp_progressbar->setMaximum(0);
}

void GameInstallDialog::threadFinished()
{
    disconnect(mp_installer, &GameInstaller::progress, this, &GameInstallDialog::installProgress);
    disconnect(mp_installer, &GameInstaller::rollbackStarted, this, &GameInstallDialog::rollbackStarted);
    if(mp_installer->installedGameInfo() == nullptr)
        QDialog::reject();
    else
        accept();
}
