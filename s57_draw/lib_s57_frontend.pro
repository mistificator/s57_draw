QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET =    lib_s57_frontend
TEMPLATE =  app

greaterThan(QT_MAJOR_VERSION, 4): win32:TARGET_EXT = .exe

DESTDIR = ../s57_bin

CONFIG +=   qt console thread

SOURCES +=\
            lib_s57_frontend_main.cpp

HEADERS  +=\
            lib_s57.h

win32:LIBS +=\
            ../s57_bin/lib_s57.lib \
            ../s57_bin/lib_prti99.lib

unix:LIBS +=\
            ../s57_bin/lib_s57.a \
            ../s57_bin/lib_prti99.a

unix:QMAKE_LFLAGS += -ldl

include (ogr.pri)

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_BUILD = 0

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"

VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

