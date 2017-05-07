/***********************************************************************************************
 *                                                                                             *
 * This file is part of the qpcopl project, the graphical PC tools for Open PS2 Loader.        *
 *                                                                                             *
 * qpcopl is free software: you can redistribute it and/or modify it under the terms of        *
 * the GNU General Public License as published by the Free Software Foundation,                *
 * either version 3 of the License, or (at your option) any later version.                     *
 *                                                                                             *
 * qpcopl is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;        *
 * without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  *
 * See the GNU General Public License for more details.                                        *
 *                                                                                             *
 * You should have received a copy of the GNU General Public License along with MailUnit.      *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/


#ifndef __QPCOPL_GAMEREPOSITORY__
#define __QPCOPL_GAMEREPOSITORY__

#include <functional>
#include <QObject>
#include <QDir>
#include <QLinkedList>

#include "Game.h"
#include "ValidationException.h"

class GameRepository : public QObject
{
    Q_OBJECT

public:
    explicit GameRepository(QObject * _parent = nullptr);
    void reloadFromUlConfig(const QDir & _config_dir);
    inline const QString & directory() const;
    inline const QString & file() const;
    inline const QLinkedList<Game> & games() const;
    void addGame(const Game & _game);
    void deleteGame(const QString & _id);
    void renameGame(const QString & _id, const QString & _new_name);
    void setGameCover(const QString _id, QString & _filepath);
    void removeGameCover(const QString _id);
    const Game * game(const QString & _id) const;

signals:
    void gameAdded(const QString & _id);
    void gameRenamed(const QString & _id);
    void gameDeleted(const QString & _id);

private:
    void loadCovers();
    void renameGameConfig(Game & _game, const QString & _new_name);
    void renameGameFiles(Game & _game, const QString & _new_name);
    void deleteGameConfig(const QString _id);
    void deleteGameFiles(Game & _game);
    Game & findGame(const QString & _id);

private:
    QString m_config_directory;
    QString m_config_filepath;
    QLinkedList<Game> m_games;
};

const QString & GameRepository::directory() const
{
    return m_config_directory;
}

const QString & GameRepository::file() const
{
    return m_config_filepath;
}

const QLinkedList<Game> & GameRepository::games() const
{
    return m_games;
}


inline void validateGameName(const QString & _name)
{
    if(_name.toUtf8().size() > MAX_GAME_NAME_LENGTH)
        throw ValidationException(QObject::tr("Maximum name length is %1 bytes").arg(MAX_GAME_NAME_LENGTH));
}

inline void validateGameId(const QString & _id)
{
    if(_id.toLatin1().size() > MAX_GAME_ID_LENGTH)
        throw ValidationException(QObject::tr("Maximum image name length is %1 bytes").arg(MAX_GAME_ID_LENGTH));
}

QString makeGamePartName(const QString & _id, const QString & _name, quint8 _part);

#endif // __QPCOPL_GAMEREPOSITORY__
