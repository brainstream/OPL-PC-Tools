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

#include <cstring>
#include <OplPcTools/Exception.h>
#include <OplPcTools/File.h>
#include <OplPcTools/Settings.h>
#include <OplPcTools/UlConfigGameStorage.h>

#define MT_CD  0x12
#define MT_DVD 0x14
#define UL_CONFIG_FILENAME "ul.cfg"

using namespace OplPcTools;

namespace {

const QString g_image_prefix("ul.");

struct RawConfigRecord
{
    explicit RawConfigRecord(const Game & _game);
    char name[UlConfigGameStorage::max_name_length];
    char image[GameStorage::max_id_length];
    quint8 parts;
    quint8 media;
    quint8 pad[15];
} __attribute__((packed));

RawConfigRecord::RawConfigRecord(const Game & _game)
{
    memset(this, 0, sizeof(RawConfigRecord));
    QByteArray name_bytes = _game.title().toUtf8();
    QByteArray image_bytes = _game.id().toLatin1();
    memcpy(this->image, g_image_prefix.toLatin1().constData(), g_image_prefix.size());
    memcpy(&this->image[g_image_prefix.size()], image_bytes.constData(), image_bytes.size());
    memcpy(this->name , name_bytes.constData(), name_bytes.size());
    this->media = _game.mediaType() == MediaType::DVD ? MT_DVD : MT_CD;
    this->parts = _game.partCount();
    this->pad[4] = 0x08; // To be like USBA
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

// This function originally was taken from the OPL project (iso2opl.c).
// https://github.com/ifcaro/Open-PS2-Loader
quint32 crc32(const QString & _string)
{
    std::string string = _string.toStdString();
    quint32 * crctab = new quint32[0x400];
    int crc = 0;
    int count = 0;
    for(int table = 0; table < 256; ++table)
    {
        crc = table << 24;
        for(count = 8; count > 0; --count)
        {
            if (crc < 0)
                crc = crc << 1;
            else
                crc = (crc << 1) ^ 0x04C11DB7;
        }
        crctab[255 - table] = crc;
    }
    do
    {
        int byte = string[count++];
        crc = crctab[byte ^ ((crc >> 24) & 0xFF)] ^ ((crc << 8) & 0xFFFFFF00);
    } while (string[count - 1] != 0);
    delete [] crctab;
    return crc;
}

bool validateGame(const Game & _game)
{
    if(_game.partCount() > 10) return false;
    for(const QChar & ch : _game.id())
        if(!ch.isPrint()) return false;
    for(const QChar & ch : _game.title())
        if(!ch.isPrint()) return false;
    return true;
}

inline void throwUlCorrupted()
{
    throw ValidationException(QObject::tr("%1 is corrupted").arg(UL_CONFIG_FILENAME));
}

} // namespace

UlConfigGameStorage::UlConfigGameStorage(QObject * _parent /*= nullptr*/) :
    GameStorage(_parent)
{
}

GameInstallationType UlConfigGameStorage::installationType() const
{
    return GameInstallationType::UlConfig;
}

bool UlConfigGameStorage::performLoading(const QDir & _directory)
{
    const Settings & settings = Settings::instance();
    m_config_filepath = _directory.absoluteFilePath(UL_CONFIG_FILENAME);
    QFile file(m_config_filepath);
    openFile(file, QIODevice::ReadWrite);
    const size_t record_size = sizeof(RawConfigRecord);
    if(settings.flag(Settings::Flag::ValidateUlCfg) && file.size() % record_size != 0)
        throwUlCorrupted();
    char * buffer = new char[record_size];
    for(;;)
    {
        size_t read_bytes = file.read(buffer, record_size);
        if(read_bytes < record_size)
            break;
        RawConfigRecord * raw_record = reinterpret_cast<RawConfigRecord *>(buffer);
        Game * game = createGame(QString::fromLatin1(&raw_record->image[g_image_prefix.size()],
            strlen(raw_record->image) - g_image_prefix.size()));
        if(raw_record->name[max_name_length - 1] == '\0')
            game->setTitle(QString::fromUtf8(raw_record->name, strlen(raw_record->name)));
        else
            game->setTitle(QString::fromUtf8(raw_record->name, max_name_length));
        game->setPartCount(raw_record->parts);
        switch(raw_record->media)
        {
        case MT_CD:
            game->setMediaType(MediaType::CD);
            break;
        case MT_DVD:
            game->setMediaType(MediaType::DVD);
            break;
        default:
            game->setMediaType(MediaType::Unknown);
            break;
        }
        if(settings.flag(Settings::Flag::ValidateUlCfg) && !validateGame(*game))
            throwUlCorrupted();
    }
    delete [] buffer;
    return true;
}

bool UlConfigGameStorage::performRenaming(const Game & _game, const QString & _title)
{
    validateTitle(_title);
    QFile file(m_config_filepath);
    openFile(file, QIODevice::ReadWrite);
    size_t offset = findRecordOffset(file, _game.id());
    if(!~offset)
        throw ValidationException(QObject::tr("Config record was not found"));
    file.seek(offset);
    char data[sizeof(RawConfigRecord::name)];
    memset(&data, 0, sizeof(RawConfigRecord::name));
    QByteArray name_bytes = _title.toUtf8();
    strncpy(data, name_bytes.constData(), name_bytes.size());
    if(file.write(data, sizeof(data)) != sizeof(data))
        throw IOException(QObject::tr("An error occurred while writing data to file"));
    return true;
}

bool UlConfigGameStorage::performRegistration(const Game & _game)
{
    validateTitle(_game.title());
    validateId(g_image_prefix + _game.id());
    QFile file(m_config_filepath);
    openFile(file, QIODevice::WriteOnly | QIODevice::Append);
    RawConfigRecord record(_game);
    const char * data = reinterpret_cast<const char *>(&record);
    if(file.write(data, sizeof(RawConfigRecord)) != sizeof(RawConfigRecord))
        throw IOException(QObject::tr("An error occurred while writing data to file"));
    return true;
}

QString UlConfigGameStorage::makePartFilename(const QString & _id, const QString & _name, quint8 _part)
{
    QString crc = QString("%1").arg(crc32(_name.toUtf8().constData()), 8, 16, QChar('0')).toUpper();
    return QString("ul.%1.%2.%3").arg(crc).arg(_id).arg(_part, 2, 10, QChar('0'));
}

void UlConfigGameStorage::validateTitle(const QString & _title)
{
    if(_title.toUtf8().size() > max_name_length)
        throw ValidationException(QObject::tr("Maximum name length is %1 bytes").arg(max_name_length));
}

bool UlConfigGameStorage::performDeletion(const Game & _game)
{
    deleteGameConfig(_game.id());
    deletePartFiles(_game);
    return true;
}

void UlConfigGameStorage::deleteGameConfig(const QString _id)
{
    QFile config(m_config_filepath);
    openFile(config, QIODevice::ReadOnly);
    size_t offset = findRecordOffset(config, _id);
    if(!~offset)
        throw ValidationException(tr("Unable to locate Game \"%1\" in the config file").arg(_id));
    QFile temp_file(m_config_filepath + ".tmp");
    openFile(temp_file, QIODevice::WriteOnly | QIODevice::Truncate);
    if(offset > 0)
    {
        config.seek(0);
        temp_file.write(config.read(offset));
    }
    config.seek(offset + sizeof(RawConfigRecord));
    temp_file.write(config.readAll());
    temp_file.flush();
    config.close();
    temp_file.close();
    QString config_bk = m_config_filepath + ".bk";
    if(!QFile::rename(m_config_filepath, config_bk))
        throw IOException(QObject::tr("Unable to backup config file"));
    QFile::rename(temp_file.fileName(), config.fileName());
    QFile::remove(config_bk);
}

void UlConfigGameStorage::deletePartFiles(const Game & _game)
{
    QDir root_dir(m_config_filepath);
    root_dir.cdUp();
    for(int part = 0; part < _game.partCount(); ++part)
    {
        QString path = root_dir.absoluteFilePath(makePartFilename(_game.id(), _game.title(), part));
        QFile::remove(path);
    }
}
