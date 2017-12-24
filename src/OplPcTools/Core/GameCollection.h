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
#include <QVector>
#include <OplPcTools/Core/Game.h>

namespace OplPcTools {
namespace Core {

class GameCollection final : public QObject
{
    Q_OBJECT

public:
    explicit GameCollection(QObject * _parent = nullptr);
    ~GameCollection() override;
    inline const QString & directory() const;
    void load(const QString & _directory);
    const Game * findGame(const QString & _id) const;
    Game * findGame(const QString & _id);
    inline int count() const;
    inline const Game & operator [](int _index) const;
    inline Game & operator [](int _index);

signals:
    void loaded();
    void gameDeleted(const QString & _game_id);
    void gameAdded(const QString & _game_id);
    void gameChanged(const QString & _game_id);

private:
    QString m_directory;
    QVector<Game *> m_games;
};

const QString & GameCollection::directory() const
{
    return m_directory;
}

int GameCollection::count() const
{
    return m_games.count();
}

const Game & GameCollection::operator [](int _index) const
{
    return *m_games[_index];
}

Game & GameCollection::operator [](int _index)
{
    return *m_games[_index];
}

} // namespace Core
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMECOLLECTION__
