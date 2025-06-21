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

#include <QSettings>
#include <QMessageBox>
#include <QToolTip>
#include <OplPcTools/Exception.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/VmcCreateDialog.h>
#include <OplPcTools/UI/BusySmartThread.h>
#include <OplPcTools/UI/GameConfigWidget.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

namespace SettingsKey {

const QString config_version("OplCfgVersion");

} // namespace SettingsKey

GameConfigurationVersion getConfigVersionFromSettings(GameConfigurationVersion _defualt)
{
    QSettings settings;
    int version =  settings.value(SettingsKey::config_version, -1).toInt();
    switch(version)
    {
    case static_cast<int>(GameConfigurationVersion::OPLv093):
        return GameConfigurationVersion::OPLv093;
    case static_cast<int>(GameConfigurationVersion::OPLv100):
        return GameConfigurationVersion::OPLv100;
    default:
        return _defualt;
    }
}

QString gameVideoModeToString(VideoMode mode)
{
    switch(mode)
    {
    case VideoMode::NTSC:
        return "NTSC";
    case VideoMode::NTSC_Non_Interlaced:
        return "NTSC Non-Interlaced";
    case VideoMode::PAL:
        return "PAL";
    case VideoMode::PAL_Non_Interlaced:
        return "PAL Non-Interlaced";
    case VideoMode::PAL_60Hz:
        return "PAL @60Hz";
    case VideoMode::PAL_60Hz_Non_Interlaced:
        return "PAL @60Hz Non-Interlaced";
    case VideoMode::PS1_NTSC_HDTV_480p_60Hz:
        return "PS1 NTSC (HDTV 480p @60Hz)";
    case VideoMode::PS1_PAL_HDTV_576p_50Hz:
        return "PS1 PAL (HDTV 576p @50Hz)";
    case VideoMode::HDTV_480p_60Hz:
        return "HDTV 480p @60Hz";
    case VideoMode::HDTV_576p_50Hz:
        return "HDTV 576p @50Hz";
    case VideoMode::HDTV_720p_60Hz:
        return "HDTV 720p @60Hz";
    case VideoMode::HDTV_1080i_60Hz:
        return "HDTV 1080i @60Hz";
    case VideoMode::HDTV_1080i_60Hz_Non_Interlaced:
        return "HDTV 1080i @60Hz Non-Interlaced";
    case VideoMode::HDTV_1080p_60Hz:
        return "HDTV 1080p @60Hz";
    case VideoMode::VGA_640x480p_60Hz:
        return "VGA 640x480p @60Hz";
    case VideoMode::VGA_640x480p_72Hz:
        return "VGA 640x480p @72Hz";
    case VideoMode::VGA_640x480p_75Hz:
        return "VGA 640x480p @75Hz";
    case VideoMode::VGA_640x480p_85Hz:
        return "VGA 640x480p @85Hz";
    case VideoMode::VGA_640x480i_60Hz:
        return "VGA 640x480i @60Hz";
    case VideoMode::VGA_640x960i_60Hz:
        return "VGA 640x960i @60Hz";
    case VideoMode::VGA_800x600p_56Hz:
        return "VGA 800x600p @56Hz";
    case VideoMode::VGA_800x600p_60Hz:
        return "VGA 800x600p @60Hz";
    case VideoMode::VGA_800x600p_72Hz:
        return "VGA 800x600p @72Hz";
    case VideoMode::VGA_800x600p_75Hz:
        return "VGA 800x600p @75Hz";
    case VideoMode::VGA_800x600p_85Hz:
        return "VGA 800x600p @85Hz";
    case VideoMode::VGA_1024x768p_60Hz:
        return "VGA 1024x768p @60Hz";
    case VideoMode::VGA_1024x768p_70Hz:
        return "VGA 1024x768p @70Hz";
    case VideoMode::VGA_1024x768p_75Hz:
        return "VGA 1024x768p @75Hz";
    case VideoMode::VGA_1024x768p_85Hz:
        return "VGA 1024x768p @85Hz";
    case VideoMode::VGA_1280x1024p_60Hz:
        return "VGA 1280x1024p @60Hz";
    case VideoMode::VGA_1280x1024p_75Hz:
        return "VGA 1280x1024p @75Hz";
    default:
        return "<UNKNOWN>";
    }
}

} // namespace

