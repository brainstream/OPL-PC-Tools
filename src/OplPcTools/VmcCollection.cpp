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

#include <QVector>
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
        VmcSize size = VmcSize::_8M;
        switch(file.size() / (1024 * 1024)) // FIXME: read VMC
        {
        case 16:
            size = VmcSize::_16M;
            break;
        case 32:
            size = VmcSize::_32M;
            break;
        case 64:
            size = VmcSize::_64M;
            break;
        case 128:
            size = VmcSize::_128M;
            break;
        case 256:
            size = VmcSize::_256M;
            break;
        }
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

const Vmc * VmcCollection::operator[](const QUuid & _uuid) const
{
    return findVmc(_uuid);
}

const Vmc * VmcCollection::createVmc(const QString & _title, VmcSize _size)
{
    for(const Vmc * vmc: *mp_vmcs)
    {
        if(_title == vmc->title())
            throw Exception(tr("VMC with name \"%1\" already exists").arg(_title));
    }
    validateFilename(_title);
    QString vmc_filename = makeFilename(_title);
    QFile file(vmc_filename);
    openFile(file, QFile::WriteOnly);

    // FIXME: THE TEST DATA
    //
    QByteArray buffer(1024 * 1024, '\0');
    for(int i = 0; i < static_cast<int>(_size); ++i)
        file.write(buffer);
    file.close();
    //
    // END OF THE TEST DATA

    Vmc * vmc = new Vmc(vmc_filename, _title, _size);
    mp_vmcs->append(vmc);
    emit vmcAdded(vmc->uuid());
    return vmc;
}

void VmcCollection::renameVmc(const QUuid & _uuid, const QString & _title)
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

Vmc * VmcCollection::findVmc(const QUuid & _uuid) const
{
    for(Vmc * vmc : *mp_vmcs)
    {
        if(vmc->uuid() == _uuid)
            return vmc;
    }
    return nullptr;
}

void VmcCollection::deleteVmc(const QUuid & _uuid)
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
