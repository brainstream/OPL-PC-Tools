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

#ifndef __QPCOPL_CHOOSEOPTICALDISCDIALOG__
#define __QPCOPL_CHOOSEOPTICALDISCDIALOG__

#include <QSharedPointer>
#include "Device.h"
#include "ui_ChooseOpticalDiscDialog.h"

class ChooseOpticalDiscDialog : public QDialog, private Ui::ChooseOpticalDiscDialog
{
    Q_OBJECT

public:
    explicit ChooseOpticalDiscDialog(QWidget * _parent = nullptr);
    QList<QSharedPointer<Device>> devices() const;

private slots:
    void deviceSelectionChanged();

private:
    void fixDeviceTitle(Device & _device) const;
};

#endif // __QPCOPL_CHOOSEOPTICALDISCDIALOG__
