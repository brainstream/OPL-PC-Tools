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

#include <QItemDelegate>
#include <QFileInfo>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <OplPcTools/Core/Device.h>
#include <OplPcTools/Core/Iso9660DeviceSource.h>
#include <OplPcTools/Core/BinCueDeviceSource.h>
#include <OplPcTools/Core/OpticalDriveDeviceSource.h>
#include <OplPcTools/Core/Settings.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/ChooseOpticalDiscDialog.h>
#include <OplPcTools/UI/GameRenameDialog.h>
#include <OplPcTools/UI/GameInstallerActivity.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

namespace SettingsKey {
    const char * iso_dir = "ISODirectory";
} // namespace SettingsKey

const int g_progressbar_max_value = 1000;
const char * g_iso_ext = ".iso";
const char * g_bin_ext = ".bin";

enum class GameInstallationStatus
{
    Queued,
    Installation,
    Registration,
    Done,
    Error,
    RollingBack
};

class GameInstallerActivityIntent : public Intent
{
public:
    Activity * createActivity(QWidget * _parent) override
    {
        return new GameInstallerActivity(_parent);
    }
};

class TaskListItem : public QTreeWidgetItem
{
public:
    TaskListItem(QSharedPointer<Core::Device> _device, QTreeWidget * _widget);
    QVariant data(int _column, int _role) const;
    inline Core::Device & device();
    void rename(const QString & _new_name);
    void setStatus(GameInstallationStatus _status);
    void setError(const QString & _message);
    inline GameInstallationStatus status() const;
    inline const QString & errorMessage() const;
    inline void setMediaType(Core::MediaType _media_type);
    inline bool isSplittingUpEnabled() const;
    inline void enabelSplittingUp(bool _enable);
    inline bool isRenamingEnabled() const;
    inline void enabelRenaming(bool _enable);
    inline bool isMovingEnabled() const;
    inline void enabelMoving(bool _enable);

private:
    QSharedPointer<Core::Device> m_device_ptr;
    GameInstallationStatus m_status;
    QString m_error_message;
    bool m_is_splitting_up_enabled;
    bool m_is_renaming_enabled;
    bool m_is_moving_enabled;
};

class TaskListViewDelegate : public QItemDelegate
{
public:
    TaskListViewDelegate(QObject * _parent = nullptr) :
        QItemDelegate(_parent)
    {
    }

    void paint(QPainter * _painter, const QStyleOptionViewItem & _option, const QModelIndex & _index ) const override;
};

} // namespace

TaskListItem::TaskListItem(QSharedPointer<Core::Device> _device, QTreeWidget * _widget) :
    QTreeWidgetItem(_widget, QTreeWidgetItem::UserType),
    m_device_ptr(_device),
    m_status(GameInstallationStatus::Queued)
{
    const Core::Settings & settings = Core::Settings::instance();
    m_is_splitting_up_enabled = settings.splitUpIso();
    m_is_renaming_enabled = settings.renameIso();
    m_is_moving_enabled = settings.moveIso() && !_device->isReadOnly();
}

QVariant TaskListItem::data(int _column, int _role) const
{
    if(_role != Qt::DisplayRole)
        return QVariant();
    if(_column == 0)
        return m_device_ptr->title();
    switch(m_status)
    {
    case GameInstallationStatus::Done:
        return QObject::tr("Done");
    case GameInstallationStatus::Error:
        return QObject::tr("Error");
    case GameInstallationStatus::Installation:
        return QObject::tr("Installation");
    case GameInstallationStatus::Queued:
        return QObject::tr("Queued");
    case GameInstallationStatus::Registration:
        return QObject::tr("Registration");
    case GameInstallationStatus::RollingBack:
        return QObject::tr("Rolling back");
    default:
        return QString();
    }
}

Core::Device & TaskListItem::device()
{
    return *m_device_ptr;
}

void TaskListItem::rename(const QString & _new_name)
{
    m_device_ptr->setTitle(_new_name);
    emitDataChanged();
}