GameConfigWidget::GameConfigWidget(const Game & _game, QWidget * _parent /*= nullptr*/) :
    QWidget( _parent),
    mr_vmcs(Library::instance().vmcs()),
    mr_game(_game),
    m_config_version(getConfigVersionFromSettings(GameConfigurationVersion::OPLv100))
{
    setupUi(this);
    if(!loadConfiguration())
    {
        setDisabled(true);
        return;
    }
    initControls();
    void (QComboBox::*opl_version_changed)(int) = &QComboBox::currentIndexChanged;
    connect(mp_combo_opl_version, opl_version_changed, [this](int) { onOplVerstionChanged(); });
    connect(mp_radio_disable_gsm, &QRadioButton::toggled, [this](bool) { onGsmStateChanged(); });
    connect(mp_radio_enable_gsm, &QRadioButton::toggled, [this](bool) { onGsmStateChanged(); });
    connect(mp_radio_use_global_gsm, &QRadioButton::toggled, [this](bool) { onGsmStateChanged(); });
    connect(&mr_vmcs, &VmcCollection::vmcAdded, this, &GameConfigWidget::onVmcAdded);
    connect(&mr_vmcs, &VmcCollection::vmcDeleted, this, &GameConfigWidget::onVmcDeleted);
    connect(&mr_vmcs, &VmcCollection::vmcRenamed, this, &GameConfigWidget::onVmcRenamed);
    connect(mp_btn_create_vmc0, &QPushButton::clicked, this, &GameConfigWidget::createVmc0);
    connect(mp_btn_create_vmc1, &QPushButton::clicked, this, &GameConfigWidget::createVmc1);
    connect(mp_btn_id_from_game, &QPushButton::clicked, this, &GameConfigWidget::fillGameIdFromGame);
    connect(mp_btn_clear, &QPushButton::clicked, this, &GameConfigWidget::clear);
    connect(mp_btn_save, &QPushButton::clicked, this, &GameConfigWidget::save);
    connect(mp_btn_delete, &QPushButton::clicked, this, &GameConfigWidget::remove);
}

bool GameConfigWidget::loadConfiguration()
{
    try
    {
        QString filename = GameConfiguration::makeFilename(Library::instance().directory(), mr_game.id());
        m_config_ptr = GameConfiguration::load(filename);
        return true;
    }
    catch(const Exception & _exception)
    {
        Application::showErrorMessage(_exception.message());
    }
    catch(...)
    {
        Application::showErrorMessage();
    }
    return false;
}

void GameConfigWidget::initControls()
{
    mp_combo_opl_version->addItem("≤ 9.3.0", static_cast<int>(GameConfigurationVersion::OPLv093));
    mp_combo_opl_version->addItem("≥ 1.0.0", static_cast<int>(GameConfigurationVersion::OPLv100));
    mp_combo_opl_version->setCurrentIndex(m_config_version == GameConfigurationVersion::OPLv093 ? 0 : 1);
    mp_combo_vmc0->addItem("", QUuid(nullptr));
    mp_combo_vmc1->addItem("", QUuid(nullptr));
    int count = mr_vmcs.count();
    for(int i = 0; i < count; ++i)
    {
        const Vmc * vmc = mr_vmcs[i];
        mp_combo_vmc0->addItem(vmc->title(), vmc->uuid().quuid());
        mp_combo_vmc1->addItem(vmc->title(), vmc->uuid().quuid());
    }
    sortVmcComboBoxes();
    setVmcComboBoxValue(mp_combo_vmc0, m_config_ptr->vmc0);
    setVmcComboBoxValue(mp_combo_vmc1, m_config_ptr->vmc1);
    mp_edit_elf->setText(m_config_ptr->custom_elf);
    mp_edit_game_id->setText(m_config_ptr->game_id);
    mp_checkbox_mode_1->setChecked(m_config_ptr->is_mode_1_enabled);
    mp_checkbox_mode_2->setChecked(m_config_ptr->is_mode_2_enabled);
    mp_checkbox_mode_3->setChecked(m_config_ptr->is_mode_3_enabled);
    mp_checkbox_mode_4->setChecked(m_config_ptr->is_mode_4_enabled);
    mp_checkbox_mode_5->setChecked(m_config_ptr->is_mode_5_enabled);
    mp_checkbox_mode_6->setChecked(m_config_ptr->is_mode_6_enabled);
    mp_checkbox_mode_7->setChecked(m_config_ptr->is_mode_7_enabled);
    mp_checkbox_mode_8->setChecked(m_config_ptr->is_mode_8_enabled);
    mp_groupbox_gsm->setChecked(m_config_ptr->is_gsm_enabled);
    mp_checkbox_skip_fmv->setChecked(m_config_ptr->is_gsm_skip_fmv_enabled);
    mp_spinbox_hpos->setValue(m_config_ptr->gsm_x_offset);
    mp_spinbox_vpos->setValue(m_config_ptr->gsm_y_offset);
    mp_checkbox_emulate_field_flipping->setChecked(m_config_ptr->is_gsm_emulate_field_flipping_enabled);
    reinitOplVersionSensitiveControls();
    if(m_config_ptr->is_gsm_enabled)
    {
        if(m_config_ptr->is_global_gsm_enabled)
        {
            if(mp_radio_use_global_gsm->isEnabled())
                mp_radio_use_global_gsm->setChecked(true);
            else
                mp_radio_disable_gsm->setChecked(true);
        }
        else
        {
            mp_radio_enable_gsm->setChecked(true);
        }
    }
    else
    {
        mp_radio_disable_gsm->setChecked(true);
    }
    mp_combo_video_mode->setCurrentIndex(m_config_ptr->gsm_video_mode);
    onGsmStateChanged();
}

