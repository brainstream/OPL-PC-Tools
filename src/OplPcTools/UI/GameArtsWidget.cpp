/***********************************************************************************************
 * Copyright © 2017-2021 Sergey Smolyannikov aka brainstream                                   *
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

#include <QStandardPaths>
#include <QShortcut>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QCheckBox>
#include <QApplication>
#include <OplPcTools/Exception.h>
#include <OplPcTools/Settings.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/GameArtsWidget.h>
#include <OplPcTools/UI/BusySmartThread.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

static const char * g_settings_key_cover_dir = "PixmapDirectory";

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

GameArtsWidget::GameArtsWidget(const QString & _game_id, OplPcTools::GameArtManager & _art_manager, QWidget * _parent /*= nullptr*/) :
    QWidget(_parent),
    mr_art_manager(_art_manager),
    m_game_id(_game_id),
    mp_item_context_menu(nullptr),
    m_downloading_type(GameArtType::Icon)
{
    setupUi(this);
    setupShortcuts();
    mp_list_arts->setContextMenuPolicy(Qt::CustomContextMenu);
    mp_item_context_menu = new QMenu(mp_list_arts);
    mp_item_context_menu->addAction(mp_action_change_art);
    mp_item_context_menu->addAction(mp_action_download_art);
    mp_item_context_menu->addAction(mp_action_delete_art);
    initGameArts();
    connect(mp_list_arts, &QListWidget::customContextMenuRequested, this, &GameArtsWidget::showItemContextMenu);
    connect(mp_action_change_art, &QAction::triggered, this, &GameArtsWidget::changeGameArt);
    connect(mp_action_download_art, &QAction::triggered, this, &GameArtsWidget::downloadGameArt);
    connect(mp_action_delete_art, &QAction::triggered, this, &GameArtsWidget::deleteGameArt);
    connect(mp_list_arts, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *){ changeGameArt(); });
    connect(&mr_art_manager, &GameArtManager::downloadStarted, this, &GameArtsWidget::onDownloadStarted);
    connect(&mr_art_manager, &GameArtManager::downloadProgress, this, &GameArtsWidget::onDownloadProgress);
    connect(&mr_art_manager, &GameArtManager::downloadCompleted, this, &GameArtsWidget::onDownloadCompleted);
}

void GameArtsWidget::setupShortcuts()
{
    QShortcut * shortcut = new QShortcut(QKeySequence("Del"), mp_list_arts);
    mp_action_delete_art->setShortcut(shortcut->key());
    connect(shortcut, &QShortcut::activated, this, &GameArtsWidget::deleteGameArt);
    shortcut = new QShortcut(QKeySequence("Space"), mp_list_arts);
    mp_action_change_art->setShortcut(shortcut->key());
    connect(shortcut, &QShortcut::activated, this, &GameArtsWidget::changeGameArt);
}

void GameArtsWidget::initGameArts()
{
    addArtListItem(GameArtType::Icon, tr("Icon"));
    addArtListItem(GameArtType::Front, tr("Front Cover"));
    addArtListItem(GameArtType::Back, tr("Back Cover"));
    addArtListItem(GameArtType::Spine, tr("Spine Cover"));
    addArtListItem(GameArtType::Screenshot1, tr("Screenshot #1"));
    addArtListItem(GameArtType::Screenshot2, tr("Screenshot #2"));
    addArtListItem(GameArtType::Background, tr("Background"));
    addArtListItem(GameArtType::Logo, tr("Logo"));
}

void GameArtsWidget::addArtListItem(GameArtType _type, const QString & _text)
{
    mp_list_arts->addItem(new ArtListItem(_type, _text, mr_art_manager.load(m_game_id, _type)));
}

void GameArtsWidget::showItemContextMenu(const QPoint & _point)
{
    ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->itemAt(_point));
    if(item == nullptr) return;
    mp_action_delete_art->setEnabled(item->hasPixmap());
    mp_item_context_menu->exec(mp_list_arts->mapToGlobal(_point));
}

void GameArtsWidget::changeGameArt()
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
        Application::activeWindow(),
        tr("Choose a Picture"), dirpath,
        tr("Pictures") + " (*.png *.jpg *.jpeg *.bmp)");
    if(filename.isEmpty())
        return;
    settings.setValue(g_settings_key_cover_dir, QFileInfo(filename).absoluteDir().absolutePath());
    startBusySmartThread([this, item, filename]() {
        QPixmap pixmap = mr_art_manager.setArt(m_game_id, item->type(), filename);
        item->setPixmap(pixmap);
    });
}

