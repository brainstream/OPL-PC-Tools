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
#include "LibCDIO.h"
#include "Exception.h"
#include "ChooseOpticalDiscDialog.h"

namespace {

QString readDeviceLabel(CdIo_t * _device)
{
    cdio_iso_analysis_t analysis = {};
    lsn_t session = 0;
    track_t first_track = cdio_get_first_track_num(_device);
    if(first_track == CDIO_INVALID_TRACK || cdio_get_last_session(_device, &session) != DRIVER_OP_SUCCESS)
        return QString();
    cdio_guess_cd_type(_device, session, first_track, &analysis);
    return QString(analysis.iso_label).trimmed();
}

class DeviceListItem : public QTreeWidgetItem
{
public:
    DeviceListItem(QTreeWidget * _view, const QString & _device, const QString _title);
    QVariant data(int _column, int _role) const;
    inline const ChooseOpticalDiscDialog::DeviceInfo & device() const;

private:
    ChooseOpticalDiscDialog::DeviceInfo m_device_info;
};

DeviceListItem::DeviceListItem(QTreeWidget * _view, const QString & _device, const QString _title) :
    QTreeWidgetItem(_view, QTreeWidgetItem::UserType)
{
    setIcon(0, QIcon(":/icons/media-optical"));
    m_device_info.device = _device;
    m_device_info.title = _title;
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

} // namespace

ChooseOpticalDiscDialog::ChooseOpticalDiscDialog(QWidget * _parent /*= nullptr*/) :
    QDialog(_parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint)
{
    setupUi(this);
    mp_label_error->setVisible(false);
    mp_tree_devices->setColumnWidth(0, 180);
    mp_button_box->button(QDialogButtonBox::Open)->setDisabled(true);
    try
    {
        initLibCDIO();
        char ** devices = cdio_get_devices(DRIVER_DEVICE);
        for(size_t i = 0; devices[i]; ++i)
        {
            CdIo * cdio = cdio_open_cd(devices[i]);
            if(!cdio) continue;
            mp_tree_devices->addTopLevelItem(new DeviceListItem(mp_tree_devices, devices[i], readDeviceLabel(cdio)));
            cdio_destroy(cdio);
        }
    }
    catch(const Exception & exception)
    {
        m_error_message = exception.message();
    }
    if(m_error_message.isEmpty() && mp_tree_devices->topLevelItemCount() == 0)
    {
        m_error_message = tr("There are no available CD/DVD drives");
    }
    if(!m_error_message.isEmpty())
    {
        mp_label_error->setText(m_error_message);
        mp_label_error->setVisible(true);
        mp_tree_devices->setVisible(false);
    }
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
