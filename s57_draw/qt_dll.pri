#based on https://github.com/Satius/qt5/blob/master/qtbase/mkspecs/features/qt.prf

win32: {
   contains(CONFIG, shared): {
      # this variable is read by qmake in qmake/generators/win32/msvc_vcproj.cpp
      # function VcprojGenerator::initDeploymentTool(), which contains some hardcoded
      # library names (the ones that were static in Qt 4)
      # it probably doesn't work anymore and should not be in this file
      QMAKE_QT_DLL = 1
   }
}

