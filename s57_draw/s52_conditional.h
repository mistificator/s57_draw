#ifndef S52_CONDITIONAL_H
#define S52_CONDITIONAL_H

#ifdef LIB_S57_DLL
    #define LIB_S57_EXPORT __declspec(dllexport)
#else
    #define LIB_S57_EXPORT
#endif

#include <QString>
#include "ogrsf_frmts.h"

namespace S52_CS
{
    LIB_S57_EXPORT QString translate(const QString & _layer_name, const QString & _cs, OGRFeature * _feat, int _collection_idx = 0);
}

#endif // S52_CONDITIONAL_H
