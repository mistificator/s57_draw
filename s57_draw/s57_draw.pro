QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET =    s57_draw
TEMPLATE =  app
DESTDIR = ../s57_bin

CONFIG(RUSSIAN) {
DEFINES += RUSSIAN
}

# hack for conflicting compiler versions: here must be version of Qt compiler
#win32:DEFINES += _MSC_VER=1600

#greaterThan(QT_MAJOR_VERSION, 4): win32:TARGET_EXT = .exe
win32:TARGET_EXT = .exe

SOURCES  +=\
            s57_draw_main.cpp \
            s57_drawwindow.cpp

HEADERS  += \
            s57_extendedscene.h \
            s57_drawwindow.h \
            s57_extendedscenemods.h

FORMS    += \
    S57_DrawWindow.ui

LIBS += -L$$OUT_PWD/../s57_bin -llib_s57 -llib_s57_ext

unix:QMAKE_LFLAGS += -ldl

greaterThan(QT_MAJOR_VERSION, 5) {
    CONFIG += c++17
} else {
    CONFIG += c++11
}

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

