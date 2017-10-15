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
#include <OplPcTools/Core/IOException.h>
#include <OplPcTools/Core/ValidationException.h>
#include <OplPcTools/Core/Device.h>
#include <OplPcTools/Core/Iso9660DeviceSource.h>
#include <OplPcTools/Core/GameCollection.h>

#define MT_CD  0x12
#define MT_DVD 0x14
#define UL_CONFIG_FILENAME "ul.cfg"

namespace {

const QString g_image_prefix("ul.");
const QString g_cd_dir("CD");
const QString g_dvd_dir("DVD");
const QString g_art_dir("ART");
const QString g_cover_suffix("_COV");

struct RawConfigRecord
{
    explicit RawConfigRecord(const Game & _game);
    char name[g_max_game_name_length];
    char image[g_max_game_id_length];
    quint8 parts;
    quint8 media;
    quint8 pad[15];
};

RawConfigRecord::RawConfigRecord(const Game & _game)
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

void openFile(QFile & _file, QIODevice::OpenMode _flags)
{
    if(!_file.open(_flags))
        throw IOException(QObject::tr("Unable to open file \"%1\"").arg(_file.fileName()));
}

void renameFile(const QString & _old_filename, const QString & _new_filename)
{
    if(!QFile::rename(_old_filename, _new_filename))
        throw IOException(QObject::tr("Unable to rename file \"%1\" to \"%2\"").arg(_old_filename).arg(_new_filename));
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

const QString & GameCollection::cdDirectory()
{
    return g_cd_dir;
}

const QString & GameCollection::dvdDirectory()
{
    return g_dvd_dir;
}

void GameCollection::reload(const QDir & _directory)
{
    m_directory = _directory.path();
    m_config_filepath = _directory.absoluteFilePath(UL_CONFIG_FILENAME);
    m_games.clear();
    loadUlConfig();
    loadDirs();
    loadPixmaps();
}

void GameCollection::loadUlConfig()
{
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
        Game game;
        if(raw_record->name[g_max_game_name_length - 1] == '\0')
            game.title = QString::fromUtf8(raw_record->name, strlen(raw_record->name));
        else
            game.title = QString::fromUtf8(raw_record->name, g_max_game_name_length);
        game.id = QString::fromLatin1(&raw_record->image[g_image_prefix.size()], strlen(raw_record->image) - g_image_prefix.size());
        game.part_count = raw_record->parts;
        game.installation_type = GameInstallationType::UlConfig;
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
}

void GameCollection::loadDirs()
{
    loadDir(MediaType::CD, g_cd_dir);
    loadDir(MediaType::DVD, g_dvd_dir);
}

void GameCollection::loadDir(MediaType _media_type, const QString & _dir)
{
    QDir dir(m_directory);
    if(!dir.cd(_dir))
        return;
    for(const QString & iso : dir.entryList({ "*.iso" }))
    {
        Device image(QSharedPointer<DeviceSource>(new Iso9660DeviceSource(dir.absoluteFilePath(iso))));
        if(!image.init())
            break;
        Game game;
        game.id = image.gameId();
        game.media_type = _media_type;
        game.installation_type = GameInstallationType::Directory;
        game.part_count = 1;
        game.title = QFileInfo(image.filepath()).fileName();
        game.title = game.title.left(game.title.lastIndexOf('.'));
        if(game.title.startsWith(game.id))
            game.title = game.title.right(game.title.size() - game.id.size() - 1);
        m_games.append(game);
    }
}

void GameCollection::loadPixmaps()
{
    QDir art_dir(m_directory);
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
    if(_game.installation_type == GameInstallationType::UlConfig)
        registerGame(_game);
    m_games.append(_game);
}

void GameCollection::registerGame(const Game & _game)
{
    validateGameName(_game.title, GameInstallationType::UlConfig);
    validateGameId(g_image_prefix + _game.id);
    QFile file(m_config_filepath);
    openFile(file, QIODevice::WriteOnly | QIODevice::Append);
    if(game(_game.id))
        throw ValidationException(QObject::tr("Game \"%1\" already registered").arg(_game.id));
    RawConfigRecord record(_game);
    const char * data = reinterpret_cast<const char *>(&record);
    if(file.write(data, sizeof(RawConfigRecord)) != sizeof(RawConfigRecord))
        throw IOException(QObject::tr("An error occurred while writing data to file"));
}

void GameCollection::deleteGame(const QString & _id)
{
    auto iterator = std::find_if(m_games.begin(), m_games.end(), [_id](const Game & game) {
        return game.id == _id;
    });
    if(iterator == m_games.end())
        throw ValidationException(tr("Game \"%1\" is not loaded").arg(_id));
    if(iterator->installation_type == GameInstallationType::UlConfig)
    {
        deleteGameConfig(_id);
        deletePartFiles(*iterator);
    }
    else
    {
        deleteIsoFile(*iterator);
    }
    deletePixmaps(*iterator);
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

void GameCollection::deletePartFiles(Game & _game)
{
    QDir root_dir(m_directory);
    for(int part = 0; part < _game.part_count; ++part)
    {
        QString path = root_dir.absoluteFilePath(makeGamePartName(_game.id, _game.title, part));
        QFile::remove(path);
    }
}

void GameCollection::deleteIsoFile(Game & _game)
{
    QDir dir(m_directory);
    dir.cd(_game.media_type == MediaType::CD ? g_cd_dir : g_dvd_dir);
    QString path = dir.absoluteFilePath(_game.title) + ".iso";
    if(QFile::exists(path))
        QFile::remove(path);
    else
        QFile::remove(dir.absoluteFilePath(makeGameIsoFilename(_game.title, _game.id)));
}

void GameCollection::deletePixmaps(Game & _game)
{
    if(!_game.cover_filepath.isEmpty())
        QFile::remove(_game.cover_filepath);
    if(!_game.icon_filepath.isEmpty())
        QFile::remove(_game.icon_filepath);
}

void GameCollection::renameGame(const QString & _id, const QString & _new_name)
{
    Game & game = findGame(_id);
    validateGameName(_new_name, game.installation_type);
    if(game.installation_type == GameInstallationType::UlConfig)
    {
        renameGameConfig(game, _new_name);
        renamePartFiles(game, _new_name);
    }
    else
    {
        renameIsoFile(game, _new_name);
    }
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

void GameCollection::renamePartFiles(Game & _game, const QString & _new_name)
{
    QList<QString> files;
    QDir root_dir(m_directory);
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
        renameFile(files[part], new_path);
    }
}

void GameCollection::renameIsoFile(Game & _game, const QString & _new_name)
{
    QDir dir(m_directory);
    dir.cd(_game.media_type == MediaType::CD ? g_cd_dir : g_dvd_dir);
    QString old_filename = dir.absoluteFilePath(_game.title + ".iso");
    QString new_filename;
    if(QFileInfo::exists(old_filename))
    {
        new_filename = dir.absoluteFilePath(_new_name + ".iso");
    }
    else
    {
        old_filename = dir.absoluteFilePath(makeGameIsoFilename(_game.title, _game.id));
        new_filename = dir.absoluteFilePath(makeGameIsoFilename(_new_name, _game.id));
    }
    renameFile(old_filename, new_filename);
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
    QDir art_dir(m_directory);
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
