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

#include <cmath>
#include <QMessageBox>
#include "IsoRecoverDialog.h"

namespace {

QString sizeToHumanReadebleFormat(quint64 _bytes)
{
    float num = _bytes;
    QStringList list { QObject::tr("KiB"), QObject::tr("MiB"), QObject::tr("GiB") };
    QStringListIterator i(list);
    QString unit(QObject::tr("bytes"));
    while(num >= 1024.0 && i.hasNext())
    {
        unit = i.next();
        num /= 1024.0;
    }
    return QString().setNum(num,'f',2) + " " + unit;
}

} // namespace

IsoRecoverDialog::IsoRecoverDialog(const Game & _game, const QString & _game_dirpath, const QString & _iso_filepath, QWidget * _parent) :
    QDialog(_parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    mp_parent(_parent),
    m_return_code(QDialog::Accepted),
    mp_work_thread(nullptr)
{
    setupUi(this);
    mp_recoverer = new IsoRecoverer(_game, _game_dirpath, _iso_filepath, this);
    mp_progress_bar->setValue(0);
    mp_progress_bar->setMaximum(s_progress_max);
}

IsoRecoverDialog::~IsoRecoverDialog()
{
    delete mp_recoverer;
}

void IsoRecoverDialog::showEvent(QShowEvent * _event)
{
    QDialog::showEvent(_event);
    if(mp_work_thread != nullptr)
        return;
    mp_work_thread = new LambdaThread([this]() {
        mp_recoverer->recover();
    }, this);
    connect(mp_work_thread, &QThread::finished, this, &IsoRecoverDialog::threadFinished);
    connect(mp_work_thread, &QThread::finished, mp_work_thread, &QThread::deleteLater);
    connect(mp_work_thread, &LambdaThread::exception, this, &IsoRecoverDialog::recoveryError);
    connect(mp_recoverer, &IsoRecoverer::progress, this, &IsoRecoverDialog::recoveryProgress);
    connect(mp_recoverer, &IsoRecoverer::rollbackStarted, this, &IsoRecoverDialog::rollbackStarted);
    mp_work_thread->start(QThread::HighestPriority);
}

void IsoRecoverDialog::recoveryProgress(quint64 _total_bytes, quint64 _processed_bytes)
{
    if(_total_bytes == _processed_bytes)
    {
        mp_button_box->setDisabled(true);
        mp_label_status->setText("Almost done. Please, wait a few seconds...");
        mp_progress_bar->setMaximum(0);
        mp_progress_bar->setValue(0);
    }
    else
    {
        double progress = floor((static_cast<double>(_processed_bytes) / _total_bytes) * s_progress_max);
        mp_progress_bar->setValue(progress);
        mp_label_status->setText(tr("Processed %1 from %2")
            .arg(sizeToHumanReadebleFormat(_processed_bytes))
            .arg(sizeToHumanReadebleFormat(_total_bytes)));
    }
}

void IsoRecoverDialog::rollbackStarted()
{
    m_return_code = QDialog::Rejected;
    mp_progress_bar->setMaximum(0);
    mp_progress_bar->setValue(0);
    mp_label_status->setText(tr("Rolling back..."));
}

void IsoRecoverDialog::threadFinished()
{
    QDialog::done(m_return_code);
}

void IsoRecoverDialog::recoveryError(QString _message)
{
    m_return_code = QDialog::Rejected;
    QMessageBox::critical(mp_parent, QString(), _message);
}

void IsoRecoverDialog::reject()
{
    m_return_code = QDialog::Rejected;
    mp_button_box->setDisabled(true);
    mp_work_thread->requestInterruption();
}