void GameConfigWidget::reinitOplVersionSensitiveControls()
{
    fillVideoModeComboBox();
    if(m_config_version == GameConfigurationVersion::OPLv093)
    {
        if(mp_radio_use_global_gsm->isChecked())
            mp_radio_disable_gsm->setChecked(true);
        mp_radio_use_global_gsm->setEnabled(false);
        mp_checkbox_emulate_field_flipping->setEnabled(false);
        mp_checkbox_mode_4->setText(tr("Mode 4 ― 0 PSS Mode"));
        mp_checkbox_mode_7->setEnabled(true);
        mp_checkbox_mode_7->setText(tr("Mode 7 ― High Module Storage"));
        mp_checkbox_mode_8->setEnabled(true);
        mp_checkbox_mode_8->setText(tr("Mode 8 ― Hide DEV9 Module"));
        mp_checkbox_skip_fmv->setEnabled(true);
    }
    else
    {
        mp_radio_use_global_gsm->setEnabled(true);
        mp_checkbox_emulate_field_flipping->setEnabled(true);
        mp_checkbox_mode_4->setText(tr("Mode 4 ― Skip Videos"));
        mp_checkbox_mode_7->setChecked(false);
        mp_checkbox_mode_7->setText(tr("Mode 7 ― Deprecated"));
        mp_checkbox_mode_7->setDisabled(true);
        mp_checkbox_mode_8->setText(tr("Mode 8 ― Deprecated"));
        mp_checkbox_mode_8->setChecked(false);
        mp_checkbox_mode_8->setDisabled(true);
        mp_checkbox_skip_fmv->setChecked(false);
        mp_checkbox_skip_fmv->setDisabled(true);
    }
}

void GameConfigWidget::sortVmcComboBoxes()
{
    mp_combo_vmc0->model()->sort(0);
    mp_combo_vmc1->model()->sort(0);
}

void GameConfigWidget::setVmcComboBoxValue(QComboBox * _combobox, const QString & _value)
{
    if(_value.isEmpty())
        return;
    int count = _combobox->count();
    for(int i = 0; i < count; ++i)
    {
        if(_combobox->itemText(i) == _value)
        {
            _combobox->setCurrentIndex(i);
            return;
        }
    }
    _combobox->setEditText(_value);
}

void GameConfigWidget::fillVideoModeComboBox()
{
    int index = mp_combo_video_mode->currentIndex();
    mp_combo_video_mode->clear();
    QVector<VideoMode> modes = GameConfiguration::supportedVideoModes(m_config_version);
    for(int i = 0; i < modes.size(); ++i)
        mp_combo_video_mode->addItem(gameVideoModeToString(modes[i]), i);
    if(index >= 0)
        mp_combo_video_mode->setCurrentIndex(index);
}

