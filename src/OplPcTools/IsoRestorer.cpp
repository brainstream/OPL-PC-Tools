/***********************************************************************************************
 * Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                   *
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

#include <QFile>
#include <QDir>
#include <QThread>
#include <OplPcTools/UlConfigGameStorage.h>
#include <OplPcTools/Exception.h>
#include <OplPcTools/IsoRestorer.h>

using namespace OplPcTools;

IsoRestorer::IsoRestorer(const Game & _game, const QString & _game_dirpath, const QString & _iso_filepath, QObject * _parent /*= nullptr*/) :
    QObject(_parent),
    mr_game(_game),
    m_game_dirpath(_game_dirpath),
    m_iso_filepath(_iso_filepath)
{
}

bool IsoRestorer::restore()
{
    QFile iso(m_iso_filepath);
    if(!iso.open(QIODevice::WriteOnly | QIODevice::Truncate))
        throw IOException(tr("Unable to open file to write: \"%1\"").arg(m_iso_filepath));
    QStringList filenames;
    filenames.reserve(mr_game.partCount());
    QDir games_dir(m_game_dirpath);
    quint64 all_files_total_size = 0;
    for(quint8 part = 0; part < mr_game.partCount(); ++part)
    {
        QString filename = games_dir.absoluteFilePath(UlConfigGameStorage::makePartFilename(mr_game.id(), mr_game.title(), part));
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
        if(!file.open(QIODevice::ReadOnly))
            throw IOException(tr("Unable to open file to read: \"%1\"").arg(filename));
        for(int i = 0; ; ++i)
        {
            if(QThread::currentThread()->isInterruptionRequested())
            {
                rollback();
                return false;
            }
            qint64 read_bytes = file.read(buffer.data(), batch_size);
            if(read_bytes > 0)
            {
                qint64 write_bytes = iso.write(buffer.constData(), read_bytes);
                if(write_bytes <= 0)
                {
                    rollback();
                    throw IOException(tr("Unable to write a data into the file: \"%1\"").arg(m_iso_filepath));
                }
                total_write_bytes += write_bytes;
                if(i % 5 == 0 || total_write_bytes == all_files_total_size)
                {
                    iso.flush();
                }
                emit progress(all_files_total_size, total_write_bytes);
            }
            else if(read_bytes < 0)
            {
                rollback();
                throw IOException(tr("Unable to read the file: \"%1\"").arg(filename));
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
