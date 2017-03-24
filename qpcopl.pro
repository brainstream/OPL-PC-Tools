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

SOURCES +=\
    src/MainWindow.cpp \
    src/Main.cpp \
    src/UlConfig.cpp \
    src/GameRenameDialog.cpp \
    src/GameInstaller.cpp \
    src/GameInstallDialog.cpp \
    src/Iso9660GameInstallerSource.cpp

HEADERS  += \
    src/MainWindow.h \
    src/UlConfig.h \
    src/IOException.h \
    src/Exception.h \
    src/GameRenameDialog.h \
    src/ValidationException.h \
    src/GameInstaller.h \
    src/GameInstallerSource.h \
    src/GameInstallDialog.h \
    src/Iso9660GameInstallerSource.h \
    src/GameInstallThread.h

FORMS += \
    src/MainWindow.ui \
    src/GameRenameDialog.ui \
    src/GameInstallDialog.ui

RESOURCES += \
    src/Resources.qrc

OTHER_FILES += \
    LICENSE.txt

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libcdio libiso9660
}

win32 {
    RC_FILE = src\Resources.rc
}