void GameConfigWidget::onOplVerstionChanged()
{
    int conf_version = mp_combo_opl_version->currentData().toInt();
    m_config_version = static_cast<GameConfigurationVersion>(conf_version);
    QSettings().setValue(SettingsKey::config_version, conf_version);
    reinitOplVersionSensitiveControls();
}

void GameConfigWidget::onGsmStateChanged()
{
    bool enabled = mp_radio_enable_gsm->isChecked();
    bool use_global_gsm = mp_radio_use_global_gsm->isChecked();
    mp_groupbox_gsm_settings->setEnabled(enabled && !use_global_gsm);
}

void GameConfigWidget::onVmcAdded(const Uuid & _id)
{
    const Vmc * vmc = mr_vmcs[_id];
    mp_combo_vmc0->addItem(vmc->title(), vmc->uuid().quuid());
    mp_combo_vmc1->addItem(vmc->title(), vmc->uuid().quuid());
    sortVmcComboBoxes();
}

void GameConfigWidget::onVmcDeleted(const Uuid & _id)
{
    deleteVmcComboBoxItem(mp_combo_vmc0, _id);
    deleteVmcComboBoxItem(mp_combo_vmc1, _id);
}

void GameConfigWidget::deleteVmcComboBoxItem(QComboBox * _combobox, const Uuid & _id)
{
    int index = getVmcComboBoxItemIndex(_combobox, _id);
    if(index >= 0) _combobox->removeItem(index);
}

int GameConfigWidget::getVmcComboBoxItemIndex(QComboBox * _combobox, const Uuid & _id) const
{
    int count = _combobox->count();
    for(int i = 0; i < count; ++i)
    {
        if(_id == _combobox->itemData(i).toUuid())
            return i;
    }
    return -1;
}

void GameConfigWidget::onVmcRenamed(const Uuid & _id)
{
    const Vmc * vmc = mr_vmcs[_id];
    renameVmcComboBoxItem(mp_combo_vmc0, _id, vmc->title());
    renameVmcComboBoxItem(mp_combo_vmc1, _id, vmc->title());
    sortVmcComboBoxes();
}

void GameConfigWidget::renameVmcComboBoxItem(QComboBox * _combobox, const Uuid & _id, const QString & _title)
{
    int index = getVmcComboBoxItemIndex(_combobox, _id);
    if(index >= 0) _combobox->setItemText(index, _title);
}

void GameConfigWidget::createVmc0()
{
    const Vmc * vmc = createVmc();
    if(vmc)
    {
        int index = getVmcComboBoxItemIndex(mp_combo_vmc0, vmc->uuid());
        if(index >= 0) mp_combo_vmc0->setCurrentIndex(index);
    }
}

void GameConfigWidget::createVmc1()
{
    const Vmc * vmc = createVmc();
    if(vmc)
    {
        int index = getVmcComboBoxItemIndex(mp_combo_vmc1, vmc->uuid());
        if(index >= 0) mp_combo_vmc1->setCurrentIndex(index);
    }
}

const Vmc * GameConfigWidget::createVmc()
{
    VmcCreateDialog dlg(this);
    dlg.setTitle(mr_game.title());
    if(dlg.exec() == QDialog::Accepted)
    {
        return dlg.createdVmc();
    }
    return nullptr;
}

void GameConfigWidget::fillGameIdFromGame()
{
    mp_edit_game_id->setText(mr_game.id());
}

void GameConfigWidget::clear()
{
    clearForm();
    QString filename = GameConfiguration::makeFilename(Library::instance().directory(), mr_game.id());
    if(QFile::exists(filename))
        saveAs(filename);
    QToolTip::showText(mapToGlobal(mp_btn_clear->pos() - QPoint(0, 50)),
        tr("Configuration cleared"), this, QRect(), 3000);
}

