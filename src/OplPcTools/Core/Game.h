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

#ifndef __OPLPCTOOLS_GAME__
#define __OPLPCTOOLS_GAME__

#include <QString>
#include <QObject>

namespace OplPcTools {
namespace Core {

class Game final
{
public:
    explicit inline Game(const QString & _id);
    Game(const Game &) = default;
    Game & operator = (const Game &) = default;
    inline const QString & id() const;
    inline void setTitle(const QString & _title);
    inline const QString & title() const;

private:
    QString m_id;
    QString m_title;
};

Game::Game(const QString & _id) :
    m_id(_id)
{
    m_title = QObject::tr("<Untitled>");
}

const QString & Game::id() const
{
    return m_id;
}

void Game::setTitle(const QString & _title)
{
    if(m_title != _title)
        m_title = _title;
}

const QString & Game::title() const
{
    return m_title;
}

} // namespace Core
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAME__
