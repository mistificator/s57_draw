#-------------------------------------------------
#
# Project created by QtCreator 2012-02-23T18:01:13
#
#-------------------------------------------------

QT       += core gui opengl

TARGET =    s57_draw
TEMPLATE =  app
DESTDIR = ../s57_bin

SOURCES  +=\
            s57_draw_main.cpp \
            s57_drawwindow.cpp

HEADERS  += \
            s57_buildscene.h \
            s57_drawwindow.h

FORMS    += \
    S57_DrawWindow.ui

LIBS +=\
            ../s57_bin/lib_s57.lib

include (ogr.pri)

OTHER_FILES += \
            dic.txt \
            s57_draw.ts \
            s57_draw.qm

RESOURCES += \
            s57_draw.qrc

TRANSLATIONS += s57_draw.ts
