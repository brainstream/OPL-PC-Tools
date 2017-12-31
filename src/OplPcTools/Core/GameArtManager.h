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

#ifndef __OPLPCTOOLS_GAMEARTMANAGER__
#define __OPLPCTOOLS_GAMEARTMANAGER__

#include <set>
#include <QDir>
#include <QPixmap>
#include <QMap>

namespace OplPcTools {
namespace Core {

enum class GameArtType
{
    Icon,
    Front,
    Back,
    Spine,
    Screenshot1,
    Screenshot2,
    Background
};

class GameArtManager final
{
    Q_DISABLE_COPY(GameArtManager)

public:
    explicit GameArtManager(const QDir & _base_directory);
    ~GameArtManager();
    void addCacheType(GameArtType _type);
    void removeCacheType(GameArtType _type, bool _clear_cache);
    QPixmap load(const QString & _game_id, GameArtType _type);

private:
    QString m_directory_path;
    QMap<QString, QMap<GameArtType, QPixmap *>> m_cache;
    std::set<GameArtType> m_cached_types;
};

} // namespace Core
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMEARTMANAGER__
