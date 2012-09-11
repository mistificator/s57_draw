#-------------------------------------------------
#
# Project created by QtCreator 2012-02-23T18:01:13
#
#-------------------------------------------------

QT       += core gui opengl

TARGET =    s57_draw
TEMPLATE =  app
DESTDIR = ..\s57_bin

SOURCES  +=\
            s57_draw_main.cpp \
            s52_symbol.cpp \
            s52_conditional.cpp \
            s52_utils.cpp \
            s57_buildscene.cpp \
            s57_drawwindow.cpp

HEADERS  += \
            s52_symbol.h \
            s52_utils.h \
            s57_buildscene.h \
            s57_drawwindow.h \
            s52_conditional.h

FORMS    += \
    S57_DrawWindow.ui

include (ogr.pri)

OTHER_FILES += \
            dic.txt \
            s57_draw.ts \
            s57_draw.qm

RESOURCES += \
            s57_draw.qrc

TRANSLATIONS += s57_draw.ts
