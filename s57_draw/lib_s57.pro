QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32:TARGET =    lib_s57
unix:TARGET =    _s57

greaterThan(QT_MAJOR_VERSION, 4): win32:TARGET_EXT = .dll

unix:CONFIG += staticlib

TEMPLATE = lib
DESTDIR = ../s57_bin
DLLDESTDIR = ../s57_bin

SOURCES +=\
        lib_s57_main.cpp \
        s52_symbol.cpp \
        s52_conditional.cpp \
        s52_utils.cpp \
        s57_buildscene.cpp


HEADERS  +=\
        s52_symbol.h \
        s52_utils.h \
        s57_buildscene.h \
        s52_conditional.h \
        lib_s57.h \
        lib_prti99.h

DEFINES += \
        LIB_S57_DLL

win32:LIBS +=\
        ../s57_bin/lib_prti99.lib

unix:LIBS +=\
        ../s57_bin/lib_prti99.a

include (ogr.pri)

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_BUILD = 0

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"

VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

unix:QMAKE_CXXFLAGS += -std=c++11
