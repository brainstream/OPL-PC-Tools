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

#include <QFile>
#include <QDir>
#include <QThread>
#include <OplPcTools/Library.h>
#include <OplPcTools/UlConfigGameStorage.h>
#include <OplPcTools/Exception.h>
#include <OplPcTools/IsoRestorer.h>
#include <OplPcTools/File.h>

using namespace OplPcTools;

IsoRestorer::IsoRestorer(const Game & _game, const QString & _iso_filepath, QObject * _parent /*= nullptr*/) :
    QObject(_parent),
    mr_game(_game),
    m_iso_filepath(_iso_filepath)
{
}

bool IsoRestorer::restore()
{
    QFile iso(m_iso_filepath);
    openFile(iso, QIODevice::WriteOnly | QIODevice::Truncate);
    QStringList filenames;
    filenames.reserve(mr_game.partCount());
    QDir games_dir(Library::instance().directory());
    quint64 all_files_total_size = 0;
    for(quint8 part = 0; part < mr_game.partCount(); ++part)
    {
        QString filename = games_dir.absoluteFilePath(
            UlConfigGameStorage::makePartFilename(mr_game.id(), mr_game.title(), part));
        filenames.append(filename);
        QFileInfo file_info(filename);
        if(!file_info.exists())
        {
            rollback();
            throw IOException(tr("File not found: \"%1\"").arg(filename));
        }
        all_files_total_size += file_info.size();
    }
    quint64 total_write_bytes = 0;
    const qint64 batch_size = 2048 * 2048;
    QByteArray buffer(batch_size, Qt::Uninitialized);
    for(const QString & filename : filenames)
    {
        QFile file(filename);
        openFile(file, QIODevice::ReadOnly);
        for(int i = 0; ; ++i)
        {
            if(QThread::currentThread()->isInterruptionRequested())
            {
                rollback();
                return false;
            }
            qint64 read_bytes = 0;
            try
            {
                read_bytes = readFile(file, buffer.data(), batch_size);
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
                    qint64 write_bytes = writeFile(iso, buffer.constData(), read_bytes);
                    total_write_bytes += write_bytes;
                }
                catch(...)
                {
                    rollback();
                    throw;
                }
                if(i % 5 == 0 || total_write_bytes == all_files_total_size)
                {
                    iso.flush();
                }
                emit progress(all_files_total_size, total_write_bytes);
            }
            if(read_bytes == 0 || read_bytes < batch_size)
            {
                break;
            }
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
