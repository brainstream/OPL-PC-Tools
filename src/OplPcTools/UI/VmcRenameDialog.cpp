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

#include <OplPcTools/UI/VmcRenameDialog.h>
#include <OplPcTools/File.h>
#include <OplPcTools/FilenameValidator.h>
#include <QPushButton>

using namespace OplPcTools::UI;

VmcRenameDialog::VmcRenameDialog(const QString & _name, QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    setupUi(this);
    mp_edit_name->setValidator(new FilenameValidator(g_filename_forbidden_characters, this));
    connect(mp_button_box, &QDialogButtonBox::accepted, this, &VmcRenameDialog::accept);
    connect(mp_button_box, &QDialogButtonBox::rejected, this, &VmcRenameDialog::reject);
    connect(mp_edit_name, &QLineEdit::textChanged, this, &VmcRenameDialog::setSaveButtonState);
    mp_edit_name->setText(_name);
    mp_edit_name->selectAll();
    mp_edit_name->setFocus();
}

void VmcRenameDialog::setSaveButtonState()
{
    mp_button_box->button(QDialogButtonBox::Save)->setEnabled(mp_edit_name->hasAcceptableInput());
}

QString VmcRenameDialog::name() const
{
    return mp_edit_name->text();
}
