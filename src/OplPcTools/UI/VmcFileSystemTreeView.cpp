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

#include <OplPcTools/UI/VmcFileSystemTreeView.h>
#include <QDragEnterEvent>
#include <QMimeData>

using namespace OplPcTools::UI;

VmcFileSystemTreeView::VmcFileSystemTreeView(QWidget * _parent) :
    QTreeView(_parent)
{
}

void VmcFileSystemTreeView::dragEnterEvent(QDragEnterEvent * _event) {
    if(isAccepatableDropData(_event->mimeData()))
    {
        _event->setDropAction(Qt::CopyAction);
        _event->accept();
    }
    else
    {
        QTreeView::dragEnterEvent(_event);
    }
}

bool VmcFileSystemTreeView::isAccepatableDropData(const QMimeData * _data)
{
    return _data->hasUrls();
}

void VmcFileSystemTreeView::dragMoveEvent(QDragMoveEvent * _event)
{
    if(isAccepatableDropData(_event->mimeData()))
        _event->acceptProposedAction();
    else
    {
        QTreeView::dragMoveEvent(_event);
    }
}

void VmcFileSystemTreeView::dropEvent(QDropEvent * _event)
{
    emit filesDropped(*_event->mimeData());
}
