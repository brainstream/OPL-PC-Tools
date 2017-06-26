/***********************************************************************************************
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

#include <algorithm>
#include <QFile>
#include <QTemporaryFile>
#include "IOException.h"
#include "ValidationException.h"
#include "GameCollection.h"

#define MT_CD  0x12
#define MT_DVD 0x14
#define UL_CONFIG_FILENAME "ul.cfg"

namespace {

const QString g_image_prefix("ul.");
const QString g_art_dir("ART");
const QString g_cover_suffix("_COV");

struct RawConfigRecord
{
    explicit RawConfigRecord(const Game & _game)
    {
        memset(this, 0, sizeof(RawConfigRecord));
        QByteArray name_bytes = _game.title.toUtf8();
        QByteArray image_bytes = _game.id.toLatin1();
        memcpy(this->image, g_image_prefix.toLatin1().constData(), g_image_prefix.size());
        memcpy(&this->image[g_image_prefix.size()], image_bytes.constData(), image_bytes.size());
        memcpy(this->name , name_bytes.constData(), name_bytes.size());
        this->media = _game.media_type == MediaType::DVD ? MT_DVD : MT_CD;
        this->parts = _game.part_count;
        this->pad[4] = 0x08; // To be like USBA
    }

    char name[MAX_GAME_NAME_LENGTH];
    char image[MAX_GAME_ID_LENGTH];
    quint8 parts;
    quint8 media;
    quint8 pad[15];
};

void openFile(QFile & _file, QIODevice::OpenMode _flags)
{
    if(!_file.open(_flags))
        throw IOException(QObject::tr("Unable to open file \"%1\"").arg(_file.fileName()));
}

size_t findRecordOffset(QFile & _file, const QString & _id, RawConfigRecord * _result = nullptr)
{
    const QByteArray image_qbytes = (g_image_prefix + _id).toLatin1();
    const char * image_bytes = image_qbytes.constData();
    char record_bytes[sizeof(RawConfigRecord)];
    for(size_t offset = 0; ;)
    {
        quint64 read = _file.read(record_bytes, sizeof(record_bytes));
        if(read < sizeof(record_bytes))
            break;
        RawConfigRecord * record = reinterpret_cast<RawConfigRecord *>(record_bytes);
        if(strncmp(image_bytes, record->image, image_qbytes.length()) == 0)
        {
            if(_result)
                memcpy(_result, &record, sizeof(RawConfigRecord));
            return offset;
        }
        offset += sizeof(RawConfigRecord);
    }
    return ~0;
}

} // namespace


GameCollection::GameCollection(QObject * _parent /*= nullptr*/) :
    QObject(_parent)
{
}

void GameCollection::reloadFromUlConfig(const QDir & _config_dir)
{
    m_config_directory = _config_dir.path();
    m_config_filepath = _config_dir.absoluteFilePath(UL_CONFIG_FILENAME);
    m_games.clear();
    QFile file(m_config_filepath);
    openFile(file, QIODevice::ReadWrite);
    const size_t record_size = sizeof(RawConfigRecord);
    char * buffer = new char[record_size];
    for(;;)
    {
        size_t read_bytes = file.read(buffer, record_size);
        if(read_bytes < record_size)
            break;
        RawConfigRecord * raw_record = reinterpret_cast<RawConfigRecord *>(buffer);
        Game game = { };
        if(raw_record->name[MAX_GAME_NAME_LENGTH - 1] == '\0')
            game.title = QString::fromUtf8(raw_record->name, strlen(raw_record->name));
        else
            game.title = QString::fromUtf8(raw_record->name, MAX_GAME_NAME_LENGTH);
        game.id = QString::fromLatin1(&raw_record->image[g_image_prefix.size()], strlen(raw_record->image) - g_image_prefix.size());
        game.part_count = raw_record->parts;
        switch(raw_record->media)
        {
        case MT_CD:
            game.media_type = MediaType::CD;
            break;
        case MT_DVD:
            game.media_type = MediaType::DVD;
            break;
        default:
            game.media_type = MediaType::Unknown;
            break;
        }
        m_games.append(game);
    }
    delete [] buffer;
    loadPixmaps();
}

