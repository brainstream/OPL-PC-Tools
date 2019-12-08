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

#include <QPushButton>
#include <OplPcTools/UlConfigGameStorage.h>
#include <OplPcTools/DirectoryGameStorage.h>
#include <OplPcTools/Exception.h>
#include <OplPcTools/UI/GameRenameDialog.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

bool GameRenameDialog::UlConfigNameValidator::validate(const QString & _name)
{
    int bytes_left = UlConfigGameStorage::max_name_length - _name.toUtf8().length();
    if(bytes_left < 0)
    {
        m_message = QObject::tr("Length exceeded by %1 byte(s)").arg(-bytes_left);
        return false;
    }
    else
    {
        m_message = QObject::tr("%1 byte(s) left").arg(bytes_left);
        return true;
    }
}

const QString GameRenameDialog::UlConfigNameValidator::message() const
{
    return m_message;
}

GameRenameDialog::FilenameValidator::FilenameValidator() :
    m_is_invalid(false)
{
}

bool GameRenameDialog::FilenameValidator::validate(const QString & _name)
{
    m_is_invalid = false;
    try
    {
        DirectoryGameStorage::validateTitle(_name);
    }
    catch(const ValidationException & _exception)
    {
        m_is_invalid = true;
        m_message = _exception.message();
    }
    return !m_is_invalid;
}

const QString GameRenameDialog::FilenameValidator::message() const
{
    return m_is_invalid ? m_message : QString();
}

GameRenameDialog::GameRenameDialog(const QString & _initial_name,
                                   GameInstallationType _installation_type,
                                   QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    if(_installation_type == GameInstallationType::Directory)
        mp_validator = new FilenameValidator();
    else
        mp_validator = new UlConfigNameValidator();
    setupUi(this);
    mp_edit_name->setText(_initial_name);
    mp_edit_name->selectAll();
}

GameRenameDialog::~GameRenameDialog()
{
    delete mp_validator;
}

QString GameRenameDialog::name() const
{
    return mp_edit_name->text();
}

void GameRenameDialog::nameChanged(const QString & _name)
{
    mp_button_box->button(QDialogButtonBox::Ok)->setDisabled(!mp_validator->validate(_name));
    mp_label_message->setText(mp_validator->message());
}
