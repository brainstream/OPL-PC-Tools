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

#include <QScopedPointer>
#include <QRegExp>
#include <QFileInfo>
#include "Device.h"

#define ISO9660_OFFSET 0x8000

namespace {

template<typename IntType>
struct LBInt
{
    IntType le;
    IntType be;

    operator IntType() const
    {
#if defined(__x86_64) || defined(__i386) || defined(_M_I86) || defined(_M_X64)
        return le;
#else
#   error "Only Little Endian CPUs are supported"
#endif

    }
} __attribute__((packed));

using LBInt16 = LBInt<qint16>;
using LBInt32 = LBInt<qint32>;

struct DateTimeDR
{
    qint8 year;
    qint8 month;
    qint8 day;
    qint8 hour;
    qint8 minute;
    qint8 second;
    qint8 timezone_offset;
} __attribute__((packed));

struct FileFlags
{
    bool hidden: 1;
    bool directory: 1;
    bool associated: 1;
    bool extend_attr_has_info : 1;
    bool extend_attr_has_perms: 1;
    bool unused_1: 1;
    bool unused_2: 1;
    bool is_not_final_dir: 1;
};

struct FileRecord
{
    qint8 record_length;
    qint8 extended_attr_record_length;
    LBInt32 extent_location;
    LBInt32 data_length;
    DateTimeDR recording_date;
    FileFlags file_flags;
    qint8 file_unit_size;
    qint8 interleave_gap_size;
    LBInt16 volume_sequence_number;
    qint8 filename_length;
    char filename[1];
} __attribute__((packed));

struct DateTimePVD
{
    char year[4];
    char month[2];
    char day[2];
    char hour[2];
    char minute[2];
    char second[2];
    char second_hundredths[2];
    qint8 timezone_offset;
} __attribute__((packed));

enum class VolumeDescriptorType : qint8
{
    BootRecord = 0,
    PrimaryVolumeDescriptor = 1,
    SupplementaryVolumeDescriptor  = 2,
    VolumePartitionDescriptor  = 3
};

struct BasicVolumeDescriptor
{
    VolumeDescriptorType type;
    char id[5];
    qint8 version;
} __attribute__((packed));

struct VolumeDescriptor : BasicVolumeDescriptor
{
    qint8 data[2041];
} __attribute__((packed));

struct PrimaryVolumeDescriptor : BasicVolumeDescriptor
{
    qint8 unused_1;
    char system_id[32];
    char volume_id[32];
    qint8 unused_2[8];
    LBInt32 block_count;
    qint8 unused_3[32];
    LBInt16 volume_set_size;
    LBInt16 volume_number;
    LBInt16 block_size;
    LBInt32 path_table_size;
    qint32 path_table_location_le;
    qint32 optional_path_table_location_le;
    qint32 path_table_location_be;
    qint32 optional_path_table_location_be;
    FileRecord root_directory;
    char volume_set_id[128];
    char publisher_id[128];
    char data_preparer_id[128];
    char application_id[128];
    char copyright_file_id[38];
    char abstract_file_id[36];
    char bibliographic_file_id[37];
    DateTimePVD creation_date;
    DateTimePVD modification_date;
    DateTimePVD expiration_date;
    DateTimePVD effective_date;
    qint8 file_structure_version;
    qint8 unused_4;
    qint8 application_data[512];
    qint8 unused_5[653];
} __attribute__((packed));

} // namespace

class Device::Iso9660Data final
{
    Q_DISABLE_COPY(Iso9660Data)

public:
    Iso9660Data(const QString _filepath);
    ~Iso9660Data();
    inline bool isInitialized() const;
    inline qint16 blockSize() const;
    inline qint32 blockCount() const;
    inline bool isPlayStationDisc() const;
    inline QString title() const;
    inline const QString & gameId() const;

private:
    bool readPrimaryVolumeDescriptor(QFile & _file);
    bool readConfig(QFile & _file);
    FileRecord * readFileRecord(const char * _start_ptr);
    bool parseConfig(QFile & _file, FileRecord * _file_record);
    bool readGameId(const QByteArray & _config);

private:
    bool m_is_initialized;
    PrimaryVolumeDescriptor * mp_descriptor;
    QString m_game_id;
};

Device::Iso9660Data::Iso9660Data(const QString _filepath) :
    m_is_initialized(false),
    mp_descriptor(nullptr)
{
    QFile file(_filepath);
    if(file.open(QFile::ReadOnly))
        return;
    if(!readPrimaryVolumeDescriptor(file))
        return;
    if(!readConfig(file))
        return;
    m_is_initialized = true;
}

Device::Iso9660Data::~Iso9660Data()
{
    delete mp_descriptor;
}

bool Device::Iso9660Data::readPrimaryVolumeDescriptor(QFile & _file)
{
    if(!_file.seek(ISO9660_OFFSET))
        return false;
    QScopedPointer<VolumeDescriptor> descriptor_ptr(new VolumeDescriptor());
    if(_file.read(reinterpret_cast<char *>(descriptor_ptr.data()), sizeof(VolumeDescriptor)) != sizeof(VolumeDescriptor))
        return false;
    if(descriptor_ptr->type != VolumeDescriptorType::PrimaryVolumeDescriptor)
        return false;
    if(strncmp("CD001", descriptor_ptr->id, 5))
        return false;
    mp_descriptor = reinterpret_cast<PrimaryVolumeDescriptor *>(descriptor_ptr.take());
    return true;
}

