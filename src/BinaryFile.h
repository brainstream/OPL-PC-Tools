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

#ifndef __OPLPCTOOLS_BINARYFILE__
#define __OPLPCTOOLS_BINARYFILE__

#include <QFile>

class BinaryFile : public QFile
{
public:
#ifdef _WIN32
    /*
     * The hack to read MS Windows drive files not supported by Qt. F/I: "\\.\E:"
     */

    explicit BinaryFile(const QString & _filepath);
    ~BinaryFile() override;
    bool open(OpenMode mode) override;
    void close() override;

private:
    void free();

private:
    FILE * mp_std_file;
#else // _WIN32
    explicit BinaryFile(const QString & _filepath) :
        QFile(_filepath)
    {
    }
#endif // _WIN32
};

#endif // __OPLPCTOOLS_BINARYFILE__
