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

#ifndef __OPLPCTOOLS_VMCDETAILSACTIVITY__
#define __OPLPCTOOLS_VMCDETAILSACTIVITY__

#include <OplPcTools/Vmc.h>
#include <OplPcTools/UI/Activity.h>
#include <OplPcTools/UI/Intent.h>
#include <OplPcTools/MemoryCard/FileSystem.h>
#include "ui_VmcDetailsActivity.h"

namespace OplPcTools {
namespace UI {


class VmcFileSystemViewModel;

class VmcDetailsActivity : public Activity, private Ui::VmcDetailsActivity
{
    Q_OBJECT

public:
    explicit VmcDetailsActivity(const Vmc & _vmc, QWidget * _parent = nullptr);
    static QSharedPointer<Intent> createIntent(const Vmc & _vmc);

private:
    void setupShortcuts();
    void showErrorMessage(const QString & _message = QString());
    void hideErrorMessage();
    void loadFileManager();
    QString getFsEncoding() const;
    void setIconSize();
    void navigate(const MemoryCard::Path & _path);
    void onFsListItemActivated(const QModelIndex & _index);
    void onFsBackButtonClick();
    void onEncodingChanged();
    void renameVmc();
    void showTreeContextMenu(const QPoint & _point);
    void createDirectory();
    void renameEntry();
    void upload();

private:
    const Vmc & mr_vmc;
    QSharedPointer<MemoryCard::FileSystem> m_vmc_fs_ptr;
    VmcFileSystemViewModel * mp_model;
};


} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMCDETAILSACTIVITY__
