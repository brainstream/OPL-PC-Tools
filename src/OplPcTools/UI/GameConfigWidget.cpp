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

#include <OplPcTools/Library.h>
#include <OplPcTools/UI/VmcCreateDialog.h>
#include <OplPcTools/UI/GameConfigWidget.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

GameConfigWidget::GameConfigWidget(const Game & _game, QWidget * _parent /*= nullptr*/) :
    QWidget( _parent),
    mr_game(_game),
    mr_vmcs(Library::instance().vmcs())
{
    setupUi(this);
    initVmcComboboxes();
    connect(&mr_vmcs, &VmcCollection::vmcAdded, this, &GameConfigWidget::onVmcAdded);
    connect(&mr_vmcs, &VmcCollection::vmcDeleted, this, &GameConfigWidget::onVmcDeleted);
    connect(&mr_vmcs, &VmcCollection::vmcRenamed, this, &GameConfigWidget::onVmcRenamed);
    connect(mp_btn_create_vmc1, &QPushButton::clicked, this, &GameConfigWidget::createVmc1);
    connect(mp_btn_create_vmc2, &QPushButton::clicked, this, &GameConfigWidget::createVmc2);
}

void GameConfigWidget::initVmcComboboxes()
{
    mp_combo_vmc1->addItem("", QUuid(nullptr));
    mp_combo_vmc2->addItem("", QUuid(nullptr));
    int count = mr_vmcs.count();
    for(int i = 0; i < count; ++i)
    {
        const Vmc * vmc = mr_vmcs[i];
        mp_combo_vmc1->addItem(vmc->title(), vmc->uuid());
        mp_combo_vmc2->addItem(vmc->title(), vmc->uuid());
    }
    sortVmcComboboxes();
}

void GameConfigWidget::sortVmcComboboxes()
{
    mp_combo_vmc1->model()->sort(0);
    mp_combo_vmc2->model()->sort(0);
}

void GameConfigWidget::onVmcAdded(const QUuid & _id)
{
    const Vmc * vmc = mr_vmcs[_id];
    mp_combo_vmc1->addItem(vmc->title(), vmc->uuid());
    mp_combo_vmc2->addItem(vmc->title(), vmc->uuid());
    sortVmcComboboxes();
}

void GameConfigWidget::onVmcDeleted(const QUuid & _id)
{
    deleteVmcComboboxItem(mp_combo_vmc1, _id);
    deleteVmcComboboxItem(mp_combo_vmc2, _id);
}

void GameConfigWidget::deleteVmcComboboxItem(QComboBox * _combobox, const QUuid & _id)
{
    int index = getVmcComboboxItemIndex(_combobox, _id);
    if(index >= 0) _combobox->removeItem(index);
}

int GameConfigWidget::getVmcComboboxItemIndex(QComboBox * _combobox, const QUuid & _id) const
{
    int count = _combobox->count();
    for(int i = 0; i < count; ++i)
    {
        if(_combobox->itemData(i).toUuid() == _id)
            return i;
    }
    return -1;
}

void GameConfigWidget::onVmcRenamed(const QUuid & _id)
{
    const Vmc * vmc = mr_vmcs[_id];
    renameVmcComboboxItem(mp_combo_vmc1, _id, vmc->title());
    renameVmcComboboxItem(mp_combo_vmc2, _id, vmc->title());
    sortVmcComboboxes();
}

void GameConfigWidget::renameVmcComboboxItem(QComboBox * _combobox, const QUuid & _id, const QString & _title)
{
    int index = getVmcComboboxItemIndex(_combobox, _id);
    if(index >= 0) _combobox->setItemText(index, _title);
}

void GameConfigWidget::createVmc1()
{
    const Vmc * vmc = createVmc();
    if(vmc)
    {
        int index = getVmcComboboxItemIndex(mp_combo_vmc1, vmc->uuid());
        if(index >= 0) mp_combo_vmc1->setCurrentIndex(index);
    }
}

void GameConfigWidget::createVmc2()
{
    const Vmc * vmc = createVmc();
    if(vmc)
    {
        int index = getVmcComboboxItemIndex(mp_combo_vmc2, vmc->uuid());
        if(index >= 0) mp_combo_vmc2->setCurrentIndex(index);
    }
}

const Vmc * GameConfigWidget::createVmc()
{
    VmcCreateDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted)
    {
        return dlg.createdVmc();
    }
    return nullptr;
}
