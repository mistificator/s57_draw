QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32:TARGET =    lib_s57_ext
unix:TARGET =    _s57_ext

!isEmpty(QMAKE_QT_DLL) {
    win32:TARGET_EXT = .dll
}

unix:CONFIG += staticlib

TEMPLATE = lib
DESTDIR = ../s57_bin
DLLDESTDIR = ../s57_bin

HEADERS  +=\
        s57_buildscene.h \
    s57_extendedscene.h \
    s57_extendedscenemods.h

DEFINES += \
        LIB_S57_EXT_DLL

win32:LIBS +=\
        ../s57_bin/lib_prti99.lib \
        ../s57_bin/lib_s57.lib

unix:LIBS +=\
        ../s57_bin/lib_prti99.a \
        ../s57_bin/lib_s57.a

include (ogr.pri)

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_BUILD = 0

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"

VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

SOURCES += \
    s57_extendedscene.cpp \
    s57_extendedscenemods.cpp

unix: QMAKE_CXXFLAGS += -std=c++11
