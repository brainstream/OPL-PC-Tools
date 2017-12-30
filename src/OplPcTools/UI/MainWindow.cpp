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

#include <QMessageBox>
#include <OplPcTools/Core/Settings.h>
#include <OplPcTools/UI/MainWindow.h>
#include <OplPcTools/UI/GameCollectionWidget.h>
#include <OplPcTools/UI/GameDetailsWidget.h>
#include <OplPcTools/UI/AboutDialog.h>
#include "ui_MainWindow.h"

using namespace OplPcTools;
using namespace OplPcTools::UI;

namespace {
namespace SettingsKey {

const char * wnd_geometry = "WindowGeometry";

} // namespace SettingsKey
} // namespace

class MainWindow::Private : public Ui::MainWindow { };

MainWindow::MainWindow(QWidget * _parent /*= nullptr*/) :
    QMainWindow(_parent),
    mp_private(new Private)
{
    mp_collection = new Core::GameCollection(this);
    mp_private->setupUi(this);
    GameCollectionWidget * game_collection_widget = new GameCollectionWidget(*this);
    mp_private->stacked_widget->addWidget(game_collection_widget);
    QSettings settings;
    restoreGeometry(settings.value(SettingsKey::wnd_geometry).toByteArray());
    if(Core::Settings::instance().reopenLastSestion())
        game_collection_widget->tryLoadRecentDirectory();
}

MainWindow::~MainWindow()
{
    delete mp_private;
}

void MainWindow::closeEvent(QCloseEvent * _event)
{
    QMainWindow::closeEvent(_event);
    QSettings settings;
    settings.setValue(SettingsKey::wnd_geometry, saveGeometry());
}

Core::GameCollection & MainWindow::collection() const
{
   return *mp_collection;
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

void MainWindow::showGameInstaller()
{
    GameDetailsWidget * widget = new GameDetailsWidget(*this);
    widget->setGameId("TODO: INSTALL GAME");
    int index = mp_private->stacked_widget->addWidget(widget);
    mp_private->stacked_widget->setCurrentIndex(index);
}

void MainWindow::showIsoRecoverer()
{

}

void MainWindow::showGameDetails(const QString & _id)
{
    GameDetailsWidget * widget = new GameDetailsWidget(*this);
    widget->setGameId(_id);
    int index = mp_private->stacked_widget->addWidget(widget);
    mp_private->stacked_widget->setCurrentIndex(index);
}