bool Device::Iso9660Data::readConfig(QFile & _file)
{
    qint32 data_location = mp_descriptor->root_directory.extent_location * mp_descriptor->block_size;
    qint32 data_length = mp_descriptor->root_directory.data_length;
    if(!_file.seek(data_location))
        return false;
    char * data = new char[data_length];
    if(!_file.read(data, data_length))
    {
        delete [] data;
        return false;
    }
    char * data_ptr = nullptr;
    bool result = false;
    for(int32_t processed = 0; processed < data_length;)
    {
        data_ptr = data + processed;
        FileRecord * record = readFileRecord(data_ptr);
        if(record == nullptr)
            break;
        processed += record->record_length;
        if(strcmp("SYSTEM.CNF;1", record->filename) == 0)
        {
            result = parseConfig(_file, record);
            break;
        }
        delete record;;
    }
    delete [] data;
    return result;
}

FileRecord * Device::Iso9660Data::readFileRecord(const char * _start_ptr)
{
    const int8_t recodrd_length = *reinterpret_cast<const int8_t *>(_start_ptr);
    if(recodrd_length == 0)
        return nullptr;
    FileRecord * record = new FileRecord();
    memcpy(record, _start_ptr, recodrd_length);
    return record;
}

bool Device::Iso9660Data::parseConfig(QFile & _file, FileRecord * _file_record)
{
    int32_t file_location = _file_record->extent_location * mp_descriptor->block_size;
    if(!_file.seek(file_location))
        return false;
    QByteArray config = _file.read(_file_record->data_length);
    if(config.size() < _file_record->data_length)
        return false;
    return readGameId(config);
}

bool Device::Iso9660Data::readGameId(const QByteArray & _config)
{
    QRegExp regexp("BOOT\\d*\\s*=\\s*cdrom0:\\\\(.*);1", Qt::CaseInsensitive);
    QString data_string = QString::fromUtf8(_config);
    if(regexp.indexIn(data_string) >= 0)
    {
        m_game_id = regexp.cap(1);
        return !m_game_id.isEmpty();
    }
    return false;
}

bool Device::Iso9660Data::isInitialized() const
{
    return m_is_initialized;
}

qint16 Device::Iso9660Data::blockSize() const
{
    return m_is_initialized ? mp_descriptor->block_size : -1;
}

qint32 Device::Iso9660Data::blockCount() const
{
    return m_is_initialized ? mp_descriptor->block_count : -1;
}

bool Device::Iso9660Data::isPlayStationDisc() const
{
    if(!m_is_initialized)
        return false;
    const char psid[] = "PLAYSTATION";
    return strncmp(psid, mp_descriptor->system_id, sizeof(psid)) == 0;
}

const QString & Device::Iso9660Data::gameId() const
{
    return m_game_id;
}

QString Device::Iso9660Data::title() const
{
    return m_is_initialized ?
        QString::fromLatin1(mp_descriptor->volume_id, sizeof(PrimaryVolumeDescriptor::volume_id)).trimmed() :
        QString();
}

Device::Device(const QString & _filepath) :
    m_is_initialized(false),
    m_filepath(_filepath),
    mp_iso9660(nullptr),
    mp_read_file(nullptr)
{
}

Device::~Device()
{
    delete mp_iso9660;
    delete mp_read_file;
}

const QString & Device::filepath() const
{
    return m_filepath;
}

const Device::Iso9660Data * Device::iso9660() const
{
    return mp_iso9660;
}

bool Device::init()
{
    m_is_initialized = initialize();
    return m_is_initialized;
}

bool Device::initialize()
{
    if(mp_iso9660 == nullptr)
        mp_iso9660 = new Iso9660Data(m_filepath);
    return mp_iso9660->isInitialized();
}

bool Device::isInitialized() const
{
    return m_is_initialized;
}

bool Device::isPlayStationDisc() const
{
    return mp_iso9660->isPlayStationDisc();
}

QString Device::title() const
{
    return m_is_initialized ? mp_iso9660->title() : QString();
}

const QString Device::gameId() const
{
    return m_is_initialized ? mp_iso9660->gameId() : QString();
}

bool Device::open()
{
    close();
    mp_read_file = new QFile(m_filepath);
    if(mp_read_file->open(QFile::ReadOnly))
        return true;
    close();
    return false;
}

void Device::close()
{
    delete mp_read_file;
    mp_read_file = nullptr;
}

bool Device::isOpen() const
{
    return mp_read_file && mp_read_file->isOpen();
}

bool Device::seek(quint64 _offset)
{
    return mp_read_file && mp_read_file->seek(_offset);
}

quint64 Device::read(char * _buffer, quint64 _max_length)
{
    if(mp_read_file == nullptr)
        return 0;
    return mp_read_file->read(_buffer, _max_length);
}

OpticalDrive::OpticalDrive(const QString & _filepath) :
    Device(_filepath)
{
}

bool OpticalDrive::initialize()
{
    bool result = Device::initialize();
    if(result)
        m_media_type = getMediaType(filepath());
    return result;
}

size_t OpticalDrive::size() const
{
    const Iso9660Data * iso = iso9660();
    return iso == nullptr ? 0 : iso->blockCount() * iso->blockSize();
}

MediaType OpticalDrive::mediaType() const
{
    return m_media_type;
}

Iso9660Image::Iso9660Image(const QString & _filepath) :
    Device(_filepath),
    m_media_type(MediaType::unknown)
{
}

bool Iso9660Image::initialize()
{
    if(!Device::initialize())
        return false;
    QFileInfo info(filepath());
    if(!info.exists())
        return false;
    m_size = info.size();
    return true;
}

size_t Iso9660Image::size() const
{
    return m_size;
}

MediaType Iso9660Image::mediaType() const
{
    return m_media_type;
}

void Iso9660Image::setMediaType(MediaType _media_type)
{
    m_media_type = _media_type;
}
