QT       += core gui

TARGET =    lib_s57
TEMPLATE =  lib


SOURCES +=\
        lib_s57_main.cpp \
        s57buildscene.cpp \
        s52_symbol.cpp

HEADERS  +=\
        s57buildscene.h \
        s52_symbol.h \
        lib_s57.h

DEFINES += LIB_S57_DLL

include (ogr.pri)
