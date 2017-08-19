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

#include "BinaryFile.h"

#ifdef _WIN32

BinaryFile::BinaryFile(const QString & _filepath) :
    QFile(_filepath),
    mp_std_file(nullptr)
{
}

BinaryFile::~BinaryFile()
{
    free();
}

bool BinaryFile::open(QIODevice::OpenMode mode)
{
    free();
    bool result = QFile::open(mode);
    if(result)
        return result;
    if(mode == ReadOnly)
    {
        mp_std_file = std::fopen(fileName().toLocal8Bit(), "rb");
        if(mp_std_file)
            result = QFile::open(mp_std_file, mode);
        if(!result)
            free();
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
    free();
}

#endif // _WIN32
