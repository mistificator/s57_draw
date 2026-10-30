#include "s52_utils.h"

#define TRUE    1
#define FALSE   0

static double _MARparamVal[] = {
    0.0,      // NONE
    TRUE,     // SHOW_TEXT
    FALSE,    // TWO_SHADES (FALSE = 4 оттенка воды, как в OpenCPN)
    10.0,     // SAFETY_CONTOUR (10 м — включает голубой для 18ft/30ft зон)
    10.0,     // SAFETY_DEPTH
    5.0,      // SHALLOW_CONTOUR (5 м — ярко-голубой для 6ft/12ft/18ft зон)
    20.0,     // DEEP_CONTOUR (20 м — зоны до 60ft = 18.2м остаются голубыми DEPMD)
    TRUE,     // SHALLOW_PATTERN
    FALSE,    // SHIPS_OUTLINE
    0.0,      // DISTANCE_TAGS
    0.0,      // TIME_TAGS
    TRUE,     // FULL_SECTORS
    FALSE,    // SYMBOLIZED_BND
    TRUE,     // SYMPLIFIED_PNT
    'O',      // S52_MAR_DISP_CATEGORY --OTHER
    0,        // S52_MAR_COLOR_PALETTE --DAY_BRIGHT
    16.0      // NUM
};


double S52_getMarinerParam(S52_MAR_param_t param)
{
    return _MARparamVal[param];
}

int S52_setMarinerParam(S52_MAR_param_t param, double val)
{
    if (S52_MAR_NONE<param && param<S52_MAR_NUM)
        _MARparamVal[param] = val;
    else
        return FALSE;

    return TRUE;
}
