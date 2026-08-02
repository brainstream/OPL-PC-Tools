################################################################################################
# Copyright © 2017-2026 Sergey Smolyannikov aka brainstream                                    #
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
# You should have received a copy of the GNU General Public License along with OPL PC Tools.   #
# If not, see <http://www.gnu.org/licenses/>.                                                  #
#                                                                                              #
################################################################################################

cmake_minimum_required(VERSION 3.16)
project("OPL PC Tools" LANGUAGES CXX)

message("CMake version: " ${CMAKE_VERSION})
if(CMAKE_COMPILER_IS_GNUCXX)
    message("GNU g++ version: " ${CMAKE_CXX_COMPILER_VERSION})
else()
    message(FATAL_ERROR "GNU g++ is required")
endif()
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
    message("Static linking with GCC is enabled")
endif()

set(OPT_EXE_NAME "oplpctools")
set(OPT_VERSION_MAJOR  4)
set(OPT_VERSION_MINOR  0)
set(OPT_VERSION_SUFFIX "(development)")
set(OPT_MIN_QT6_VERSION "6.8.0")
set(OPT_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src/OplPcTools)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_SKIP_BUILD_RPATH ON)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Application will be built in the Debug configuration")
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

set(OPT_SANITYZER "" CACHE STRING "Enable a sanitizer")
set_property(CACHE OPT_SANITYZER PROPERTY STRINGS "" "ASAN" "UBSAN" "TSAN")

#######################
# Libraries
#######################

set(QT6_DIR "" CACHE PATH "Path to Qt6 installation")

set(QT_REQUIRED_COMPONENTS Widgets Network LinguistTools)


if(QT6_DIR)
    list(INSERT CMAKE_PREFIX_PATH 0 "${QT6_DIR}")
endif()

find_package(Qt6 COMPONENTS ${QT_REQUIRED_COMPONENTS})
if(Qt6_FOUND)
    if(Qt6_VERSION VERSION_LESS OPT_MIN_QT6_VERSION)
        message(FATAL_ERROR
            "Error: Found Qt6 version ${Qt6_VERSION}, but at least "
            "version ${OPT_MIN_QT6_VERSION} is required"
        )
    endif()
else()
    message(FATAL_ERROR "Qt6 not found")
endif()

#######################
# Sources
#######################

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_BINARY_DIR}
)

file(GLOB_RECURSE OPT_SRC
    "${OPT_SRC_DIR}/*.h"
    "${OPT_SRC_DIR}/*.cpp"
    "${OPT_SRC_DIR}/*.ui"
    "${OPT_SRC_DIR}/*.qrc"
)

if(WIN32)
    list(APPEND OPT_SRC
        ${OPT_SRC_DIR}/Resources/Resources.rc
    )
endif()

#######################
# lz4
#######################

set(LZ4_BUILD_CLI OFF CACHE BOOL "Build lz4 program" FORCE)
set(LZ4_BUILD_LEGACY_LZ4C OFF CACHE BOOL "Build lz4c program" FORCE)
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lz4/build/cmake
    ${CMAKE_CURRENT_BINARY_DIR}/lz4
    EXCLUDE_FROM_ALL
)

#######################
# Building
#######################

if(WIN32)
    add_executable(${OPT_EXE_NAME} WIN32 ${OPT_SRC} ${QM_FILES})
else()
    add_executable(${OPT_EXE_NAME} ${OPT_SRC} ${QM_FILES})
endif()

target_compile_options(${OPT_EXE_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

if(OPT_SANITYZER STREQUAL "ASAN")
    message(STATUS "Using ADDRESS sanitizer")
    target_link_options(${OPT_EXE_NAME} PRIVATE -fsanitize=address)
elseif(OPT_SANITYZER STREQUAL "UBSAN")
    message(STATUS "Using UNDEFINED BEHAVIOR sanitizer")
    target_link_options(${OPT_EXE_NAME} PRIVATE -fsanitize=undefined)
elseif(OPT_SANITYZER STREQUAL "TSAN")
    message(STATUS "Using THREAD sanitizer")
    target_link_options(${OPT_EXE_NAME} PRIVATE -fsanitize=thread)
elseif(DEFINED OPT_SANITYZER AND NOT OPT_SANITYZER STREQUAL "")
    message(FATAL_ERROR "Unknown sanitizer is selected: \"${OPT_SANITYZER}\"")
endif()

target_link_libraries(${OPT_EXE_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    lz4
)

add_custom_target(misc SOURCES
    .gitignore
    LICENSE.txt
    README.md
    NOTES.txt
    Dockerfile
    .dockerignore
    make-release-linux.sh
    release/assets/linux/make-desktop-file.sh
    release/assets/linux/oplpctools.desktop
    release/assets/linux/oplpctools.sh
)

#######################
# Definitions
#######################

target_compile_definitions(${OPT_EXE_NAME} PRIVATE
    -DQT_DEPRECATED_WARNINGS
    -D_OPLPCTOOLS_VERSION_MAJOR=${OPT_VERSION_MAJOR}
    -D_OPLPCTOOLS_VERSION_MINOR=${OPT_VERSION_MINOR}
    -D_OPLPCTOOLS_VERSION_SUFFIX=${OPT_VERSION_SUFFIX}
    -D_OPLPCTOOLS_EXE=${OPT_EXE_NAME}
)

#######################
# Localization
#######################

file(GLOB_RECURSE TRANSLATIONS
    "${OPT_SRC_DIR}/Translations/*.ts"
)

qt_add_translations(${OPT_EXE_NAME}
    LUPDATE_TARGET "translation_update"
    LRELEASE_TARGET "translation_release"
    TS_FILES ${TRANSLATIONS}
)
add_dependencies(${OPT_EXE_NAME} translation_update)
