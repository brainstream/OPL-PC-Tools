################################################################################################
# Copyright © 2017-2019 Sergey Smolyannikov aka brainstream                                    #
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

cmake_minimum_required(VERSION 3.2)
project("OPL PC Tools")

message("CMake version: " ${CMAKE_VERSION})
if(CMAKE_COMPILER_IS_GNUCXX)
    message("GNU g++ version: " ${CMAKE_CXX_COMPILER_VERSION})
else()
    message(FATAL_ERROR "GNU g++ is required")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OPT_EXE_NAME "oplpctools")
set(OPT_VERSION_MAJOR  2)
set(OPT_VERSION_MINOR  2)
set(OPT_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src/OplPcTools)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_SKIP_BUILD_RPATH ON)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Application will be built in the Debug configuration")
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

#######################
# Libraries
#######################

set(QT5_DIR "" CACHE PATH "Provides a path to search Qt5 installation")
find_package(Qt5Core HINTS ${QT5_DIR})
if(Qt5Core_FOUND)
    message("Qt version: ${Qt5Core_VERSION_STRING}")
else()
    message(FATAL_ERROR "ERROR: Qt not found. Please specify a path to Qt installation using the QT5_DIR variable.")
endif()
find_package(Qt5Widgets REQUIRED HINTS ${QT5_DIR})
find_package(Qt5Network REQUIRED HINTS ${QT5_DIR})
find_package(Qt5LinguistTools REQUIRED HINTS ${QT5_DIR})

#######################
# Sources
#######################

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_BINARY_DIR}
)

set(OPT_SRC_MOC
    ${OPT_SRC_DIR}/UI/Application.h
    ${OPT_SRC_DIR}/GameArtManager.h
    ${OPT_SRC_DIR}/GameCollection.h
    ${OPT_SRC_DIR}/IsoRestorer.h
    ${OPT_SRC_DIR}/GameStorage.h
    ${OPT_SRC_DIR}/UlConfigGameStorage.h
    ${OPT_SRC_DIR}/DirectoryGameStorage.h
    ${OPT_SRC_DIR}/GameInstaller.h
    ${OPT_SRC_DIR}/DirectoryGameInstaller.h
    ${OPT_SRC_DIR}/Updater.h
    ${OPT_SRC_DIR}/UlConfigGameInstaller.h
    ${OPT_SRC_DIR}/UI/AboutDialog.h
    ${OPT_SRC_DIR}/UI/LambdaThread.h
    ${OPT_SRC_DIR}/UI/GameCollectionActivity.h
    ${OPT_SRC_DIR}/UI/MainWindow.h
    ${OPT_SRC_DIR}/UI/GameDetailsActivity.h
    ${OPT_SRC_DIR}/UI/ClickableLabel.h
    ${OPT_SRC_DIR}/UI/IsoRestorerActivity.h
    ${OPT_SRC_DIR}/UI/GameInstallerActivity.h
    ${OPT_SRC_DIR}/UI/GameRenameDialog.h
    ${OPT_SRC_DIR}/UI/ChooseOpticalDiscDialog.h
    ${OPT_SRC_DIR}/UI/SettingsDialog.h
)

set(OPT_SRC_UI
    ${OPT_SRC_DIR}/UI/MainWindow.ui
    ${OPT_SRC_DIR}/UI/GameCollectionActivity.ui
    ${OPT_SRC_DIR}/UI/GameDetailsActivity.ui
    ${OPT_SRC_DIR}/UI/AboutDialog.ui
    ${OPT_SRC_DIR}/UI/GameRenameDialog.ui
    ${OPT_SRC_DIR}/UI/IsoRestorerActivity.ui
    ${OPT_SRC_DIR}/UI/GameInstallerActivity.ui
    ${OPT_SRC_DIR}/UI/ChooseOpticalDiscDialog.ui
    ${OPT_SRC_DIR}/UI/SettingsDialog.ui
)

set(OPT_SRC_RES
    ${OPT_SRC_DIR}/Resources/Resources.qrc
)

qt5_wrap_cpp(OPT_SRC_MOC ${OPT_SRC_MOC})
qt5_wrap_ui(OPT_SRC_UI ${OPT_SRC_UI})
qt5_add_resources(OPT_SRC_RES ${OPT_SRC_RES})

