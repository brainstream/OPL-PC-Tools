################################################################################################
#                                                                                              #
# This file is part of the OPL PC Tools project, the graphical PC tools for Open PS2 Loader.   #
#                                                                                              #
# OPL PC Tools is free software: you can redistribute it and/or modify it under the terms of   #
# the GNU General Public License as published by the Free Software Foundation,                 #
# either version 3 of the License, or (at your option) any later version.                      #
#                                                                                              #
# OPL PC Tools is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;   #
# without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   #
# See the GNU General Public License for more details.                                         #
#                                                                                              #
# You should have received a copy of the GNU General Public License along with MailUnit.       #
# If not, see <http://www.gnu.org/licenses/>.                                                  #
#                                                                                              #
################################################################################################

QT += core gui widgets

TARGET = oplpctools
TEMPLATE = app

CONFIG += c++14

VERSION = "1.2"

DEFINES += "_OPLPCTOOLS_VERSION=$$VERSION"

linux {
    contains(QMAKE_HOST.arch, x86_64): {
        arch = "amd64"
    } else {
        arch = "i386"
    }
    deb.target = deb
    deb.commands = /bin/bash $$_PRO_FILE_PWD_/packages/make-deb.sh "$$OUT_PWD/$$TARGET" "$$VERSION" $$arch
    QMAKE_EXTRA_TARGETS += deb
}

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
    src/Device_MacOSX.cpp \
    src/UlConfigGameInstaller.cpp \
    src/DirectoryGameInstaller.cpp \
    src/BinCueDeviceSource.cpp \
    src/OpticalDriveDeviceSource.cpp

HEADERS  += \
    src/MainWindow.h \
    src/IOException.h \
    src/Exception.h \
    src/GameRenameDialog.h \
    src/ValidationException.h \
    src/GameInstaller.h \
    src/GameInstallDialog.h \
    src/Game.h \
    src/MediaType.h \
    src/AboutDialog.h \
    src/ChooseOpticalDiscDialog.h \
    src/SettingsDialog.h \
    src/Settings.h \
    src/Device.h \
    src/LambdaThread.h \
    src/IsoRecoverer.h \
    src/GameCollection.h \
    src/IsoRecoverDialog.h \
    src/GameInstallationType.h \
    src/UlConfigGameInstaller.h \
    src/DirectoryGameInstaller.h \
    src/DeviceSource.h \
    src/BinCueDeviceSource.h \
    src/OpticalDriveDeviceSource.h \
    src/Iso9660DeviceSource.h

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
    packages/debian/usr/share/applications/oplpctools.desktop \
    packages/make-deb.sh \
    packages/debian/DEBIAN/control \
    packages/debian/usr/share/doc/oplpctools/changelog.Debian \
    packages/debian/usr/share/doc/oplpctools/copyright


TRANSLATIONS += \
    translations/oplpctools_ru.ts

win32 {
    RC_FILE = src/Resources.rc
}

DISTFILES += \
    LICENSE.txt
