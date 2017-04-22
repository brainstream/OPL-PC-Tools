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

#include "LibCDIO.h"
#include <QFile>
#include "IOException.h"
#include "ValidationException.h"
#include "Iso9660GameInstallerSource.h"
#include "LibCDIO.h"

struct Iso9660GameInstallerSource::Data
{
    Data(const QString & _iso_path) :
        is_initialized(false),
        iso_file(_iso_path),
        iso(nullptr),
        type(MediaType::unknown)
    {
    }

    bool is_initialized;
    QFile iso_file;
    iso9660_t * iso;
    MediaType type;
    QString game_id;
};

Iso9660GameInstallerSource::Iso9660GameInstallerSource(const QString & _iso_path) :
    mp_data(new Data(_iso_path))
{
}

Iso9660GameInstallerSource::~Iso9660GameInstallerSource()
{
    iso9660_close(mp_data->iso);
    delete mp_data;
}

void Iso9660GameInstallerSource::init() const
{
    if(mp_data->is_initialized)
    {
        if(mp_data->iso == nullptr)
            throw IOException(QObject::tr("ISO file was not opened successfully"));
        return;
    }
    mp_data->is_initialized = true;
    initLibCDIO();
    mp_data->iso = iso9660_open(mp_data->iso_file.fileName().toUtf8().constData());
    initGameId();
}

void Iso9660GameInstallerSource::initGameId() const
{
    if(mp_data->iso == nullptr)
        return;
    CdioList_t * dirlist = iso9660_ifs_readdir(mp_data->iso, "/");
    if(!dirlist)
        return;
    try
    {
        mp_data->game_id = readGameId(dirlist);
    }
    catch(...)
    {
        _cdio_list_free(dirlist, true);
        throw;
    }
    _cdio_list_free(dirlist, true);
}

QString Iso9660GameInstallerSource::gameId() const
{
    init();
    return mp_data->game_id;
}

void Iso9660GameInstallerSource::seek(quint64 _offset)
{
    if(mp_data->iso_file.isOpen())
        mp_data->iso_file.seek(_offset);
}

ssize_t Iso9660GameInstallerSource::read(QByteArray & _buffer)
{
    if(!mp_data->iso_file.isOpen())
    {
        if(!mp_data->iso_file.open(QIODevice::ReadOnly))
            throw IOException(QObject::tr("Unable to reade file \"%1\"").arg(mp_data->iso_file.fileName()));
    }
    return static_cast<ssize_t>(mp_data->iso_file.read(_buffer.data(), _buffer.size()));
}

QByteArray Iso9660GameInstallerSource::read(lsn_t _lsn, quint32 _blocks) const
{
    init();
    size_t buffer_size = _blocks * ISO_BLOCKSIZE + 1;
    QByteArray buffer(buffer_size, Qt::Initialization::Uninitialized);
    iso9660_iso_seek_read(mp_data->iso, buffer.data(), _lsn, _blocks);
    return buffer;
}

quint64 Iso9660GameInstallerSource::size() const
{
    return mp_data->iso_file.size();
}

void Iso9660GameInstallerSource::setType(MediaType _type)
{
    mp_data->type = _type;
}

MediaType Iso9660GameInstallerSource::type() const
{
    return mp_data->type;
}
