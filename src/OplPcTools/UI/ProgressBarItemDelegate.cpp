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
    _painter->eraseRect(_option.rect);

    QStyledItemDelegate::paint(_painter, _option, _index);
    if(!mr_source.isProgressBarEnabled(_index))
        return;

    const int maximum = mr_source.maxProgressValue(_index);
    const int current = mr_source.currentProgressValue(_index);
    QStyleOptionProgressBar progress_bar_option = { };
    progress_bar_option.state = QStyle::State_Enabled;
    progress_bar_option.direction = _option.direction;
    progress_bar_option.rect = _option.rect;
    progress_bar_option.fontMetrics = _option.fontMetrics;
    progress_bar_option.minimum = 0;
    progress_bar_option.maximum = maximum;
    progress_bar_option.textAlignment = Qt::AlignCenter;
    progress_bar_option.textVisible = true;
    progress_bar_option.progress = current;
    progress_bar_option.text = QString::asprintf("%d%%", progress_bar_option.progress / (maximum / 100));
    QStyleOption progress_indicator_option = { };
    progress_indicator_option.state = QStyle::State_Enabled;
    progress_indicator_option.direction = _option.direction;
    progress_indicator_option.rect = _option.rect;
    progress_indicator_option.rect.setWidth(
        progress_indicator_option.rect.width() * progress_bar_option.progress / maximum);
    progress_indicator_option.fontMetrics = _option.fontMetrics;
    progress_indicator_option.palette.setColor(QPalette::Highlight,
        progress_indicator_option.palette.color(QPalette::Highlight).darker(150));
    QStyle * style = QApplication::style();
    style->drawPrimitive(QStyle::PE_IndicatorProgressChunk, &progress_indicator_option, _painter);
    style->drawControl(QStyle::CE_ProgressBarLabel, &progress_bar_option, _painter);
}
