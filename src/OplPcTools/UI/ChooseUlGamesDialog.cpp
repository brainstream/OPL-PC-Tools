/***********************************************************************************************
 * Copyright © 2017-2025 Sergey Smolyannikov aka brainstream                                   *
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

#include <OplPcTools/UI/ChooseUlGamesDialog.h>
#include <QPushButton>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

class GameListItem : public QListWidgetItem
{
public:
    explicit GameListItem(const Game & _game, QListWidget * _listview) :
        QListWidgetItem(_game.title(), _listview, UserType),
        m_game_id(_game.uuid())
    {
        setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren);
        setCheckState(Qt::Unchecked);
    }

    const Uuid & gameId() const
    {
        return m_game_id;
    }

private:
    const Uuid m_game_id;
};

} // namespace

ChooseUlGamesDialog::ChooseUlGamesDialog(const UlConfigGameStorage & _storage, QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint)
{
    setupUi(this);
    updateUiState();
    connect(mp_button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(mp_button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(mp_list_games, &QListWidget::itemChanged, this, &ChooseUlGamesDialog::onListItemChanged);
    connect(mp_checkbox_select_all,
        &QCheckBox::stateChanged,
        this,
        &ChooseUlGamesDialog::onSelectAllCheckboxStateChanged);
    m_total_games_count = _storage.count();
    for(int i = 0; i < m_total_games_count; ++i)
        mp_list_games->addItem(new GameListItem(*_storage[i], mp_list_games));
}

void ChooseUlGamesDialog::updateUiState()
{
    int selected_count = m_selected_games.count();
    mp_button_box->button(QDialogButtonBox::Ok)->setDisabled(selected_count == 0);
    mp_checkbox_select_all->blockSignals(true);
    if(selected_count < m_total_games_count)
        mp_checkbox_select_all->setCheckState(Qt::Unchecked);
    else
        mp_checkbox_select_all->setCheckState(Qt::Checked);
    mp_checkbox_select_all->blockSignals(false);
}

void ChooseUlGamesDialog::onListItemChanged(QListWidgetItem * _item)
{
    GameListItem * gli = static_cast<GameListItem *>(_item);
    if(gli->checkState())
        m_selected_games.insert(gli->gameId());
    else
        m_selected_games.remove(gli->gameId());
    updateUiState();
}

void ChooseUlGamesDialog::onSelectAllCheckboxStateChanged(int _state)
{
    switch(_state)
    {
    case Qt::Checked:
        for(int i = 0; i < m_total_games_count; ++i)
            mp_list_games->item(i)->setCheckState(Qt::Checked);
        break;
    case Qt::Unchecked:
        for(int i = 0; i < m_total_games_count; ++i)
            mp_list_games->item(i)->setCheckState(Qt::Unchecked);
        break;
    }
}
