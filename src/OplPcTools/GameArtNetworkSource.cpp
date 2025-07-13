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

#include <OplPcTools/GameArtNetworkSource.h>
#include <OplPcTools/ApplicationInfo.h>
#include <QNetworkAccessManager>
#include <QImageReader>
#include <QHash>

using namespace OplPcTools;

GameArtDownloaderTask::GameArtDownloaderTask(quint64 _id, QNetworkAccessManager * _network) :
    QObject(_network),
    m_id(_id),
    mp_network(_network)
{
    connect(_network, &QObject::destroyed, this, [this]() { mp_network = nullptr; });
}

void GameArtDownloaderTask::cancel()
{
    if(mp_network)
    {
        QList<QNetworkReply *> replies = mp_network->findChildren<QNetworkReply *>();
        foreach(QNetworkReply * reply, replies)
            reply->abort();
    }
}

GameArtNetworkSource::GameArtNetworkSource(
    const GameArtNetworkTask & _task,
    QNetworkReply * _reply,
    QObject * _parent
) :
    QObject(_parent),
    m_task(_task),
    mp_reply(_reply)
{
    connect(_reply, &QNetworkReply::finished, this, &GameArtNetworkSource::onReplyFinished);
#if QT_VERSION_MAJOR < 6
    connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onReplyError(QNetworkReply::NetworkError)));
#else
    connect(_reply, &QNetworkReply::errorOccurred, this, &GameArtNetworkSource::onReplyError);
#endif
}

void GameArtNetworkSource::onReplyFinished()
{
    if(mp_reply->error() != QNetworkReply::NoError)
        return;
    QImageReader reader(mp_reply);
    reader.setAutoDetectImageFormat(true);
    if(reader.canRead())
    {
        m_pixmap = QPixmap::fromImageReader(&reader);
        emit ready();
    }
    else
    {
        emitError(false);
    }
    emit complete();
}

void GameArtNetworkSource::emitError(bool _canceled)
{
    QString additional_message = mp_reply->errorString();
    QString delimiter = additional_message.isEmpty() ? "" : ". ";
    emit error(
        _canceled,
        tr("Unable to download picture: %1%2%3").arg(m_task.art_properties.name, delimiter, additional_message));
}

void GameArtNetworkSource::onReplyError(QNetworkReply::NetworkError _error)
{
    emitError(_error == QNetworkReply::OperationCanceledError);
    emit complete();
}

GameArtDownloader::GameArtDownloader(QObject * _parent) :
    QObject(_parent),
    m_next_task_id(1),
    m_game_art_props(makeGameArtProperies())
{
}

GameArtDownloaderTask * GameArtDownloader::downloadArts(const QString & _game_id, QList<GameArtType> _types)
{
    const quint32 task_id = m_next_task_id++;
    struct DownloadContext
    {
        std::atomic_int task_count;
        QHash<GameArtType, QString> errors;
    };
    DownloadContext * context = new DownloadContext
    {
        .task_count = _types.size(),
        .errors = QHash<GameArtType, QString>()
    };
    foreach(GameArtType type, _types)
        context->errors[type] = QString();
    QNetworkAccessManager * network = new QNetworkAccessManager(this);
    GameArtDownloaderTask * task = new GameArtDownloaderTask(task_id, network);
    foreach(GameArtType type, _types)
    {
        GameArtNetworkTask task
        {
            .task_id = task_id,
            .game_id = _game_id,
            .art_type = type,
            .art_properties = m_game_art_props[type]
        };
        QNetworkRequest request(makeUrl(_game_id, type));
        request.setHeader(QNetworkRequest::UserAgentHeader, APPLICATION_DISPLAY_NAME);
        QNetworkReply * reply = network->get(request);
        GameArtNetworkSource * src = new GameArtNetworkSource(task, reply, this);
        connect(src, &GameArtNetworkSource::complete, this, [this, task_id, network, src, reply, context]() {
            int prev_task_count = context->task_count.fetch_sub(1);
            src->deleteLater();
            reply->deleteLater();
            if(prev_task_count == 1)
            {
                network->deleteLater();
                QStringList filtered_errors;
                foreach(const QString & err, context->errors)
                    if(!err.isEmpty()) filtered_errors.append(err);
                emit taskComplete(task_id, filtered_errors);
                delete context;
            }
        });
        connect(src, &GameArtNetworkSource::error, this, [context, task](bool __canceled, const QString & __message) {
            if(!__canceled)
            {
                context->errors[task.art_type] = __message;
            }
        });
        connect(src, &GameArtNetworkSource::ready, this, [this, task, src]() {
            emit downloadComplete(task, *src);
        });
    }
#ifdef __clang_analyzer__
    [[clang::suppress]] // Potential leak of memory pointed to by 'context' [clang-analyzer-cplusplus.NewDeleteLeaks]
#endif
    return task;
}

QString GameArtDownloader::makeUrl(const QString & _game_id, GameArtType _type) const
{
    const GameArtProperties & props = m_game_art_props[_type];
    return QString(
        "https://ia903209.us.archive.org/view_archive.php"
        "?archive=/34/items/ps2-opl-cover-art-set/PS2_OPL_ART_kira.7z"
        "&file=ART/%1%2.png")
        .arg(_game_id, props.suffix);
}
