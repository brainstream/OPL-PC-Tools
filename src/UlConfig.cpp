#include <QFile>
#include "UlConfig.h"
#include "IOException.h"
#include "ValidationException.h"

#define MT_CD  0x12
#define MT_DVD 0x14

using namespace Ul;

namespace {

struct RawConfigRecord
{
    char name[UL_MAX_GAME_NAME_LENGTH];
    char image[15];
    quint8 parts;
    quint8 media;
    quint8 pad[15];
};

void openFile(QFile & _file, QIODevice::OpenModeFlag _flags)
{
    if(!_file.open(_flags))
        throw IOException(QObject::tr("Unable to open file \"%1\"").arg(_file.fileName()));
}

size_t findRecord(QFile & _file, const QString & _image, RawConfigRecord * _result = nullptr)
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


QList<ConfigRecord> Ul::loadConfig(const QString & _filepath)
{
    QFile file(_filepath);
    openFile(file, QIODevice::ReadOnly);
    QList<ConfigRecord> results;
    const size_t record_size = sizeof(RawConfigRecord);
    char * buffer = new char[record_size];
    for(;;)
    {
        size_t read_bytes = file.read(buffer, record_size);
        if(read_bytes < record_size)
            break;
        RawConfigRecord * raw_record = reinterpret_cast<RawConfigRecord *>(buffer);
        ConfigRecord record = {};
        if(raw_record->name[UL_MAX_GAME_NAME_LENGTH - 1] == '\0')
            record.name = QString::fromUtf8(raw_record->name, strlen(raw_record->name));
        else
            record.name = QString::fromUtf8(raw_record->name, UL_MAX_GAME_NAME_LENGTH);
        record.image = QString::fromLatin1(raw_record->image, sizeof(RawConfigRecord::image));
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
        results.append(record);
    }
    delete [] buffer;
    return results;
}

void Ul::addConfigRecord(const ConfigRecord & _config, const QString & _filepath)
{

}

void Ul::deleteConfigRecord(const QString _image, const QString & _filepath)
{

}

void Ul::renameConfigRecord(const QString _image, const QString & _new_name, const QString & _filepath)
{
    QByteArray name_bytes = _new_name.toUtf8();
    if(name_bytes.size() > UL_MAX_GAME_NAME_LENGTH)
        throw ValidationException(QObject::tr("Maximum name length is %1 bytes").arg(UL_MAX_GAME_NAME_LENGTH));
    QFile file(_filepath);
    openFile(file, QIODevice::ReadWrite);
    size_t offset = findRecord(file, _image);
    if(!~offset)
        throw ValidationException(QObject::tr("Config record was not found"));
    file.seek(offset);
    char data[sizeof(RawConfigRecord::name)];
    memset(&data, 0, sizeof(RawConfigRecord::name));
    strncpy(data, name_bytes.constData(), name_bytes.size());
    if(file.write(data, sizeof(data)) != sizeof(data))
        throw IOException("An error occurred while writing data to file");
}
