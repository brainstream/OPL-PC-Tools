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

#include <OplPcTools/UI/ProgressBarItemDelegate.h>
#include <QApplication>
#include <QPainter>

using namespace OplPcTools::UI;

ProgressBarItemDelegate::ProgressBarItemDelegate(const ProgressBarItemDelegateSource & _source, QObject * _parent) :
    QStyledItemDelegate(_parent),
    mr_source(_source)
{
}

void ProgressBarItemDelegate::paint(
    QPainter * _painter,
    const QStyleOptionViewItem & _option,
    const QModelIndex & _index) const
{
    QStyledItemDelegate::paint(_painter, _option, _index);
    if(!mr_source.isProgressBarEnabled(_index))
        return;

    QStyleOptionProgressBar progress_bar_option = {};
    progress_bar_option.textVisible = true;
    progress_bar_option.state = QStyle::State_Enabled;
    progress_bar_option.direction = QApplication::layoutDirection();
    progress_bar_option.state |= QStyle::State_Horizontal | QStyle::State_Enabled;
    progress_bar_option.rect = _option.rect;
    progress_bar_option.minimum = 0;
    progress_bar_option.maximum = mr_source.maxProgressValue(_index);
    progress_bar_option.progress = mr_source.currentProgressValue(_index);

    int progress = progress_bar_option.progress;
    if(progress_bar_option.maximum > 100)
        progress = static_cast<int>(progress * (100.0f / progress_bar_option.maximum));

    progress_bar_option.text = QString::number(progress) + "%";

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progress_bar_option, _painter);
}