void GameConfigWidget::clearForm()
{
    mp_edit_elf->clear();
    mp_edit_game_id->clear();
    mp_checkbox_mode_1->setChecked(false);
    mp_checkbox_mode_2->setChecked(false);
    mp_checkbox_mode_3->setChecked(false);
    mp_checkbox_mode_4->setChecked(false);
    mp_checkbox_mode_5->setChecked(false);
    mp_checkbox_mode_6->setChecked(false);
    mp_checkbox_mode_7->setChecked(false);
    mp_checkbox_mode_8->setChecked(false);
    mp_combo_video_mode->setCurrentIndex(-1);
    mp_combo_vmc0->setCurrentIndex(-1);
    mp_combo_vmc1->setCurrentIndex(-1);
    mp_groupbox_gsm->setChecked(false);
    mp_checkbox_skip_fmv->setChecked(false);
    mp_checkbox_emulate_field_flipping->setChecked(false);
    mp_spinbox_hpos->clear();
    mp_spinbox_vpos->clear();
    mp_radio_disable_gsm->setChecked(true);
}

void GameConfigWidget::save()
{
    saveAs(GameConfiguration::makeFilename(Library::instance().directory(), mr_game.id()));
    QToolTip::showText(mapToGlobal(mp_btn_save->pos() - QPoint(0, 50)),
        tr("Configuration saved"), this, QRect(), 3000);
}

void GameConfigWidget::saveAs(const QString & _filename)
{
    startSmartThread([this, _filename]() {
        m_config_ptr->custom_elf = mp_edit_elf->text();
        m_config_ptr->game_id = mp_edit_game_id->text();
        m_config_ptr->is_mode_1_enabled = mp_checkbox_mode_1->isChecked();
        m_config_ptr->is_mode_2_enabled = mp_checkbox_mode_2->isChecked();
        m_config_ptr->is_mode_3_enabled = mp_checkbox_mode_3->isChecked();
        m_config_ptr->is_mode_4_enabled = mp_checkbox_mode_4->isChecked();
        m_config_ptr->is_mode_5_enabled = mp_checkbox_mode_5->isChecked();
        m_config_ptr->is_mode_6_enabled = mp_checkbox_mode_6->isChecked();
        m_config_ptr->is_mode_7_enabled = mp_checkbox_mode_7->isChecked();
        m_config_ptr->is_mode_8_enabled = mp_checkbox_mode_8->isChecked();
        m_config_ptr->gsm_video_mode = mp_combo_video_mode->currentIndex();
        m_config_ptr->vmc0 = mp_combo_vmc0->currentText();
        m_config_ptr->vmc1 = mp_combo_vmc1->currentText();
        bool is_gsm_enabled = mp_radio_enable_gsm->isChecked();
        bool is_gsm_global_enabled = mp_radio_use_global_gsm->isChecked();
        m_config_ptr->is_gsm_enabled = is_gsm_enabled || is_gsm_global_enabled;
        m_config_ptr->is_global_gsm_enabled = is_gsm_global_enabled;
        m_config_ptr->is_gsm_skip_fmv_enabled = mp_checkbox_skip_fmv->isChecked();
        m_config_ptr->is_gsm_emulate_field_flipping_enabled = mp_checkbox_emulate_field_flipping->isChecked();
        m_config_ptr->gsm_x_offset = mp_spinbox_hpos->value();
        m_config_ptr->gsm_y_offset = mp_spinbox_vpos->value();
        m_config_ptr->save(*m_config_ptr, _filename);
    });
}

void GameConfigWidget::remove()
{
    QString filename = GameConfiguration::makeFilename(Library::instance().directory(), mr_game.id());
    QFile config(filename);
    if(!config.exists() || QMessageBox::question(this, tr("Confirmation"),
        tr("Are you sure you want to delete file?\n%1").arg(filename),
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
    {
        QToolTip::showText(mapToGlobal(mp_btn_delete->pos() - QPoint(0, 50)),
            tr("Configuration file does not exist"), this, QRect(), 3000);
        return;
    }
    startSmartThread([this, filename]() {
        QFile::remove(filename);
        clearForm();
    });
}

void GameConfigWidget::startSmartThread(std::function<void()> _lambda)
{
    BusySmartThread * thread = new BusySmartThread(_lambda, this);
    connect(thread, &BusySmartThread::finished, thread, &BusySmartThread::deleteLater);
    connect(thread, &BusySmartThread::exception, [](const QString & message) {
        Application::showErrorMessage(message);
    });
    thread->setSpinnerDisplayTimeout(300);
    thread->start();
}
