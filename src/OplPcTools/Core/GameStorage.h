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

#ifndef __OPLPCTOOLS_GAMESTORAGE__
#define __OPLPCTOOLS_GAMESTORAGE__

#include <QDir>
#include <QVector>
#include <QObject>
#include <OplPcTools/Core/Game.h>

namespace OplPcTools {
namespace Core {

class GameStorage : public QObject
{
    Q_OBJECT

public:
    explicit GameStorage(QObject * _parent = nullptr);
    virtual ~GameStorage();
    const Game * operator [](int _index) const;
    int count() const;
    const Game * findGame(const QString & _id) const;

    virtual bool load(const QDir & _directory) = 0;
    virtual bool renameGame(const QString & _id, const QString & _title) = 0;
    virtual bool renameGame(const int _index, const QString & _title) = 0;
    virtual bool registerGame(const Game & _game) = 0;

signals:
    void gameRegistered(const QString & _game_id);

protected:
    void clear();
    Game * createGame(const QString & _id);
    Game * findNonConstGame(const QString & _id) const;
    Game * gameAt(int _index) const;

private:
    QVector<Game *> m_games;
};

} // namespace Core
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMESTORAGE__
