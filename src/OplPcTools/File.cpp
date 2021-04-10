/***********************************************************************************************
 * Copyright © 2017-2021 Sergey Smolyannikov aka brainstream                                   *
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

#ifdef __linux
#   include <fcntl.h>
#endif

#include <OplPcTools/FilenameValidator.h>
#include <OplPcTools/File.h>

namespace {

#ifdef __linux
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

QSharedPointer<QFile> openFileToSyncWrite(const QString & _filename)
{
    QSharedPointer<QFile> file = QSharedPointer<QFile>(new SynchronouslyWritableFile(_filename));
    QIODevice::OpenMode mode = QIODevice::WriteOnly;
#ifdef __linux
    int fd = open(_filename.toLocal8Bit(), O_SYNC | O_RDWR | O_CREAT, 0664);
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
        if(FilenameValidator::s_disallowed_characters.contains(chr))
            return false;
    }
    return true;
}

void validateFilename(const QString & _filename)
{
    if(!isFilenameValid(_filename))
        throw ValidationException(QString("%1: %2")
            .arg(QObject::tr("The following characters are not allowed"))
            .arg(FilenameValidator::s_disallowed_characters)
        );
}

} // namespace OplPcTools
