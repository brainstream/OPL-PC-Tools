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

#include <OplPcTools/GameArt.h>
#include <OplPcTools/GameArtSource.h>
#include <QNetworkReply>
#include <QObject>

namespace OplPcTools {

struct GameArtNetworkTask
{
    quint32 task_id;
    QString game_id;
    GameArtType art_type;
    GameArtProperties art_properties;
};

class GameArtNetworkSource : public QObject, public GameArtSource
{
    Q_OBJECT

public:
    GameArtNetworkSource(const GameArtNetworkTask & _task, QNetworkReply * _reply, QObject * _parent);
    const QPixmap & pixmap() const override { return m_pixmap; }

signals:
    void ready();
    void error(bool _canceled, const QString & _message);
    void complete();

private slots:
    void onReplyFinished();
    void onReplyError(QNetworkReply::NetworkError _error);

private:
    void emitError(bool _canceled);

private:
    const GameArtNetworkTask m_task;
    QNetworkReply * mp_reply;
    QPixmap m_pixmap;
};

class GameArtDownloaderTask : public QObject
{
    Q_OBJECT

public:
    GameArtDownloaderTask(quint64 _id, QNetworkAccessManager * _network);
    quint64 id() const { return m_id; }

public slots:
    void cancel();

private:
    const quint64 m_id;
    QNetworkAccessManager * mp_network;
};

class GameArtDownloader : public QObject
{
    Q_OBJECT

public:
    explicit GameArtDownloader(QObject * _parent);
    GameArtDownloaderTask * downloadArts(const QString & _game_id, QList<GameArtType> _types);

signals:
    void downloadComplete(
        const OplPcTools::GameArtNetworkTask & _task,
        const OplPcTools::GameArtNetworkSource & _source);
    void taskComplete(quint32 _task_id, const QStringList & _errors);

private:
    QString makeUrl(const QString & _game_id, GameArtType _type) const;

private:
    quint32 m_next_task_id;
    QMap<GameArtType, GameArtProperties> m_game_art_props;
};

} // namespace OplPcTools
