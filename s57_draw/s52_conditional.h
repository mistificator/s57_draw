#ifndef S52_CONDITIONAL_H
#define S52_CONDITIONAL_H

#if defined(_WIN32) && defined(LIB_S57_DLL) && !defined(LIB_S57_EXPORT)
    #define LIB_S57_EXPORT __declspec(dllexport)
#elif !defined(LIB_S57_EXPORT)
    #define LIB_S57_EXPORT
#endif

#include <QString>
#include "ogrsf_frmts.h"

namespace S52_CS
{
    LIB_S57_EXPORT QString translate(const QString & _layer_name, const QString & _cs, OGRFeature * _feat, int _collection_idx = 0);
}

#endif // S52_CONDITIONAL_H
