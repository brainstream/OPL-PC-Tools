/***********************************************************************************************
 * Copyright © 2017-2026 Sergey Smolyannikov aka brainstream                                   *
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

#ifndef __OPLPCTOOLS_GAMECONFIGWIDGET__
#define __OPLPCTOOLS_GAMECONFIGWIDGET__

#include <functional>
#include <OplPcTools/Game.h>
#include <OplPcTools/GameConfiguration.h>
#include <OplPcTools/VmcCollection.h>
#include "ui_GameConfigWidget.h"

namespace OplPcTools {
namespace UI {

class GameConfigWidget : public QWidget, private Ui::GameConfigWidget
{
    Q_OBJECT

public:
    explicit GameConfigWidget(const Game & _game, QWidget * _parent = nullptr);

private:
    bool loadConfiguration();
    void initControls();
    void reinitOplVersionSensitiveControls();
    void sortVmcComboBoxes();
    void setVmcComboBoxValue(QComboBox * _combobox, const QString & _value);
    void fillVideoModeComboBox();
    void onOplVerstionChanged();
    void onGsmStateChanged();
    void onVmcAdded(const Uuid & _id);
    void onVmcDeleted(const Uuid & _id);
    void deleteVmcComboBoxItem(QComboBox * _combobox, const Uuid & _id);
    int getVmcComboBoxItemIndex(QComboBox * _combobox, const Uuid & _id) const;
    void onVmcRenamed(const Uuid & _id);
    void renameVmcComboBoxItem(QComboBox * _combobox, const Uuid & _id, const QString & _title);
    void createVmc0();
    void createVmc1();
    const OplPcTools::Vmc * createVmc();
    void fillGameIdFromGame();
    void clear();
    void clearForm();
    void save();
    void saveAs(const QString & _filename);
    void remove();
    void startSmartThread(std::function<void()> _lambda);

private:
    VmcCollection & mr_vmcs;
    const Game & mr_game;
    QSharedPointer<GameConfiguration> m_config_ptr;
    GameConfigurationVersion m_config_version;

};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_GAMECONFIGWIDGET__
