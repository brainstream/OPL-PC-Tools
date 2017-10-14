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

INCLUDEPATH += .

linux {
    contains(QMAKE_HOST.arch, x86_64): {
        arch = "amd64"
    } else {
        arch = "i386"
    }
    deb.target = deb
    deb.commands = /bin/bash $$_PRO_FILE_PWD_/../packages/make-deb.sh "$$OUT_PWD/$$TARGET" "$$VERSION" $$arch
    QMAKE_EXTRA_TARGETS += deb
}

SOURCES += \
    OplPcTools/Main.cpp \
    OplPcTools/Core/BinCueDeviceSource.cpp \
    OplPcTools/Core/Device_FreeBSD.cpp \
    OplPcTools/Core/Device_Linux.cpp \
    OplPcTools/Core/Device_MacOSX.cpp \
    OplPcTools/Core/Device_Windows.cpp \
    OplPcTools/Core/Device.cpp \
    OplPcTools/Core/DirectoryGameInstaller.cpp \
    OplPcTools/Core/Game.cpp \
    OplPcTools/Core/GameCollection.cpp \
    OplPcTools/Core/GameInstaller.cpp \
    OplPcTools/Core/IsoRecoverer.cpp \
    OplPcTools/Core/OpticalDriveDeviceSource.cpp \
    OplPcTools/Core/UlConfigGameInstaller.cpp \
    OplPcTools/Misc/Settings.cpp \
    OplPcTools/UI/AboutDialog.cpp \
    OplPcTools/UI/ChooseOpticalDiscDialog.cpp \
    OplPcTools/UI/GameInstallDialog.cpp \
    OplPcTools/UI/GameRenameDialog.cpp \
    OplPcTools/UI/IsoRecoverDialog.cpp \
    OplPcTools/UI/MainWindow.cpp \
    OplPcTools/UI/ManageGameArtsDialog.cpp \
    OplPcTools/UI/SettingsDialog.cpp

HEADERS += \
    OplPcTools/Core/BinCueDeviceSource.h \
    OplPcTools/Core/Device.h \
    OplPcTools/Core/DeviceSource.h \
    OplPcTools/Core/DirectoryGameInstaller.h \
    OplPcTools/Core/Game.h \
    OplPcTools/Core/GameCollection.h \
    OplPcTools/Core/GameInstallationType.h \
    OplPcTools/Core/GameInstaller.h \
    OplPcTools/Core/Iso9660DeviceSource.h \
    OplPcTools/Core/IsoRecoverer.h \
    OplPcTools/Core/MediaType.h \
    OplPcTools/Core/OpticalDriveDeviceSource.h \
    OplPcTools/Core/UlConfigGameInstaller.h \
    OplPcTools/Misc/Exception.h \
    OplPcTools/Misc/IOException.h \
    OplPcTools/Misc/LambdaThread.h \
    OplPcTools/Misc/Settings.h \
    OplPcTools/Misc/ValidationException.h \
    OplPcTools/UI/AboutDialog.h \
    OplPcTools/UI/ChooseOpticalDiscDialog.h \
    OplPcTools/UI/GameInstallDialog.h \
    OplPcTools/UI/GameRenameDialog.h \
    OplPcTools/UI/IsoRecoverDialog.h \
    OplPcTools/UI/MainWindow.h \
    OplPcTools/UI/ManageGameArtsDialog.h \
    OplPcTools/UI/SettingsDialog.h

FORMS += \
    OplPcTools/UI/AboutDialog.ui \
    OplPcTools/UI/ChooseOpticalDiscDialog.ui \
    OplPcTools/UI/GameInstallDialog.ui \
    OplPcTools/UI/GameRenameDialog.ui \
    OplPcTools/UI/IsoRecoverDialog.ui \
    OplPcTools/UI/MainWindow.ui \
    OplPcTools/UI/ManageGameArtsDialog.ui \
    OplPcTools/UI/SettingsDialog.ui

RESOURCES += \
    OplPcTools/Resources/Resources.qrc

win32 {
    RC_FILE = OplPcTools/Resources/Resources.rc
}

TRANSLATIONS += \
    Translations/oplpctools_ru.ts
