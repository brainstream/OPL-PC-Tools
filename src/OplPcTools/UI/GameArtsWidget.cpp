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

#include <QStandardPaths>
#include <QShortcut>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QCheckBox>
#include <QApplication>
#include <QPushButton>
#include <OplPcTools/Exception.h>
#include <OplPcTools/Settings.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/GameArtFileSource.h>
#include <OplPcTools/GameArtNetworkSource.h>
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

ArtListItem * findListItemOfType(QListWidget & _list, GameArtType _type)
{
    for(int i = 0; i < _list.count(); ++i)
    {
        ArtListItem * item = static_cast<ArtListItem *>(_list.item(i));
        if(item && item->type() == _type)
            return item;
    }
    return nullptr;
}

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
    mp_item_context_menu(nullptr)
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
    connect(mp_list_arts, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *){ changeGameArt(); });
    connect(&mr_art_manager, &GameArtManager::artChanged, this, &GameArtsWidget::onArtChanged);
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
    auto properties = makeGameArtProperies();
    for(auto it = properties.constKeyValueBegin(); it != properties.constKeyValueEnd(); ++it)
        addArtListItem(it->first, it->second.name);
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

void GameArtsWidget::onArtChanged(const QString & _game_id, GameArtType _type)
{
    if(_game_id == m_game_id)
    {
        ArtListItem * item = findListItemOfType(*mp_list_arts, _type);
        item->setPixmap(mr_art_manager.load(_game_id, _type));
        mp_list_arts->doItemsLayout();
    }
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
        mr_art_manager.setArt(m_game_id, item->type(), GameArtFileSource(filename));
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
    });
}

void GameArtsWidget::downloadAllGameArts()
{
    QList<GameArtType> art_types;
    {
        QMessageBox question_box(this);
        question_box.setWindowTitle(tr("Download Pictures"));
        QPushButton * download_all_btn = question_box.addButton(tr("Replace existing"), QMessageBox::ActionRole);
        QPushButton * download_missing_btn = question_box.addButton(tr("Missing only"), QMessageBox::ActionRole);
        question_box.addButton(QMessageBox::Cancel);
        question_box.setDefaultButton(download_missing_btn);
        question_box.exec();
        if(question_box.clickedButton() == download_all_btn)
        {
            art_types =
            {
                GameArtType::Icon,
                GameArtType::Front,
                GameArtType::Back,
                GameArtType::Spine,
                GameArtType::Screenshot1,
                GameArtType::Screenshot2,
                GameArtType::Background,
                GameArtType::Logo
            };
        }
        else if(question_box.clickedButton() == download_missing_btn)
        {
            for(int i = 0; i < mp_list_arts->count(); ++i)
            {
                ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->item(i));
                if(item && !item->hasPixmap())
                    art_types.append(item->type());
            }
        }
        else
        {
            return;
        }
    }
    downloadGameArts(art_types);
}

void GameArtsWidget::downloadGameArts(const QList<GameArtType> & _types)
{
    GameArtDownloader * downloader = new GameArtDownloader(this);
    BusyDialog * dialog = new BusyDialog(this);
    connect(downloader, &GameArtDownloader::taskComplete, this, [=](quint32, const QStringList & __errors) {
       downloader->deleteLater();
       dialog->hide();
       dialog->deleteLater();
       if(!__errors.isEmpty())
           Application::showErrorMessage(__errors.join("\n\n"));
    });
    connect(downloader, &GameArtDownloader::downloadComplete, this, [this](
        const OplPcTools::GameArtNetworkTask & __task,
        const OplPcTools::GameArtNetworkSource & __source
    ) {
        mr_art_manager.setArt(m_game_id, __task.art_type, __source);
    });
    downloader->downloadArts(m_game_id, _types);
    dialog->open();
}

void GameArtsWidget::downloadGameArt()
{
    ArtListItem * item = static_cast<ArtListItem *>(mp_list_arts->currentItem());
    if(item == nullptr) return;
    downloadGameArts({ item->type() });
}
