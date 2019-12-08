/***********************************************************************************************
 * Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                   *
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

#ifndef __OPLPCTOOLS_CLICKABLELABEL__
#define __OPLPCTOOLS_CLICKABLELABEL__

#include <QLabel>
#include <QMouseEvent>

namespace OplPcTools {
namespace UI {

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget * _parent = nullptr, Qt::WindowFlags _flags = Qt::WindowFlags()) :
        QLabel(_parent, _flags),
        m_pressed(false),
        m_mouse_in(false)
    {
    }

    explicit ClickableLabel(const QString & _text, QWidget * _parent = nullptr, Qt::WindowFlags _flags = Qt::WindowFlags()) :
        QLabel(_text, _parent, _flags),
        m_pressed(false),
        m_mouse_in(false)
    {
    }

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent * _event) override
    {
        m_pressed = _event->button() == Qt::LeftButton;
    }

    void mouseReleaseEvent(QMouseEvent * _event) override
    {
        if(_event->button() != Qt::LeftButton)
            return;
        m_mouse_in = rect().contains(_event->pos());
        if(m_pressed && m_mouse_in)
        {
            m_pressed = false;
            emit clicked();
        }
    }

    void enterEvent(QEvent *) override
    {
        m_mouse_in = true;
        QFont fnt = font();
        fnt.setUnderline(true);
        setFont(fnt);
    }

    void leaveEvent(QEvent *) override
    {
        m_pressed = false;
        m_mouse_in = false;
        QFont fnt = font();
        fnt.setUnderline(false);
        setFont(fnt);
    }

private:
    bool m_mouse_in;
    bool m_pressed;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_CLICKABLELABEL__
