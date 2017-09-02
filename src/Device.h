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

#ifndef __OPLPCTOOLS_DEVICE__
#define __OPLPCTOOLS_DEVICE__

#include <QString>
#include <QList>
#include "BinaryFile.h"
#include "MediaType.h"

struct DeviceName
{
    QString name;
    QString filename;
};

QList<DeviceName> loadDriveList();

class Device
{   
    Q_DISABLE_COPY(Device)

public:
    explicit Device(const QString & _filepath);
    virtual ~Device();
    const QString & filepath() const;
    bool init();
    bool isInitialized() const;
    inline QString title() const;
    inline void setTitle(const QString _title);
    virtual quint64 size() const = 0;
    inline MediaType mediaType() const;
    inline void setMediaType(MediaType _media_type);
    inline const QString gameId() const;
    bool open();
    void close();
    bool isOpen() const;
    bool seek(quint64 _offset);
    qint64 read(QByteArray & _buffer);

protected:
    class Iso9660;
    virtual bool initialize(Iso9660 & _iso);

private:
    bool m_is_initialized;
    const QString m_filepath;
    BinaryFile * mp_read_file;
    MediaType m_media_type;
    QString m_id;
    QString m_title;
};

QString Device::title() const
{
    return m_title;
}

void Device::setTitle(const QString _title)
{
    m_title = _title;
}

const QString Device::gameId() const
{
    return m_id;
}

MediaType Device::mediaType() const
{
    return m_media_type;
}

void Device::setMediaType(MediaType _media_type)
{
    m_media_type = _media_type;
}



class Iso9660Image : public Device
{
public:
    Iso9660Image(const QString & _filepath);
    quint64 size() const override;

protected:
    bool initialize(Iso9660 & _iso) override;

private:
    quint64 m_size;
};



class OpticalDrive : public Device
{
public:
    OpticalDrive(const QString & _filepath);
    quint64 size() const override;

protected:
    bool initialize(Iso9660 & _iso) override;

private:
    quint64 m_size;
};

#endif // __OPLPCTOOLS_DEVICE__