void GameCollection::loadPixmaps()
{
    QDir art_dir(m_config_directory);
    if(!art_dir.cd(g_art_dir)) return;
    QFileInfoList files = art_dir.entryInfoList(QStringList { "*.png", "*.jpeg", "*.jpg", "*.bmp" });
    for(Game & game : m_games)
    {
         QString cover_filename = game.id + "_COV";
         QString ico_filename = game.id + "_ICO";
         for(const QFileInfo & file : files)
         {
             const QString filename = file.completeBaseName();
             try
             {
                 if(filename == cover_filename)
                 {
                     game.cover.load(file.absoluteFilePath());
                     game.cover_filepath = file.absoluteFilePath();
                 }
                 else if(filename == ico_filename)
                 {
                     game.icon.load(file.absoluteFilePath());
                     game.icon_filepath = file.absoluteFilePath();
                 }

             }
             catch(...)
             {
                 continue;
             }
             if(!game.cover_filepath.isEmpty() && !game.icon_filepath.isNull())
                 break;
         }
    }
}

void GameCollection::addGame(const Game & _game)
{
    validateGameName(_game.title);
    validateGameId(g_image_prefix + _game.id);
    QFile file(m_config_filepath);
    openFile(file, QIODevice::WriteOnly | QIODevice::Append);
    if(game(_game.id))
        throw ValidationException(QObject::tr("Game \"%1\" already registered").arg(_game.id));
    RawConfigRecord record(_game);
    const char * data = reinterpret_cast<const char *>(&record);
    if(file.write(data, sizeof(RawConfigRecord)) != sizeof(RawConfigRecord))
        throw IOException(QObject::tr("An error occurred while writing data to file"));
    m_games.append(_game);
}

void GameCollection::deleteGame(const QString & _id)
{
    auto iterator = std::find_if(m_games.begin(), m_games.end(), [_id](const Game & game) {
        return game.id == _id;
    });
    if(iterator == m_games.end())
        throw ValidationException(tr("Game \"%1\" is not loaded").arg(_id));
    deleteGameConfig(_id);
    deleteGameFiles(*iterator);
    m_games.erase(iterator);
}

void GameCollection::deleteGameConfig(const QString _id)
{
    QFile config(m_config_filepath);
    openFile(config, QIODevice::ReadOnly);
    size_t offset = findRecordOffset(config, _id);
    if(!~offset)
        throw ValidationException(tr("Unable to locate Game \"%1\" in the config file").arg(_id));
    QTemporaryFile temp_file;
    temp_file.setAutoRemove(true);
    temp_file.open();
    config.seek(0);
    temp_file.write(config.read(offset));
    config.seek(offset + sizeof(RawConfigRecord));
    temp_file.write(config.readAll());
    config.close();
    temp_file.close();
    QString config_bk = m_config_filepath + "_bk";
    if(!QFile::rename(m_config_filepath, config_bk))
        throw IOException(QObject::tr("Unable to backup config file"));
    temp_file.rename(m_config_filepath);
    temp_file.setAutoRemove(false);
    QFile::remove(config_bk);
}

void GameCollection::deleteGameFiles(Game & _game)
{
    QDir root_dir(m_config_directory);
    for(int part = 0; part < _game.part_count; ++part)
    {
        QString path = root_dir.absoluteFilePath(makeGamePartName(_game.id, _game.title, part));
        QFile::remove(path);
    }
    if(!_game.cover_filepath.isEmpty())
        QFile::remove(_game.cover_filepath);
    if(!_game.icon_filepath.isEmpty())
        QFile::remove(_game.icon_filepath);
}

void GameCollection::renameGame(const QString & _id, const QString & _new_name)
{
    validateGameName(_new_name);
    Game & game = findGame(_id);
    renameGameConfig(game, _new_name);
    renameGameFiles(game, _new_name);
    game.title = _new_name;
}

