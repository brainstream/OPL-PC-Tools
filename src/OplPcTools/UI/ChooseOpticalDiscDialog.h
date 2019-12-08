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

#ifndef __OPLPCTOOLS_CHOOSEOPTICALDISCDIALOG__
#define __OPLPCTOOLS_CHOOSEOPTICALDISCDIALOG__

#include <QSharedPointer>
#include <OplPcTools/Device.h>
#include "ui_ChooseOpticalDiscDialog.h"

namespace OplPcTools {
namespace UI {

class ChooseOpticalDiscDialog : public QDialog, private Ui::ChooseOpticalDiscDialog
{
    Q_OBJECT

public:
    explicit ChooseOpticalDiscDialog(QWidget * _parent = nullptr);
    QList<QSharedPointer<OplPcTools::Device>> devices() const;

private slots:
    void deviceSelectionChanged();

private:
    void fixDeviceTitle(OplPcTools::Device & _device) const;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_CHOOSEOPTICALDISCDIALOG__
