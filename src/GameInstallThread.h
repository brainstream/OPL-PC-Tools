///***********************************************************************************************
// *                                                                                             *
// * This file is part of the OPL PC Tools project, the graphical PC tools for Open PS2 Loader.  *
// *                                                                                             *
// * OPL PC Tools is free software: you can redistribute it and/or modify it under the terms of  *
// * the GNU General Public License as published by the Free Software Foundation,                *
// * either version 3 of the License, or (at your option) any later version.                     *
// *                                                                                             *
// * OPL PC Tools is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;  *
// * without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  *
// * See the GNU General Public License for more details.                                        *
// *                                                                                             *
// * You should have received a copy of the GNU General Public License along with MailUnit.      *
// * If not, see <http://www.gnu.org/licenses/>.                                                 *
// *                                                                                             *
// ***********************************************************************************************/

//#ifndef __OPLPCTOOLS_GAMEINSTALLTHREAD__
//#define __OPLPCTOOLS_GAMEINSTALLTHREAD__

//#include <QThread>
//#include "GameInstaller.h"
//#include "Exception.h"

//class GameInstallThread : public QThread
//{
//    Q_OBJECT

//public:
//    explicit GameInstallThread(GameInstaller & _installer, QObject * _parent = nullptr) :
//        QThread(_parent),
//        mr_installer(_installer)
//    {
//    }

//    void run() override
//    {
//        try
//        {
//            mr_installer.install();
//        }
//        catch(const Exception & ex)
//        {
//            emit exception(ex.message());
//        }
//        catch(...)
//        {
//            emit exception(tr("An unknown error has occurred"));
//        }
//    }

//signals:
//    void exception(QString _message);

//private:
//    GameInstaller & mr_installer;
//};

//#endif // __OPLPCTOOLS_GAMEINSTALLTHREAD__
