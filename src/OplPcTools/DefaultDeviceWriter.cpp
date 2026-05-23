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

#include <OplPcTools/DefaultDeviceWriter.h>
#include <OplPcTools/DeviceReader.h>
#include <OplPcTools/File.h>
#include <OplPcTools/Exception.h>
#include <QThread>

using namespace OplPcTools;

DefaultDeviceWriter::DefaultDeviceWriter(QObject * _parent) :
    DeviceWriter(_parent)
{
}

bool DefaultDeviceWriter::write(DeviceReader & _reader, const QString & _destination)
{
    const qsizetype read_size = 4194304;
    QByteArray bytes(read_size, Qt::Initialization::Uninitialized);
    QSharedPointer<QFile> dest = createFileToSyncWrite(_destination);
    const quint64 iso_size = _reader.size();
    _reader.seek(0);
    for(quint64 total_read_bytes = 0, write_operation = 0; total_read_bytes < iso_size; ++write_operation)
    {
        qint64 read_bytes = _reader.read(bytes);
        if(read_bytes < 0)
        {
            dest->close();
            QFile::remove(_destination);
            throw IOException(tr("An error occurred during reading the source medium"));
        }
        else if(read_bytes > 0)
        {
            // Some formats have data behind the end (hello, NRG)
            const qint64 to_write = std::min(static_cast<qint64>(iso_size - total_read_bytes), read_bytes);
            if(dest->write(bytes.constData(), to_write) != to_write)
            {
                dest->close();
                QFile::remove(_destination);
                throw IOException(tr("Unable to write a data into the file: \"%1\"").arg(dest->fileName()));
            }
            if(++write_operation % 5 == 0)
                dest->flush();
            total_read_bytes += read_bytes;
            emit progress(iso_size, total_read_bytes);
        }
        if(read_bytes < read_size)
        {
            emit progress(iso_size, iso_size);
            break;
        }
        if(QThread::currentThread()->isInterruptionRequested())
        {
            dest->close();
            QFile::remove(_destination);
            return false;
        }
    }
    return true;
}
