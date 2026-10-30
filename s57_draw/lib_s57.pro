QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32:TARGET =    lib_s57
unix:TARGET =    _s57

include (qt_dll.pri)

!isEmpty(QMAKE_QT_DLL):{
    win32:TARGET_EXT = .dll
}

unix:CONFIG += staticlib

TEMPLATE = lib
DESTDIR = ../s57_bin
DLLDESTDIR = ../s57_bin

SOURCES +=\
        lib_s57_main.cpp \
        s52_symbol.cpp \
        s52_conditional.cpp \
        s52_utils.cpp \
        s57_buildscene.cpp \
        s52_fixeditem.cpp


HEADERS  +=\
        s52_symbol.h \
        s52_utils.h \
        s57_buildscene.h \
        s52_conditional.h \
        lib_s57.h \
        s52_fixeditem.h

DEFINES += \
        LIB_S57_DLL

include (ogr.pri)

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_BUILD = 0

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"

VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

greaterThan(QT_MAJOR_VERSION, 5) {
    CONFIG += c++17
} else {
    CONFIG += c++11
}
