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

CONFIG += c++1z

VERSION = "2.0"

DEFINES += \
    "_OPLPCTOOLS_VERSION=$$VERSION" \
    QT_DEPRECATED_WARNINGS \

INCLUDEPATH += .

SOURCES += \
    OplPcTools/Main.cpp \
    OplPcTools/UI/MainWindow.cpp \
    OplPcTools/UI/GameCollectionWidget.cpp \
    OplPcTools/UI/GameDetailsWidget.cpp \
    OplPcTools/Core/GameCollection.cpp

HEADERS += \
    OplPcTools/UI/MainWindow.h \
    OplPcTools/UI/GameCollectionWidget.h \
    OplPcTools/UI/UIContext.h \
    OplPcTools/UI/GameDetailsWidget.h \
    OplPcTools/Core/GameCollection.h \
    OplPcTools/Core/Game.h

FORMS += \
    OplPcTools/UI/MainWindow.ui \
    OplPcTools/UI/GameCollectionWidget.ui \
    OplPcTools/UI/GameDetailsWidget.ui

RESOURCES += \
    OplPcTools/Resources.qrc

win32 {
    RC_FILE = OplPcTools/Resources.rc
}
