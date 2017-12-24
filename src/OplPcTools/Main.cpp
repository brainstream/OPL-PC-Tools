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

#include <QApplication>
#include <OplPcTools/UI/MainWindow.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("oplpctools");
    a.setApplicationDisplayName("Open PlayStation 2 Loader PC Tools");
    a.setApplicationVersion(QT_STRINGIFY(_OPLPCTOOLS_VERSION));
    a.setOrganizationName("brainstream");
    OplPcTools::UI::MainWindow wnd;
    wnd.show();
    return a.exec();
}
