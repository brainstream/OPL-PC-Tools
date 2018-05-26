/***********************************************************************************************
 * Copyright © 2017-2018 Sergey Smolyannikov aka brainstream                                   *
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

#include <OplPcTools/Settings.h>
#include <OplPcTools/Updater.h>
#include <OplPcTools/UI/SettingsDialog.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

SettingsDialog::SettingsDialog(QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    setupUi(this);
    Settings & settings = Settings::instance();
    mp_checkbx_reopen_last_catalog->setChecked(settings.reopenLastSestion());
    mp_checkbox_confirm_game_deletion->setChecked(settings.confirmGameDeletion());
    mp_checkbox_confirm_picture_deletion->setChecked(settings.confirmPixmapDeletion());
    mp_groupbox_donot_splitup->setChecked(!settings.splitUpIso());
    mp_checkbox_add_id->setChecked(settings.renameIso());
    mp_checkobx_move_iso->setChecked(settings.moveIso());
    if(Updater::isSupported())
        mp_checkbox_check_new_versions->setChecked(settings.checkNewVersion());
    else
        mp_checkbox_check_new_versions->setEnabled(false);
}

void SettingsDialog::accept()
{
    Settings & settings = Settings::instance();
    settings.setReopenLastSestion(mp_checkbx_reopen_last_catalog->isChecked());
    settings.setConfirmGameDeletion(mp_checkbox_confirm_game_deletion->isChecked());
    settings.setConfirmPixmapDeletion(mp_checkbox_confirm_picture_deletion->isChecked());
    settings.setSplitUpIso(!mp_groupbox_donot_splitup->isChecked());
    settings.setRenameIso(mp_checkbox_add_id->isChecked());
    settings.setMoveIso(mp_checkobx_move_iso->isChecked());
    if(mp_checkbox_check_new_versions->isEnabled())
        settings.setCheckNewVersion(mp_checkbox_check_new_versions->isChecked());
    QDialog::accept();
}
