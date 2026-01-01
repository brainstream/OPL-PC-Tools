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

#include <QStringList>

namespace OplPcTools {

class TextEncoding final
{
private:
    TextEncoding();

public:
    static QStringList availableCodecs();
    static constexpr QString latin1() { return "ISO-8859-1"; }
};

class TextDecoder final
{
    Q_DISABLE_COPY(TextDecoder)

private:
    class Private;

public:
    explicit TextDecoder(const QString & _codec);
    TextDecoder(TextDecoder && _decoder);
    ~TextDecoder();
    TextDecoder & operator = (TextDecoder && _decoder);
    QString codecName() const;
    QString decode(const QByteArray & _bytes) const;

private:
    Private * mp_private;
};

} // namespace OplPcTools
