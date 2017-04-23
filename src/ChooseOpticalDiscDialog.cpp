/***********************************************************************************************
 *                                                                                             *
 * This file is part of the qpcopl project, the graphical PC tools for Open PS2 Loader.        *
 *                                                                                             *
 * qpcopl is free software: you can redistribute it and/or modify it under the terms of        *
 * the GNU General Public License as published by the Free Software Foundation,                *
 * either version 3 of the License, or (at your option) any later version.                     *
 *                                                                                             *
 * qpcopl is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;        *
 * without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  *
 * See the GNU General Public License for more details.                                        *
 *                                                                                             *
 * You should have received a copy of the GNU General Public License along with MailUnit.      *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#include <QPushButton>
#include <QThread>
#include "LibCDIO.h"
#include "Exception.h"
#include "ChooseOpticalDiscDialog.h"

namespace {

class DeviceListItem : public QTreeWidgetItem
{
public:
    DeviceListItem(QTreeWidget * _view, const ChooseOpticalDiscDialog::DeviceInfo & _device);
    QVariant data(int _column, int _role) const;
    inline const ChooseOpticalDiscDialog::DeviceInfo & device() const;

private:
    ChooseOpticalDiscDialog::DeviceInfo m_device_info;
};

DeviceListItem::DeviceListItem(QTreeWidget * _view, const ChooseOpticalDiscDialog::DeviceInfo & _device) :
    QTreeWidgetItem(_view, QTreeWidgetItem::UserType),
    m_device_info(_device)
{
    setIcon(0, QIcon(":/icons/media-optical"));
}

QVariant DeviceListItem::data(int _column, int _role) const
{
    if(_role == Qt::DisplayRole)
    {
        switch(_column)
        {
        case 0:
            return m_device_info.device;
        case 1:
            return m_device_info.title;
        }
    }
    return QTreeWidgetItem::data(_column, _role);
}

const ChooseOpticalDiscDialog::DeviceInfo & DeviceListItem::device() const
{
    return m_device_info;
}

class InitializationThread : public QThread
{
public:
    explicit InitializationThread(QObject * _parent = nullptr);
    inline const QString & errorMessage() const;
    inline const QList<ChooseOpticalDiscDialog::DeviceInfo> & devices() const;

protected:
    void run() override;

private:
    QString readDeviceLabel(CdIo_t * _device, quint8 _attempt = 0);

private:
    static const quint8 s_max_read_label_attempts = 3;
    QString m_error_message;
    QList<ChooseOpticalDiscDialog::DeviceInfo> m_devices;
};

InitializationThread::InitializationThread(QObject * _parent /*= nullptr*/) :
    QThread(_parent)
{
}

void InitializationThread::run()
{
    try
    {
        initLibCDIO();
        char ** devices = cdio_get_devices(DRIVER_DEVICE);
        for(size_t i = 0; devices[i]; ++i)
        {
            CdIo * cdio = cdio_open_cd(devices[i]);
            if(!cdio) continue;
            cdio_close_tray(devices[i], nullptr);
            if(cdio_get_discmode(cdio) == CDIO_DISC_MODE_ERROR)
                continue;
            ChooseOpticalDiscDialog::DeviceInfo device_info;
            device_info.device = devices[i];
            device_info.title = readDeviceLabel(cdio);
            m_devices.append(device_info);
            cdio_destroy(cdio);
        }
        cdio_free_device_list(devices);
    }
    catch(const Exception & exception)
    {
        m_error_message = exception.message();
    }
}

QString InitializationThread::readDeviceLabel(CdIo_t * _device, quint8 _attempt /*= 0*/)
{
    /// If a drive tray was opened and we have close it, the cdio_guess_cd_type function
    /// returns an empty label value. Second attempt is usually successful.

    cdio_iso_analysis_t analysis = {};
    lsn_t session = 0;
    track_t first_track = cdio_get_first_track_num(_device);
    if(first_track == CDIO_INVALID_TRACK || cdio_get_last_session(_device, &session) != DRIVER_OP_SUCCESS)
        return QString();
    cdio_guess_cd_type(_device, session, first_track, &analysis);
    if((!analysis.iso_label || !analysis.iso_label[0]) && _attempt < s_max_read_label_attempts)
    {
        return readDeviceLabel(_device, _attempt + 1);
    }
    return QString(analysis.iso_label).trimmed();
}

const QString & InitializationThread::errorMessage() const
{
    return m_error_message;
}

const QList<ChooseOpticalDiscDialog::DeviceInfo> & InitializationThread::devices() const
{
    return m_devices;
}

} // namespace

ChooseOpticalDiscDialog::ChooseOpticalDiscDialog(QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint)
{
    setupUi(this);
    mp_widget_content->setVisible(false);
    mp_label_error->setVisible(false);
    mp_button_box->button(QDialogButtonBox::Open)->setDisabled(true);
    mp_tree_devices->setColumnWidth(0, 180);
    InitializationThread * thread = new InitializationThread(this);
    connect(thread, &QThread::finished, thread, [this, thread]() {
        mp_widget_loading->setVisible(false);
        mp_widget_content->setVisible(true);
        for(const DeviceInfo & device_info : thread->devices())
        {
            mp_tree_devices->addTopLevelItem(new DeviceListItem(mp_tree_devices, device_info));
        }
        QString error_message = thread->errorMessage();
        if(error_message.isEmpty() && mp_tree_devices->topLevelItemCount() == 0)
            error_message = tr("There are no available CD/DVD drives");
        if(!error_message.isEmpty())
        {
            mp_label_error->setText(m_error_message);
            mp_label_error->setVisible(true);
            mp_tree_devices->setVisible(false);
        }
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void ChooseOpticalDiscDialog::deviceSelectionChanged()
{
    mp_button_box->button(QDialogButtonBox::Open)->setDisabled(mp_tree_devices->selectedItems().isEmpty());
}

QList<ChooseOpticalDiscDialog::DeviceInfo> ChooseOpticalDiscDialog::devices() const
{
    QList<ChooseOpticalDiscDialog::DeviceInfo> result;
    QModelIndexList indexes = mp_tree_devices->selectionModel()->selectedRows();
    for(const QModelIndex & index : indexes)
    {
        DeviceListItem * item = static_cast<DeviceListItem *>(mp_tree_devices->topLevelItem(index.row()));
        result.append(item->device());
    }
    return result;
}
