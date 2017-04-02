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

#include <QRegExp>
#include <atomic>
#include <cstring>
#include <cmath>
#include <cdio/cdio.h>
#include <cdio/iso9660.h>
#include "IOException.h"
#include "ValidationException.h"
#include "Iso9660GameInstallerSource.h"

namespace {

void initLibCDIO()
{
    static std::atomic_bool cdio_initialized(false);
    static bool initialization_error = false;
    if(initialization_error)
        throw Exception(QObject::tr("libcdio library failed initialization"));
    if(cdio_initialized.exchange(true))
        return;
    if(!cdio_init())
    {
        initialization_error = false;
        initLibCDIO();
    }
}

} // namespace

Iso9660GameInstallerSource::Iso9660GameInstallerSource(const QString & _iso_path) :
    m_file(_iso_path),
    m_type(MediaType::unknown)
{
}

QString Iso9660GameInstallerSource::gameId() const
{
    initLibCDIO();
    const QString config_filename = "SYSTEM.CNF;1";
    iso9660_t * iso = iso9660_open(m_file.fileName().toUtf8().constData());
    if(iso == nullptr)
        throw IOException(QObject::tr("Unable to open ISO file \"%1\"").arg(m_file.fileName()));
    CdioList_t * dirlist = iso9660_ifs_readdir(iso, "/");
    if(!dirlist)
    {
        iso9660_close(iso);
        throw IOException(QObject::tr("Unable to read ISO file \"%1\"").arg(m_file.fileName()));
    }
    CdioListNode_t * node;
    QString result;
    _CDIO_LIST_FOREACH(node, dirlist)
    {
        iso9660_stat_t * statbuf = static_cast<iso9660_stat_t *>(_cdio_list_node_data(node));
        if(config_filename.compare(statbuf->filename, Qt::CaseInsensitive) != 0)
            continue;
        if(statbuf->size > 10000)
        {
            _cdio_list_free(dirlist, true);
            iso9660_close(iso);
            throw IOException(QObject::tr("%1 file is too long: %2 bytes").arg(config_filename).arg(statbuf->size));
        }
        long int blocks = static_cast<long int>(ceil(static_cast<double>(statbuf->size) / ISO_BLOCKSIZE));
        size_t buffer_size = blocks * ISO_BLOCKSIZE + 1;
        char * buffer = new char[buffer_size];
        memset(buffer, 0, buffer_size);
        iso9660_iso_seek_read(iso, buffer, statbuf->lsn, blocks);
        QRegExp regexp("BOOT\\d*\\s*=\\s*cdrom0:\\\\(.*);1", Qt::CaseInsensitive);
        if(regexp.indexIn(buffer) >= 0)
            result = regexp.cap(1);
        delete [] buffer;
        break;
    }
    _cdio_list_free(dirlist, true);
    iso9660_close(iso);
    if(result.isEmpty())
        throw ValidationException(QObject::tr("%1 file has invalid format").arg(config_filename));
    return result;
}

QByteArray Iso9660GameInstallerSource::read(quint64 _length)
{
    if(!m_file.isOpen())
    {
        if(!m_file.open(QIODevice::ReadOnly))
            throw IOException(QObject::tr("Unable to reade file \"%1\"").arg(m_file.fileName()));
    }
    return m_file.read(_length);
}

quint64 Iso9660GameInstallerSource::size() const
{
    return m_file.size();
}

void Iso9660GameInstallerSource::setType(MediaType _type)
{
    m_type = _type;
}

MediaType Iso9660GameInstallerSource::type() const
{
    return m_type;
}
