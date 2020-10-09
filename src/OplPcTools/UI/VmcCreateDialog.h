/***********************************************************************************************
 * Copyright © 2017-2020 Sergey Smolyannikov aka brainstream                                   *
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

#ifndef __OPLPCTOOLS_VMCCREATEDIALOG__
#define __OPLPCTOOLS_VMCCREATEDIALOG__

#include "ui_VmcCreateDialog.h"

namespace OplPcTools {
namespace UI {

class VmcCreateDialog : public QDialog, private Ui::VmcCreateDialog
{
    Q_OBJECT

public:
    explicit VmcCreateDialog(QWidget * _parent = nullptr);

private:
    void create();
    void onFilenameChanged();
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMCCREATEDIALOG__
