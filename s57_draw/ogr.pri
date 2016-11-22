GDAL_PATH = ../gdal_x86

win32:contains(QMAKE_HOST.arch, x86_64) {
    GDAL_PATH = ../gdal_x64
}

STATIC_LINK = 1

eval(STATIC_LINK = 0) {
LIBS += $$GDAL_PATH/gdal_i.lib
}

eval(STATIC_LINK = 1) {
LIBS += $$GDAL_PATH/gdal.lib \
        $$GDAL_PATH/port/cpl.lib \
        $$GDAL_PATH/ogr/ogr.lib \
        $$GDAL_PATH/ogr/ogrsf_frmts/ogrsf_frmts.lib \
        $$GDAL_PATH/frmts/iso8211/libiso8211.lib
}

INCLUDEPATH +=  $$GDAL_PATH/ogr \
                $$GDAL_PATH/ogr/ogrsf_frmts \
                $$GDAL_PATH/ogr/ogrsf_frmts/s57 \
                $$GDAL_PATH/port \
                $$GDAL_PATH/gcore \
                $$GDAL_PATH/frmts/iso8211
