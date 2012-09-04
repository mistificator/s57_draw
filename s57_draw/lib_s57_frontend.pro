QT       += core gui

TARGET =    lib_s57_frontend
TEMPLATE =  app

CONFIG +=   qt console thread


SOURCES +=\
            lib_s57_frontend_main.cpp

HEADERS  +=\
            lib_s57.h

LIBS +=\
            ../lib_s57_bin/release/lib_s57.lib

include (ogr.pri)
