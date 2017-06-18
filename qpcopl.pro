################################################################################################
#                                                                                              #
# This file is part of the qpcopl project, the graphical PC tools for Open PS2 Loader.         #
#                                                                                              #
# qpcopl is free software: you can redistribute it and/or modify it under the terms of         #
# the GNU General Public License as published by the Free Software Foundation,                 #
# either version 3 of the License, or (at your option) any later version.                      #
#                                                                                              #
# qpcopl is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;         #
# without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   #
# See the GNU General Public License for more details.                                         #
#                                                                                              #
# You should have received a copy of the GNU General Public License along with MailUnit.       #
# If not, see <http://www.gnu.org/licenses/>.                                                  #
#                                                                                              #
################################################################################################

QT += core gui widgets

TARGET = qpcopl
TEMPLATE = app

CONFIG += c++14

SOURCES +=\
    src/MainWindow.cpp \
    src/Main.cpp \
    src/GameRenameDialog.cpp \
    src/GameInstaller.cpp \
    src/GameInstallDialog.cpp \
    src/AboutDialog.cpp \
    src/ChooseOpticalDiscDialog.cpp \
    src/SettingsDialog.cpp \
    src/Settings.cpp \
    src/Device.cpp \
    src/Device_Linux.cpp \
    src/Device_Windows.cpp \
    src/Device_FreeBSD.cpp \
    src/IsoRecoverer.cpp \
    src/GameCollection.cpp \
    src/Game.cpp \
    src/IsoRecoverDialog.cpp \
    src/Device_MacOSX.cpp

HEADERS  += \
    src/MainWindow.h \
    src/IOException.h \
    src/Exception.h \
    src/GameRenameDialog.h \
    src/ValidationException.h \
    src/GameInstaller.h \
    src/GameInstallDialog.h \
    src/GameInstallThread.h \
    src/Game.h \
    src/MediaType.h \
    src/AboutDialog.h \
    src/GameInstallationTask.h \
    src/ChooseOpticalDiscDialog.h \
    src/SettingsDialog.h \
    src/Settings.h \
    src/Device.h \
    src/LambdaThread.h \
    src/IsoRecoverer.h \
    src/GameCollection.h \
    src/IsoRecoverDialog.h

FORMS += \
    src/MainWindow.ui \
    src/GameRenameDialog.ui \
    src/GameInstallDialog.ui \
    src/AboutDialog.ui \
    src/ChooseOpticalDiscDialog.ui \
    src/SettingsDialog.ui \
    src/IsoRecoverDialog.ui

RESOURCES += \
    src/Resources.qrc

OTHER_FILES += \
    .gitignore \
    LICENSE.txt


TRANSLATIONS += \
    translations/qpcopl_ru.ts

win32 {
    RC_FILE = src/Resources.rc
}
