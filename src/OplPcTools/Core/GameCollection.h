/***********************************************************************************************
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

#ifndef __OPLPCTOOLS_GAMECOLLECTION__
#define __OPLPCTOOLS_GAMECOLLECTION__

#include <QObject>
#include <QDir>
#include <OplPcTools/Core/Game.h>
#include <OplPcTools/Core/UlConfigGameStorage.h>
#include <OplPcTools/Core/DirectoryGameStorage.h>

namespace OplPcTools {
namespace Core {

class GameCollection final : public QObject
{
    Q_OBJECT

public:
    explicit GameCollection(QObject * _parent = nullptr);
    ~GameCollection() override;
    void load(const QDir & _directory);
    const QString & directory() const;
    const Game * findGame(const QString & _id) const;
    int count() const;
    const Game * operator [](int _index) const;
    void addGame(const Game & _game);
    bool renameGame(const Game & _game, const QString & _title);
    void deleteGame(const Game & _game);

signals:
    void loaded();
    void gameAboutToBeDeleted(const QString _game_id);
    void gameDeleted(const QString & _game_id);
    void gameAdded(const QString & _game_id);
    void gameRenamed(const QString & _game_id);

private:
    GameStorage & storage(GameInstallationType _installation_type) const;

private:
    QString m_directory;
    UlConfigGameStorage * mp_ul_conf_storage;
    DirectoryGameStorage * mp_dir_storage;
};

} // namespace Core
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMECOLLECTION__
