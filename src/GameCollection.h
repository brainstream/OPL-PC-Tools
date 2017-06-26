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

#include <QDir>
#include <QLinkedList>
#include "Game.h"

class GameCollection : public QObject
{
    Q_OBJECT

public:
    explicit GameCollection(QObject * _parent = nullptr);
    void reloadFromUlConfig(const QDir & _config_dir);
    inline const QString & directory() const;
    inline const QString & file() const;
    inline const QLinkedList<Game> & games() const;
    void addGame(const Game & _game);
    void deleteGame(const QString & _id);
    void renameGame(const QString & _id, const QString & _new_name);
    void setGameCover(const QString _id, QString & _filepath);
    void removeGameCover(const QString _id);
    void setGameIcon(const QString _id, QString & _filepath);
    void removeGameIcon(const QString _id);
    const Game * game(const QString & _id) const;

private:
    void loadPixmaps();
    void loadPixmap(QPixmap & _pixmap, const QString & _filepath);
    QString savePixmap(QPixmap & _pixmap, const QString & _filename);
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

const QString & GameCollection::directory() const
{
    return m_config_directory;
}

const QString & GameCollection::file() const
{
    return m_config_filepath;
}

const QLinkedList<Game> & GameCollection::games() const
{
    return m_games;
}

#endif // __OPLPCTOOLS_GAMECOLLECTION__
