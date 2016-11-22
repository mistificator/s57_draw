QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET =    lib_s57
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

LIBS +=\
        ../s57_bin/lib_prti99.lib

include (ogr.pri)
