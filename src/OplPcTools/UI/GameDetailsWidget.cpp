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

#include <QDebug>
#include <OplPcTools/UI/GameDetailsWidget.h>
#include "ui_GameDetailsWidget.h"

using namespace OplPcTools::UI;

class GameDetailsWidget::UITemplate : public Ui::GameDetailsWidget { };

GameDetailsWidget::GameDetailsWidget(UIContext & _context, QWidget * _parent /*= nullptr*/) :
    QWidget(_parent),
    mp_ui(new UITemplate),
    mr_context(_context)
{
    mp_ui->setupUi(this);
    connect(mp_ui->btn_close, &QPushButton::clicked, this, &GameDetailsWidget::deleteLater);
}

GameDetailsWidget::~GameDetailsWidget()
{
    delete mp_ui;
    qDebug() << "GameDetailsWidget destroyed";
}

void GameDetailsWidget::setGameId(const QString & _id)
{
    m_game_id = _id;
}

const QString & GameDetailsWidget::gameId() const
{
    return m_game_id;
}

void GameDetailsWidget::showEvent(QShowEvent * _event)
{
    Q_UNUSED(_event)
    init();
}

void GameDetailsWidget::init()
{
    mp_ui->label_game_title->setText(m_game_id);
}
