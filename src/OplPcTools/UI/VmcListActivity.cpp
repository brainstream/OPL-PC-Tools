/***********************************************************************************************
 * Copyright © 2017-2020 Sergey Smolyannikov aka brainstream                                   *
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

#include <OplPcTools/Settings.h>
#include <OplPcTools/UI/VmcListActivity.h>
#include <QPixmap>
#include <QAbstractItemModel>
#include <QShortcut>


using namespace OplPcTools::UI;

namespace {

namespace SettingsKey {

const char * icons_size = "VmcListIconSize";

} // namespace SettingsKey

class VmcListActivityIntent : public Intent
{
public:
    Activity * createActivity(QWidget * _parent) override
    {
        return new VmcListActivity(_parent);
    }

    QString activityClass() const
    {
        return "VmcList";
    }
};

class VmcTreeModel final : public QAbstractItemModel
{
public:
    VmcTreeModel();
    QModelIndex index(int _row, int _column, const QModelIndex & _parent) const override;
    QModelIndex parent(const QModelIndex & _child) const override;
    int rowCount(const QModelIndex & _parent) const override;
    int columnCount(const QModelIndex & _parent) const override;
    QVariant data(const QModelIndex & _index, int _role) const override;
    QVariant headerData(int _section, Qt::Orientation _orientation, int _role) const override;

private:
    QPixmap m_icon;
};

} // namespace

VmcTreeModel::VmcTreeModel():
    m_icon(":/images/vmc")
{
}

QModelIndex VmcTreeModel::index(int _row, int _column, const QModelIndex & _parent) const
{
    Q_UNUSED(_parent);
    return createIndex(_row, _column);
}

QModelIndex VmcTreeModel::parent(const QModelIndex & _child) const
{
    Q_UNUSED(_child);
    return QModelIndex();
}

int VmcTreeModel::rowCount(const QModelIndex & _parent) const
{
    return _parent.isValid() ? 0 : 5;
}

int VmcTreeModel::columnCount(const QModelIndex & _parent) const
{
    Q_UNUSED(_parent);
    return 2;
}

QVariant VmcTreeModel::data(const QModelIndex & _index, int _role) const
{
    if(_index.column() == 0)
    {
        switch (_role) {
        case Qt::DisplayRole:
            return QString("Test VMC #%1").arg(_index.row());
        case Qt::DecorationRole:
            return QIcon(m_icon);
        }
    }
    else if(_role == Qt::DisplayRole)
    {
        return QString("8 MB");
    }
    return QVariant();
}

QVariant VmcTreeModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    Q_UNUSED(_orientation);
    if(_role == Qt::DisplayRole)
    {
        switch (_section) {
        case 0:
            return QObject::tr("Name");
        case 1:
            return QObject::tr("Size");
        }
    }
    return QVariant();
}


VmcListActivity::VmcListActivity(QWidget * _parent /*= nullptr*/):
    Activity(_parent)
{
    setupUi(this);
    setupShortcuts();
    VmcTreeModel * model = new VmcTreeModel();
    mp_proxy_model = new QSortFilterProxyModel(this);
    mp_proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mp_proxy_model->setSourceModel(model);
    mp_proxy_model->setDynamicSortFilter(true);
    mp_tree_vmcs->setModel(mp_proxy_model);
    mp_tree_vmcs->header()->setStretchLastSection(false);
    mp_tree_vmcs->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    connect(this, &VmcListActivity::destroyed, this, &VmcListActivity::saveSettings);
    connect(mp_btn_back, &QPushButton::clicked, this, &VmcListActivity::deleteLater);
    connect(mp_slider_icons_size, &QSlider::valueChanged, this, &VmcListActivity::changeIconSize);
    connect(mp_edit_filter, &QLineEdit::textChanged, mp_proxy_model, &QSortFilterProxyModel::setFilterFixedString);
    applySettings();
}

void VmcListActivity::setupShortcuts()
{
    QShortcut * shortcut = new QShortcut(QKeySequence("Back"), this);
    connect(shortcut, &QShortcut::activated, this, &VmcListActivity::close);
    shortcut = new QShortcut(QKeySequence("Esc"), this);
    connect(shortcut, &QShortcut::activated, this, &VmcListActivity::close);
    shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    mp_edit_filter->setPlaceholderText(QString("%1 (%2)")
        .arg(mp_edit_filter->placeholderText())
        .arg(shortcut->key().toString()));
    connect(shortcut, &QShortcut::activated, [this]() { mp_edit_filter->setFocus(); });
}

void VmcListActivity::applySettings()
{
    QSettings settings;
    QVariant icons_size_value = settings.value(SettingsKey::icons_size);
    int icons_size = 2;
    if(!icons_size_value.isNull() && icons_size_value.canConvert(QVariant::Int))
    {
        icons_size = icons_size_value.toInt();
        if(icons_size > 4) icons_size = 4;
        else if(icons_size < 1) icons_size = 1;
    }
    mp_slider_icons_size->setValue(icons_size);
    changeIconSize();
}

void VmcListActivity::changeIconSize()
{
    int size = mp_slider_icons_size->value() * 16;
    mp_tree_vmcs->setIconSize(QSize(size, size));
}

void VmcListActivity::saveSettings()
{
    QSettings settings;
    settings.setValue(SettingsKey::icons_size, mp_slider_icons_size->value());
}

QSharedPointer<Intent> VmcListActivity::createIntent()
{
    return QSharedPointer<Intent>(new VmcListActivityIntent);
}
