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

#include <QSharedPointer>
#include <QStandardPaths>
#include <QShortcut>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QCheckBox>
#include <OplPcTools/Core/Exception.h>
#include <OplPcTools/Core/Settings.h>
#include <OplPcTools/UI/GameRenameDialog.h>
#include <OplPcTools/UI/GameDetailsWidget.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

static const char * g_settings_key_cover_dir = "PixmapDirectory";

class GameDetailsWidgetIntent : public Intent
{
public:
    GameDetailsWidgetIntent(UIContext & _context, OplPcTools::Core::GameArtManager & _art_manager, const QString & _game_id) :
        mr_context(_context),
        mr_art_manager(_art_manager),
        m_game_id(_game_id)
    {
    }

    QWidget * createWidget(QWidget * _parent) override
    {
        GameDetailsWidget * widget = new GameDetailsWidget(mr_context, mr_art_manager, _parent);
        widget->setGameId(m_game_id);
        return widget;
    }

private:
    UIContext & mr_context;
    OplPcTools::Core::GameArtManager & mr_art_manager;
    const QString m_game_id;
};

class ArtListItem : public QListWidgetItem
{
public:
    ArtListItem(Core::GameArtType _type, const QString & _title, const QPixmap & _pixmap, QListWidget * _view = nullptr);
    void setPixmap(const QPixmap & _pixmap);
    inline Core::GameArtType type() const;
    inline bool hasPixmap() const;
    QVariant data(int _role) const override;

private:
    static QSharedPointer<QPixmap> s_default_pixmap_ptr;
    Core::GameArtType m_type;
    QString m_title;
    QPixmap m_pixmap;
    bool m_has_pixmap;
};

} // namespace

QSharedPointer<QPixmap> ArtListItem::s_default_pixmap_ptr = QSharedPointer<QPixmap>();

ArtListItem::ArtListItem(Core::GameArtType _type, const QString & _title, const QPixmap & _pixmap, QListWidget * _view /*= nullptr*/) :
    QListWidgetItem(_view, QListWidgetItem::UserType),
    m_type(_type),
    m_title(_title),
    m_has_pixmap(false)
{
    setPixmap(_pixmap);
}

void ArtListItem::setPixmap(const QPixmap & _pixmap)
{
    m_has_pixmap = !_pixmap.isNull();
    if(m_has_pixmap)
    {
        m_pixmap = _pixmap;
    }
    else
    {
        if(s_default_pixmap_ptr.isNull())
        {
            s_default_pixmap_ptr.reset(new QPixmap(QPixmap(":/images/no-image")
                .scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        }
        m_pixmap = *s_default_pixmap_ptr;
    }
}

Core::GameArtType ArtListItem::type() const
{
    return m_type;
}

bool ArtListItem::hasPixmap() const
{
    return m_has_pixmap;
}

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
    mp_game(nullptr),
    mp_item_context_menu(nullptr),
    mp_action_change_art(nullptr),
    mp_action_remove_art(nullptr)
{
    setupUi(this);
    setupShortcuts();
    mp_list_arts->setContextMenuPolicy(Qt::CustomContextMenu);
    mp_item_context_menu = new QMenu(mp_list_arts);
    mp_action_change_art = new QAction(tr("Change Picture..."));
    mp_action_remove_art = new QAction(tr("Remove Picture"));
    mp_item_context_menu->addAction(mp_action_change_art);
    mp_item_context_menu->addAction(mp_action_remove_art);
    initGameControls();
    connect(mp_btn_close, &QPushButton::clicked, this, &GameDetailsWidget::deleteLater);
    connect(mp_list_arts, &QListWidget::customContextMenuRequested, this, &GameDetailsWidget::showItemContextMenu);
    connect(mp_action_change_art, &QAction::triggered, this, &GameDetailsWidget::changeGameArt);
    connect(mp_action_remove_art, &QAction::triggered, this, &GameDetailsWidget::removeGameArt);
    connect(mp_label_title, &ClickableLabel::doubleClicked, this, &GameDetailsWidget::renameGame);
}

QSharedPointer<Intent> GameDetailsWidget::createIntent(UIContext & _context, OplPcTools::Core::GameArtManager & _art_manager, const QString & _game_id)
{
    return QSharedPointer<Intent>(new GameDetailsWidgetIntent(_context, _art_manager, _game_id));
}

void GameDetailsWidget::setupShortcuts()
{
    QShortcut * shortcut = new QShortcut(QKeySequence("Back"), this);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsWidget::deleteLater);
    shortcut = new QShortcut(QKeySequence("Esc"), this);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsWidget::deleteLater);
    shortcut = new QShortcut(QKeySequence("F2"), this);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsWidget::renameGame);
    shortcut = new QShortcut(QKeySequence("Del"), mp_list_arts);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsWidget::removeGameArt);
}

