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

#ifndef __OPLPCTOOLS_VMCDETAILSACTIVITY__
#define __OPLPCTOOLS_VMCDETAILSACTIVITY__

#include <OplPcTools/Vmc.h>
#include <OplPcTools/VmcFS.h>
#include <OplPcTools/UI/Activity.h>
#include <OplPcTools/UI/Intent.h>
#include "ui_VmcDetailsActivity.h"

namespace OplPcTools {
namespace UI {


class VmcFileSystemViewModel;

class VmcDetailsActivity : public Activity, private Ui::VmcDetailsActivity
{
    Q_OBJECT

public:
    explicit VmcDetailsActivity(const Vmc & _vmc, QWidget * _parent = nullptr);


public:
    static QSharedPointer<Intent> createIntent(const Vmc & _vmc);

private:
    void setupShortcuts();
    void showErrorMessage(const QString & _message = QString());
    void hideErrorMessage();
    void loadVmcFS();
    void setupView();
    void setIconSize();
    void navigate(const QString & _path);
    void onFsListItemActivated(const QModelIndex & _index);
    void onFsBackButtonClick();
    void renameVmc();

private:
    const Vmc & mr_vmc;
    QSharedPointer<VmcFS> m_fs_ptr;
    VmcFileSystemViewModel * mp_model;
};


} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMCDETAILSACTIVITY__
