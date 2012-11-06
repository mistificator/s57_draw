QT       += core gui

TARGET =    lib_s57_frontend
TEMPLATE =  app

DESTDIR = ../s57_bin

CONFIG +=   qt console thread

SOURCES +=\
            lib_s57_frontend_main.cpp

HEADERS  +=\
            lib_s57.h

LIBS +=\
            ../s57_bin/lib_s57.lib

include (ogr.pri)
