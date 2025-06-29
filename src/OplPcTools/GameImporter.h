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

#pragma once

#include <OplPcTools/GameArtManager.h>
#include <OplPcTools/Game.h>
#include <QObject>

namespace OplPcTools {

struct GameImportPorgress
{
    enum class State
    {
        Preparing,
        PartsCopying,
        GameRegistration,
        ArtsRegistration,
        Rollback,
        Done,
        Cancelled,
        Error
    };

    GameImportPorgress() :
        total_parts_bytes(0),
        done_parts_bytes(0),
        state(State::Preparing)
    {
    }

    void reset()
    {
        total_parts_bytes = 0;
        done_parts_bytes = 0;
        state = State::Preparing;
    }

    bool isRunning() const
    {
        switch(state)
        {
        case State::Preparing:
        case State::Done:
        case State::Cancelled:
        case State::Error:
            return false;
        default:
            return true;
        }
    }

    quint64 total_parts_bytes;
    quint64 done_parts_bytes;
    State state;
};

class GameImporter : public QObject
{
    Q_OBJECT

private:
    struct FileCopyTask
    {
        FileCopyTask(const QString & _src_filename, const QString & _dest_filename) :
            src(_src_filename),
            dest(_dest_filename)
        {
        }

        QFile src;
        QFile dest;
    };

public:
    GameImporter(
        const QString & _source_directory,
        GameArtManager & _art_manager,
        const Game & _game,
        QObject * _parent);
    bool import();

signals:
    void progress(const OplPcTools::GameImportPorgress & _progress);
    void error(const QString & _message);
    void rollbackStarted();
    void rollbackFinished();

private:
    void rollback(std::list<FileCopyTask> & _tasks);
    bool processTasks(std::list<FileCopyTask> & _tasks);
    bool processTask(FileCopyTask & _task);
    void copyArts();

private:
    OplPcTools::GameImportPorgress m_progress;
    GameArtManager & mr_art_manager;
    const QString m_source_directory;
    const Game m_game;
};

} // namespace OplPcTools
