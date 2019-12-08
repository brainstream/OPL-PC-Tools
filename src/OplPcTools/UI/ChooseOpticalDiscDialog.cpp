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

#include <QPushButton>
#include <QThread>
#include <OplPcTools/Exception.h>
#include <OplPcTools/UI/ChooseOpticalDiscDialog.h>
#include <OplPcTools/OpticalDriveDeviceSource.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

struct DeviceDisplayData
{
    QString name;
    QSharedPointer<Device> device;
};

class DeviceListItem : public QTreeWidgetItem
{
public:
    DeviceListItem(QTreeWidget * _view, const DeviceDisplayData & _data);
    QVariant data(int _column, int _role) const;
    inline QSharedPointer<Device> device() const;

private:
    DeviceDisplayData m_data;
};

DeviceListItem::DeviceListItem(QTreeWidget * _view, const DeviceDisplayData & _data) :
    QTreeWidgetItem(_view, QTreeWidgetItem::UserType),
    m_data(_data)
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
            return m_data.name;
        case 1:
            return m_data.device->title();
        }
    }
    return QTreeWidgetItem::data(_column, _role);
}

QSharedPointer<Device> DeviceListItem::device() const
{
    return m_data.device;
}

class InitializationThread : public QThread
{
public:
    explicit InitializationThread(QObject * _parent = nullptr);
    inline const QString & errorMessage() const;
    inline QList<DeviceDisplayData> & devices();

protected:
    void run() override;

private:
    QString m_error_message;
    QList<DeviceDisplayData> m_devices;
};

InitializationThread::InitializationThread(QObject * _parent /*= nullptr*/) :
    QThread(_parent)
{
}

void InitializationThread::run()
{
    try
    {
        QList<DeviceName> devices = loadDriveList();
        for(const DeviceName & device_name : devices)
        {
            DeviceDisplayData display_data;
            display_data.device = QSharedPointer<Device>(new Device(
                QSharedPointer<DeviceSource>(new OpticalDriveDeviceSource(device_name.filename))));
            display_data.name = device_name.name;
            if(display_data.device->init())
                m_devices.append(display_data);
        }
    }
    catch(const Exception & exception)
    {
        m_error_message = exception.message();
    }
}

const QString & InitializationThread::errorMessage() const
{
    return m_error_message;
}

QList<DeviceDisplayData> & InitializationThread::devices()
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
        QList<DeviceDisplayData> & devices = thread->devices();
        for(const DeviceDisplayData & display_data : devices)
        {
            fixDeviceTitle(*display_data.device);
            mp_tree_devices->addTopLevelItem(new DeviceListItem(mp_tree_devices, display_data));
        }
        mp_tree_devices->sortItems(0, Qt::AscendingOrder);
        QString error_message = thread->errorMessage();
        if(error_message.isEmpty() && devices.isEmpty())
            error_message = tr("There are no available CD/DVD drives");
        if(error_message.isEmpty())
        {
            mp_widget_content->setVisible(true);
        }
        else
        {
            mp_label_error->setText(error_message);
            mp_label_error->setVisible(true);
        }
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void ChooseOpticalDiscDialog::fixDeviceTitle(Device & _device) const
{
    QString title = _device.title();
    if(title.isEmpty())
        title = _device.gameId();
    bool first_letter = true;
    for(int i = 0; i < title.size(); ++i)
    {
        QChar symbol = title[i];
        if(symbol == '_')
        {
            title[i] = ' ';
            first_letter = true;
        }
        else if(!first_letter && symbol.isUpper())
        {
            title[i] = symbol.toLower();
        }
        else
        {
            first_letter = false;
        }
    }
    _device.setTitle(title.trimmed());
}

void ChooseOpticalDiscDialog::deviceSelectionChanged()
{
    mp_button_box->button(QDialogButtonBox::Open)->setDisabled(mp_tree_devices->selectedItems().isEmpty());
}

QList<QSharedPointer<Device> > ChooseOpticalDiscDialog::devices() const
{
    QList<QSharedPointer<Device>> result;
    QModelIndexList indexes = mp_tree_devices->selectionModel()->selectedRows();
    for(const QModelIndex & index : indexes)
    {
        DeviceListItem * item = static_cast<DeviceListItem *>(mp_tree_devices->topLevelItem(index.row()));
        result.append(item->device());
    }
    return result;
}
