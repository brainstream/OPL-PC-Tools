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

#include <OplPcTools/UI/AcceptCheatDialog.h>
#include <QPushButton>

using namespace OplPcTools::UI;

AcceptCheatDialog::AcceptCheatDialog(const QString & _text, QWidget * _parent) :
    QDialog(_parent)
{
    setupUi(this);
    mp_text_edit_cheat->setText(_text);
    connect(mp_button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(mp_button_box->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &QDialog::accept);
    connect(mp_button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
