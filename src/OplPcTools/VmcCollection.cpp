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

#include <QVector>
#include <OplPcTools/VmcFS.h>
#include <OplPcTools/File.h>
#include <OplPcTools/VmcCollection.h>

using namespace OplPcTools;

VmcCollection::VmcCollection(QObject * _parent /*= nullptr*/) :
    QObject(_parent),
    mp_vmcs(new QVector<Vmc *>)
{
}

VmcCollection::~VmcCollection()
{
    if(mp_vmcs)
    {
        for(const Vmc * vmc: *mp_vmcs)
            delete vmc;
    }
    delete mp_vmcs;
}


bool VmcCollection::load(const QDir & _base_directory)
{
    mp_vmcs->clear();
    QString vmc_dir_path = _base_directory.absoluteFilePath("VMC");
    m_directory = QDir(vmc_dir_path);
    QFileInfo fi(vmc_dir_path);
    if(fi.exists())
    {
        if(!fi.isDir())
            return false;
    }
    else
    {
        return true;
    }
    for(const QString & filename : m_directory.entryList({ "*.bin" }, QDir::Files | QDir::Readable))
    {
        QString title = filename.left(filename.lastIndexOf("."));
        QFileInfo file(m_directory.absoluteFilePath(filename));
        uint32_t size = file.size() / (1024 * 1024);
        mp_vmcs->append(new Vmc(file.absoluteFilePath(), title, size));
    }
    return true;
}


bool VmcCollection::isLoaded() const
{
    return mp_vmcs != nullptr;
}

const int VmcCollection::count() const
{
    return isLoaded() ? mp_vmcs->size() : 0;
}

const Vmc * VmcCollection::operator[](int _index) const
{
    return isLoaded() && count() > _index && _index >= 0 ? (*mp_vmcs)[_index] : nullptr;
}

const Vmc * VmcCollection::operator[](const Uuid & _uuid) const
{
    return findVmc(_uuid);
}

const Vmc * VmcCollection::createVmc(const QString & _title, uint32_t _size_mib)
{
    for(const Vmc * vmc: *mp_vmcs)
    {
        if(_title == vmc->title())
            throw Exception(tr("VMC with name \"%1\" already exists").arg(_title));
    }
    validateFilename(_title);
    ensureDirectoryExists();
    QString vmc_filename = makeFilename(_title);
    VmcFS::create(vmc_filename, _size_mib);
    Vmc * vmc = new Vmc(vmc_filename, _title, _size_mib);
    mp_vmcs->append(vmc);
    emit vmcAdded(vmc->uuid());
    return vmc;
}

void VmcCollection::ensureDirectoryExists()
{
    m_directory.mkpath(".");
}

void VmcCollection::renameVmc(const Uuid & _uuid, const QString & _title)
{
    validateFilename(_title);
    Vmc * vmc = findVmc(_uuid);
    if(vmc == nullptr || vmc->title() == _title)
        return;
    QString old_title = vmc->title();
    QString src = makeFilename(old_title);
    QString dest = makeFilename(_title);
    renameFile(src, dest);
    vmc->setTitle(_title);
    emit vmcRenamed(_uuid);
}

QString VmcCollection::makeFilename(const QString & _vmc_title) const
{
    return m_directory.absoluteFilePath(_vmc_title + ".bin");
}

Vmc * VmcCollection::findVmc(const Uuid & _uuid) const
{
    for(Vmc * vmc : *mp_vmcs)
    {
        if(vmc->uuid() == _uuid)
            return vmc;
    }
    return nullptr;
}

void VmcCollection::deleteVmc(const Uuid & _uuid)
{
    for(int i = 0; i < mp_vmcs->size(); ++i)
    {
        Vmc * vmc = mp_vmcs->value(i);
        if(vmc->uuid() == _uuid)
        {
            if(QFile::remove(makeFilename(vmc->title())))
            {
                emit vmcAboutToBeDeleted(_uuid);
                mp_vmcs->removeAt(i);
                delete vmc;
                emit vmcDeleted(_uuid);
            }
            return;
        }
    }
}
