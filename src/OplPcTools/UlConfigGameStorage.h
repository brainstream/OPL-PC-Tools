/***********************************************************************************************
 * Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                   *
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

#ifndef __OPLPCTOOLS_ULCONFIGGAMESTORAGE__
#define __OPLPCTOOLS_ULCONFIGGAMESTORAGE__

#include <OplPcTools/GameStorage.h>

namespace OplPcTools {

class UlConfigGameStorage final : public GameStorage
{
    Q_OBJECT

public:
    explicit UlConfigGameStorage(QObject * _parent = nullptr);
    GameInstallationType installationType() const override;

    static void validateTitle(const QString & _title);
    static QString makePartFilename(const QString & _id, const QString & _name, quint8 _part);

public:
    static const quint16 max_name_length = 32;

protected:
    bool performLoading(const QDir & _directory) override;
    bool performRenaming(const Game & _game, const QString & _title) override;
    bool performRegistration(const Game & _game) override;
    bool performDeletion(const Game & _game) override;

private:
    void deleteGameConfig(const QString _id);
    void deletePartFiles(const Game & _game);

private:
    QString m_config_filepath;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_ULCONFIGGAMESTORAGE__
