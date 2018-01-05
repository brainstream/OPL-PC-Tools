/***********************************************************************************************
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

namespace OplPcTools {
namespace UI {

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget * _parent = nullptr, Qt::WindowFlags _flags = Qt::WindowFlags()) :
        QLabel(_parent, _flags)
    {
    }

    explicit ClickableLabel(const QString & _text, QWidget * _parent = nullptr, Qt::WindowFlags _flags = Qt::WindowFlags()) :
        QLabel(_text, _parent, _flags)
    {
    }

signals:
    void doubleClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent *) override
    {
        emit doubleClicked();
    }
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_CLICKABLELABEL__
