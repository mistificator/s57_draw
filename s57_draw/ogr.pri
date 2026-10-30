GDAL_PATH = $$PWD/../gdal/gdal

DEFINES += \
    CPL_DISABLE_DLL \
    NOMINMAX

LIBS += -L$$OUT_PWD/../s57_bin -lgdal_s57

win32: LIBS += -lws2_32 -lodbc32 -lodbccp32 -luser32 -ladvapi32 -lshell32 -lole32

INCLUDEPATH +=  $$GDAL_PATH/ogr \
                $$GDAL_PATH/ogr/ogrsf_frmts \
                $$GDAL_PATH/ogr/ogrsf_frmts/s57 \
                $$GDAL_PATH/port \
                $$GDAL_PATH/gcore \
                $$GDAL_PATH/frmts/iso8211
