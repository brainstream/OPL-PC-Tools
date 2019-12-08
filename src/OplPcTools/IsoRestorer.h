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

#ifndef __OPLPCTOOLS_ISORESTORER__
#define __OPLPCTOOLS_ISORESTORER__

#include <QObject>
#include <OplPcTools/Game.h>

namespace OplPcTools {

class IsoRestorer final : public QObject
{
    Q_OBJECT

public:
    IsoRestorer(const Game & _game, const QString & _game_dirpath, const QString & _iso_filepath, QObject * _parent = nullptr);
    bool restore();

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

} // namespace OplPcTools

#endif // __OPLPCTOOLS_ISORESTORER__
