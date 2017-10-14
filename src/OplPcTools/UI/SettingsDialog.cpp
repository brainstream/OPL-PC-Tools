/***********************************************************************************************
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

#include <OplPcTools/Misc/Settings.h>
#include <OplPcTools/UI/SettingsDialog.h>

SettingsDialog::SettingsDialog(QWidget * _parent) :
    QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    setupUi(this);
    Settings & settings = Settings::instance();
    mp_checkbox_reopen_last_session->setChecked(settings.reopenLastSestion());
    mp_checkbox_confirm_game_deletion->setChecked(settings.confirmGameDeletion());
    mp_checkbox_confirm_pixmap_deletion->setChecked(settings.confirmPixmapDeletion());
    mp_radiobtn_split_up->setChecked(settings.splitUpIso());
    mp_checkbox_rename_iso->setChecked(settings.renameIso());
    mp_checkbox_move_iso->setChecked(settings.moveIso());
}

void SettingsDialog::accept()
{
    Settings & settings = Settings::instance();
    settings.setReopenLastSestion(mp_checkbox_reopen_last_session->isChecked());
    settings.setConfirmGameDeletion(mp_checkbox_confirm_game_deletion->isChecked());
    settings.setConfirmPixmapDeletion(mp_checkbox_confirm_pixmap_deletion->isChecked());
    settings.setSplitUpIso(mp_radiobtn_split_up->isChecked());
    settings.setMoveIso(mp_checkbox_move_iso->isChecked());
    settings.setRenameIso(mp_checkbox_rename_iso->isChecked());
    QDialog::accept();
}
