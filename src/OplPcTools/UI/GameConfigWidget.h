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

#ifndef __OPLPCTOOLS_GAMECONFIGWIDGET__
#define __OPLPCTOOLS_GAMECONFIGWIDGET__

#include <QUuid>
#include <OplPcTools/Game.h>
#include <OplPcTools/VmcCollection.h>
#include "ui_GameConfigWidget.h"

namespace OplPcTools::UI {

class GameConfigWidget : public QWidget, private Ui::GameConfigWidget
{
    Q_OBJECT

public:
    explicit GameConfigWidget(const Game & _game, QWidget * _parent = nullptr);

private:
    void initVmcComboboxes();
    void sortVmcComboboxes();
    void onVmcAdded(const QUuid & _id);
    void onVmcDeleted(const QUuid & _id);
    void deleteVmcComboboxItem(QComboBox * _combobox, const QUuid & _id);
    int getVmcComboboxItemIndex(QComboBox * _combobox, const QUuid & _id) const;
    void onVmcRenamed(const QUuid & _id);
    void renameVmcComboboxItem(QComboBox * _combobox, const QUuid & _id, const QString & _title);
    void createVmc1();
    void createVmc2();
    const OplPcTools::Vmc * createVmc();

private:
    VmcCollection & mr_vmcs;
    const Game & mr_game;
};

} // namespace OplPcTools::UI

#endif // __OPLPCTOOLS_GAMECONFIGWIDGET__
