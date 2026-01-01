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

#ifndef __OPLPCTOOLS_GAMEMCOLLECTION__
#define __OPLPCTOOLS_GAMEMCOLLECTION__

#include <QObject>
#include <QDir>
#include <OplPcTools/Game.h>
#include <OplPcTools/UlConfigGameStorage.h>
#include <OplPcTools/DirectoryGameStorage.h>

namespace OplPcTools {

class GameCollection final : public QObject
{
    Q_OBJECT

public:
    explicit GameCollection(QObject * _parent = nullptr);
    ~GameCollection() override;
    void load(const QDir & _directory);
    bool isLoaded() const;
    const QString directory() const { return m_directory; }
    const Game * findGame(const Uuid & _uuid) const;
    int count() const;
    const Game * operator [](int _index) const;
    void addGame(const Game & _game);
    void renameGame(const Game & _game, const QString & _title);
    void deleteGame(const Game & _game);
    bool contains(const QString & _game_id) const;

signals:
    void gameAboutToBeDeleted(const OplPcTools::Uuid & _uuid);
    void gameDeleted(const OplPcTools::Uuid & _uuid);
    void gameAdded(const OplPcTools::Uuid & _uuid);
    void gameRenamed(const OplPcTools::Uuid & _uuid);

private:
    GameStorage & storage(GameInstallationType _installation_type) const;

private:
    QString m_directory;
    UlConfigGameStorage * mp_ul_conf_storage;
    DirectoryGameStorage * mp_dir_storage;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMEMCOLLECTION__
