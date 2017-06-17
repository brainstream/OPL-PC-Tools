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

#ifndef __QPCOPL_ISORECOVERER__
#define __QPCOPL_ISORECOVERER__

#include <QObject>
#include "Game.h"

class IsoRecoverer final : public QObject
{
    Q_OBJECT

public:
    IsoRecoverer(const Game & _game, const QString & _game_dirpath, const QString & _iso_filepath, QObject * _parent = nullptr);
    bool recover();

signals:
    void progress(quint64 _total_bytes, quint64 _done_bytes);
    void rollbackStarted();
    void rollbackFinished();

private:
    void rollback();

private:
    const Game & mr_game;
    const QString m_game_dirpath;
    const QString m_iso_filepath;
};

#endif // __QPCOPL_ISORECOVERER__
