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

#include <OplPcTools/GameStorage.h>
#include <OplPcTools/Constants.h>
#include <OplPcTools/Device/DeviceSource.h>

namespace OplPcTools {

class DirectoryGameStorage : public GameStorage
{
    Q_OBJECT

public:
    struct FindIsoResult
    {
        QString path;
        bool is_name_included_id;
    };

public:
    explicit DirectoryGameStorage(QObject * _parent = nullptr);
    static QString makeIsoFilename(const QString & _title, const QString & _id, const QString _ext);
    static QString makeIsoFilename(const QString & _title, const QString _ext);
    static QString makeGameIsoFilename(const QString & _title, const QString & _id, const QString _ext);

protected:
    virtual const char * filenameExtenstion() const = 0;
    virtual const char * filenamePattern() const = 0;
    virtual DeviceSource * newDeviceSource(const QString & _filepath) const = 0;
    bool performLoading(const QDir & _directory) override;
    bool performRenaming(const Game & _game, const QString & _title) override;
    bool performRegistration(const Game & _game) override;
    bool performDeletion(const Game & _game) override;
    static std::optional<FindIsoResult> findIsoFile(
        const Game & _game,
        const QString & _base_directory,
        const QString & _filename_extenstion);

private:
    std::optional<FindIsoResult> findIsoFile(const Game & _game) const;
    void loadDirectory(MediaType _media_type);

private:
    QString m_base_directory;
};

class Iso9660GameStorage final : public DirectoryGameStorage
{
    Q_OBJECT

public:
    explicit Iso9660GameStorage(QObject * _parent = nullptr);
    GameInstallationType installationType() const override;
    static std::optional<FindIsoResult> findIsoFile(const Game & _game, const QString & _base_directory);

protected:
    const char * filenameExtenstion() const override;
    const char * filenamePattern() const override;
    DeviceSource * newDeviceSource(const QString & _filepath) const override;

private:
    static const char * s_filename_extenstion;
};

class ZisoGameStorage final : public DirectoryGameStorage
{
    Q_OBJECT

public:
    explicit ZisoGameStorage(QObject * _parent = nullptr);
    GameInstallationType installationType() const override;
    static std::optional<FindIsoResult> findIsoFile(const Game & _game, const QString & _base_directory);

protected:
    const char * filenameExtenstion() const override;
    const char * filenamePattern() const override;
    DeviceSource * newDeviceSource(const QString & _filepath) const override;

private:
    static const char * s_filename_extenstion;
};

} // namespace OplPcTools
