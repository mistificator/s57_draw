QT       += core gui

TARGET =    lib_s57
TEMPLATE = lib
DESTDIR = ..\s57_bin
DLLDESTDIR = ..\s57_bin

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
        s57_drawwindow.h \
        s52_conditional.h \
        lib_s57.h


DEFINES += LIB_S57_DLL

include (ogr.pri)
