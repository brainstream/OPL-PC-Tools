/***********************************************************************************************
 * Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                   *
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

#ifndef __OPLPCTOOLS_APPLICATION__
#define __OPLPCTOOLS_APPLICATION__

#include <QApplication>
#include <QWidget>
#include <OplPcTools/GameCollection.h>
#include <OplPcTools/UI/Intent.h>
#include <OplPcTools/UI/MainWindow.h>

namespace OplPcTools {
namespace UI {

class Application : public QApplication
{
    Q_OBJECT

protected:
    Application(int & _argc, char ** _argv);

public:
    ~Application() override;
    void showMainWindow();
    void showMessage(const QString & _title, const QString & _message);
    void showErrorMessage();
    void showErrorMessage(const QString & _message);
    bool pushActivity(Intent & _intent);
    GameCollection & gameCollection() const;

    static Application & instance();

private:
    MainWindow * ensureMainWindow();

private:
    MainWindow * mp_main_window;
    GameCollection * mp_game_collection;
};

} // namespace UI
} // namespace OplPcTools

#endif // __OPLPCTOOLS_APPLICATION__
