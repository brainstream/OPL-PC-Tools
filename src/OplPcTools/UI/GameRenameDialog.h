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

#ifndef __OPLPCTOOLS_GAMERENAMEDIALOG__
#define __OPLPCTOOLS_GAMERENAMEDIALOG__

#include <OplPcTools/GameInstallationType.h>
#include "ui_GameRenameDialog.h"

namespace OplPcTools {
namespace UI {

class GameRenameDialog : public QDialog, private Ui::GameRenameDialog
{
    Q_OBJECT

private:
    class Validator
    {
    public:
        virtual ~Validator() { }
        virtual bool validate(const QString & _name) = 0;
        virtual const QString message() const = 0;
    };

    class UlConfigNameValidator : public GameRenameDialog::Validator
    {
    public:
        bool validate(const QString & _name) override;
        const QString message() const override;

    private:
        QString m_message;
    };

    class FilenameValidator : public GameRenameDialog::Validator
    {
    public:
        FilenameValidator();
        bool validate(const QString & _name) override;
        const QString message() const override;

    private:
        bool m_is_invalid;
        QString m_message;
    };

public:
    GameRenameDialog(const QString & _initial_name, OplPcTools::GameInstallationType _installation_type, QWidget *_parent = nullptr);
    ~GameRenameDialog() override;
    QString name() const;

private slots:
    void nameChanged(const QString & _name);

private:
    Validator * mp_validator;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMERENAMEDIALOG__
