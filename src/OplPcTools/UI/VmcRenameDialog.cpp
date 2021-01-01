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

#include <QPushButton>
#include <OplPcTools/File.h>
#include <OplPcTools/UI/VmcRenameDialog.h>

using namespace OplPcTools::UI;

VmcRenameDialog::VmcRenameDialog(const QString & _name, QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    setupUi(this);
    connect(mp_edit_name, &QLineEdit::textChanged, this, &VmcRenameDialog::onNameChanged);
    connect(mp_button_box, &QDialogButtonBox::accepted, this, &VmcRenameDialog::accept);
    connect(mp_button_box, &QDialogButtonBox::rejected, this, &VmcRenameDialog::reject);
    mp_edit_name->setText(_name);
    mp_edit_name->selectAll();
    mp_edit_name->setFocus();
}

QString VmcRenameDialog::name() const
{
    return mp_edit_name->text();
}

void VmcRenameDialog::onNameChanged(const QString & _name)
{
    QString error_message;
    try
    {
        validateFilename(_name);
    }
    catch(const Exception & exception)
    {
        error_message = exception.message();
    }
    mp_button_box->button(QDialogButtonBox::Ok)->setDisabled(!error_message.isEmpty());
    mp_label_error_message->setText(error_message);
}
