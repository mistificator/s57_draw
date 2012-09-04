#include "s52_utils.h"

#define TRUE    1
#define FALSE   0

static double _MARparamVal[] = {
    0.0,      // NONE
    TRUE,     // SHOW_TEXT
    TRUE,     // TWO_SHADES

    8.0,     // SAFETY_CONTOUR

    5.0,      // SAFETY_DEPTH

    3.0,      // SHALLOW_CONTOUR
    10.0,     // DEEP_CONTOUR

    TRUE,    // SHALLOW_PATTERN

    FALSE,    // SHIPS_OUTLINE
    0.0,      // DISTANCE_TAGS
    0.0,      // TIME_TAGS
    TRUE,     // FULL_SECTORS
    TRUE,     // SYMBOLIZED_BND

    TRUE,     // SYMPLIFIED_PNT

//    'D',      // S52_MAR_DISP_CATEGORY --DISPLAYBASE
//    'S',      // S52_MAR_DISP_CATEGORY --STANDARD
    'O',      // S52_MAR_DISP_CATEGORY --OTHER

//    0,        // S52_MAR_COLOR_PALETTE --DAY_BRIGHT
    1,        // S52_MAR_COLOR_PALETTE --DAY_BLACKBACK
//    2,        // S52_MAR_COLOR_PALETTE --DAY_WHITEBACK
//    3,        // S52_MAR_COLOR_PALETTE --DUSK
//    4,        // S52_MAR_COLOR_PALETTE --NIGHT

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
