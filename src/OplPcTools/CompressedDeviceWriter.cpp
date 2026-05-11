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

#include <OplPcTools/CompressedDeviceWriter.h>
#include <OplPcTools/Zso.h>
#include <OplPcTools/File.h>
#include <QThread>

static_assert(Q_BYTE_ORDER == Q_LITTLE_ENDIAN, "CompressedDeviceWriter: only little-endian hosts (ZSO file is LE)");

using namespace OplPcTools;

CompressedDeviceWriter::CompressedDeviceWriter(QObject * _parent) :
    DeviceWriter(_parent)
{
}

bool CompressedDeviceWriter::write(DeviceReader & _reader, const QString & _destination)
{
    const quint32 zso_block_size = 2048;
    const float accaptable_ratio = .95;

    QByteArray src_buffer(zso_block_size, Qt::Initialization::Uninitialized);
    QByteArray compress_buffer(zso_block_size, Qt::Initialization::Uninitialized);
    QByteArray cache_buffer(zso_block_size * 2000, Qt::Initialization::Uninitialized);
    qsizetype cache_buffer_position = 0;
    QSharedPointer<QFile> dest = createFileToSyncWrite(_destination);
    dest->seek(0);

    const ZsoHeader header
    {
        .magic = g_zso_magic,
        .header_size = sizeof(ZsoHeader),
        .total_bytes = _reader.size(),
        .block_size = zso_block_size,
        .version = g_zso_version_1,
        .align = 1,
        .reserved = { 0, 0 }
    };

    dest->write(reinterpret_cast<const char *>(&header), sizeof(ZsoHeader));
    quint32 index_count = header.total_bytes / header.block_size + 1;
    if(header.total_bytes % header.block_size)
        ++index_count;
    std::vector<qint32> index(index_count * sizeof(qint32), '\0');
    dest->write(reinterpret_cast<const char *>(index.data()), index_count * sizeof(qint32));
    dest->flush();

    _reader.seek(0);
    quint64 processed_bytes = 0;
    for(size_t i = 0; ; ++i)
    {
        if(cache_buffer.size() - cache_buffer_position < header.block_size)
        {
            dest->write(cache_buffer.constData(), cache_buffer_position);
            dest->flush();
            cache_buffer_position = 0;
            emit progress(header.total_bytes, processed_bytes);
        }

        const qint64 read_bytes = _reader.read(src_buffer);
        if(read_bytes < 0)
        {
            dest->close();
            QFile::remove(_destination);
            throw IOException(tr("An error occurred during reading the source medium"));
        }

        const int compressed_size = compressZsoBlock(src_buffer, compress_buffer);
        if(compressed_size < 0)
        {
            dest->close();
            QFile::remove(_destination);
            throw IOException(tr("An error occurred during compressing the source data"));
        }

        index[i] = static_cast<qint32>(dest->pos() + cache_buffer_position) >> 1;

        if(
            compressed_size > 0 &&
            compressed_size < read_bytes &&
            static_cast<float>(compressed_size) / read_bytes < accaptable_ratio)
        {
            index[i] &= 0b01111111'11111111'11111111'11111111;
            cache_buffer.replace(cache_buffer_position, compressed_size, compress_buffer.constData(), compressed_size);
            cache_buffer_position += compressed_size;
            if(compressed_size % 2)
            {
                // alignement
                cache_buffer[cache_buffer_position] = 0;
                ++cache_buffer_position;
            }
        }
        else
        {
            index[i] |= 0b10000000'00000000'00000000'00000000;
            cache_buffer.replace(cache_buffer_position, read_bytes, src_buffer.constData(), read_bytes);
            cache_buffer_position += read_bytes;
        }

        processed_bytes += read_bytes;

        if(read_bytes < header.block_size)
        {
            if(cache_buffer_position)
                dest->write(cache_buffer.constData(), cache_buffer_position);
            break;
        }

        if(QThread::currentThread()->isInterruptionRequested())
        {
            dest->close();
            QFile::remove(_destination);
            return false;
        }
    }

    index[index_count - 1] = (static_cast<qint32>(dest->pos() + 1) >> 1);

    dest->seek(sizeof(ZsoHeader));
    dest->write(reinterpret_cast<const char *>(index.data()), index_count * sizeof(qint32));
    dest->close();

    return true;
}
