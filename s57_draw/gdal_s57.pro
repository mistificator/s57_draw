TEMPLATE = aux
TARGET = gdal_s57
DESTDIR = ../s57_bin

GDAL_ROOT = $$clean_path($$PWD/../gdal/gdal)
GDAL_DIR_WIN = $$shell_path($$GDAL_ROOT)
OUT_DIR_WIN = $$shell_path($$clean_path($$OUT_PWD/../s57_bin))
OUT_LIB_WIN = $$shell_path($$clean_path($$OUT_PWD/../s57_bin/gdal_s57.lib))

NMAKE_ARGS = WIN64=YES
CONFIG(debug, debug|release) {
    NMAKE_ARGS += DEBUG=1
}

build_gdal.target = $$OUT_LIB_WIN
build_gdal.commands = \
    if not exist $$shell_quote($$OUT_DIR_WIN) mkdir $$shell_quote($$OUT_DIR_WIN) && \
    cd /d $$shell_quote($$GDAL_DIR_WIN) && \
    nmake /f makefile.vc $$NMAKE_ARGS staticlib && \
    copy /Y $$shell_quote($$GDAL_DIR_WIN\\gdal.lib) $$shell_quote($$OUT_LIB_WIN)

QMAKE_EXTRA_TARGETS += build_gdal
PRE_TARGETDEPS += $$OUT_LIB_WIN
QMAKE_CLEAN += $$OUT_LIB_WIN
