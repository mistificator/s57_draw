QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET =    s57_draw
TEMPLATE =  app
DESTDIR = ../s57_bin

win32:TARGET_EXT = .exe

SOURCES  +=\
            s57_draw_main.cpp \
            s57_drawwindow.cpp

HEADERS  += \
            s57_buildscene.h \
            s57_drawwindow.h

FORMS    += \
    S57_DrawWindow.ui

win32:LIBS +=\
            ../s57_bin/lib_s57.lib \
            ../s57_bin/lib_prti99.lib

unix:LIBS +=\
            ../s57_bin/lib_s57.a \
            ../s57_bin/lib_prti99.a

unix:QMAKE_LFLAGS += -ldl

OTHER_FILES += \
            dic.txt \
            s57_draw.ts \
            s57_draw.qm \
            scrollbar.css

RESOURCES += \
            s57_draw.qrc

TRANSLATIONS += s57_draw.ts

include (ogr.pri)

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_BUILD = 0

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"

VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

