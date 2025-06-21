/***********************************************************************************************
 * Copyright © 2017-2025 Sergey Smolyannikov aka brainstream                                   *
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

#ifndef __OPLPCTOOLS_VMCRENAMEDIALOG__
#define __OPLPCTOOLS_VMCRENAMEDIALOG__

#include "ui_VmcRenameDialog.h"

namespace OplPcTools {
namespace UI {

class VmcRenameDialog : public QDialog, private Ui::VmcRenameDialog
{
    Q_OBJECT

public:
    explicit VmcRenameDialog(const QString & _name, QWidget * _parent = nullptr);
    QString name() const;

private:
    void onNameChanged(const QString & _name);
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMCRENAMEDIALOG__
