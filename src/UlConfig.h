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

#include <QList>
#include <QDir>
#include <QSharedPointer>
#include "MediaType.h"

#define UL_CONFIG_FILENAME "ul.cfg"

struct UlConfigRecord
{
    UlConfigRecord() :
        type(MediaType::unknown),
        parts(0)
    {
    }

    QString name;
    QString image;
    MediaType type;
    quint8 parts;
};

class UlConfig final
{
private:
    explicit UlConfig(const QDir & _config_dir);
    void load();

public:
    static QSharedPointer<UlConfig> load(const QDir & _config_dir);
    inline const QString & directory() const;
    inline const QString & file() const;
    inline const QList<UlConfigRecord> records() const;
    void addRecord(const UlConfigRecord & _config);
    void deleteRecord(const QString _image);
    void renameRecord(const QString _image, const QString & _new_name);

private:
    UlConfigRecord * findRecord(const QString & _image);

private:
    QString m_config_directory;
    QString m_config_filepath;
    QList<UlConfigRecord> m_records;
};

const QString & UlConfig::directory() const
{
    return m_config_directory;
}

const QString & UlConfig::file() const
{
    return m_config_filepath;
}

const QList<UlConfigRecord> UlConfig::records() const
{
    return m_records;
}

#endif // __QPCOPL_ULCONFIG__
