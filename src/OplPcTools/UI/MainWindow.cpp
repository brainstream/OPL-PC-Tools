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

#include <OplPcTools/UI/MainWindow.h>
#include <OplPcTools/UI/GameCollectionWidget.h>
#include <OplPcTools/UI/GameDetailsWidget.h>
#include "ui_MainWindow.h"

using namespace OplPcTools::UI;

class MainWindow::MainWindowUI : public Ui::MainWindow { };

MainWindow::MainWindow(QWidget * _parent /*= nullptr*/) :
    QMainWindow(_parent),
    mp_ui(new MainWindowUI)
{
    mp_collection = new OplPcTools::Core::GameCollection(this);
    mp_ui->setupUi(this);
    mp_ui->stacked_widget->addWidget(new GameCollectionWidget(*this));
}

MainWindow::~MainWindow()
{
    delete mp_ui;
}

OplPcTools::Core::GameCollection & MainWindow::collection() const
{
   return *mp_collection;
}

void MainWindow::showGameInstaller()
{
    GameDetailsWidget * widget = new GameDetailsWidget(*this);
    widget->setGameId("TODO: INSTALL GAME");
    int index = mp_ui->stacked_widget->addWidget(widget);
    mp_ui->stacked_widget->setCurrentIndex(index);
}

void MainWindow::showIsoRecoverer()
{

}

void MainWindow::showGameDetails(const QString & _id)
{
    GameDetailsWidget * widget = new GameDetailsWidget(*this);
    widget->setGameId(_id);
    int index = mp_ui->stacked_widget->addWidget(widget);
    mp_ui->stacked_widget->setCurrentIndex(index);
}