void GameDetailsWidget::renameGame()
{
    if(mp_game == nullptr) return;
    GameRenameDialog dlg(mp_game->title(), mp_game->installationType(), this);
    if(dlg.exec() != QDialog::Accepted)
        return;
    try
    {
        if(mr_context.collection().renameGame(*mp_game, dlg.name()))
            mp_label_title->setText(dlg.name());
    }
    catch(const Core::Exception & exception)
    {
        mr_context.showErrorMessage(exception.message());
    }
}

void GameDetailsWidget::showItemContextMenu(const QPoint & _point)
{
    ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->itemAt(_point));
    if(item == nullptr) return;
    mp_action_remove_art->setEnabled(item->hasPixmap());
    mp_item_context_menu->exec(mp_list_arts->mapToGlobal(_point));
}

void GameDetailsWidget::changeGameArt()
{
    try
    {
        ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->currentItem());
        if(item == nullptr)
            return;
        QSettings settings;
        QString dirpath = settings.value(g_settings_key_cover_dir).toString();
        if(dirpath.isEmpty())
        {
            QStringList dirpaths = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
            if(!dirpaths.isEmpty())
                dirpath = dirpaths.first();
        }
        QString filename = QFileDialog::getOpenFileName(
            this, tr("Choose a Picture"), dirpath,
            tr("Pictures") + " (*.png *.jpg *.jpeg *.bmp)");
        if(filename.isEmpty())
            return;
        settings.setValue(g_settings_key_cover_dir, QFileInfo(filename).absoluteDir().absolutePath());
        QPixmap pixmap = mr_art_manager.setArt(mp_game->id(), item->type(), filename);
        item->setPixmap(pixmap);
        mp_list_arts->doItemsLayout();
    }
    catch(const Core::Exception & exception)
    {
        mr_context.showErrorMessage(exception.message());
    }
}

void GameDetailsWidget::removeGameArt()
{
    try
    {
        ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->currentItem());
        if(item == nullptr || !item->hasPixmap()) return;
        if(Core::Settings::instance().confirmPixmapDeletion())
        {
            QCheckBox * checkbox = new QCheckBox("Don't show again");
            QMessageBox message_box(QMessageBox::Question, tr("Remove Picture"),
                        tr("The %1 will be deleted.\nContinue?").arg(item->text()),
                        QMessageBox::Yes | QMessageBox::No);
            message_box.setDefaultButton(QMessageBox::Yes);
            message_box.setCheckBox(checkbox);
            if(message_box.exec() != QMessageBox::Yes)
                return;
            if(checkbox->isChecked())
                Core::Settings::instance().setConfirmPixmapDeletion(false);
        }
        mr_art_manager.deleteArt(mp_game->id(), item->type());
        item->setPixmap(QPixmap());
        mp_list_arts->doItemsLayout();
    }
    catch(const Core::Exception & exception)
    {
        mr_context.showErrorMessage(exception.message());
    }
}

void GameDetailsWidget::setGameId(const QString & _id)
{
    mp_game = mr_context.collection().findGame(_id);
    initGameControls();
}

void GameDetailsWidget::initGameControls()
{
    if(mp_game == nullptr)
    {
        clearGameControls();
        return;
    }
    mp_label_title->setText(mp_game->title());
    mp_list_arts->clear();
    addArtListItem(Core::GameArtType::Icon, tr("Icon"));
    addArtListItem(Core::GameArtType::Front, tr("Front Cover"));
    addArtListItem(Core::GameArtType::Back, tr("Back Cover"));
    addArtListItem(Core::GameArtType::Spine, tr("Spine Cover"));
    addArtListItem(Core::GameArtType::Screenshot1, tr("Screenshot #1"));
    addArtListItem(Core::GameArtType::Screenshot2, tr("Screenshot #2"));
    addArtListItem(Core::GameArtType::Background, tr("Background"));
    addArtListItem(Core::GameArtType::Logo, tr("Logo"));
}

void GameDetailsWidget::addArtListItem(Core::GameArtType _type, const QString & _text)
{
    mp_list_arts->addItem(new ArtListItem(_type, _text, mr_art_manager.load(mp_game->id(), _type)));
}

void GameDetailsWidget::clearGameControls()
{
    mp_label_title->clear();
    mp_list_arts->clear();
}
