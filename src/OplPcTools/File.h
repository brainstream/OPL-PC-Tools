/***********************************************************************************************
 * Copyright © 2017-2026 Sergey Smolyannikov aka brainstream                                   *
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

#include <OplPcTools/Exception.h>
#include <QSharedPointer>
#include <QFile>

namespace OplPcTools {

inline void openFile(QFile & _file, QIODevice::OpenMode _flags)
{
    if(!_file.open(_flags))
        throw IOException(QObject::tr("Unable to open file \"%1\"").arg(_file.fileName()));
}

enum __OpenFileSyncModes
{
    OFSM_READ = 0x1,
    OFSM_WRITE = 0x2,
    OFSM_READ_WRITE = OFSM_READ | OFSM_WRITE,
    OFSM_TRUNCATE = 0x4,
    OFSM_APPEND = 0x8,
    OFSM_CREATE = 0x10
};

Q_DECLARE_FLAGS(OpenFileSyncMode, __OpenFileSyncModes)
Q_DECLARE_OPERATORS_FOR_FLAGS(OpenFileSyncMode)

QSharedPointer<QFile> createFileToSyncWrite(const QString & _filename);
QSharedPointer<QFile> openFileToSyncWrite(const QString & _filename, OpenFileSyncMode _mode);

inline void renameFile(const QString & _old_filename, const QString & _new_filename)
{
    if(!QFile::rename(_old_filename, _new_filename))
        throw IOException(QObject::tr("Unable to rename file \"%1\" to \"%2\"").arg(_old_filename, _new_filename));
}

inline qint64 writeFile(QFile & _file, const char * _data, qint64 _length)
{
    qint64 write_bytes = _file.write(_data, _length);
    if(write_bytes <= 0)
        throw IOException(QObject::tr("Unable to write a data into the file: \"%1\"").arg(_file.fileName()));
    return write_bytes;
}

inline qint64 readFile(QFile & _file, char * _buffer, qint64 _length)
{
    qint64 read_bytes = _file.read(_buffer, _length);
    if(read_bytes < 0)
        throw IOException(QObject::tr("Unable to read the file: \"%1\"").arg(_file.fileName()));
    return read_bytes;
}

inline void removeFile(QFile & _file)
{
    if(!_file.remove())
        throw IOException(QObject::tr("Unable to delete file: \"%1\"").arg(_file.fileName()));
}

inline void removeFile(const QString & _filename)
{
    QFile file(_filename);
    removeFile(file);
}

constexpr char g_filename_forbidden_characters[] = "<>:\"/\\|?*";

bool isFilenameValid(const QString & _filename);

} // namespace OplPcTools
