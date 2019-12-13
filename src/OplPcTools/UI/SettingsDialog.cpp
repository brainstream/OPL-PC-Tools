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
    mp_checkbx_reopen_last_catalog->setChecked(settings.flag(Settings::Flag::ReopenLastSession));
    mp_checkbox_confirm_game_deletion->setChecked(settings.flag(Settings::Flag::ConfirmGameDeletion));
    mp_checkbox_confirm_picture_deletion->setChecked(settings.flag(Settings::Flag::ConfirmPixmapDeletion));
    mp_checkbox_donot_splitup->setChecked(!settings.flag(Settings::Flag::SplitUpIso));
    mp_checkbox_add_id->setChecked(settings.flag(Settings::Flag::RenameIso));
    mp_checkobx_move_iso->setChecked(settings.flag(Settings::Flag::MoveIso));
    mp_checkbox_validate_ulcfg->setChecked(settings.flag(Settings::Flag::ValidateUlCfg));
    if(Updater::isSupported())
        mp_checkbox_check_new_versions->setChecked(settings.flag(Settings::Flag::CheckNewVersion));
    else
        mp_checkbox_check_new_versions->setEnabled(false);
    mp_tabs->setCurrentIndex(0);
}

void SettingsDialog::accept()
{
    Settings & settings = Settings::instance();
    settings.setFlag(Settings::Flag::ReopenLastSession, mp_checkbx_reopen_last_catalog->isChecked());
    settings.setFlag(Settings::Flag::ConfirmGameDeletion, mp_checkbox_confirm_game_deletion->isChecked());
    settings.setFlag(Settings::Flag::ConfirmPixmapDeletion, mp_checkbox_confirm_picture_deletion->isChecked());
    settings.setFlag(Settings::Flag::SplitUpIso, !mp_checkbox_donot_splitup->isChecked());
    settings.setFlag(Settings::Flag::RenameIso, mp_checkbox_add_id->isChecked());
    settings.setFlag(Settings::Flag::MoveIso, mp_checkobx_move_iso->isChecked());
    settings.setFlag(Settings::Flag::ValidateUlCfg, mp_checkbox_validate_ulcfg->isChecked());
    settings.setFlag(Settings::Flag::CheckNewVersion,
        mp_checkbox_check_new_versions->isEnabled() && mp_checkbox_check_new_versions->isChecked());
    QDialog::accept();
}
