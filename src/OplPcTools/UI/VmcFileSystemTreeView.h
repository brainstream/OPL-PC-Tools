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

#include <QTreeView>

namespace OplPcTools {
namespace UI {

class VmcFileSystemTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit VmcFileSystemTreeView(QWidget * _parent = nullptr);

signals:
    void filesDropped(const QDropEvent & _event);

protected:
    void dragEnterEvent(QDragEnterEvent * _event) override;
    void dragMoveEvent(QDragMoveEvent * _event) override;
    void dropEvent(QDropEvent * _event) override;

private:
    bool isAccepatableDropData(const QMimeData * _data);
};

} // namespace UI
} // namespace OplPcTools
