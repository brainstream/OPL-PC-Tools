QT += core gui widgets

TARGET = qpcopl
TEMPLATE = app

SOURCES +=\
    src/MainWindow.cpp \
    src/Main.cpp \
    src/UlConfig.cpp \
    src/GameRenameDialog.cpp

HEADERS  += \
    src/MainWindow.h \
    src/UlConfig.h \
    src/IOException.h \
    src/Exception.h \
    src/GameRenameDialog.h \
    src/ValidationException.h

FORMS += \
    src/MainWindow.ui \
    src/GameRenameDialog.ui

RESOURCES += \
    src/Resources.qrc

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libcdio libiso9660
}

win32 {
    RC_FILE = src\Resources.rc
}
