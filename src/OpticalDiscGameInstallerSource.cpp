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

#include <string>
#include <cdio/cdio.h>
#include <cdio/iso9660.h>
#include <cdio/cd_types.h>
#include <QObject>
#include "OpticalDiscGameInstallerSource.h"
#include "IOException.h"

struct OpticalDiscGameInstallerSource::Data
{
    Data(const char * _device_name) :
        is_initialized(false),
        device(nullptr),
        device_name(_device_name),
        size(0),
        media_type(MediaType::unknown)
    {
    }

    bool is_initialized;
    CdIo_t * device;
    std::string device_name;
    QString game_id;
    QString label;
    size_t size;
    MediaType media_type;
};

OpticalDiscGameInstallerSource::OpticalDiscGameInstallerSource(const char * _device) :
    mp_data(new Data(_device))
{
}

OpticalDiscGameInstallerSource::~OpticalDiscGameInstallerSource()
{
    cdio_destroy(mp_data->device);
    delete mp_data;
}

void OpticalDiscGameInstallerSource::init() const
{
    if(mp_data->is_initialized)
    {
        if(mp_data == nullptr)
            throw IOException(QObject::tr("Optical drive device was not opened successfully"));
        return;
    }
    mp_data->is_initialized = true;
    initLibCDIO();
    mp_data->device = cdio_open_cd(mp_data->device_name.c_str());
    cdio_close_tray(mp_data->device_name.c_str(), nullptr);
    cdio_get_media_changed(mp_data->device);
    cdio_set_speed(mp_data->device, 128);
    initGameId();
    initLabel();
    initMediaType();
}

void OpticalDiscGameInstallerSource::initGameId() const
{
    if(mp_data->device == nullptr)
        return;
    CdioList_t * dirlist = iso9660_fs_readdir(mp_data->device, "/", false);
    if(dirlist == nullptr)
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

void OpticalDiscGameInstallerSource::initLabel() const
{
    if(mp_data->device == nullptr)
        return;
    lsn_t session = 0;
    if(cdio_get_last_session(mp_data->device, &session) != DRIVER_OP_SUCCESS)
        return;
    track_t track = cdio_get_first_track_num(mp_data->device);
    if(track == CDIO_INVALID_TRACK)
        return;
    cdio_iso_analysis_t analysis = {};
    cdio_guess_cd_type(mp_data->device, session, track, &analysis);
    mp_data->label = analysis.iso_label;
    mp_data->size = analysis.isofs_size * ISO_BLOCKSIZE;
    if(mp_data->label.isEmpty())
        mp_data->label = QString::fromStdString(mp_data->device_name);
}

void OpticalDiscGameInstallerSource::initMediaType() const
{
    discmode_t mode = cdio_get_discmode(mp_data->device);
    if(cdio_is_discmode_dvd(mode))
        mp_data->media_type = MediaType::dvd;
    else if(cdio_is_discmode_cdrom(mode))
        mp_data->media_type = MediaType::cd;
}


QString OpticalDiscGameInstallerSource::gameId() const
{
    init();
    return mp_data->game_id;
}

size_t OpticalDiscGameInstallerSource::read(QByteArray & _buffer)
{
    init();
    if(cdio_get_media_changed(mp_data->device))
        throw IOException(QObject::tr("Optical disc has changed since source was initialized"));
    ssize_t result = cdio_read(mp_data->device, _buffer.data(), _buffer.size());
    return result == static_cast<ssize_t>(-1) ? 0 : result;
}

QByteArray OpticalDiscGameInstallerSource::read(lsn_t _lsn, quint32 _blocks) const
{
    init();
    size_t buffer_size = _blocks * ISO_BLOCKSIZE + 1;
    QByteArray buffer(buffer_size, Qt::Initialization::Uninitialized);
    cdio_read_data_sectors(mp_data->device, buffer.data(), _lsn, ISO_BLOCKSIZE, _blocks);
    return buffer;
}

quint64 OpticalDiscGameInstallerSource::size() const
{
    init();
    return mp_data->size;
}

MediaType OpticalDiscGameInstallerSource::type() const
{
    return mp_data->media_type;
}

