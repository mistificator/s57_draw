QT       += core gui

TARGET =    lib_s57_frontend
TEMPLATE =  app

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

include (ogr.pri)
