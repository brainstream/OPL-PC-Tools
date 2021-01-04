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

#include <QSettings>
#include <OplPcTools/Exception.h>
#include <OplPcTools/Library.h>
#include <OplPcTools/UI/Application.h>
#include <OplPcTools/UI/VmcCreateDialog.h>
#include <OplPcTools/UI/GameConfigWidget.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {

namespace SettingsKey {

static const QString config_version("OplCfgVersion");

} // namespace SettingsKey

enum OplVersion: int
{
    OPL_093,
    OPL_100
};

int getConfigVersionFromSettings(int _defualt)
{
    QSettings settings;
    int version =  settings.value(SettingsKey::config_version, -1).toInt();
    switch(version)
    {
    case OPL_093:
        return OPL_093;
    case OPL_100:
        return OPL_100;
    default:
        return _defualt;
    }
}

QString gameVideoModeToString(GameVideoMode mode)
{
    switch(mode)
    {
    case GameVideoMode::NTSC:
        return "NTSC";
    case GameVideoMode::NTSC_Non_Interlaced:
        return "NTSC Non-Interlaced";
    case GameVideoMode::PAL:
        return "PAL";
    case GameVideoMode::PAL_Non_Interlaced:
        return "PAL Non-Interlaced";
    case GameVideoMode::PAL_60Hz:
        return "PAL @60Hz";
    case GameVideoMode::PAL_60Hz_Non_Interlaced:
        return "PAL @60Hz Non-Interlaced";
    case GameVideoMode::PS1_NTSC_HDTV_480p_60Hz:
        return "PS1 NTSC (HDTV 480p @60Hz)";
    case GameVideoMode::PS1_PAL_HDTV_576p_50Hz:
        return "PS1 PAL (HDTV 576p @50Hz)";
    case GameVideoMode::HDTV_480p_60Hz:
        return "HDTV 480p @60Hz";
    case GameVideoMode::HDTV_576p_50Hz:
        return "HDTV 576p @50Hz";
    case GameVideoMode::HDTV_720p_60Hz:
        return "HDTV 720p @60Hz";
    case GameVideoMode::HDTV_1080i_60Hz:
        return "HDTV 1080i @60Hz";
    case GameVideoMode::HDTV_1080i_60Hz_Non_Interlaced:
        return "HDTV 1080i @60Hz Non-Interlaced";
    case GameVideoMode::HDTV_1080p_60Hz:
        return "HDTV 1080p @60Hz";
    case GameVideoMode::VGA_640x480p_60Hz:
        return "VGA 640x480p @60Hz";
    case GameVideoMode::VGA_640x480p_72Hz:
        return "VGA 640x480p @72Hz";
    case GameVideoMode::VGA_640x480p_75Hz:
        return "VGA 640x480p @75Hz";
    case GameVideoMode::VGA_640x480p_85Hz:
        return "VGA 640x480p @85Hz";
    case GameVideoMode::VGA_640x480i_60Hz:
        return "VGA 640x480i @60Hz";
    case GameVideoMode::VGA_640x960i_60Hz:
        return "VGA 640x960i @60Hz";
    case GameVideoMode::VGA_800x600p_56Hz:
        return "VGA 800x600p @56Hz";
    case GameVideoMode::VGA_800x600p_60Hz:
        return "VGA 800x600p @60Hz";
    case GameVideoMode::VGA_800x600p_72Hz:
        return "VGA 800x600p @72Hz";
    case GameVideoMode::VGA_800x600p_75Hz:
        return "VGA 800x600p @75Hz";
    case GameVideoMode::VGA_800x600p_85Hz:
        return "VGA 800x600p @85Hz";
    case GameVideoMode::VGA_1024x768p_60Hz:
        return "VGA 1024x768p @60Hz";
    case GameVideoMode::VGA_1024x768p_70Hz:
        return "VGA 1024x768p @70Hz";
    case GameVideoMode::VGA_1024x768p_75Hz:
        return "VGA 1024x768p @75Hz";
    case GameVideoMode::VGA_1024x768p_85Hz:
        return "VGA 1024x768p @85Hz";
    case GameVideoMode::VGA_1280x1024p_60Hz:
        return "VGA 1280x1024p @60Hz";
    case GameVideoMode::VGA_1280x1024p_75Hz:
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
    m_opl_version(getConfigVersionFromSettings(OPL_093))
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
    mp_combo_opl_version->addItem("≤ 9.3.0", OPL_093);
    mp_combo_opl_version->addItem("≥ 1.0.0", OPL_100);
    mp_combo_opl_version->setCurrentIndex(m_opl_version == OPL_093 ? 0 : 1);
    mp_combo_vmc0->addItem("", QUuid(nullptr));
    mp_combo_vmc1->addItem("", QUuid(nullptr));
    int count = mr_vmcs.count();
    for(int i = 0; i < count; ++i)
    {
        const Vmc * vmc = mr_vmcs[i];
        mp_combo_vmc0->addItem(vmc->title(), vmc->uuid());
        mp_combo_vmc1->addItem(vmc->title(), vmc->uuid());
    }
    sortVmcComboBoxes();
    setVmcComboBoxValue(mp_combo_vmc0, m_config_ptr->vmc0());
    setVmcComboBoxValue(mp_combo_vmc1, m_config_ptr->vmc1());
    mp_edit_elf->setText(m_config_ptr->customElf());
    mp_edit_game_id->setText(m_config_ptr->gameId());
    mp_checkbox_mode_1->setChecked(m_config_ptr->isMode1Enalbed());
    mp_checkbox_mode_2->setChecked(m_config_ptr->isMode2Enalbed());
    mp_checkbox_mode_3->setChecked(m_config_ptr->isMode3Enalbed());
    mp_checkbox_mode_4->setChecked(m_config_ptr->isMode4Enalbed());
    mp_checkbox_mode_5->setChecked(m_config_ptr->isMode5Enalbed());
    mp_checkbox_mode_6->setChecked(m_config_ptr->isMode6Enalbed());
    mp_checkbox_mode_7->setChecked(m_config_ptr->isMode7Enalbed());
    mp_checkbox_mode_8->setChecked(m_config_ptr->isMode8Enalbed());
    mp_groupbox_gsm->setChecked(m_config_ptr->isGsmEnabled());
    mp_checkbox_skip_fmv->setChecked(m_config_ptr->gsmSkipFmv());
    mp_spinbox_hpos->setValue(m_config_ptr->gsmXOffset());
    mp_spinbox_vpos->setValue(m_config_ptr->gsmYOffset());
    reinitOplVersionSensitiveControls();
    if(m_config_ptr->isGsmEnabled())
    {
        if(m_config_ptr->isGlobalGsmEnabled())
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
    mp_combo_video_mode->setCurrentIndex(m_config_ptr->gsmVideoMode());
    mp_checkbox_emulate_field_flipping->setChecked(m_config_ptr->isEmulationFieldFlippingEnabledd());
    onGsmStateChanged();
}

void GameConfigWidget::reinitOplVersionSensitiveControls()
{
    fillVideoModeComboBox();
    if(m_opl_version == OPL_093)
    {
        mp_radio_disable_gsm->setChecked(true);
        mp_radio_use_global_gsm->setEnabled(false);
        mp_checkbox_emulate_field_flipping->setChecked(false);
        mp_checkbox_emulate_field_flipping->setEnabled(false);
    }
    else
    {
        mp_radio_use_global_gsm->setEnabled(true);
        mp_checkbox_emulate_field_flipping->setEnabled(true);
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
    if(m_opl_version == OPL_093)
    {
        GameVideoMode_v093 mode;
        addVideoModeToComboBox(mode.ntsc);
        addVideoModeToComboBox(mode.ntsc_non_interlaced);
        addVideoModeToComboBox(mode.pal);
        addVideoModeToComboBox(mode.pal_non_interlaced);
        addVideoModeToComboBox(mode.pal_60hz);
        addVideoModeToComboBox(mode.pal_60hz_non_interlaced);
        addVideoModeToComboBox(mode.ps1_ntsc_hdtv_480p_60hz);
        addVideoModeToComboBox(mode.ps1_pal_hdtv_576p_50hz);
        addVideoModeToComboBox(mode.hdtv_480p_60hz);
        addVideoModeToComboBox(mode.hdtv_576p_50hz);
        addVideoModeToComboBox(mode.hdtv_720p_60hz);
        addVideoModeToComboBox(mode.hdtv_1080i_60hz);
        addVideoModeToComboBox(mode.hdtv_1080i_60hz_non_interlaced);
        addVideoModeToComboBox(mode.hdtv_1080p_60hz);
        addVideoModeToComboBox(mode.vga_640x480p_60hz);
        addVideoModeToComboBox(mode.vga_640x480p_72hz);
        addVideoModeToComboBox(mode.vga_640x480p_75hz);
        addVideoModeToComboBox(mode.vga_640x480p_85hz);
        addVideoModeToComboBox(mode.vga_640x480i_60hz);
    }
    else
    {
        GameVideoMode_v100 mode;
        addVideoModeToComboBox(mode.ntsc);
        addVideoModeToComboBox(mode.ntsc_non_interlaced);
        addVideoModeToComboBox(mode.pal);
        addVideoModeToComboBox(mode.pal_non_interlaced);
        addVideoModeToComboBox(mode.pal_60hz);
        addVideoModeToComboBox(mode.pal_60hz_non_interlaced);
        addVideoModeToComboBox(mode.ps1_ntsc_hdtv_480p_60hz);
        addVideoModeToComboBox(mode.ps1_pal_hdtv_576p_50hz);
        addVideoModeToComboBox(mode.hdtv_480p_60hz);
        addVideoModeToComboBox(mode.hdtv_576p_50hz);
        addVideoModeToComboBox(mode.hdtv_720p_60hz);
        addVideoModeToComboBox(mode.hdtv_1080i_60hz);
        addVideoModeToComboBox(mode.hdtv_1080i_60hz_non_interlaced);
        addVideoModeToComboBox(mode.vga_640x480p_60hz);
        addVideoModeToComboBox(mode.vga_640x480p_72hz);
        addVideoModeToComboBox(mode.vga_640x480p_75hz);
        addVideoModeToComboBox(mode.vga_640x480p_85hz);
        addVideoModeToComboBox(mode.vga_640x960i_60hz);
        addVideoModeToComboBox(mode.vga_800x600p_56hz);
        addVideoModeToComboBox(mode.vga_800x600p_60hz);
        addVideoModeToComboBox(mode.vga_800x600p_72hz);
        addVideoModeToComboBox(mode.vga_800x600p_75hz);
        addVideoModeToComboBox(mode.vga_800x600p_85hz);
        addVideoModeToComboBox(mode.vga_1024x768p_60hz);
        addVideoModeToComboBox(mode.vga_1024x768p_70hz);
        addVideoModeToComboBox(mode.vga_1024x768p_75hz);
        addVideoModeToComboBox(mode.vga_1024x768p_85hz);
        addVideoModeToComboBox(mode.vga_1280x1024p_60hz);
        addVideoModeToComboBox(mode.vga_1280x1024p_75hz);
    }
    if(index >= 0)
        mp_combo_video_mode->setCurrentIndex(index);
}

void GameConfigWidget::addVideoModeToComboBox(const GameVideoModeItem & _item)
{
    mp_combo_video_mode->addItem(gameVideoModeToString(_item.mode), _item.index);
}

void GameConfigWidget::onOplVerstionChanged()
{
    m_opl_version = mp_combo_opl_version->currentData().toInt();
    QSettings().setValue(SettingsKey::config_version, m_opl_version);
    reinitOplVersionSensitiveControls();
}

void GameConfigWidget::onGsmStateChanged()
{
    bool enabled = mp_radio_enable_gsm->isChecked();
    bool use_global_gsm = mp_radio_use_global_gsm->isChecked();
    mp_groupbox_gsm_settings->setEnabled(enabled && !use_global_gsm);
}

void GameConfigWidget::onVmcAdded(const QUuid & _id)
{
    const Vmc * vmc = mr_vmcs[_id];
    mp_combo_vmc0->addItem(vmc->title(), vmc->uuid());
    mp_combo_vmc1->addItem(vmc->title(), vmc->uuid());
    sortVmcComboBoxes();
}

void GameConfigWidget::onVmcDeleted(const QUuid & _id)
{
    deleteVmcComboBoxItem(mp_combo_vmc0, _id);
    deleteVmcComboBoxItem(mp_combo_vmc1, _id);
}

void GameConfigWidget::deleteVmcComboBoxItem(QComboBox * _combobox, const QUuid & _id)
{
    int index = getVmcComboBoxItemIndex(_combobox, _id);
    if(index >= 0) _combobox->removeItem(index);
}

int GameConfigWidget::getVmcComboBoxItemIndex(QComboBox * _combobox, const QUuid & _id) const
{
    int count = _combobox->count();
    for(int i = 0; i < count; ++i)
    {
        if(_combobox->itemData(i).toUuid() == _id)
            return i;
    }
    return -1;
}

void GameConfigWidget::onVmcRenamed(const QUuid & _id)
{
    const Vmc * vmc = mr_vmcs[_id];
    renameVmcComboBoxItem(mp_combo_vmc0, _id, vmc->title());
    renameVmcComboBoxItem(mp_combo_vmc1, _id, vmc->title());
    sortVmcComboBoxes();
}

void GameConfigWidget::renameVmcComboBoxItem(QComboBox * _combobox, const QUuid & _id, const QString & _title)
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
    save();
}

void GameConfigWidget::save()
{
    m_config_ptr->setCustomElf(mp_edit_elf->text());
    m_config_ptr->setGameId(mp_edit_game_id->text());
    m_config_ptr->setMode1Enabled(mp_checkbox_mode_1->isChecked());
    m_config_ptr->setMode2Enabled(mp_checkbox_mode_2->isChecked());
    m_config_ptr->setMode3Enabled(mp_checkbox_mode_3->isChecked());
    m_config_ptr->setMode4Enabled(mp_checkbox_mode_4->isChecked());
    m_config_ptr->setMode5Enabled(mp_checkbox_mode_5->isChecked());
    m_config_ptr->setMode6Enabled(mp_checkbox_mode_6->isChecked());
    m_config_ptr->setMode7Enabled(mp_checkbox_mode_7->isChecked());
    m_config_ptr->setMode8Enabled(mp_checkbox_mode_8->isChecked());
    m_config_ptr->setVideoMode(mp_combo_video_mode->currentIndex());
    m_config_ptr->setVmc0(mp_combo_vmc0->currentText());
    m_config_ptr->setVmc1(mp_combo_vmc1->currentText());
    bool is_gsm_enabled = mp_groupbox_gsm->isChecked();
    bool is_gsm_global_enabled = mp_radio_use_global_gsm->isChecked();
    m_config_ptr->setGsmEnabled(is_gsm_enabled || is_gsm_global_enabled);
    m_config_ptr->setGlobalGsmEnabled(is_gsm_global_enabled);
    m_config_ptr->setSkipFmv(mp_checkbox_skip_fmv->isChecked());
    m_config_ptr->setEmulationFieldFlippingEnabledd(mp_checkbox_emulate_field_flipping->isChecked());
    m_config_ptr->setXOffset(mp_spinbox_hpos->value());
    m_config_ptr->setYOffset(mp_spinbox_vpos->value());
    m_config_ptr->save();
}
