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
#include <QTranslator>
#include <QStandardPaths>
#include <OplPcTools/UI/MainWindow.h>

QTranslator * setupTranslator();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("oplpctools");
    a.setApplicationVersion(QT_STRINGIFY(_OPLPCTOOLS_VERSION));
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
    const QString filename = QString("%1_%2.qm").arg(app->applicationName()).arg(locale);
    QString filepath = QDir(app->applicationDirPath()).absoluteFilePath(filename);
    if(!QFile::exists(filepath))
    {
        filepath = QStandardPaths::locate(QStandardPaths::AppDataLocation, filename);
        if(filepath.isEmpty())
            return nullptr;
    }
    QTranslator * translator = new QTranslator();
    if(translator->load(filepath))
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