void TaskListItem::setStatus(GameInstallationStatus _status)
{
    m_status = _status;
    m_error_message = QString();
    emitDataChanged();
}

void TaskListItem::setError(const QString & _message)
{
    m_status = GameInstallationStatus::Error;
    m_error_message = _message;
    emitDataChanged();
}

GameInstallationStatus TaskListItem::status() const
{
    return m_status;
}

const QString & TaskListItem::errorMessage() const
{
    return m_error_message;
}

void TaskListItem::setMediaType(Core::MediaType _media_type)
{
    m_device_ptr->setMediaType(_media_type);
}

bool TaskListItem::isSplittingUpEnabled() const
{
    return m_is_splitting_up_enabled;
}

void TaskListItem::enabelSplittingUp(bool _enable)
{
    m_is_splitting_up_enabled = _enable;
}

bool TaskListItem::isRenamingEnabled() const
{
    return m_is_renaming_enabled;
}

void TaskListItem::enabelRenaming(bool _enable)
{
    m_is_renaming_enabled = _enable;
}

bool TaskListItem::isMovingEnabled() const
{
    return m_is_moving_enabled;
}

void TaskListItem::enabelMoving(bool _enable)
{
    m_is_moving_enabled = _enable;
}

void TaskListViewDelegate::paint(QPainter * _painter, const QStyleOptionViewItem & _option, const QModelIndex & _index ) const
{
    if (_index.column() != 1) { // TODO: const
        QItemDelegate::paint(_painter, _option, _index);
        return;
    }
    QStyleOptionProgressBar progress_bar_option;
    progress_bar_option.state = QStyle::State_Enabled;
    progress_bar_option.direction = QApplication::layoutDirection();
    progress_bar_option.rect = _option.rect;
    progress_bar_option.fontMetrics = QApplication::fontMetrics();
    progress_bar_option.minimum = 0;
    progress_bar_option.maximum = g_progressbar_max_value;
    progress_bar_option.textAlignment = Qt::AlignCenter;
    progress_bar_option.textVisible = true;
    int progress = 320; // TODO: value
    progress_bar_option.progress = progress < 0 ? 0 : progress;
    progress_bar_option.text = QString::asprintf("%d%%", progress_bar_option.progress);
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progress_bar_option, _painter);
}

GameInstallerActivity::GameInstallerActivity(QWidget * _parent /*= nullptr*/) :
    Activity(_parent),
    mp_working_thread(nullptr)
{
    setupUi(this);
    mp_tree_tasks->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mp_tree_tasks->setItemDelegate(new TaskListViewDelegate(this));
    mp_btn_cancel->setDisabled(true);
    mp_btn_install->setDisabled(true);
    connect(mp_tree_tasks, &QTreeWidget::itemSelectionChanged, this, &GameInstallerActivity::taskSelectionChanged);
    connect(mp_btn_add_image, &QPushButton::clicked, [this]() { addDiscImage(); });
    connect(mp_btn_add_disc, &QPushButton::clicked, this, &GameInstallerActivity::addDisc);
    connect(mp_btn_remove, &QPushButton::clicked, this, &GameInstallerActivity::removeGame);
    connect(mp_btn_rename, &QPushButton::clicked, this, &GameInstallerActivity::renameGame);
    connect(mp_tree_tasks, &QTreeWidget::doubleClicked, this, &GameInstallerActivity::renameGame);
    connect(mp_radio_mtauto, &QRadioButton::clicked, this, &GameInstallerActivity::mediaTypeChanged);
    connect(mp_radio_mtdvd, &QRadioButton::clicked, this, &GameInstallerActivity::mediaTypeChanged);
    connect(mp_radio_mtcd, &QRadioButton::clicked, this, &GameInstallerActivity::mediaTypeChanged);
    connect(mp_checkbox_move, &QCheckBox::clicked, this, &GameInstallerActivity::moveOptionChanged);
    connect(mp_checkbox_rename, &QCheckBox::clicked, this, &GameInstallerActivity::renameOptionChanged);
    connect(mp_radio_split_up, &QRadioButton::clicked, this, &GameInstallerActivity::splitUpOptionChanged);
    connect(mp_radio_dnot_split_up, &QRadioButton::clicked, this, &GameInstallerActivity::splitUpOptionChanged);
    taskSelectionChanged();
}

