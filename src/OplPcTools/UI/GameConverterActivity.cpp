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

#include <OplPcTools/UI/GameConverterActivity.h>
#include <OplPcTools/UI/ChooseImportGamesDialog.h>
#include <OplPcTools/Library.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

class GameConverterActivityIntent : public Intent
{
public:
    Activity *createActivity(QWidget * _parent) override
    {
        return new GameConverterActivity(_parent);
    }

    QString activityClass() const override
    {
        return "GameConverter";
    }
};

enum class TargetFormat
{
    None,
    Ul,
    Iso,
    Zso
};

class GameListItem : public QTreeWidgetItem
{
public:
    explicit GameListItem(const Game & _game);
    QVariant data(int _column, int _role) const override;
    const Game & game() const;
    void setTargetFormat(TargetFormat _format);
    TargetFormat targetFormat() const;

private:
    QString targetToString() const;

private:
    const Game m_game;
    TargetFormat m_target_format;
};

} // namespace

QVariant GameListItem::data(int _column, int _role) const
{
    if(_role != Qt::DisplayRole)
        return QVariant();
    switch(_column)
    {
    case 0:
        return m_game.title();
    case 1:
        return m_game.formatName();
    case 2:
        return targetToString();
    }
    return QVariant();
}

inline GameListItem::GameListItem(const Game & _game) :
    QTreeWidgetItem(QTreeWidgetItem::UserType),
    m_game(_game),
    m_target_format(TargetFormat::None)
{
}

inline const Game & GameListItem::game() const
{
    return m_game;
}

void GameListItem::setTargetFormat(TargetFormat _format)
{
    if(_format != m_target_format)
    {
        m_target_format = _format;
        emitDataChanged(); // TODO: replace QTreeWidget with QTreeView to update independent cells
    }
}

inline TargetFormat GameListItem::targetFormat() const
{
    return m_target_format;
}

QString GameListItem::targetToString() const
{
    switch(m_target_format)
    {
    case TargetFormat::Ul:
        return g_game_format_ul;
    case TargetFormat::Iso:
        return g_game_format_iso;
    case TargetFormat::Zso:
        return g_game_format_zso;
    default:
        return QString();
    }
}

QSharedPointer<Intent> GameConverterActivity::createIntent()
{
    return QSharedPointer<Intent>(new GameConverterActivityIntent());
}

GameConverterActivity::GameConverterActivity(QWidget * _parent) :
    Activity(_parent)
{
    setupUi(this);
    QPushButton * btn_convert = mp_button_box->button(QDialogButtonBox::Apply);
    btn_convert->setText(tr("Convert"));
    btn_convert->setIcon(QIcon(":/images/start"));
    mp_tree_games->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mp_tree_games->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    mp_tree_games->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    mp_tree_games->header()->setSectionResizeMode(3, QHeaderView::Fixed);
    mp_widget_details->setVisible(false);
    mp_label_details_placeholder->setVisible(true);
    connect(mp_btn_back, &QPushButton::clicked, this, &QObject::deleteLater);
    connect(btn_convert, &QPushButton::clicked, this, &GameConverterActivity::convert);
    connect(mp_btn_remove, &QPushButton::clicked, this, &GameConverterActivity::removeGame);
    connect(mp_btn_add, &QPushButton::clicked, this, &GameConverterActivity::addGames);
    connect(mp_tree_games, &QTreeWidget::itemSelectionChanged, this, &GameConverterActivity::onGameSelectionChanged);
    connect(mp_radio_target_ul, &QRadioButton::clicked, this, &GameConverterActivity::onFormatChanged);
    connect(mp_radio_target_iso, &QRadioButton::clicked, this, &GameConverterActivity::onFormatChanged);
    connect(mp_radio_target_zso, &QRadioButton::clicked, this, &GameConverterActivity::onFormatChanged);
}

void GameConverterActivity::onGameSelectionChanged()
{
    QList<QTreeWidgetItem *> selected_items = mp_tree_games->selectedItems();
    if(selected_items.empty())
    {
        mp_widget_details->setVisible(false);
        mp_label_details_placeholder->setVisible(true);
        return;
    }
    mp_widget_details->setVisible(true);
    mp_label_details_placeholder->setVisible(false);
    if(selected_items.count() == 1)
        mp_label_game_title->setText(static_cast<GameListItem *>(selected_items[0])->game().title());
    else
        mp_label_game_title->setText(tr("[Multiple games selected]"));

    TargetFormat shared_target_format = static_cast<const GameListItem *>(selected_items[0])->targetFormat();
    bool have_shared_format = true;
    for(qsizetype i = 1; i < selected_items.size(); ++i)
    {
        const GameListItem * game_list_item = static_cast<const GameListItem *>(selected_items[i]);
        if(shared_target_format != game_list_item->targetFormat())
        {
            have_shared_format = false;
            break;
        }
    }

    foreach(QRadioButton * rb, mp_group_box_target->findChildren<QRadioButton *>())
    {
        rb->setAutoExclusive(false);
        rb->setChecked(false);
        rb->setAutoExclusive(true);
    }
    if(have_shared_format)
    {
        switch(shared_target_format)
        {
        case TargetFormat::Ul:
            mp_radio_target_ul->setChecked(true);
            break;
        case TargetFormat::Iso:
            mp_radio_target_iso->setChecked(true);
            break;
        case TargetFormat::Zso:
            mp_radio_target_zso->setChecked(true);
            break;
        default:
            break;
        }
    }
}

void GameConverterActivity::addGames()
{
    GameCollection & games = Library::instance().games();
    ChooseImportGamesDialog dlg(games, this);
    if(dlg.exec() != QDialog::Accepted)
        return;
    foreach(const Uuid game_id, dlg.selectedGameIds())
    {
        const Game * game = games.findGame(game_id);
        if(game)
        {
            mp_tree_games->addTopLevelItem(new GameListItem(*game));
        }
    }
}

void GameConverterActivity::onFormatChanged(bool _checked)
{
    if(!_checked)
        return;

    QList<QTreeWidgetItem *> selected_items = mp_tree_games->selectedItems();
    if(selected_items.empty())
        return;

    TargetFormat format;
    if(mp_radio_target_ul->isChecked())
        format = TargetFormat::Ul;
    else if(mp_radio_target_iso->isChecked())
        format = TargetFormat::Iso;
    else if(mp_radio_target_zso->isChecked())
        format = TargetFormat::Zso;
    else
        return;

    foreach(QTreeWidgetItem * item, selected_items)
        static_cast<GameListItem *>(item)->setTargetFormat(format);
}

void GameConverterActivity::removeGame()
{

}

void GameConverterActivity::convert()
{

}