void GameCollection::renameGameConfig(Game & _game, const QString & _new_name)
{
    QFile file(m_config_filepath);
    openFile(file, QIODevice::ReadWrite);
    size_t offset = findRecordOffset(file, _game.id);
    if(!~offset)
        throw ValidationException(QObject::tr("Config record was not found"));
    file.seek(offset);
    char data[sizeof(RawConfigRecord::name)];
    memset(&data, 0, sizeof(RawConfigRecord::name));
    QByteArray name_bytes = _new_name.toUtf8();
    strncpy(data, name_bytes.constData(), name_bytes.size());
    if(file.write(data, sizeof(data)) != sizeof(data))
        throw IOException(QObject::tr("An error occurred while writing data to file"));
}

void GameCollection::renameGameFiles(Game & _game, const QString & _new_name)
{
    QList<QString> files;
    QDir root_dir(m_config_directory);
    for(quint8 part = 0; part < _game.part_count; ++part)
    {
        QString part_path = root_dir.absoluteFilePath(makeGamePartName(_game.id, _game.title, part));
        if(!QFile::exists(part_path))
            throw ValidationException(QObject::tr("File \"%1\" was not found").arg(part_path));
        files.append(part_path);
    }
    for(int part = 0; part < _game.part_count; ++part)
    {
        QString new_path = root_dir.absoluteFilePath(makeGamePartName(_game.id, _new_name, part));
        QFile::rename(files[part], new_path);
    }
}

void GameCollection::setGameCover(const QString _id, QString & _filepath)
{
    const int cover_width = 140;
    const int cover_height = 200;
    Game & game = findGame(_id);
    QPixmap pixmap;
    loadPixmap(pixmap, _filepath);
    pixmap = pixmap.scaled(cover_width, cover_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    game.cover_filepath = savePixmap(pixmap, QString("%1_COV.png").arg(game.id));
    game.cover = pixmap;
}

void GameCollection::loadPixmap(QPixmap & _pixmap, const QString & _filepath)
{
    try
    {
        _pixmap.load(_filepath);
    }
    catch(...)
    {
        throw IOException(tr("Unable to load the cover image file \"%1\"").arg(_filepath));
    }
    if(_pixmap.isNull())
    {
        throw IOException(tr("Unabel to load the picture from file \"%1\"").arg(_filepath));
    }
}

QString GameCollection::savePixmap(QPixmap & _pixmap, const QString & _filename)
{
    QDir art_dir(m_config_directory);
    art_dir.mkdir(g_art_dir);
    if(!art_dir.cd(g_art_dir))
        throw IOException(tr("Unabel to create or open the directory \"%1\"").arg(art_dir.absolutePath()));
    QString filename = art_dir.absoluteFilePath(_filename);
    if(!_pixmap.save(filename, "png"))
       throw IOException(tr("Unabel to write image to file \"%1\"").arg(filename));
    return filename;
}

void GameCollection::removeGameCover(const QString _id)
{
    Game & game = findGame(_id);
    if(game.cover.isNull()) return;
    game.cover = QPixmap();
    QFile::remove(game.cover_filepath);
}

void GameCollection::setGameIcon(const QString _id, QString & _filepath)
{
    const int icon_width = 64;
    const int icon_height = 64;
    Game & game = findGame(_id);
    QPixmap pixmap;
    loadPixmap(pixmap, _filepath);
    pixmap = pixmap.scaled(icon_width, icon_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    game.icon_filepath = savePixmap(pixmap, QString("%1_ICO.png").arg(game.id));
    game.icon = pixmap;
}

void GameCollection::removeGameIcon(const QString _id)
{
    Game & game = findGame(_id);
    if(game.icon.isNull()) return;
    game.icon = QPixmap();
    QFile::remove(game.icon_filepath);
}

const Game * GameCollection::game(const QString & _id) const
{
    for(const Game & game : m_games)
    {
        if(game.id == _id)
            return &game;
    }
    return nullptr;
}

Game & GameCollection::findGame(const QString & _id)
{
    for(Game & game : m_games)
    {
        if(game.id == _id)
            return game;
    }
    throw ValidationException(QObject::tr("Config record is not loaded"));
}
