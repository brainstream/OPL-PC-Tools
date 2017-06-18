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

#include "MainWindow.h"
#include <QApplication>
#include <QTranslator>
#include <QStandardPaths>

QTranslator * setupTranslator();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("qpcopl");
    a.setApplicationVersion("0.1");
    a.setApplicationDisplayName("Open PlayStation 2 Loader PC Tools");
    a.setOrganizationName("brainstream");
    QTranslator * translator = setupTranslator();
    MainWindow w;
    w.show();
    int result = a.exec();
    delete translator;
    return result;
}

QTranslator * setupTranslator()
{
    QString locale = QLocale::system().name();
    locale.truncate(locale.lastIndexOf('_'));
    QCoreApplication * app = QApplication::instance();
    QString filename = QString("%1_%2.qm").arg(app->applicationName()).arg(locale);
    filename = QStandardPaths::locate(QStandardPaths::AppDataLocation, filename);
    if(filename.isEmpty())
        return nullptr;
    QTranslator * translator = new QTranslator();
    if(translator->load(filename))
    {
        app->installTranslator(translator);
        return translator;
    }
    else
    {
        delete translator;
    }
    return nullptr;
}
