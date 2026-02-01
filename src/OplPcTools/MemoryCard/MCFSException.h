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

namespace OplPcTools {
namespace MemoryCard {

DECLARE_EXCEPTION(MemoryCardFileSystemException)

class MemoryCardFileException : public MemoryCardFileSystemException
{
public:
    MemoryCardFileException(const QString & _message, const QByteArray & _path) :
        MemoryCardFileSystemException(_message),
        m_path(_path)
    {
    }

    const QByteArray & path() const
    {
        return m_path;
    }

private:
    const QByteArray m_path;
};

} // namespace MemoryCard
} // namespace OplPcTools
