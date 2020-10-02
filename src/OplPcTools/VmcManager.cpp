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

#include <OplPcTools/VmcManager.h>
#include <QVector>

using namespace OplPcTools;

class VmcManager::VmcList final
{
public:
    VmcList()
    {
        vmcs = {
            new Vmc("Test #1", VmcSize::_8M),
            new Vmc("Test #2", VmcSize::_128M),
            new Vmc("Test #3", VmcSize::_16M),
            new Vmc("Test #4", VmcSize::_8M),
            new Vmc("Test #5", VmcSize::_64M),
        };
    }

    ~VmcList()
    {
        for(const Vmc * vmc: vmcs)
            delete vmc;
    }

    QVector<Vmc *> vmcs;
};

VmcManager::VmcManager(QObject * _parent /*= nullptr*/) :
    QObject(_parent),
    mp_vmcs(nullptr)
{
}

VmcManager::~VmcManager()
{
    delete mp_vmcs;
}


void VmcManager::load(const QDir & _directory)
{

}

bool VmcManager::isLoaded() const
{
    return mp_vmcs != nullptr;
}

const int VmcManager::count() const
{
    return isLoaded() ? mp_vmcs->vmcs.size() : 0;
}

const Vmc * VmcManager::operator[](int _index) const
{
    return isLoaded() && count() > _index && _index >= 0 ? mp_vmcs->vmcs[_index] : nullptr;
}
