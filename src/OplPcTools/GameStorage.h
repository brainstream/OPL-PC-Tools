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

#ifndef __OPLPCTOOLS_GAMESTORAGE__
#define __OPLPCTOOLS_GAMESTORAGE__

#include <QDir>
#include <QVector>
#include <QObject>
#include <OplPcTools/Game.h>

namespace OplPcTools {

class GameStorage : public QObject
{
    Q_OBJECT

public:
    explicit GameStorage(QObject * _parent = nullptr);
    virtual ~GameStorage();
    const Game * operator [](int _index) const;
    const Game * findGame(const Uuid & _uuid) const;
    bool load(const QDir & _directory);
    int count() const;
    bool renameGame(const Uuid & _uuid, const QString & _title);
    bool renameGame(const int _index, const QString & _title);
    bool registerGame(const Game & _game);
    bool deleteGame(const Uuid & _uuid);

    virtual GameInstallationType installationType() const = 0;

    static void validateId(const QString & _id);

public:
    static const quint16 max_id_length   = 15;

signals:
    void loaded();
    void gameRegistered(const Uuid & _uuid);
    void gameRenamed(const Uuid & _uuid);
    void gameAboutToBeDeleted(const Uuid & _uuid);
    void gameDeleted(const Uuid & _uuid);


protected:
    Game * createGame(const QString & _game_id);
    Game * findNonConstGame(const Uuid & _uuid) const;
    Game * gameAt(int _index) const;

    virtual bool performLoading(const QDir & _directory) = 0;
    virtual bool performRenaming(const Game & _game, const QString & _title) = 0;
    virtual bool performRegistration(const Game & _game) = 0;
    virtual bool performDeletion(const Game & _game) = 0;

private:
    void clear();
    bool renameGame(Game * _game, const QString & _title);

private:
    QVector<Game *> m_games;
};

} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMESTORAGE__
