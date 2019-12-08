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

#include <QRegExp>
#include <QFileInfo>
#include <OplPcTools/Device.h>

#define ISO9660_OFFSET 0x8000

using namespace OplPcTools;

namespace {

template<typename IntType>
struct LBInt
{
    IntType le;
    IntType be;

    operator IntType() const
    {
#if BYTE_ORDER == LITTLE_ENDIAN
        return le;
#else
        return be;
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

class Iso9660 final
{
    Q_DISABLE_COPY(Iso9660)

public:
    explicit Iso9660(DeviceSource & _source);
    ~Iso9660();
    inline bool isInitialized() const;
    inline qint16 blockSize() const;
    inline qint32 blockCount() const;
    inline bool isPlayStationDisc() const;
    inline QString title() const;
    inline const QString & gameId() const;

private:
    bool readPrimaryVolumeDescriptor(DeviceSource & _source);
    bool readConfig(DeviceSource & _source);
    bool parseConfig(DeviceSource & _source, const FileRecord * _file_record);
    bool readGameId(const QByteArray & _config);

private:
    bool m_is_initialized;
    PrimaryVolumeDescriptor * mp_descriptor;
    QString m_game_id;
};

} // namespace

Iso9660::Iso9660(DeviceSource & _source) :
    m_is_initialized(false),
    mp_descriptor(nullptr)
{
    if(!readPrimaryVolumeDescriptor(_source))
        return;
    if(!readConfig(_source))
        return;
    m_is_initialized = true;
}

Iso9660::~Iso9660()
{
    delete mp_descriptor;
}

bool Iso9660::readPrimaryVolumeDescriptor(DeviceSource & _source)
{
    if(!_source.seek(ISO9660_OFFSET))
        return false;
    QByteArray buffer(sizeof(VolumeDescriptor), Qt::Uninitialized);
    if(_source.read(buffer) != sizeof(VolumeDescriptor))
        return false;
    VolumeDescriptor * descriptor = reinterpret_cast<VolumeDescriptor *>(buffer.data());
    if(descriptor->type != VolumeDescriptorType::PrimaryVolumeDescriptor)
        return false;
    if(strncmp("CD001", descriptor->id, 5))
        return false;
    mp_descriptor = new PrimaryVolumeDescriptor;
    memcpy(mp_descriptor, descriptor, sizeof(VolumeDescriptor));
    return true;
}

bool Iso9660::readConfig(DeviceSource & _source)
{
    qint32 data_location = mp_descriptor->root_directory.extent_location * mp_descriptor->block_size;
    qint32 data_length = mp_descriptor->root_directory.data_length;
    if(!_source.seek(data_location))
        return false;
    QByteArray buffer(data_length, Qt::Uninitialized);
    if(_source.read(buffer) <= 0)
        return false;
    const char * data_ptr = buffer.constData();
    bool result = false;
    for(int32_t processed = 0; processed < data_length;)
    {
        data_ptr = buffer.constData() + processed;
        const FileRecord * record = reinterpret_cast<const FileRecord *>(data_ptr);
        if(record->record_length == 0)
            break;
        processed += record->record_length;
        if(strcmp("SYSTEM.CNF;1", record->filename) == 0)
            result = parseConfig(_source, record);
        if(result) break;
    }
    return result;
}

bool Iso9660::parseConfig(DeviceSource & _source, const FileRecord * _file_record)
{
    quint32 file_location = _file_record->extent_location * mp_descriptor->block_size;
    if(!_source.seek(file_location))
        return false;
    QByteArray config(_file_record->data_length, Qt::Uninitialized);
    if(_source.read(config) < _file_record->data_length)
        return false;
    return readGameId(config);
}

bool Iso9660::readGameId(const QByteArray & _config)
{
    QRegExp regexp("BOOT2\\s*=\\s*cdrom0:\\\\(.*);1", Qt::CaseInsensitive);
    QString data_string = QString::fromUtf8(_config);
    if(regexp.indexIn(data_string) >= 0)
    {
        m_game_id = regexp.cap(1);
        return !m_game_id.isEmpty();
    }
    return false;
}

bool Iso9660::isInitialized() const
{
    return m_is_initialized;
}

qint16 Iso9660::blockSize() const
{
    return m_is_initialized ? mp_descriptor->block_size : -1;
}

qint32 Iso9660::blockCount() const
{
    return m_is_initialized ? mp_descriptor->block_count : -1;
}

bool Iso9660::isPlayStationDisc() const
{
    if(!m_is_initialized)
        return false;
    const char psid[] = "PLAYSTATION";
    return strncmp(psid, mp_descriptor->system_id, strlen(psid)) == 0;
}

const QString & Iso9660::gameId() const
{
    return m_game_id;
}

QString Iso9660::title() const
{
    return m_is_initialized ?
        QString::fromLatin1(mp_descriptor->volume_id, sizeof(PrimaryVolumeDescriptor::volume_id)).trimmed() :
        QString();
}

Device::Device(QSharedPointer<DeviceSource> _source) :
    m_is_initialized(false),
    m_source_ptr(_source),
    m_media_type(MediaType::Unknown)
{
}

const QString Device::filepath() const
{
    return m_source_ptr->filepath();
}

bool Device::init()
{
    if(m_source_ptr->isOpen())
        m_source_ptr->seek(0);
    else if(!m_source_ptr->open())
        return false;
    Iso9660 * iso = new Iso9660(*m_source_ptr);
    if(iso->isInitialized() && iso->isPlayStationDisc())
    {
        m_title = iso->title();
        m_id = iso->gameId();
        m_size = static_cast<quint64>(iso->blockCount()) * iso->blockSize();
        m_is_initialized = true;
    }
    delete iso;
    m_source_ptr->seek(0);
    return m_is_initialized;
}

bool Device::isInitialized() const
{
    return m_is_initialized;
}

bool Device::open()
{
    close();
    return m_source_ptr->open();
}

void Device::close()
{
    m_source_ptr->close();
}

bool Device::isOpen() const
{
    return m_source_ptr->isOpen();
}

bool Device::seek(quint64 _offset)
{
    return m_source_ptr->seek(_offset);
}

qint64 Device::read(QByteArray & _buffer)
{
    return m_source_ptr->read(_buffer);
}
