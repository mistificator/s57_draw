QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET =    lib_s57_frontend
TEMPLATE =  app

#greaterThan(QT_MAJOR_VERSION, 4): win32:TARGET_EXT = .exe
win32:TARGET_EXT = .exe

DESTDIR = ../s57_bin

CONFIG +=   qt console thread

SOURCES +=\
            lib_s57_frontend_main.cpp

HEADERS  +=\
            lib_s57.h

LIBS += -L$$OUT_PWD/../s57_bin -llib_s57

unix:QMAKE_LFLAGS += -ldl

include (ogr.pri)

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_BUILD = 0

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"

VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

