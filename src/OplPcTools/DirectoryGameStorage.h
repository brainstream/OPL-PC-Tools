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

namespace OplPcTools {

class DirectoryGameStorage final : public GameStorage
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
    GameInstallationType installationType() const override;

    static QString makeIsoFilename(const QString & _title, const QString & _id, const QString _ext);
    static QString makeIsoFilename(const QString & _title, const QString _ext);
    static QString makeGameIsoFilename(const QString & _title, const QString & _id, const QString _ext);
    static QString getFilenameExtension(const Game & _game);
    static std::optional<FindIsoResult> findIsoFile(const QString & _storage_directory, const Game & _game);

protected:
    bool performLoading(const QDir & _directory) override;
    bool performRenaming(const Game & _game, const QString & _title) override;
    bool performRegistration(const Game & _game) override;
    bool performDeletion(const Game & _game) override;

private:
    void loadDirectory(MediaType _media_type);

private:
    QString m_base_directory;
};

} // namespace OplPcTools
