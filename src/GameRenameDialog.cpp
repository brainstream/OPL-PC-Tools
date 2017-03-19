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

#include <QPushButton>
#include "GameRenameDialog.h"
#include "UlConfig.h"

GameRenameDialog::GameRenameDialog(const QString & _initial_name, QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    setupUi(this);
    mp_edit_name->setText(_initial_name);
    mp_edit_name->selectAll();
}

QString GameRenameDialog::name() const
{
    return mp_edit_name->text();
}

void GameRenameDialog::nameChanged(const QString & _name)
{
    int bytes_left = UL_MAX_GAME_NAME_LENGTH - _name.toUtf8().length();
    if(bytes_left < 0)
    {
        mp_label_bytes->setText(tr("Length exceeded by %1 byte(s)").arg(-bytes_left));
        mp_button_box->button(QDialogButtonBox::Ok)->setDisabled(true);
    }
    else
    {
        mp_label_bytes->setText(tr("%1 byte(s) left").arg(bytes_left));
        mp_button_box->button(QDialogButtonBox::Ok)->setDisabled(_name.isEmpty());
    }
}