QSharedPointer<Intent> GameInstallerActivity::createIntent()
{
    return QSharedPointer<Intent>(new GameInstallerActivityIntent);
}

void GameInstallerActivity::taskSelectionChanged()
{
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    mp_btn_remove->setVisible(item);
    mp_btn_rename->setVisible(item);
    if(!item)
    {
        mp_widget_task_details->hide();
        return;
    }
    mp_widget_task_details->show();
    if(item->status() == GameInstallationStatus::Error)
        mp_label_error_message->setText(item->errorMessage());
    else
        mp_label_error_message->clear();
    mp_label_title->setText(item->device().title());
    switch(item->device().mediaType())
    {
    case Core::MediaType::CD:
        mp_radio_mtcd->setChecked(true);
        break;
    case Core::MediaType::DVD:
        mp_radio_mtdvd->setChecked(true);
        break;
    default:
        mp_radio_mtauto->setChecked(true);
        break;
    }
    bool split_up = item->isSplittingUpEnabled();
    mp_radio_split_up->setChecked(split_up);
    mp_radio_dnot_split_up->setChecked(!split_up);
    mp_checkbox_move->setChecked(item->isMovingEnabled());
    mp_checkbox_rename->setChecked(item->isRenamingEnabled());
    mp_checkbox_move->setDisabled(split_up || item->device().isReadOnly());
    mp_checkbox_rename->setDisabled(split_up);
}

void GameInstallerActivity::addDiscImage()
{
    QSettings settings;
    QString filter = tr("All Supported Images (*%1 *%2);;ISO Images (*%1);;Bin Files(*%2)").arg(g_iso_ext).arg(g_bin_ext);
    QString iso_dir = settings.value(SettingsKey::iso_dir).toString();
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select PS2 Disc Image Files"), iso_dir, filter);
    if(files.isEmpty()) return;
    settings.setValue(SettingsKey::iso_dir, QFileInfo(files[0]).absolutePath());
    for(const QString & file : files)
    {
        addDiscImage(file);
    }
}

void GameInstallerActivity::addDiscImage(const QString & _file_path)
{
    QFileInfo file_info(_file_path);
    QString absolute_image_path = file_info.absoluteFilePath();
    QTreeWidgetItem * existingTask = findTaskInList(absolute_image_path);
    if(existingTask)
    {
        mp_tree_tasks->setCurrentItem(existingTask);
        return;
    }
    Core::DeviceSource * source = nullptr;
    if(_file_path.endsWith(g_iso_ext))
        source = new Core::Iso9660DeviceSource(_file_path);
    else
        source = new Core::BinCueDeviceSource(_file_path);
    QSharedPointer<Core::Device> device(new Core::Device(QSharedPointer<Core::DeviceSource>(source)));
    if(device->init())
    {
        device->setTitle(truncateGameName(file_info.completeBaseName()));
        TaskListItem * item = new TaskListItem(device, mp_tree_tasks);
        mp_tree_tasks->insertTopLevelItem(mp_tree_tasks->topLevelItemCount(), item);
        mp_tree_tasks->setCurrentItem(item);
        mp_btn_install->setDisabled(false);
    }
    else
    {
        Application::instance().showErrorMessage(tr("Invalid file format"));
    }
}

QTreeWidgetItem * GameInstallerActivity::findTaskInList(const QString & _device_filepath) const
{
    for(int i = mp_tree_tasks->topLevelItemCount() - 1; i >= 0; --i)
    {
        TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->topLevelItem(i));
        if(item != nullptr && item->device().filepath() == _device_filepath)
            return item;
    }
    return nullptr;
}

