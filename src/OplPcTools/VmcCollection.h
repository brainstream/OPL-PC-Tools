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

#ifndef __OPLPCTOOLS_VMCCOLLECTION__
#define __OPLPCTOOLS_VMCCOLLECTION__

#include <QDir>
#include <QObject>
#include <OplPcTools/Vmc.h>

namespace OplPcTools {


class VmcCollection final : public QObject
{
    Q_OBJECT

public:
    explicit VmcCollection(QObject * _parent = nullptr);
    ~VmcCollection() override;
    const Vmc * operator[](int _index) const;
    const Vmc * operator[](const Uuid & _uuid) const;
    bool load(const QDir & _base_directory);
    bool isLoaded() const;
    const int count() const;
    const Vmc * createVmc(const QString & _title, uint32_t _size_mib);
    void renameVmc(const Uuid & _uuid, const QString & _title);
    void deleteVmc(const Uuid & _uuid);

signals:
    void vmcAdded(const Uuid & _uuid);
    void vmcRenamed(const Uuid & _uuid);
    void vmcAboutToBeDeleted(const Uuid & _uuid);
    void vmcDeleted(const Uuid & _uuid);

private:
    void ensureDirectoryExists();
    QString makeFilename(const QString & _vmc_title) const;
    Vmc * findVmc(const Uuid & _uuid) const;

private:
    QDir m_directory;
    QVector<Vmc *> * mp_vmcs;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_VMCCOLLECTION__
