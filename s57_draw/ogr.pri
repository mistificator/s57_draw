GDAL_PATH = ../gdal/gdal

win32:LIBS += $$GDAL_PATH/gdal_i.lib
unix:LIBS += $$GDAL_PATH/.libs/libgdal.a

INCLUDEPATH +=  $$GDAL_PATH/ogr \
                $$GDAL_PATH/ogr/ogrsf_frmts \
                $$GDAL_PATH/ogr/ogrsf_frmts/s57 \
                $$GDAL_PATH/port \
                $$GDAL_PATH/gcore \
                $$GDAL_PATH/frmts/iso8211
