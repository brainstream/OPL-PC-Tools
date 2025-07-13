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

#pragma once

#include "ui_ChooseImportGamesDialog.h"
#include <OplPcTools/GameCollection.h>
#include <QSet>

namespace OplPcTools {
namespace UI {

class ChooseImportGamesDialog : public QDialog, private Ui::ChooseImportGamesDialog
{
    Q_OBJECT

public:
    explicit ChooseImportGamesDialog(const GameCollection & _game_collection, QWidget * _parent = nullptr);
    QSet<Uuid> selectedGameIds() const { return m_selected_games; }

private:
    void updateUiState();
    void onTreeItemChanged(QTreeWidgetItem * _item);

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    void onSelectAllCheckboxStateChanged(Qt::CheckState _state);
#else
    void onSelectAllCheckboxStateChanged(int _state);
#endif

private:
    int m_total_games_count;
    QSet<Uuid> m_selected_games;
};

} // namespace UI
} // namespace OplPcTools
