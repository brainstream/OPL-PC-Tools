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

#include <atomic>
#include <cstring>
#include <cmath>
#include <cdio/cdio.h>
#include <QObject>
#include <QRegExp>
#include "GameInstallerSource.h"
#include "Exception.h"
#include "IOException.h"

namespace {

const QString g_system_config_filename("SYSTEM.CNF;1");

} // namespace


void GameInstallerSource::initLibCDIO() const
{
    static std::atomic_bool cdio_initialized(false);
    static bool initialization_error = false;
    if(initialization_error)
    {
        libcdio_error:
        throw Exception(QObject::tr("libcdio library failed initialization"));
    }
    if(cdio_initialized.exchange(true))
        return;
    if(!cdio_init())
    {
        initialization_error = true;
        goto libcdio_error;
    }
}

QString GameInstallerSource::readGameId(CdioList_t * _root_dir) const
{
    CdioListNode_t * node;
    _CDIO_LIST_FOREACH(node, _root_dir)
    {
        iso9660_stat_t * statbuf = static_cast<iso9660_stat_t *>(_cdio_list_node_data(node));
        if(g_system_config_filename.compare(statbuf->filename, Qt::CaseInsensitive) != 0)
            continue;
        if(statbuf->size > 10000)
        {
            throw IOException(QObject::tr("%1 file is too long: %2 bytes").arg(g_system_config_filename).arg(statbuf->size));
        }
        long int blocks = static_cast<long int>(ceil(static_cast<double>(statbuf->size) / ISO_BLOCKSIZE));
        QByteArray buffer = read(statbuf->lsn, blocks);
        QRegExp regexp("BOOT\\d*\\s*=\\s*cdrom0:\\\\(.*);1", Qt::CaseInsensitive);
        QString data_string = QString::fromUtf8(buffer);
        if(regexp.indexIn(data_string) >= 0)
        {
            QString result = regexp.cap(1);
            if(result.isEmpty())
                break;
            return result;
        }
        break;
    }
    throw Exception(QObject::tr("Unable to read the game id"));
}
