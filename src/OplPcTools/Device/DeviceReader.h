/***********************************************************************************************
 * Copyright © 2017-2026 Sergey Smolyannikov aka brainstream                                   *
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

#pragma once

#include <QString>
#include <QList>
#include <QSharedPointer>
#include <OplPcTools/Device/DeviceSource.h>

namespace OplPcTools {

struct DeviceName
{
    QString name;
    QString filename;
};

QList<DeviceName> loadDriveList();

class DeviceReader final
{
    Q_DISABLE_COPY(DeviceReader)

public:
    explicit DeviceReader(QSharedPointer<DeviceSource> _source);
    const QString filepath() const;
    bool open();
    void close();
    bool isOpen() const;
    inline bool isCompressed() const;
    inline QString title() const;
    inline void setTitle(const QString _title);
    inline quint64 size() const;
    inline const QString & gameId() const;
    bool seek(quint64 _offset);
    qint64 read(QByteArray & _buffer);

private:
    bool m_is_initialized;
    QSharedPointer<DeviceSource> m_source_ptr;
    QString m_id;
    QString m_title;
    quint64 m_size;
};

bool DeviceReader::isCompressed() const
{
    return m_source_ptr && m_source_ptr->isCompressed();
}

QString DeviceReader::title() const
{
    return m_title;
}

void DeviceReader::setTitle(const QString _title)
{
    m_title = _title;
}

quint64 DeviceReader::size() const
{
    return m_size;
}

const QString & DeviceReader::gameId() const
{
    return m_id;
}

} // namespace OplPcTools
