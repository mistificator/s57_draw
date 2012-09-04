GDAL_PATH = ../gdal
#GDAL_PATH = ../s57

LIBS += $$GDAL_PATH/gdal.lib \
        $$GDAL_PATH/port/cpl.lib \
        $$GDAL_PATH/ogr/ogr.lib \
        $$GDAL_PATH/ogr/ogrsf_frmts/ogrsf_frmts.lib \
        $$GDAL_PATH/frmts/iso8211/libiso8211.lib
INCLUDEPATH +=  $$GDAL_PATH/ogr \
                $$GDAL_PATH/ogr/ogrsf_frmts \
                $$GDAL_PATH/ogr/ogrsf_frmts/s57 \
                $$GDAL_PATH/port \
                $$GDAL_PATH/gcore \
                $$GDAL_PATH/frmts/iso8211
