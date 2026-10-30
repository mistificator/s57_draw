TEMPLATE = aux
TARGET = gdal_s57
DESTDIR = ../s57_bin

GDAL_ROOT = $$clean_path($$PWD/../gdal/gdal)
OUT_DIR = $$clean_path($$OUT_PWD/../s57_bin)

# Common flags for ./configure
CONFIGURE_ARGS = \
    --enable-static \
    --disable-shared \
    --without-libtool \
    --without-ld-shared \
    --with-hide-internal-symbols \
    --with-libjson-c=internal \
    --with-libz=internal \
    --with-libtiff=internal \
    --with-geotiff=internal \
    --without-iconv \
    --without-curl \
    --without-expat \
    --without-sqlite3 \
    --without-geos \
    --without-pg \
    --without-mysql \
    --without-odbc \
    --without-xerces \
    --without-jasper \
    --without-openjpeg \
    --without-hdf4 \
    --without-hdf5 \
    --without-netcdf \
    --without-webp \
    --without-zstd \
    --without-crypto \
    --without-python \
    --without-pam \
    --without-gif \
    --without-jpeg \
    --without-png

CONFIG(debug, debug|release) {
    CONFIGURE_ARGS += --enable-debug
}

# ---------------------------------------------------------------------------
# 1. MSVC (Windows)
# ---------------------------------------------------------------------------
win32-msvc* {
    GDAL_DIR_WIN = $$shell_path($$GDAL_ROOT)
    OUT_DIR_WIN  = $$shell_path($$OUT_DIR)
    OUT_LIB      = $$shell_path($$OUT_DIR/gdal_s57.lib)
    CPL_CFG_WIN  = $$shell_path($$GDAL_ROOT/port/cpl_config.h)
    GDAL_VER_WIN = $$shell_path($$GDAL_ROOT/gcore/gdal_version.h)
    GDALMAKE_OPT = $$shell_path($$GDAL_ROOT/GDALmake.opt)

    NMAKE_ARGS =
    contains(QT_ARCH, x86_64)|contains(QT_ARCH, arm64) {
        NMAKE_ARGS += WIN64=YES
    }
    CONFIG(debug, debug|release) {
        NMAKE_ARGS += DEBUG=1
    }

    build_gdal.target = $$OUT_LIB
    build_gdal.commands = \
        (if not exist $$shell_quote($$OUT_DIR_WIN) mkdir $$shell_quote($$OUT_DIR_WIN)) && \
        (del /Q $$shell_quote($$CPL_CFG_WIN) $$shell_quote($$GDAL_VER_WIN) $$shell_quote($$GDALMAKE_OPT)) && \
        cd /d $$shell_quote($$GDAL_DIR_WIN) && \
        nmake /f makefile.vc $$NMAKE_ARGS staticlib && \
        copy /Y $$shell_quote($$GDAL_DIR_WIN\\gdal.lib) $$shell_quote($$OUT_LIB)
}

# ---------------------------------------------------------------------------
# 2. MinGW (Windows, uses sh & make from MSYS2 in PATH)
# ---------------------------------------------------------------------------
win32-g++ {
    OUT_LIB = $$shell_path($$OUT_DIR/libgdal_s57.a)

    contains(QT_ARCH, x86_64) {
        MINGW_HOST = --host=x86_64-w64-mingw32 --build=x86_64-w64-mingw32
    } else {
        MINGW_HOST = --host=i686-w64-mingw32 --build=i686-w64-mingw32
    }

    build_gdal.target = $$OUT_LIB
    build_gdal.commands = \
        sh -c "mkdir -p \'$$OUT_DIR\' && \
        cd \'$$GDAL_ROOT\' && \
        (test -f GDALmake.opt || ./configure $$MINGW_HOST $$CONFIGURE_ARGS) && \
        make -j$(nproc) GDAL_FORMATS=\'iso8211 mem vrt\' static-lib && \
        cp -f libgdal.a \'$$OUT_DIR/libgdal_s57.a\'"
}

# ---------------------------------------------------------------------------
# 3. GCC / Clang (Linux / macOS / Unix)
# ---------------------------------------------------------------------------
unix {
    OUT_LIB = $$OUT_DIR/libgdal_s57.a

    build_gdal.target = $$OUT_LIB
    build_gdal.commands = \
        mkdir -p $$shell_quote($$OUT_DIR) && \
        cd $$shell_quote($$GDAL_ROOT) && \
        (test -f GDALmake.opt || ./configure $$CONFIGURE_ARGS) && \
        $(MAKE) GDAL_FORMATS="iso8211 mem vrt" static-lib && \
        cp -f libgdal.a $$shell_quote($$OUT_LIB)
}

QMAKE_EXTRA_TARGETS += build_gdal
PRE_TARGETDEPS += $$OUT_LIB
QMAKE_CLEAN += $$OUT_LIB
