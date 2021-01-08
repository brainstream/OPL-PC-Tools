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
#include <OplPcTools/Exception.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/FilenameValidator.h>
#include <OplPcTools/UI/VmcCreateDialog.h>

using namespace OplPcTools::UI;

VmcCreateDialog::VmcCreateDialog(QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    setupUi(this);
    mp_edit_title->setValidator(new FilenameValidator(this));
    mp_combobox_size->addItem(tr("8 MiB"), static_cast<int>(VmcSize::_8M));
    mp_combobox_size->addItem(tr("16 MiB"), static_cast<int>(VmcSize::_16M));
    mp_combobox_size->addItem(tr("32 MiB"), static_cast<int>(VmcSize::_32M));
    mp_combobox_size->addItem(tr("64 MiB"), static_cast<int>(VmcSize::_64M));
    mp_combobox_size->addItem(tr("128 MiB"), static_cast<int>(VmcSize::_128M));
    connect(mp_button_box, &QDialogButtonBox::accepted, this, &VmcCreateDialog::create);
    connect(mp_button_box, &QDialogButtonBox::rejected, this, &VmcCreateDialog::reject);
    connect(mp_edit_title, &QLineEdit::textChanged, this, &VmcCreateDialog::onFilenameChanged);
    onFilenameChanged();
}


void VmcCreateDialog::create()
{
    try
    {
        mp_created_vmc = Library::instance().vmcs().createVmc(
            mp_edit_title->text(),
            static_cast<VmcSize>(mp_combobox_size->currentData().toInt())
        );
        accept();
    }
    catch(const Exception & exception)
    {
        mp_label_error_message->setText(exception.message());
    }
}

void VmcCreateDialog::onFilenameChanged()
{
    QString filename = mp_edit_title->text().trimmed();
    mp_button_box->button(QDialogButtonBox::Save)->setDisabled(filename.isEmpty());
}
