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

#ifdef _WIN32

#include <windows.h>
#include <cdio/iso9660.h>
#include "Win32CdIoHack.h"


namespace {

HANDLE handle(void * _user_data)
{
    // Access to the h_device_handle field of the _img_private_t structure, defined in win32.h (libcdio)
    return *reinterpret_cast<HANDLE *>(&reinterpret_cast<unsigned char *>(_user_data)[6404]);
}

__cdecl off_t lseekHack(void * _user_data, off_t _offset, int _whence)
{
    DWORD method;
    switch(_whence)
    {
    case SEEK_SET:
        method = FILE_BEGIN;
        break;
    case SEEK_END:
        method = FILE_END;
        break;
    default:
        method = FILE_CURRENT;
        break;
    }
    DWORD result = SetFilePointer(handle(_user_data), _offset, nullptr, method);
    return result;
}

__cdecl ssize_t readHack(void * _user_data, void * _buff, size_t _size)
{
    DWORD read_bytes;
    if(ReadFile(handle(_user_data), _buff, _size, &read_bytes, nullptr))
        return read_bytes;
    return -1;
}

} // namespace

void hackCdIo(CdIo_t * _cdio)
{
    // cdio_private.h. Get the op field is the second field in the _CdIo struct.
    unsigned char * funcs = &reinterpret_cast<unsigned char *>(_cdio)[sizeof(driver_id_t)];
    // Installation the missing functions.
    // For some reason, the libcdio sets NULL in the lseek and the read fiels.
    void * lseek_ptr = reinterpret_cast<void *>(&lseekHack);
    void * read_ptr = reinterpret_cast<void *>(&readHack);
    memcpy(&funcs[34 * sizeof(void *)], &lseek_ptr, sizeof(void *));
    memcpy(&funcs[35 * sizeof(void *)], &read_ptr, sizeof(void *));
}

#endif
