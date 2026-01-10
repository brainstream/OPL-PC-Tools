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

#include <OplPcTools/UI/VmcFileNameDialog.h>
#include <OplPcTools/MemoryCard/FSEntryNameValidator.h>
#include <OplPcTools/FilenameValidator.h>
#include <QPushButton>

using namespace OplPcTools::UI;

VmcFileNameDialog::VmcFileNameDialog(QWidget * _parent) :
    QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    setupUi(this);
    setTitle(false);
    FilenameValidator * name_validator = new FilenameValidator(MemoryCard::g_entry_name_forbidden_characters, this);
    name_validator->setMaxLength(MemoryCard::g_max_entry_name_length); // TODO: validate length of encoded string
    mp_edit_name->setValidator(name_validator);
    mp_label_error_message->setHidden(true);
    connect(mp_edit_name, &QLineEdit::textChanged, this, &VmcFileNameDialog::setSaveButtonState);
    connect(mp_button_box, &QDialogButtonBox::accepted, this, &VmcFileNameDialog::accept);
    connect(mp_button_box, &QDialogButtonBox::rejected, this, &VmcFileNameDialog::reject);
    setSaveButtonState();
}

void VmcFileNameDialog::setCurrentFilename(const QString & _filename)
{
    mp_edit_name->setText(_filename);
    mp_edit_name->setSelection(0, _filename.length());
}

const QString VmcFileNameDialog::currentFilename() const
{
    return mp_edit_name->text();
}

void VmcFileNameDialog::setTitle(bool _is_directory)
{
    setWindowTitle(_is_directory ? tr("Directory Name") : tr("File Name"));
}

void VmcFileNameDialog::setSaveButtonState()
{
    mp_button_box->button(QDialogButtonBox::Save)->setEnabled(mp_edit_name->hasAcceptableInput());
}
