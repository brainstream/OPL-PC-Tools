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

#ifdef _WIN32
#   include <windows.h>
#endif
#include <OplPcTools/OpticalDriveDeviceSource.h>

using namespace OplPcTools;

namespace {

#ifdef _WIN32

/*
 * The hack to read MS Windows drive files not supported by Qt. F/I: "\\.\E:"
 */
class BinaryFile : public QFile
{
public:
    explicit BinaryFile(const QString & _filepath);
    ~BinaryFile() override;
    bool open(OpenMode mode) override;
    void close() override;

private:
    void free();

private:
    FILE * mp_std_file;

};

BinaryFile::BinaryFile(const QString & _filepath) :
    QFile(_filepath),
    mp_std_file(nullptr)
{
}

BinaryFile::~BinaryFile()
{
    this->free();
}

bool BinaryFile::open(QIODevice::OpenMode mode)
{
    this->free();
    bool result = QFile::open(mode);
    if(result)
        return result;
    if(mode == ReadOnly)
    {
        mp_std_file = std::fopen(fileName().toLocal8Bit(), "rb");
        if(mp_std_file)
            result = QFile::open(mp_std_file, mode);
        if(!result)
            this->free();
    }
    return result;
}

void BinaryFile::free()
{
    if(mp_std_file)
    {
        std::fclose(mp_std_file);
        mp_std_file = nullptr;
    }
}

void BinaryFile::close()
{
    QFile::close();
    this->free();
}

#else // _WIN32

using BinaryFile = QFile;

#endif // _WIN32

} // namespace


OpticalDriveDeviceSource::OpticalDriveDeviceSource(const QString & _filepath) :
    mp_file(new BinaryFile(_filepath))
{
}

OpticalDriveDeviceSource::~OpticalDriveDeviceSource()
{
    delete mp_file;
}

QString OpticalDriveDeviceSource::filepath() const
{
    return mp_file->fileName();
}

bool OpticalDriveDeviceSource::isReadOnly() const
{
    return true;
}

bool OpticalDriveDeviceSource::open()
{
    return mp_file->open(QIODevice::ReadOnly);
}

bool OpticalDriveDeviceSource::isOpen() const
{
    return mp_file->isOpen();
}

void OpticalDriveDeviceSource::close()
{
    mp_file->close();
}

bool OpticalDriveDeviceSource::seek(qint64 _offset)
{
    return mp_file->seek(_offset);
}

qint64 OpticalDriveDeviceSource::read(QByteArray & _buffer)
{
    qint64 result = mp_file->read(_buffer.data(), _buffer.size());
#ifdef _WIN32
    if(result < 0 && GetLastError() == ERROR_SECTOR_NOT_FOUND)
        return 0;
#endif
    return result;
}
