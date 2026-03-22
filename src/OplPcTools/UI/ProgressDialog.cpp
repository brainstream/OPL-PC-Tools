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

#include <OplPcTools/UI/ProgressDialog.h>
#include <QCloseEvent>
#include <QThread>

using namespace OplPcTools::UI;

ProgressDialog::ProgressDialog(QWidget * _parent) :
    QDialog(_parent, Qt::Dialog | Qt::CustomizeWindowHint),
    m_is_cancelation_disabled(false),
    m_is_one_shot_close_allowed(false)
{
    setupUi(this);
    mp_progress_bar->setRange(0, 0);
    mp_progress_bar->setValue(0);
    connect(mp_button_box, &QDialogButtonBox::rejected, this, &ProgressDialog::canceled);
    connect(this, &QDialog::rejected, this, &ProgressDialog::canceled);
    repackLayout();
}

void ProgressDialog::disableCancelation(bool _disable)
{
    m_is_cancelation_disabled = _disable;
    mp_button_box->setVisible(!_disable);
    repackLayout();
}

void ProgressDialog::repackLayout()
{
    setMinimumSize(QSize(0, 0));
    setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    adjustSize();
    setFixedSize(sizeHint());
}

void ProgressDialog::setProgressLabelText(const QString & _text)
{
    if(QThread::currentThread() != qApp->thread())
        QMetaObject::invokeMethod(this, "setProgressLabelText", Q_ARG(QString, _text));
    else
        mp_lable_progress->setText(_text);
}

void ProgressDialog::setProgressRange(int _min_value, int _max_value)
{
    mp_progress_bar->setRange(_min_value, _max_value);
}

void ProgressDialog::setProgressMinimum(int _min_value)
{
    mp_progress_bar->setMinimum(_min_value);
}

void ProgressDialog::setProgressMaximum(int _max_value)
{
    mp_progress_bar->setMaximum(_max_value);
}

void ProgressDialog::setProgressValue(int _value)
{
    if(QThread::currentThread() != qApp->thread())
        QMetaObject::invokeMethod(this, "setProgressValue", Q_ARG(int, _value));
    else
        mp_progress_bar->setValue(_value);
}

void ProgressDialog::accept()
{
    m_is_one_shot_close_allowed = true;
    QDialog::accept();
}

void ProgressDialog::closeEvent(QCloseEvent * _event)
{
    if(m_is_one_shot_close_allowed)
    {
        m_is_one_shot_close_allowed = false;
        _event->accept();
        return;
    }

    if(m_is_cancelation_disabled)
        _event->ignore();
    else
        emit canceled();
}
