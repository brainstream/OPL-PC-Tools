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

#include <OplPcTools/UI/ChooseImportGamesDialog.h>
#include <QPushButton>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

class GameListItem : public QTreeWidgetItem
{
public:
    explicit GameListItem(const Game & _game, QTreeWidget * _widget);
    QVariant data(int _column, int _role) const override;

    const Uuid & gameId() const
    {
        return mr_game.uuid();
    }

private:
    const Game & mr_game;
};

} // namespace

GameListItem::GameListItem(const Game & _game, QTreeWidget * _widget) :
    QTreeWidgetItem(_widget, UserType),
    mr_game(_game)
{
    setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren);
    setCheckState(0, Qt::Unchecked);
}

QVariant GameListItem::data(int _column, int _role) const
{
    switch(_role)
    {
    case Qt::DisplayRole:
        if(_column == 0)
            return mr_game.title();
        else if(_column == 1)
        {
            if(mr_game.installationType() == GameInstallationType::UlConfig)
            {
                return QString("UL");
            }
            else if(mr_game.mediaType() == MediaType::CD)
            {
                return QString("CD");
            }
            else if(mr_game.mediaType() == MediaType::DVD)
            {
                return QString("DVD");
            }
        }
        break;
    default:
        break;
    }
    return QTreeWidgetItem::data(_column, _role);
}

ChooseImportGamesDialog::ChooseImportGamesDialog(const GameCollection & _game_collection, QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint)
{
    setupUi(this);
    mp_tree_games->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mp_tree_games->sortItems(0, Qt::AscendingOrder);
    updateUiState();
    connect(mp_button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(mp_button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(mp_tree_games, &QTreeWidget::itemChanged, this, &ChooseImportGamesDialog::onTreeItemChanged);
    connect(mp_checkbox_select_all,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        &QCheckBox::checkStateChanged,
#else
        &QCheckBox::stateChanged,
#endif
        this,
        &ChooseImportGamesDialog::onSelectAllCheckboxStateChanged);
    m_total_games_count = _game_collection.count();
    for(int i = 0; i < m_total_games_count; ++i)
    {
        mp_tree_games->addTopLevelItem(new GameListItem(*_game_collection[i], mp_tree_games));
    }
}

void ChooseImportGamesDialog::updateUiState()
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

void ChooseImportGamesDialog::onTreeItemChanged(QTreeWidgetItem * _item)
{
    GameListItem * gli = static_cast<GameListItem *>(_item);
    if(gli->checkState(0))
        m_selected_games.insert(gli->gameId());
    else
        m_selected_games.remove(gli->gameId());
    updateUiState();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    void ChooseImportGamesDialog::onSelectAllCheckboxStateChanged(Qt::CheckState _state)
#else
    void ChooseImportGamesDialog::onSelectAllCheckboxStateChanged(int _state)
#endif
{
    switch(_state)
    {
    case Qt::Checked:
        for(int i = 0; i < m_total_games_count; ++i)
            mp_tree_games->topLevelItem(i)->setCheckState(0, Qt::Checked);
        break;
    case Qt::Unchecked:
        for(int i = 0; i < m_total_games_count; ++i)
            mp_tree_games->topLevelItem(i)->setCheckState(0, Qt::Unchecked);
        break;
    default:
        break;
    }
}
