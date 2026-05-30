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

#include <OplPcTools/IsoRestorer.h>
#include <OplPcTools/File.h>

using namespace OplPcTools;

IsoRestorer::IsoRestorer(QSharedPointer<DeviceReader> _reader, const QString & _iso_filepath, QObject * _parent) :
    QObject(_parent),
    m_reader_ptr(_reader),
    m_iso_filepath(_iso_filepath)
{
}

bool IsoRestorer::restore()
{
    QFile iso(m_iso_filepath);
    openFile(iso, QIODevice::WriteOnly | QIODevice::Truncate);

    const quint64 target_iso_size = m_reader_ptr->size();
    quint64 total_write_bytes = 0;
    const qint64 batch_size = 2048 * 2048;
    QByteArray buffer(batch_size, Qt::Uninitialized);
    m_reader_ptr->seek(0);
    for(int i = 0; ; ++i)
    {
        qint64 read_bytes = 0;
        try
        {
            read_bytes = m_reader_ptr->read(buffer);
        }
        catch(...)
        {
            rollback();
            throw;
        }
        if(read_bytes > 0)
        {
            try
            {
                total_write_bytes += writeFile(iso, buffer.constData(), read_bytes);
            }
            catch(...)
            {
                rollback();
                throw;
            }
            if(i % 5 == 0 || total_write_bytes == target_iso_size)
            {
                iso.flush();
            }
            emit progress(target_iso_size, total_write_bytes);
        }
        if(read_bytes == 0 || read_bytes < batch_size)
        {
            break;
        }
    }
    return true;
}

void IsoRestorer::rollback()
{
    emit rollbackStarted();
    QFile::remove(m_iso_filepath);
    emit rollbackFinished();
}
