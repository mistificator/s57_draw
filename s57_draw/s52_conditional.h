#ifndef S52_CONDITIONAL_H
#define S52_CONDITIONAL_H

#include <QString>
#include "ogrsf_frmts.h"

namespace S52_CS
{
    QString translate(const QString & _layer_name, const QString & _cs, OGRFeature * _feat, int _collection_idx = 0);
}

#endif // S52_CONDITIONAL_H
