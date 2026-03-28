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

#include <OplPcTools/UI/Application.h>
#include <OplPcTools/ApplicationInfo.h>
#include <OplPcTools/GameArtManager.h>
#include <OplPcTools/GameImporter.h>
#include <OplPcTools/Settings.h>
#include <OplPcTools/Uuid.h>
#include <QDir>
#include <QMessageBox>
#include <QTranslator>
#include <QStandardPaths>
#include <QThread>

using namespace OplPcTools;
using namespace OplPcTools::UI;
using namespace OplPcTools::UI::__Private;

namespace {

class PrivateApplication : public Application
{
public:
    PrivateApplication(int & _argc, char ** _argv) :
        Application(_argc, _argv)
    {
    }
};

PrivateApplication * gp_application = nullptr;

QTranslator * setupTranslator(const QString & _base_name)
{
    QString locale = QLocale::system().name();
    locale.truncate(locale.lastIndexOf('_'));
    const QString filename = QString("%1_%2.qm").arg(_base_name, locale);
    QString filepath = QDir(gp_application->applicationDirPath()).absoluteFilePath(filename);
    if(!QFile::exists(filepath))
    {
        filepath = QStandardPaths::locate(QStandardPaths::AppDataLocation, filename);
        if(filepath.isEmpty())
            return nullptr;
    }
    QTranslator * translator = new QTranslator();
    if(translator->load(filepath))
    {
        gp_application->installTranslator(translator);
        return translator;
    }
    else
    {
        delete translator;
    }
    return nullptr;
}

} // namespace


QuestionAsker::QuestionAsker(
    const QString & _title,
    const QString & _message,
    QMessageBox::StandardButtons _buttons,
    QWidget * _parent
) :
    QObject(_parent),
    mp_parent(_parent),
    m_title(_title),
    m_message(_message),
    m_buttons(_buttons),
    m_answer(QMessageBox::NoButton)
{
}


void QuestionAsker::ask()
{
    if(gp_application->thread() == QThread::currentThread())
    {
        m_answer = QMessageBox::question(mp_parent, m_title, m_message, m_buttons);
    }
    else
    {
        QMetaObject::invokeMethod(this, &QuestionAsker::ask, Qt::BlockingQueuedConnection);
    }
}

Application::Application(int & _argc, char ** _argv) :
    QApplication(_argc, _argv),
    mp_main_window(nullptr)
{
}

Application::~Application()
{
    delete mp_main_window;
}

void Application::showMainWindow()
{
    ensureMainWindow()->show();
}

MainWindow * Application::ensureMainWindow()
{
    if(!mp_main_window)
        mp_main_window = new MainWindow();
    return mp_main_window;
}

void Application::showMessage(const QString & _title, const QString & _message)
{
    gp_application->showMessageImpl(_title, _message);
}

void Application::showMessage(const QString & _message)
{
    gp_application->showMessageImpl(gp_application->ensureMainWindow()->windowTitle(), _message);
}

void Application::showMessageImpl(const QString & _title, const QString & _message)
{
    if(thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "showMessageImpl", Q_ARG(QString, _title), Q_ARG(QString, _message));
    }
    else
    {
        QMessageBox::information(ensureMainWindow(), _title, _message);
    }
}

void Application::showErrorMessage()
{
    gp_application->showErrorMessageImpl(tr("Something went wrong"));
}

void Application::showErrorMessage(const QString & _message)
{
    gp_application->showErrorMessageImpl(_message);
}

void Application::showErrorMessageImpl(const QString & _message)
{
    if(thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, "showErrorMessageImpl", Q_ARG(QString, _message));
    }
    else
    {
        QMessageBox::critical(ensureMainWindow(), tr("Error"), _message);
    }
}

QMessageBox::StandardButton Application::askQuestion(
    const QString & _title,
    const QString & _message,
    QMessageBox::StandardButtons _buttons)
{
    QuestionAsker * asker = new QuestionAsker(_title, _message, _buttons, gp_application->ensureMainWindow());
    asker->moveToThread(gp_application->thread());
    asker->ask();
    QMessageBox::Button answer = asker->answer();
    asker->deleteLater();
    return answer;
}

bool Application::pushActivity(Intent & _intent)
{
    return gp_application->ensureMainWindow()->pushActivity(_intent);
}

int main(int _argc, char * _argv[])
{
    qRegisterMetaType<Uuid>("OplPcTools::Uuid");
    qRegisterMetaType<GameArtType>("OplPcTools::GameArtType");
    qRegisterMetaType<GameImportPorgress>("OplPcTools::GameImportPorgress");
    QApplication::setDesktopSettingsAware(true);
    gp_application = new PrivateApplication(_argc, _argv);
    gp_application->setApplicationName(APPLICATION_NAME);
    gp_application->setApplicationVersion(APPLICATION_VERSION);
    gp_application->setOrganizationName("brainstream");
    QTranslator * translator = setupTranslator(gp_application->applicationName());
    QTranslator * qt_translator = setupTranslator("qtbase");
    gp_application->showMainWindow();
    int result = gp_application->exec();
    delete gp_application;
    delete translator;
    delete qt_translator;
    gp_application = nullptr;
    return result;
}