void GameArtsWidget::startBusySmartThread(std::function<void()> _lambda)
{
    BusySmartThread * thread = new BusySmartThread(_lambda, this);
    connect(thread, &BusySmartThread::finished, mp_list_arts, &QListWidget::doItemsLayout);
    connect(thread, &BusySmartThread::finished, thread, &BusySmartThread::deleteLater);
    connect(thread, &BusySmartThread::exception, [](const QString & message) {
        Application::showErrorMessage(message);
    });
    thread->start();
}

void GameArtsWidget::deleteGameArt()
{
    ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->currentItem());
    if(item == nullptr || !item->hasPixmap()) return;
    Settings & settings = Settings::instance();
    if(settings.confirmPixmapDeletion())
    {
        QCheckBox * checkbox = new QCheckBox(tr("Do not ask again"));
        QMessageBox message_box(QMessageBox::Question, tr("Delete Picture"),
            QString("%1\n%2")
                .arg(tr("Are you sure you want to delete this picture?"))
                .arg(item->text()),
            QMessageBox::Yes | QMessageBox::No);
        message_box.setDefaultButton(QMessageBox::Yes);
        message_box.setCheckBox(checkbox);
        if(message_box.exec() != QMessageBox::Yes)
            return;
        if(checkbox->isChecked())
            settings.setConfirmPixmapDeletion(false);
    }
    startBusySmartThread([this, item]() {
        mr_art_manager.deleteArt(m_game_id, item->type());
        item->setPixmap(QPixmap());
    });
}

void GameArtsWidget::downloadGameArt()
{
    ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->currentItem());
    if(item == nullptr)
        return;
    
    m_downloading_type = item->type();
    mr_art_manager.downloadArt(m_game_id, item->type());
}

void GameArtsWidget::onDownloadStarted(const QString & _game_id, GameArtType _type)
{
    if(_game_id != m_game_id)
        return;
    
    QString artTypeName = getArtTypeName(_type).toLower();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    // Find the item and update its tooltip to show download status
    for(int i = 0; i < mp_list_arts->count(); ++i)
    {
        ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->item(i));
        if(item && item->type() == _type)
        {
            item->setToolTip(tr("Downloading %1...").arg(artTypeName));
            break;
        }
    }
}

void GameArtsWidget::onDownloadProgress(const QString & _game_id, GameArtType _type, qint64 _received, qint64 _total)
{
    if(_game_id != m_game_id || _type != m_downloading_type)
        return;
    
    QString artTypeName = getArtTypeName(_type).toLower();
    QString progressText;
    
    if(_total > 0)
    {
        int percentage = (int)((_received * 100) / _total);
        progressText = tr("Downloading %1... %2%").arg(artTypeName).arg(percentage);
    }
    else
    {
        progressText = tr("Downloading %1...").arg(artTypeName);
    }
    
    // Find the item and update its tooltip
    for(int i = 0; i < mp_list_arts->count(); ++i)
    {
        ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->item(i));
        if(item && item->type() == _type)
        {
            item->setToolTip(progressText);
            break;
        }
    }
}

void GameArtsWidget::onDownloadCompleted(const QString & _game_id, GameArtType _type, bool _success)
{
    if(_game_id != m_game_id)
        return;
    
    QApplication::restoreOverrideCursor();
    
    // Clear tooltip
    for(int i = 0; i < mp_list_arts->count(); ++i)
    {
        ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->item(i));
        if(item && item->type() == _type)
        {
            item->setToolTip("");
            break;
        }
    }
    
    if(_success)
    {
        // Find the item and refresh its pixmap
        for(int i = 0; i < mp_list_arts->count(); ++i)
        {
            ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->item(i));
            if(item && item->type() == _type)
            {
                QPixmap pixmap = mr_art_manager.load(m_game_id, _type);
                item->setPixmap(pixmap);
                mp_list_arts->doItemsLayout();
                break;
            }
        }
    }
    else
    {
        // Only show individual error messages if this is NOT a bulk download
        if(!isBulkDownload(_game_id))
        {
            QString artTypeName = getArtTypeName(_type).toLower();
            Application::showErrorMessage(tr("No %1 available for this game at archive.org").arg(artTypeName));
        }
    }
}

QString GameArtsWidget::getArtTypeName(GameArtType _type) const
{
    switch(_type)
    {
        case GameArtType::Icon: return tr("icon");
        case GameArtType::Front: return tr("front cover");
        case GameArtType::Back: return tr("back cover");
        case GameArtType::Spine: return tr("spine cover");
        case GameArtType::Screenshot1: return tr("screenshot #1");
        case GameArtType::Screenshot2: return tr("screenshot #2");
        case GameArtType::Background: return tr("background");
        case GameArtType::Logo: return tr("logo");
        default: return tr("artwork");
    }
}

bool GameArtsWidget::isBulkDownload(const QString & _game_id) const
{
    return mr_art_manager.isBulkDownloadActive(_game_id);
}
