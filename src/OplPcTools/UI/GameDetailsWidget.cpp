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

using namespace OplPcTools;
using namespace OplPcTools::UI;

GameDetailsWidget::GameDetailsWidget(UIContext & _context, QWidget * _parent /*= nullptr*/) :
    QWidget(_parent),
    mr_context(_context),
    mp_game(nullptr)
{
    setupUi(this);
    init();
    connect(mp_btn_close, &QPushButton::clicked, this, &GameDetailsWidget::deleteLater);
}

void GameDetailsWidget::setGameId(const QString & _id)
{
    mp_game = mr_context.collection().findGame(_id);
    init();
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
    if(mp_game == nullptr)
    {
        clear();
        return;
    }
    // TODO: fill out the mp_list_arts
    mp_edit_title->setDisabled(false);
    mp_edit_title->setText(mp_game->title());
    mp_btn_title_edit_accept->setDisabled(false);
    mp_btn_title_edit_cancel->setDisabled(false);
    mp_widget_art_details->hide();
    mp_edit_title->selectAll();
    mp_edit_title->setFocus();
    // TODO: activate a first art
}

void GameDetailsWidget::clear()
{
    mp_list_arts->clear();
    mp_edit_title->setDisabled(true);
    mp_btn_title_edit_accept->setDisabled(true);
    mp_btn_title_edit_cancel->setDisabled(true);
    mp_widget_art_details->hide();
}
