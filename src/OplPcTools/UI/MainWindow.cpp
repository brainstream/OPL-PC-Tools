/***********************************************************************************************
 * Copyright © 2017-2018 Sergey Smolyannikov aka brainstream                                   *
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

#include <QMessageBox>
#include <QSettings>
#include <OplPcTools/ApplicationInfo.h>
#include <OplPcTools/UI/MainWindow.h>
#include <OplPcTools/UI/AboutDialog.h>
#include <OplPcTools/UI/SettingsDialog.h>

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {
namespace SettingsKey {

const char * wnd_geometry = "WindowGeometry";

} // namespace SettingsKey
} // namespace

MainWindow::MainWindow(QWidget * _parent /*= nullptr*/) :
    QMainWindow(_parent)
{
    setupUi(this);
    setWindowTitle(APPLICATION_DISPLAY_NAME);
    QSettings settings;
    restoreGeometry(settings.value(SettingsKey::wnd_geometry).toByteArray());
    connect(mp_action_settings, &QAction::triggered, this, &MainWindow::showSettingsDialog);
}

void MainWindow::closeEvent(QCloseEvent * _event)
{
    QMainWindow::closeEvent(_event);
    QSettings settings;
    settings.setValue(SettingsKey::wnd_geometry, saveGeometry());
}

bool MainWindow::pushActivity(Intent & _intent)
{
    Activity * activity = _intent.createActivity(mp_stacked_widget);
    activity->setAttribute(Qt::WA_DeleteOnClose);
    int index = mp_stacked_widget->addWidget(activity);
    mp_stacked_widget->setCurrentIndex(index);
    if(activity->onAttach())
    {
        return true;
    }
    else
    {
        delete activity;
        return false;
    }
}

void MainWindow::showAboutDialog()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::showAboutQtDialog()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::showSettingsDialog()
{
    SettingsDialog dlg(this);
    dlg.exec();
}
