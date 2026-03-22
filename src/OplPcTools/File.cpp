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

#ifdef __linux
#   include <fcntl.h>
#endif

#include <OplPcTools/File.h>

using namespace OplPcTools;

namespace {

QIODevice::OpenMode openFileSyncModeToQtMode(OpenFileSyncMode _mode)
{
    QIODevice::OpenMode result = QIODevice::NotOpen;
    if(_mode & OFSM_READ) result |= QIODevice::ReadOnly;
    if(_mode & OFSM_WRITE) result |= QIODevice::WriteOnly;
    if(_mode & OFSM_TRUNCATE) result |= QIODevice::Truncate;
    if(_mode & OFSM_APPEND) result |= QIODevice::Append;
    if((_mode & OFSM_CREATE) == 0) result |= QIODevice::ExistingOnly;
    return result;
}

#ifdef __linux
int openFileSyncModeToStdMode(OpenFileSyncMode _mode)
{
    int result = 0;

    if((_mode & OFSM_READ_WRITE) == OFSM_READ_WRITE) result |= O_RDWR;
    else if(_mode & OFSM_READ) result |= O_RDONLY;
    else if(_mode & OFSM_WRITE) result |= O_WRONLY;

    if(_mode & OFSM_TRUNCATE) result |= O_TRUNC;
    if(_mode & OFSM_APPEND) result |= O_APPEND;
    if(_mode & OFSM_CREATE) result |= O_CREAT;

    return result;
}

class SynchronouslyWritableFile : public QFile
{
public:
    explicit SynchronouslyWritableFile(const QString & _filename) :
        QFile(_filename)
    {
        m_filename = QFile::fileName();
    }

    QString fileName() const override
    {
        return m_filename;
    }

private:
    QString m_filename;
};
#else
typedef QFile SynchronouslyWritableFile;
#endif

} // namespace {

namespace OplPcTools {

QSharedPointer<QFile> openFileToSyncWrite(const QString & _filename, OpenFileSyncMode _mode)
{
    QSharedPointer<QFile> file = QSharedPointer<QFile>(new SynchronouslyWritableFile(_filename));
    QIODevice::OpenMode mode = openFileSyncModeToQtMode(_mode);
#ifdef __linux
    int fd = open(_filename.toLocal8Bit(), openFileSyncModeToStdMode(_mode) | O_SYNC, 0664);
    bool result = fd >= 0 && file->open(fd, mode, QFile::AutoCloseHandle);
#else
    bool result = file->open(mode);
#endif
    if(!result)
        throw IOException(QObject::tr("Unable to open file to write: \"%1\"").arg(_filename));
    return file;
}

bool isFilenameValid(const QString & _filename)
{
    for(const QChar & chr : _filename)
    {
        for(size_t i = 0; g_filename_forbidden_characters[i]; ++i)
            if(chr == g_filename_forbidden_characters[i]) return false;
    }
    return true;
}

} // namespace OplPcTools
