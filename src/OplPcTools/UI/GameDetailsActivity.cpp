/***********************************************************************************************
 * Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                   *
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
#include <OplPcTools/Exception.h>
#include <OplPcTools/Settings.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/GameRenameDialog.h>
#include <OplPcTools/UI/GameDetailsActivity.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

static const char * g_settings_key_cover_dir = "PixmapDirectory";

class GameDetailsActivityIntent : public Intent
{
public:
    GameDetailsActivityIntent(OplPcTools::GameArtManager & _art_manager, const QString & _game_id) :
        mr_art_manager(_art_manager),
        m_game_id(_game_id)
    {
    }

    Activity * createActivity(QWidget * _parent) override
    {
        GameDetailsActivity * widget = new GameDetailsActivity(mr_art_manager, _parent);
        widget->setGameId(m_game_id);
        return widget;
    }

private:
    OplPcTools::GameArtManager & mr_art_manager;
    const QString m_game_id;
};

class ArtListItem : public QListWidgetItem
{
public:
    ArtListItem(GameArtType _type, const QString & _title, const QPixmap & _pixmap, QListWidget * _view = nullptr);
    void setPixmap(const QPixmap & _pixmap);
    inline GameArtType type() const;
    inline bool hasPixmap() const;
    QVariant data(int _role) const override;

private:
    static QSharedPointer<QPixmap> s_default_pixmap_ptr;
    GameArtType m_type;
    QString m_title;
    QPixmap m_pixmap;
    bool m_has_pixmap;
};

} // namespace

QSharedPointer<QPixmap> ArtListItem::s_default_pixmap_ptr = QSharedPointer<QPixmap>();

ArtListItem::ArtListItem(GameArtType _type, const QString & _title, const QPixmap & _pixmap, QListWidget * _view /*= nullptr*/) :
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

GameArtType ArtListItem::type() const
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

GameDetailsActivity::GameDetailsActivity(OplPcTools::GameArtManager & _art_manager, QWidget * _parent /*= nullptr*/) :
    Activity(_parent),
    mr_art_manager(_art_manager),
    mp_game(nullptr),
    mp_item_context_menu(nullptr)
{
    setupUi(this);
    setupShortcuts();
    mp_list_arts->setContextMenuPolicy(Qt::CustomContextMenu);
    mp_item_context_menu = new QMenu(mp_list_arts);
    mp_item_context_menu->addAction(mp_action_change_art);
    mp_item_context_menu->addAction(mp_action_delete_art);
    initGameControls();
    connect(mp_btn_close, &QPushButton::clicked, this, &GameDetailsActivity::close);
    connect(mp_list_arts, &QListWidget::customContextMenuRequested, this, &GameDetailsActivity::showItemContextMenu);
    connect(mp_action_change_art, &QAction::triggered, this, &GameDetailsActivity::changeGameArt);
    connect(mp_action_delete_art, &QAction::triggered, this, &GameDetailsActivity::deleteGameArt);
    connect(mp_label_title, &ClickableLabel::clicked, this, &GameDetailsActivity::renameGame);
}

QSharedPointer<Intent> GameDetailsActivity::createIntent(OplPcTools::GameArtManager & _art_manager, const QString & _game_id)
{
    return QSharedPointer<Intent>(new GameDetailsActivityIntent(_art_manager, _game_id));
}

void GameDetailsActivity::setupShortcuts()
{
    QShortcut * shortcut = new QShortcut(QKeySequence("Back"), this);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsActivity::close);
    shortcut = new QShortcut(QKeySequence("Esc"), this);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsActivity::close);
    shortcut = new QShortcut(QKeySequence("F2"), this);
    connect(shortcut, &QShortcut::activated, this, &GameDetailsActivity::renameGame);
    shortcut = new QShortcut(QKeySequence("Del"), mp_list_arts);
    mp_action_delete_art->setShortcut(shortcut->key());
    connect(shortcut, &QShortcut::activated, this, &GameDetailsActivity::deleteGameArt);
    shortcut = new QShortcut(QKeySequence("Space"), mp_list_arts);
    mp_action_change_art->setShortcut(shortcut->key());
    connect(shortcut, &QShortcut::activated, this, &GameDetailsActivity::changeGameArt);
}

