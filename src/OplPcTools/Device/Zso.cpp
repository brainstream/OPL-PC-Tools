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

#include <OplPcTools/Device/Zso.h>
#include <lz4.h>

using namespace OplPcTools;

int OplPcTools::compressZsoBlock(const QByteArray & _source, QByteArray & _destination)
{
    return LZ4_compress_default(_source.data(), _destination.data(), _source.size(), _destination.size());
}

bool OplPcTools::decompressZsoBlock(const QByteArray & _source, QByteArray & _destination)
{
    return LZ4_decompress_safe_partial(
        _source.constData(),
        _destination.data(),
        _source.size(),
        _destination.size(),
        _destination.size()) > 0;
}
