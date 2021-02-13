/***********************************************************************************************
 * Copyright © 2017-2021 Sergey Smolyannikov aka brainstream                                   *
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

#include <OplPcTools/UI/BusySmartThread.h>

using namespace OplPcTools::UI;

BusySmartThread::BusySmartThread(std::function<void()> _lambda, QWidget * _parent_widget) :
    QObject(_parent_widget),
    m_spinner_display_timeout(700),
    mp_parent_widget(_parent_widget),
    mp_dialog(nullptr)
{
    mp_thread = new LambdaThread(_lambda, this);
    mp_timer = new QTimer(this);
    mp_timer->setSingleShot(true);
    connect(mp_thread, &LambdaThread::exception, this, &BusySmartThread::exception);
    connect(mp_thread, &LambdaThread::finished, this, &BusySmartThread::finish);
    connect(mp_timer, &QTimer::timeout, this, &BusySmartThread::showBusyDialog);
}

BusySmartThread::~BusySmartThread()
{
    destroyBusyDialog();
}

void BusySmartThread::destroyBusyDialog()
{
    if(mp_dialog)
    {
        mp_dialog->hide();
        delete mp_dialog;
        mp_dialog = nullptr;
    }
}

void BusySmartThread::setSpinnerDisplayTimeout(uint32_t _timeout_ms)
{
    m_spinner_display_timeout = _timeout_ms;
}

void BusySmartThread::start()
{
    mp_timer->setInterval(m_spinner_display_timeout);
    mp_timer->start();
    mp_thread->start();
}

void BusySmartThread::finish()
{
    mp_timer->stop();
    destroyBusyDialog();
    emit finished();
}

void BusySmartThread::showBusyDialog()
{
    if(mp_dialog)
        return;
    mp_dialog = new BusyDialog(mp_parent_widget);
    mp_dialog->show();
}
