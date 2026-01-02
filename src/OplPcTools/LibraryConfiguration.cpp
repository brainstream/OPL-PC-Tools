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

#include <OplPcTools/LibraryConfiguration.h>
#include <OplPcTools/ApplicationInfo.h>

using namespace OplPcTools;

namespace {
namespace Key {

static const char vmc_section[] = "Vmc";
static const char vmc_fs_charset[] = "Charset";

} // namespace Key
} // namespace

LibraryConfiguration::LibraryConfiguration(const VmcCollection & _vmc_collection, QObject *_parent) :
    QObject(_parent),
    mr_vmc_collection(_vmc_collection),
    mp_settings(nullptr)
{
    connect(&_vmc_collection, &VmcCollection::vmcAboutToBeDeleted, this, &LibraryConfiguration::onVmcAboutToBeDeleted);
    connect(&_vmc_collection, &VmcCollection::vmcRenamed, this, &LibraryConfiguration::onVmcRenamed);
}

LibraryConfiguration::~LibraryConfiguration()
{
    delete mp_settings;
}

void LibraryConfiguration::load(const QDir & _library_dir)
{
    delete mp_settings;
    mp_settings = new QSettings(
        _library_dir.absoluteFilePath(QString(APPLICATION_NAME) + ".ini"),
        QSettings::IniFormat);
}

void LibraryConfiguration::onVmcAboutToBeDeleted(const Uuid & _uuid)
{
    if(const Vmc * vmc = mr_vmc_collection[_uuid])
        deleteConfigSection(makeVmcSectionName(vmc->title()));
}

void LibraryConfiguration::deleteConfigSection(const QString & _section)
{
    if(mp_settings)
    {
        mp_settings->beginGroup(_section);
        QStringList keys = mp_settings->childKeys();
        mp_settings->endGroup();
        if(!keys.empty())
        {
            mp_settings->remove(_section);
            mp_settings->sync();
        }
    }
}

QString LibraryConfiguration::makeVmcSectionName(const QString & _vmc_title) const
{
    return QString("%1_%2").arg(Key::vmc_section, _vmc_title);
}

void LibraryConfiguration::onVmcRenamed(const QString & _old_title, const Uuid & _uuid)
{
    if(const Vmc * vmc = mr_vmc_collection[_uuid])
        renameConfigSection(makeVmcSectionName(_old_title), makeVmcSectionName(vmc->title()));
}

void LibraryConfiguration::renameConfigSection(const QString & _old_section, const QString & _new_section)
{
    if(mp_settings)
    {
        mp_settings->beginGroup(_old_section);
        QStringList keys = mp_settings->childKeys();
        mp_settings->endGroup();
        if(keys.empty())
            return;
        foreach(const QString & key, keys)
        {
            mp_settings->setValue(
                QString("%1/%2").arg(_new_section, key),
                mp_settings->value(QString("%1/%2").arg(_old_section, key)));
        }
        mp_settings->remove(_old_section);
        mp_settings->sync();
    }
}

void LibraryConfiguration::setVmcFsCharset(const Vmc & _vmc, const QString & _charset)
{
    mp_settings->setValue(QString("%1/%2").arg(makeVmcSectionName(_vmc.title()), Key::vmc_fs_charset), _charset);
    mp_settings->sync();
}

QString LibraryConfiguration::vmcFsCharset(const Vmc & _vmc) const
{
    return mp_settings->value(QString("%1/%2").arg(makeVmcSectionName(_vmc.title()), Key::vmc_fs_charset)).toString();
}
