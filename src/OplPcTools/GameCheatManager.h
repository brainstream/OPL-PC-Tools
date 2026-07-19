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

#pragma once

#include <QDir>
#include <QNetworkReply>
#include <optional>

namespace OplPcTools {

class GameCheatManager
{
    Q_DISABLE_COPY_MOVE(GameCheatManager)

public:
    explicit GameCheatManager(const QDir & _library_path);
    std::optional<QString> load(const QString & _game_id) const;
    void save(const QString & _game_id, const QString & _cheat) const;
    void remove(const QString & _game_id) const;

private:
    QString makeFilename(const QString & _game_id) const;

private:
    const QDir m_library_path;
};

class GameCheatDownloader : public QObject
{
    Q_OBJECT

public:
    explicit GameCheatDownloader(const QString & _game_id, QObject * _parent = nullptr);
    void start();
    void cancel();

signals:
    void downloaded(const QString _cheat);
    void error(const QString _message);
    void finished();

private:
    const QString m_game_id;
    QNetworkReply * mp_network_replay;
    bool m_is_canceled;
};

} // namespace OplPcTools
