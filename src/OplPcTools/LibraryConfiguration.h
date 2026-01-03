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

#pragma once

#include <OplPcTools/VmcCollection.h>
#include <OplPcTools/GameConfiguration.h>
#include <QDir>
#include <QSettings>

namespace OplPcTools {

class LibraryConfiguration final : public QObject
{
    Q_OBJECT

public:
    LibraryConfiguration(const VmcCollection & _vmc_collection, QObject * _parent);
    ~LibraryConfiguration() override;
    void load(const QDir & _library_dir);
    void setVmcFsCharset(const Vmc & _vmc, const QString & _charset);
    QString vmcFsCharset(const Vmc & _vmc) const;
    void setOplVersion(GameConfigurationVersion _version);
    GameConfigurationVersion oplVersion() const;

private:
    void renameConfigSection(const QString & _old_section, const QString & _new_section);
    void deleteConfigSection(const QString & _section);
    void onVmcAboutToBeDeleted(const Uuid & _uuid);
    void onVmcRenamed(const QString & _old_title, const Uuid & _uuid);

private:
    const VmcCollection & mr_vmc_collection;
    QSettings * mp_settings;
};

} // namespace OplPcTools
