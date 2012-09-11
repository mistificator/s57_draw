#ifndef S52_UTILS_H
#define S52_UTILS_H

typedef enum _S52_MAR_param_t {
    S52_MAR_NONE            = 0,    // default
    S52_MAR_SHOW_TEXT       = 1,    // view group 23
    S52_MAR_TWO_SHADES      = 2,    // flag indicating selection of two depth shades (on/off) [default ON]
    S52_MAR_SAFETY_CONTOUR  = 3,    // selected safety contour (meters) [IMO PS 3.6]
    S52_MAR_SAFETY_DEPTH    = 4,    // selected safety depth (meters) [IMO PS 3.7]
    S52_MAR_SHALLOW_CONTOUR = 5,    // selected shallow water contour (meters) (optional)
    S52_MAR_DEEP_CONTOUR    = 6,    // selected deepwatercontour (meters) (optional)
    S52_MAR_SHALLOW_PATTERN = 7,    // flag indicating selection of shallow water highlight (on/off)(optional) [default OFF]
    S52_MAR_SHIPS_OUTLINE   = 8,    // flag indicating selection of ship scale symbol (on/off) [IMO PS 8.4]
    S52_MAR_DISTANCE_TAGS   = 9,    // selected spacing of "distance to run" tags at a route (nm)
    S52_MAR_TIME_TAGS       = 10,   // selected spacing of time tags at the pasttrack (min)
    S52_MAR_FULL_SECTORS    = 11,   // show full length light sector lines
    S52_MAR_SYMBOLIZED_BND  = 12,   // symbolized area boundaries

    S52_MAR_SYMPLIFIED_PNT  = 13,   // simplified point

    S52_MAR_DISP_CATEGORY   = 14,   // display category

    S52_MAR_COLOR_PALETTE   = 15,   // color palette

    S52_MAR_NUM             = 16    // number of parameters
} S52_MAR_param_t;

double S52_getMarinerParam(S52_MAR_param_t param);
int    S52_setMarinerParam(S52_MAR_param_t param, double val);

#endif // S52_UTILS_H
