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

#include <QShortcut>
#include <QListWidgetItem>
#include <OplPcTools/UI/GameRenameDialog.h>
#include <OplPcTools/UI/GameDetailsWidget.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

class ArtListItem : public QListWidgetItem
{
public:
    ArtListItem(const QString & _title, const QPixmap & _pixmap, QListWidget * _view = nullptr) :
        QListWidgetItem(_view, QListWidgetItem::UserType),
        m_title(_title),
        m_pixmap(_pixmap)
    {
    }

    QVariant data(int _role) const override;

private:
    QString m_title;
    QPixmap m_pixmap;
};

} // namespace


QVariant ArtListItem::data(int _role) const
{
    switch(_role)
    {
    case Qt::DisplayRole:
        return m_title;
    case Qt::DecorationRole:
        return m_pixmap;
    default:
        return QListWidgetItem::data(_role);
    }
}

GameDetailsWidget::GameDetailsWidget(UIContext & _context, OplPcTools::Core::GameArtManager & _art_manager, QWidget * _parent /*= nullptr*/) :
    QWidget(_parent),
    mr_context(_context),
    mr_art_manager(_art_manager),
    mp_game(nullptr)
{
    setupUi(this);
    setupShortcuts();
    init();
    connect(mp_btn_close, &QPushButton::clicked, this, &GameDetailsWidget::deleteLater);
}

void GameDetailsWidget::setupShortcuts()
{
    QShortcut * shortcut = new QShortcut(QKeySequence("Back"), this);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsWidget::deleteLater);
    shortcut = new QShortcut(QKeySequence("Esc"), this);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsWidget::deleteLater);
    shortcut = new QShortcut(QKeySequence("F2"), this);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsWidget::renameGame);
}

void GameDetailsWidget::renameGame()
{
    if(mp_game == nullptr) return;
    GameRenameDialog dlg(mp_game->title(), mp_game->installationType(), this);
    dlg.exec();
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

void GameDetailsWidget::init()
{
    if(mp_game == nullptr)
    {
        clear();
        return;
    }
    mp_label_title->setText(mp_game->title());
    mp_widget_art_details->hide();
    mp_list_arts->clear();
    mp_list_arts->addItem(new ArtListItem(tr("Icon"), mr_art_manager.load(mp_game->id(), Core::GameArtType::Icon)));
    mp_list_arts->addItem(new ArtListItem(tr("Front Cover"), mr_art_manager.load(mp_game->id(), Core::GameArtType::Front)));
    mp_list_arts->addItem(new ArtListItem(tr("Back Cover"), mr_art_manager.load(mp_game->id(), Core::GameArtType::Back)));
    mp_list_arts->addItem(new ArtListItem(tr("Spine Cover"), mr_art_manager.load(mp_game->id(), Core::GameArtType::Spine)));
    mp_list_arts->addItem(new ArtListItem(tr("Screenshot #1"), mr_art_manager.load(mp_game->id(), Core::GameArtType::Screenshot1)));
    mp_list_arts->addItem(new ArtListItem(tr("Screenshot #2"), mr_art_manager.load(mp_game->id(), Core::GameArtType::Screenshot2)));
    mp_list_arts->addItem(new ArtListItem(tr("Background"), mr_art_manager.load(mp_game->id(), Core::GameArtType::Background)));
    mp_list_arts->addItem(new ArtListItem(tr("Logo"), mr_art_manager.load(mp_game->id(), Core::GameArtType::Logo)));
}

void GameDetailsWidget::clear()
{
    mp_label_title->clear();
    mp_list_arts->clear();
    mp_widget_art_details->hide();
}
