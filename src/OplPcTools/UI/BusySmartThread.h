/***********************************************************************************************
 * Copyright © 2017-2025 Sergey Smolyannikov aka brainstream                                   *
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

#ifndef __BUSYSMARTTHREAD_LIBRARY__
#define __BUSYSMARTTHREAD_LIBRARY__

#include <QTimer>
#include <OplPcTools/UI/BusyDialog.h>
#include <OplPcTools/UI/LambdaThread.h>

namespace OplPcTools {
namespace UI {

class BusySmartThread : public QObject
{
    Q_OBJECT

public:
    BusySmartThread(std::function<void()> _lambda, QWidget * _parent_widget);
    ~BusySmartThread() override;
    void setSpinnerDisplayTimeout(uint32_t _timeout_ms);
    void start();

signals:
    void exception(QString _message);
    void finished();

private:
    void showBusyDialog();
    void destroyBusyDialog();
    void finish();

private:
    uint32_t m_spinner_display_timeout;
    QWidget * mp_parent_widget;
    BusyDialog * mp_dialog;
    LambdaThread * mp_thread;
    QTimer * mp_timer;
};

} // namespace UI
} // namespace OplPcTools

#endif // __BUSYSMARTTHREAD_LIBRARY__