set(OPT_SRC
    ${OPT_SRC_MOC}
    ${OPT_SRC_UI}
    ${OPT_SRC_RES}
    ${OPT_SRC_DIR}/ApplicationInfo.h
    ${OPT_SRC_DIR}/Exception.h
    ${OPT_SRC_DIR}/File.h
    ${OPT_SRC_DIR}/Maybe.h
    ${OPT_SRC_DIR}/BigEndian.h
    ${OPT_SRC_DIR}/DeviceSource.h
    ${OPT_SRC_DIR}/Iso9660DeviceSource.h
    ${OPT_SRC_DIR}/BinCueDeviceSource.h
    ${OPT_SRC_DIR}/BinCueDeviceSource.cpp
    ${OPT_SRC_DIR}/NrgDeviceSource.h
    ${OPT_SRC_DIR}/NrgDeviceSource.cpp
    ${OPT_SRC_DIR}/OpticalDriveDeviceSource.h
    ${OPT_SRC_DIR}/OpticalDriveDeviceSource.cpp
    ${OPT_SRC_DIR}/Device.h
    ${OPT_SRC_DIR}/Device.cpp
    ${OPT_SRC_DIR}/Device_FreeBSD.cpp
    ${OPT_SRC_DIR}/Device_Linux.cpp
    ${OPT_SRC_DIR}/Device_MacOSX.cpp
    ${OPT_SRC_DIR}/Device_Windows.cpp
    ${OPT_SRC_DIR}/Game.h
    ${OPT_SRC_DIR}/GameInstallationType.h
    ${OPT_SRC_DIR}/MediaType.h
    ${OPT_SRC_DIR}/DirectoryGameStorage.cpp
    ${OPT_SRC_DIR}/GameCollection.cpp
    ${OPT_SRC_DIR}/GameStorage.cpp
    ${OPT_SRC_DIR}/Settings.h
    ${OPT_SRC_DIR}/Settings.cpp
    ${OPT_SRC_DIR}/UlConfigGameStorage.cpp
    ${OPT_SRC_DIR}/IsoRestorer.cpp
    ${OPT_SRC_DIR}/GameArtManager.cpp
    ${OPT_SRC_DIR}/GameInstaller.cpp
    ${OPT_SRC_DIR}/DirectoryGameInstaller.cpp
    ${OPT_SRC_DIR}/Updater.cpp
    ${OPT_SRC_DIR}/UlConfigGameInstaller.cpp
    ${OPT_SRC_DIR}/UI/Application.cpp
    ${OPT_SRC_DIR}/UI/Intent.h
    ${OPT_SRC_DIR}/UI/Activity.h
    ${OPT_SRC_DIR}/UI/AboutDialog.cpp
    ${OPT_SRC_DIR}/UI/GameCollectionActivity.cpp
    ${OPT_SRC_DIR}/UI/GameDetailsActivity.cpp
    ${OPT_SRC_DIR}/UI/MainWindow.cpp
    ${OPT_SRC_DIR}/UI/GameRenameDialog.cpp
    ${OPT_SRC_DIR}/UI/IsoRestorerActivity.cpp
    ${OPT_SRC_DIR}/UI/GameInstallerActivity.cpp
    ${OPT_SRC_DIR}/UI/ChooseOpticalDiscDialog.cpp
    ${OPT_SRC_DIR}/UI/SettingsDialog.cpp
)

if(WIN32)
    list(APPEND OPT_SRC
        ${OPT_SRC_DIR}/Resources/Resources.rc)
endif()

#######################
# Localization
#######################

set(TRANSLATIONS
    ${OPT_SRC_DIR}/Translations/${OPT_EXE_NAME}_ru.ts
)

qt5_create_translation(QM_FILES
    ${OPT_SRC}
    ${TRANSLATIONS}
    OPTIONS "-no-obsolete"
)

#######################
# Definitions
#######################

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -D_OPLPCTOOLS_VERSION_MAJOR=${OPT_VERSION_MAJOR}
    -D_OPLPCTOOLS_VERSION_MINOR=${OPT_VERSION_MINOR}
    -D_OPLPCTOOLS_EXE=${OPT_EXE_NAME}
    ${Qt5Core_DEFINITIONS}
    ${Qt5Widgets_DEFINITIONS}
    ${Qt5Network_DEFINITIONS}
)


#######################
# Building
#######################

if(WIN32)
    add_executable(${OPT_EXE_NAME} WIN32 ${OPT_SRC} ${QM_FILES})
else()
    add_executable(${OPT_EXE_NAME} ${OPT_SRC} ${QM_FILES})
endif()

target_link_libraries(${OPT_EXE_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
)

add_custom_target(misc SOURCES
    .gitignore
    LICENSE.txt
    README.md
    NOTES.txt
    release/make-release-linux.sh
)

