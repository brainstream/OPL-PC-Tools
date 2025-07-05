/***********************************************************************************************
 * Copyright © 2017-2025 Sergey Smolyannikov aka brainstream                                   *
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

#include <OplPcTools/GameImporter.h>
#include <OplPcTools/Exception.h>
#include <OplPcTools/GameArt.h>
#include <OplPcTools/GameArtFileSource.h>
#include <OplPcTools/File.h>
#include <OplPcTools/Library.h>
#include <QThread>
#include <list>

using namespace OplPcTools;

namespace {

class ThreadCancelledException : public QException { };

} // namespace

GameImporter::GameImporter(
    const QString & _source_directory,
    GameArtManager & _art_manager,
    const Game & _game,
    QObject * _parent
) :
    QObject(_parent),
    mr_art_manager(_art_manager),
    m_source_directory(_source_directory),
    m_game(_game)
{
}

bool GameImporter::import()
{
    if(m_progress.isRunning())
        return false;
    m_progress.reset();
    std::list<FileCopyTask> tasks;
    QDir src_directory(m_source_directory);
    QDir dest_directory(Library::instance().directory());
    for(quint8 parts_idx = 0; parts_idx < m_game.partCount(); ++parts_idx)
    {
        const QString part_filename = UlConfigGameStorage::makePartFilename(m_game.id(), m_game.title(), parts_idx);
        const QString src_filiname = src_directory.absoluteFilePath(part_filename);
        const QString dest_filename = dest_directory.absoluteFilePath(part_filename);
        tasks.emplace_back(src_filiname, dest_filename);
        if(!tasks.back().src.exists())
        {
            emit error(tr("File not found: \"%1\"").arg(src_filiname));
            return false;
        }
        if(tasks.back().dest.exists())
        {
            emit error(tr("File already exists: \"%1\"").arg(dest_filename));
            return false;
        }
        m_progress.total_parts_bytes += tasks.back().src.size();
    }
    try
    {
        m_progress.state = GameImportPorgress::State::PartsCopying;
        emit progress(m_progress);
        if(processTasks(tasks))
            Library::instance().games().addGame(m_game);
    }
    catch(const ThreadCancelledException &)
    {
        m_progress.state = GameImportPorgress::State::Cancelled;
        emit progress(m_progress);
        return false;
    }
    catch(const Exception & exception)
    {
        rollback(tasks);
        emit error(exception.message());
        m_progress.state = GameImportPorgress::State::Error;
        emit progress(m_progress);
        return false;
    }
    copyArts();
    m_progress.state = GameImportPorgress::State::Done;
    emit progress(m_progress);
    return true;
}

bool GameImporter::processTasks(std::list<FileCopyTask> & _tasks)
{
    m_progress.state = GameImportPorgress::State::PartsCopying;
    emit progress(m_progress);
    for(FileCopyTask & task : _tasks)
    {
        if(!processTask(task))
            return false;
    }
    return true;
}

bool GameImporter::processTask(FileCopyTask & _task)
{
    const qint64 batch_size = 2048 * 2048;
    QByteArray buffer(batch_size, Qt::Uninitialized);
    QFile src(_task.src.fileName());
    openFile(src, QIODevice::ReadOnly);
    QSharedPointer<QFile> dest = openFileToSyncWrite(_task.dest.fileName());
    for(int i = 0; ; ++i)
    {
        if(QThread::currentThread()->isInterruptionRequested())
        {
            throw ThreadCancelledException();
        }
        qint64 read_bytes = readFile(src, buffer.data(), batch_size);
        if(read_bytes > 0)
        {
            qint64 write_bytes = writeFile(*dest, buffer.constData(), read_bytes);
            m_progress.done_parts_bytes += write_bytes;
            emit progress(m_progress);
        }
        if(read_bytes == 0 || read_bytes < batch_size)
        {
            break;
        }
    }
    return true;
}

void GameImporter::rollback(std::list<FileCopyTask> & _tasks)
{
    m_progress.state = GameImportPorgress::State::Rollback;
    emit progress(m_progress);
    for(FileCopyTask & task : _tasks)
        task.dest.remove();
}

void GameImporter::copyArts()
{
    m_progress.state = GameImportPorgress::State::ArtsRegistration;
    emit progress(m_progress);
    QDir src_directory(m_source_directory);
    src_directory.cd(GameArtManager::art_directory);
    if(!src_directory.exists())
        return;
    QMap<GameArtType, GameArtProperties> property_map = makeGameArtProperies();
    for(auto it = property_map.begin(); it != property_map.end(); ++it)
    {
        QString filename = src_directory.absoluteFilePath(GameArtManager::makeArtFilename(m_game.id(), it.value()));
        if(QFile::exists(filename))
            mr_art_manager.setArt(m_game.id(), it.key(), GameArtFileSource(filename));
    }
}
