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

#ifndef __QPCOPL_ULCONFIG__
#define __QPCOPL_ULCONFIG__

#include <QString>
#include <QList>

#define UL_MAX_GAME_NAME_LENGTH 32
#define UL_MAX_IMAGE_NAME_LENGTH 15

enum class MediaType
{
    unknown,
    cd,
    dvd
};

namespace Ul {

struct ConfigRecord
{
    ConfigRecord() :
        type(MediaType::unknown),
        parts(0)
    {
    }

    QString name;
    QString image;
    MediaType type;
    quint8 parts;
};

QList<ConfigRecord> loadConfig(const QString & _filepath);
void addConfigRecord(const ConfigRecord & _config, const QString & _filepath);
void deleteConfigRecord(const QString _image, const QString & _filepath);
void renameConfigRecord(const QString _image, const QString & _new_name, const QString & _filepath);

} // namespace Ul

#endif // __QPCOPL_ULCONFIG__
