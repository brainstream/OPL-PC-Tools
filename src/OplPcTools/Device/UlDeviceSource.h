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

#include <OplPcTools/Device/DeviceSource.h>
#include <OplPcTools/Game.h>
#include <QFile>
#include <memory>

namespace OplPcTools {

class UlDeviceSource : public DeviceSource
{
private:
    struct Part final
    {
        Part(const QString & _path, qint64 _begin, qint64 _end) :
            path(_path),
            begin(_begin),
            end(_end),
            next(nullptr)
        {
        }

        ~Part()
        {
            delete next;
        }

        QString path;
        qint64 begin;
        qint64 end;
        Part * next;
    };

public:
    explicit UlDeviceSource(const Game & _game);
    QString filepath() const override;
    bool open() override;
    bool isOpen() const override;
    void close() override;
    qint64 isoSize() const override;
    bool seek(qint64 _offset) override;
    qint64 read(QByteArray & _buffer) override;

private:
    void openPart(const Part & _part);

private:
    Game m_game;
    qint64 m_size;
    std::unique_ptr<Part> m_parts_ptr;
    const Part * mp_open_part;
    std::unique_ptr<QFile> m_file_ptr;
};

} // namespace OplPcTools
