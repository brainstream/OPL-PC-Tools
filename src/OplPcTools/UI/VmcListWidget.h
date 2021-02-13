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

#ifndef __OPLPCTOOLS_VMCLISTWIDGET__
#define __OPLPCTOOLS_VMCLISTWIDGET__

#include <functional>
#include "ui_VmcListWidget.h"
#include <QSortFilterProxyModel>
#include <OplPcTools/Vmc.h>

namespace OplPcTools {
namespace UI {


class VmcListWidget: public QWidget, private Ui::VmcListWidget
{
    Q_OBJECT

public:
    explicit VmcListWidget(QWidget * _parent = nullptr);

private:
    void setupShortcuts();
    void setIconSize();
    void renameVmc();
    void deleteVmc();
    void createVmc();
    void exportFiles();

private:
    void onVmcSelected();
    void activateItemControls(const Vmc * _vmc);
    void showTreeContextMenu(const QPoint & _point);
    void onTreeViewItemActivated(const QModelIndex & _index);
    void showVmcProperties();
    void startSmartThread(std::function<void()> _lambda);

private:
    class VmcTreeModel;
    VmcTreeModel * mp_model;
    QSortFilterProxyModel * mp_proxy_model;
    QMenu * mp_context_menu;
};


} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMCLISTWIDGET__
