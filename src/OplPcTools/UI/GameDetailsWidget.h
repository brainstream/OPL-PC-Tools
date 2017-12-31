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

#ifndef __OPLPCTOOLS_GAMEDETAILSWIDGET__
#define __OPLPCTOOLS_GAMEDETAILSWIDGET__

#include <QWidget>
#include <OplPcTools/UI/UIContext.h>
#include "ui_GameDetailsWidget.h"

namespace OplPcTools {
namespace UI {

class GameDetailsWidget : public QWidget, private Ui::GameDetailsWidget
{
    Q_OBJECT

public:
    explicit GameDetailsWidget(UIContext & _context, QWidget * _parent = nullptr);
    void setGameId(const QString & _id);
    const QString & gameId() const;

protected:
    void showEvent(QShowEvent * _event) override;

private:
    void init();

private:
    UIContext & mr_context;
    QString m_game_id;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMEDETAILSWIDGET__
