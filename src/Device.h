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

#ifndef __QPCOPL_CDIO_DEVICE__
#define __QPCOPL_CDIO_DEVICE__

#include <QString>
#include <QList>
#include <QFile>
#include "MediaType.h"

struct DeviceName
{
    QString name;
    QString filename;
};

QList<DeviceName> loadDriveList();

MediaType getMediaType(const QString & _device_filename);

class Device
{   
    Q_DISABLE_COPY(Device)

protected:
    class Iso9660Data;

public:
    explicit Device(const QString & _filepath);
    virtual ~Device();
    const QString & filepath() const;
    bool init();
    bool isInitialized() const;
    bool isPlayStationDisc() const;
    QString title() const;
    virtual size_t size() const = 0;
    virtual MediaType mediaType() const = 0;
    const QString gameId() const;
    bool open();
    void close();
    bool isOpen() const;
    bool seek(quint64 _offset);
    quint64 read(char * _buffer, quint64 _max_length);

protected:
    const Iso9660Data * iso9660() const;
    virtual bool initialize();

private:
    bool m_is_initialized;
    const QString m_filepath;
    Iso9660Data * mp_iso9660;
    QFile * mp_read_file;
};

class OpticalDrive : public Device
{
public:
    OpticalDrive(const QString & _filepath);
    size_t size() const override;
    MediaType mediaType() const override;

protected:
    bool initialize() override;

private:
    MediaType m_media_type;
};

class Iso9660Image : public Device
{
public:
    Iso9660Image(const QString & _filepath);
    size_t size() const override;
    MediaType mediaType() const override;
    void setMediaType(MediaType _media_type);

protected:
    bool initialize() override;

private:
    size_t m_size;
    MediaType m_media_type;
};

#endif // __QPCOPL_CDIO_DEVICE__
