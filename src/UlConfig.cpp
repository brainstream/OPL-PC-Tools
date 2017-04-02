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

#include <QFile>
#include <QTemporaryFile>
#include "UlConfig.h"
#include "IOException.h"
#include "ValidationException.h"
#include "Game.h"

#define MT_CD  0x12
#define MT_DVD 0x14
#define UL_CONFIG_FILENAME "ul.cfg"

namespace {

struct RawConfigRecord
{
    explicit RawConfigRecord(const UlConfigRecord & _record)
    {
        memset(this, 0, sizeof(RawConfigRecord));
        QByteArray name_bytes = _record.name.toUtf8();
        QByteArray image_bytes = _record.image.toLatin1();
        memcpy(this->image, image_bytes.constData(), image_bytes.size());
        memcpy(this->name , name_bytes.constData(), name_bytes.size());
        this->media = _record.type == MediaType::dvd ? MT_DVD : MT_CD;
        this->parts = _record.parts;
        this->pad[4] = 0x08; // To be like USBA
    }

    char name[Game::max_game_name_length];
    char image[Game::max_image_name_length];
    quint8 parts;
    quint8 media;
    quint8 pad[15];
};

void openFile(QFile & _file, QIODevice::OpenMode _flags)
{
    if(!_file.open(_flags))
        throw IOException(QObject::tr("Unable to open file \"%1\"").arg(_file.fileName()));
}

size_t findRecordOffset(QFile & _file, const QString & _image, RawConfigRecord * _result = nullptr)
{
    const QByteArray image_qbytes = _image.toLatin1();
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

UlConfig::UlConfig(const QDir & _config_dir) :
    m_config_directory(_config_dir.path()),
    m_config_filepath(_config_dir.absoluteFilePath(UL_CONFIG_FILENAME))
{
}

QSharedPointer<UlConfig> UlConfig::load(const QDir & _config_dir)
{
    UlConfig * config = new UlConfig(_config_dir);
    try
    {
        config->load();
    }
    catch(...)
    {
        delete config;
        throw;
    }
    return QSharedPointer<UlConfig>(config);
}

void UlConfig::load()
{
    QFile file(m_config_filepath);
    if(!file.open(QIODevice::ReadOnly))
    {
        return;
    }
    const size_t record_size = sizeof(RawConfigRecord);
    char * buffer = new char[record_size];
    for(;;)
    {
        size_t read_bytes = file.read(buffer, record_size);
        if(read_bytes < record_size)
            break;
        RawConfigRecord * raw_record = reinterpret_cast<RawConfigRecord *>(buffer);
        UlConfigRecord record = {};
        if(raw_record->name[Game::max_game_name_length - 1] == '\0')
            record.name = QString::fromUtf8(raw_record->name, strlen(raw_record->name));
        else
            record.name = QString::fromUtf8(raw_record->name, Game::max_game_name_length);
        record.image = QString::fromLatin1(raw_record->image, strlen(raw_record->image));
        record.parts = raw_record->parts;
        switch(raw_record->media)
        {
        case MT_CD:
            record.type = MediaType::cd;
            break;
        case MT_DVD:
            record.type = MediaType::dvd;
            break;
        default:
            record.type = MediaType::unknown;
            break;
        }
        m_records.append(record);
    }
    delete [] buffer;
}

void UlConfig::addRecord(const UlConfigRecord & _config)
{
    Game::validateGameName(_config.name);
    Game::validateGameImageName(_config.image);
    QFile file(m_config_filepath);
    openFile(file, QIODevice::WriteOnly | QIODevice::Append);
    RawConfigRecord record(_config);
    const char * data = reinterpret_cast<const char *>(&record);
    if(file.write(data, sizeof(RawConfigRecord)) != sizeof(RawConfigRecord))
        throw IOException("An error occurred while writing data to file");
    m_records.append(_config);
}

void UlConfig::deleteRecord(const QString _image)
{
    QTemporaryFile temp_file;
    temp_file.setAutoRemove(true);
    temp_file.open();
    int record_count = m_records.size();
    int item_to_delete = -1;
    for(int i = 0; i < record_count; ++i)
    {
        if(m_records[i].image == _image)
        {
            item_to_delete = i;
            continue;
        }
        RawConfigRecord raw_record(m_records[i]);
        const char * data = reinterpret_cast<const char *>(&raw_record);
        temp_file.write(data, sizeof(RawConfigRecord));
    }
    temp_file.close();
    if(item_to_delete >= 0)
        m_records.removeAt(item_to_delete);
    QString config_bk = m_config_filepath + "_bk";
    if(!QFile::rename(m_config_filepath, config_bk))
        throw IOException(QObject::tr("Unable to backup config file"));
    temp_file.rename(m_config_filepath);
    temp_file.setAutoRemove(false);
    QFile::remove(config_bk);
}

void UlConfig::renameRecord(const QString _image, const QString & _new_name)
{
    Game::validateGameName(_new_name);
    UlConfigRecord * record = findRecord(_image);
    if(!record)
        throw ValidationException(QObject::tr("Config record is not loaded"));
    QFile file(m_config_filepath);
    openFile(file, QIODevice::ReadWrite);
    size_t offset = findRecordOffset(file, _image);
    if(!~offset)
        throw ValidationException(QObject::tr("Config record was not found"));
    file.seek(offset);
    char data[sizeof(RawConfigRecord::name)];
    memset(&data, 0, sizeof(RawConfigRecord::name));
    QByteArray name_bytes = _new_name.toUtf8();
    strncpy(data, name_bytes.constData(), name_bytes.size());
    if(file.write(data, sizeof(data)) != sizeof(data))
        throw IOException("An error occurred while writing data to file");
    record->name = _new_name;
}

UlConfigRecord * UlConfig::findRecord(const QString & _image)
{
    for(UlConfigRecord & record : m_records)
    {
        if(record.image == _image)
            return &record;
    }
    return nullptr;
}