void GameDetailsActivity::renameGame()
{
    if(mp_game == nullptr) return;
    GameRenameDialog dlg(mp_game->title(), mp_game->installationType(), this);
    if(dlg.exec() != QDialog::Accepted)
        return;
    try
    {
        Application::instance().gameCollection().renameGame(*mp_game, dlg.name());
        mp_label_title->setText(dlg.name());
    }
    catch(const Exception & exception)
    {
        Application::instance().showErrorMessage(exception.message());
    }
    catch(...)
    {
        Application::instance().showErrorMessage();
    }
}

void GameDetailsActivity::showItemContextMenu(const QPoint & _point)
{
    ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->itemAt(_point));
    if(item == nullptr) return;
    mp_action_delete_art->setEnabled(item->hasPixmap());
    mp_item_context_menu->exec(mp_list_arts->mapToGlobal(_point));
}

void GameDetailsActivity::changeGameArt()
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
            Application::instance().activeWindow(),
            tr("Choose a Picture"), dirpath,
            tr("Pictures") + " (*.png *.jpg *.jpeg *.bmp)");
        if(filename.isEmpty())
            return;
        settings.setValue(g_settings_key_cover_dir, QFileInfo(filename).absoluteDir().absolutePath());
        QPixmap pixmap = mr_art_manager.setArt(mp_game->id(), item->type(), filename);
        item->setPixmap(pixmap);
        mp_list_arts->doItemsLayout();
    }
    catch(const Exception & exception)
    {
        Application::instance().showErrorMessage(exception.message());
    }
    catch(...)
    {
        Application::instance().showErrorMessage();
    }
}

void GameDetailsActivity::deleteGameArt()
{
    try
    {
        ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->currentItem());
        if(item == nullptr || !item->hasPixmap()) return;
        Settings & settings = Settings::instance();
        if(settings.flag(Settings::Flag::ConfirmPixmapDeletion))
        {
            QCheckBox * checkbox = new QCheckBox(tr("Don't show again"));
            QMessageBox message_box(QMessageBox::Question, tr("Remove Picture"),
                        tr("The %1 will be deleted.\nContinue?").arg(item->text()),
                        QMessageBox::Yes | QMessageBox::No);
            message_box.setDefaultButton(QMessageBox::Yes);
            message_box.setCheckBox(checkbox);
            if(message_box.exec() != QMessageBox::Yes)
                return;
            if(checkbox->isChecked())
                settings.setFlag(Settings::Flag::ConfirmPixmapDeletion, false);
        }
        mr_art_manager.deleteArt(mp_game->id(), item->type());
        item->setPixmap(QPixmap());
        mp_list_arts->doItemsLayout();
    }
    catch(const Exception & exception)
    {
        Application::instance().showErrorMessage(exception.message());
    }
    catch(...)
    {
        Application::instance().showErrorMessage();
    }
}

void GameDetailsActivity::setGameId(const QString & _id)
{
    mp_game = Application::instance().gameCollection().findGame(_id);
    initGameControls();
}

void GameDetailsActivity::initGameControls()
{
    if(mp_game == nullptr)
    {
        clearGameControls();
        return;
    }
    mp_label_title->setText(mp_game->title());
    mp_list_arts->clear();
    addArtListItem(GameArtType::Icon, tr("Icon"));
    addArtListItem(GameArtType::Front, tr("Front Cover"));
    addArtListItem(GameArtType::Back, tr("Back Cover"));
    addArtListItem(GameArtType::Spine, tr("Spine Cover"));
    addArtListItem(GameArtType::Screenshot1, tr("Screenshot #1"));
    addArtListItem(GameArtType::Screenshot2, tr("Screenshot #2"));
    addArtListItem(GameArtType::Background, tr("Background"));
    addArtListItem(GameArtType::Logo, tr("Logo"));
}

void GameDetailsActivity::addArtListItem(GameArtType _type, const QString & _text)
{
    mp_list_arts->addItem(new ArtListItem(_type, _text, mr_art_manager.load(mp_game->id(), _type)));
}

void GameDetailsActivity::clearGameControls()
{
    mp_label_title->clear();
    mp_list_arts->clear();
}