QString GameInstallerActivity::truncateGameName(const QString & _name) const
{
    return _name; // TODO: respect an installation type
//    const QByteArray utf8 = _name.toUtf8();
//    if(utf8.size() <= g_max_game_name_length)
//        return _name;
//    QString result = QString::fromUtf8(utf8.constData(), g_max_game_name_length);
//    for(int i = result.size() - 1; result[i] != _name[i]; --i)
//        result.truncate(i);
//    return result;
}

void GameInstallerActivity::dragEnterEvent(QDragEnterEvent * _event)
{
    for(const QUrl & url : _event->mimeData()->urls())
    {
        QString path = url.path();
        if(path.endsWith(g_iso_ext) || path.endsWith(g_bin_ext))
        {
            _event->accept();
            return;
        }
    }
    _event->ignore();
}

void GameInstallerActivity::dropEvent(QDropEvent * _event)
{
    for(const QUrl & url : _event->mimeData()->urls())
    {
        QString path = url.path();
        if(path.endsWith(g_iso_ext) || path.endsWith(g_bin_ext))
            addDiscImage(url.path());
    }
}

void GameInstallerActivity::addDisc()
{
    ChooseOpticalDiscDialog dlg(this);
    if(dlg.exec() != QDialog::Accepted)
        return;
    QList<QSharedPointer<Core::Device>> device_list = dlg.devices();
    for(QSharedPointer<Core::Device> & device : device_list)
    {
        QTreeWidgetItem * existingTask = findTaskInList(device->filepath());
        if(existingTask)
        {
            mp_tree_tasks->setCurrentItem(existingTask);
            return;
        }
        TaskListItem * item = new TaskListItem(device, mp_tree_tasks);
        mp_tree_tasks->insertTopLevelItem(mp_tree_tasks->topLevelItemCount(), item);
        mp_tree_tasks->setCurrentItem(item);
        mp_btn_install->setDisabled(false);
    }
}

void GameInstallerActivity::renameGame()
{
    if(mp_working_thread) return;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item || item->status() != GameInstallationStatus::Queued)
        return;
    GameRenameDialog dlg(item->device().title(),
            item->isSplittingUpEnabled() ? Core::GameInstallationType::UlConfig : Core::GameInstallationType::Directory, this);
    if(dlg.exec() == QDialog::Accepted)
    {
        item->rename(dlg.name());
        mp_label_title->setText(dlg.name());
    }
}

void GameInstallerActivity::removeGame()
{
    if(mp_working_thread) return;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(item->status() != GameInstallationStatus::Queued) return;
    delete item;
    if(mp_tree_tasks->topLevelItemCount() == 0)
    {
        mp_btn_install->setDisabled(true);
    }
}

void GameInstallerActivity::mediaTypeChanged(bool _checked)
{
    if(!_checked) return;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item) return;
    Core::MediaType media_type = Core::MediaType::Unknown;
    if(mp_radio_mtcd->isChecked())
        media_type = Core::MediaType::CD;
    else if(mp_radio_mtdvd->isChecked())
        media_type = Core::MediaType::DVD;
    item->setMediaType(media_type);
}

void GameInstallerActivity::splitUpOptionChanged(bool _checked)
{
    if(!_checked) return;
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item) return;
    bool split_up = mp_radio_split_up->isChecked();
    mp_checkbox_move->setDisabled(split_up || item->device().isReadOnly());
    mp_checkbox_rename->setDisabled(split_up);
    item->enabelSplittingUp(split_up);
}

void GameInstallerActivity::renameOptionChanged()
{
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item) return;
    item->enabelRenaming(mp_checkbox_rename->isChecked());
}

void GameInstallerActivity::moveOptionChanged()
{
    TaskListItem * item = static_cast<TaskListItem *>(mp_tree_tasks->currentItem());
    if(!item) return;
    item->enabelMoving(mp_checkbox_move->isChecked());
}
