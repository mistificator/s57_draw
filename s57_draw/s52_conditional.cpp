#include "s52_conditional.h"
#include "s52_utils.h"

#include <QVariant>
#include <QDebug>
#include <QVector>

template <class T>
bool lists_intersects(const QList<T> & _l1, const QList<T> & _l2);

QString DATCVR()
{
    return ("LC(HODATA01)");
}

QString DEPARE(double drval1, double drval2, bool drgare)
{
   //   Create a string of the proper color reference

    bool shallow  = true;
    QString rule_str =("AC(DEPIT)");


    if (drval1 >= 0.0 && drval2 > 0.0)
    {
        rule_str  = ("AC(DEPVS)");
    }

    if ((bool)S52_getMarinerParam(S52_MAR_TWO_SHADES))
    {
        if (drval1 >= S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR)  &&
            drval2 >  S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR))
        {
            rule_str  = ("AC(DEPDW)");
            shallow = false;
        }
    }
    else
    {
        if (drval1 >= S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR) &&
            drval2 >  S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR))
        {
            rule_str  = ("AC(DEPMS)");
        }

        if (drval1 >= S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR)  &&
                drval2 >  S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR))
        {
            rule_str  = ("AC(DEPMD)");
            shallow = false;
        }

        if (drval1 >= S52_getMarinerParam(S52_MAR_DEEP_CONTOUR)  &&
                drval2 >  S52_getMarinerParam(S52_MAR_DEEP_CONTOUR))
        {
            rule_str  = ("AC(DEPDW)");
            shallow = false;
        }

    }


//  If object is DRGARE....

    if(drgare)
    {
        if (drval1 < 0.0) //If DRVAL1 was not defined...
        {
            rule_str  = ("AC(DEPMD)");
            shallow = false;
        }
        rule_str.append((";AP(DRGARE01)"));
        rule_str.append((";LS(DASH,1,CHGRF)"));

    }


    return rule_str;

}

QString DEPCNT(double drval1, double drval2, double valdco, int quapos)
{
    QString rule_str;
    const double safety_contour = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);
    bool safe = false;
    if (drval1 <= safety_contour)
    {
          if (drval2 >= safety_contour)
          {
              safe = true;
          }
    }
    else
    {
        if (valdco == safety_contour)
        {
            safe = true;
        }
    }
    if (0 != quapos)
    {
          if ( 2 <= quapos && quapos < 10)
          {
                if (safe)
                {
                      rule_str = (";LS(DASH,2,DEPSC)");
                }
                else
                {
                      rule_str = (";LS(DASH,1,DEPCN)");
                }
          }
    }
    else
    {
          if (safe)
          {
                rule_str = (";LS(SOLD,2,DEPSC)");
          }
          else
          {
                rule_str = (";LS(SOLD,1,DEPCN)");
          }
    }
    return rule_str;

}

enum {
    color_unknown = 0,
    color_white = 1,
    color_black = 2,
    color_red = 3,
    color_green = 4,
    color_blue = 5,
    color_yellow = 6,
    color_grey = 7,
    color_brown = 8,
    color_amber = 9,
    color_violet = 10,
    color_orange = 11,
    color_magenta = 12,
    color_pink = 13,
    colot_max
};

enum {
    catlit_unknown = 0,
    catlit_directional_function = 1,
    catlit_rear_upper_light = 2,
    catlit_front_lower_light = 3,
    catlit_leading_light = 4,
    catlit_aero_light = 5,
    catlit_air_obstruction_light = 6,
    catlit_fog_detector_light = 7,
    catlit_flood_light = 8,
    catlit_strip_light = 9,
    catlit_subsidiary_light = 10,
    catlit_spotlight = 11,
    catlit_front = 12,
    catlit_rear = 13,
    catlit_lower = 14,
    catlit_upper = 15,
    catlit_moire_effect = 16,
    catlit_emergency = 17,
    catlit_bearing_light = 18,
    catlit_horizontally_disposed = 19,
    catlit_vertically_disposed = 20,
    catlit_max
};

QString LIGHTS(double /*valnmr*/, int catlit, int /*litvis*/, int color, double sectr1, double sectr2)
{
    QString rule_str;
    rule_str = "SY(LITDEF11)";
    if (catlit > catlit_unknown)
    {
        switch (catlit)
        {
        case catlit_subsidiary_light:
        case catlit_rear:
            rule_str = "SY(LIGHTS82)";
            goto lights_end;
        case catlit_spotlight:
            rule_str = "SY(LIGHTS81)";
            goto lights_end;
        }
    }

    if ((sectr1 != 0) || (sectr2 != 0))
    {
        switch (color)
        {
        case color_red:
        case color_brown:
            rule_str = "SY(LIGHTS96)";
            goto lights_end;
        case color_green:
        case color_grey:
            rule_str = "SY(LIGHTS95)";
            goto lights_end;
        case color_white:
        case color_amber:
        case color_yellow:
        case color_orange:
            rule_str = "SY(LIGHTS94)";
            goto lights_end;
        case color_black:
        case color_magenta:
        case color_violet:
        case color_pink:
        case color_blue:
            rule_str = "SY(LIGHTS90)";
            goto lights_end;
        }
    }
    else
    {
        switch (color)
        {
        case color_red:
        case color_brown:
            rule_str = "SY(LIGHTS11)";
            goto lights_end;
        case color_green:
        case color_grey:
            rule_str = "SY(LIGHTS12)";
            goto lights_end;
        case color_white:
        case color_amber:
        case color_yellow:
        case color_orange:
            rule_str = "SY(LIGHTS13)";
            goto lights_end;
        case color_black:
        case color_magenta:
        case color_violet:
        case color_pink:
        case color_blue:
            rule_str = "SY(LIGHTS14)";
            goto lights_end;
        }
    }

lights_end:

    return rule_str;
}


enum {
    tecsou_unknown = 0,
    tecsou_found_by_echo_sounder = 1,
    tecsou_found_by_side_scan_sonar = 2,
    tecsou_found_by_multi_beam = 3,
    tecsou_found_by_diver = 4,
    tecsou_found_by_lead_line = 5,
    tecsou_swept_by_wire_drag = 6,
    tecsou_found_by_laser = 7,
    tecsou_swept_by_vertical_acoustic_system = 8,
    tecsou_found_by_electromagnetic_sensor = 9,
    tecsou_photogrammetry = 10,
    tecsou_satellite_imagery = 11,
    tecsou_found_by_levelling = 12,
    tecsou_swept_by_side_scan_sonar = 13,
    tecsou_computer_generated = 14,
    tecsou_max
};


enum {
    quasou_unknown = 0,
    quasou_depth_known = 1,
    quasou_depth_unknown = 2,
    quasou_doubtful_sounding = 3,
    quasou_unreliable_sounding = 4,
    quasou_no_bottom_found_at_value_shown = 5,
    quasou_least_depth_known = 6,
    quasou_least_depth_unknown = 7,
    quasou_value_reported_not_surveyed = 8,
    quasou_value_reported_not_confirmed = 9,
    quasou_maintained_depth = 10,
    quasou_not_regularly_maintained = 11,
    quasou_max
};

QString SOUNDG(int quasou, int tecsou, double depth_value)
{
    QString rule_str;
    const double safety_depth = S52_getMarinerParam(S52_MAR_SAFETY_DEPTH);

    QString symbol;
    if (depth_value <= safety_depth)
    {
        symbol = ("SOUNDS");
    }
    else
    {
        symbol = ("SOUNDG");
    }

    switch (tecsou)
    {
    case tecsou_swept_by_wire_drag:
        rule_str.append(";SY(" + symbol + "B1)");
        break;
    }

    switch (quasou)
    {
    case quasou_doubtful_sounding:
    case quasou_unreliable_sounding:
    case quasou_no_bottom_found_at_value_shown:
        rule_str.append(";SY(" + symbol + "C2)");
        break;
    }

    if (depth_value < 0.0)
    {
        rule_str.append(";SY(" + symbol + "A1)");
        depth_value = -depth_value;
    }

    QString _depth_str;
    int _digit = 0;
    do
    {
        _depth_str.prepend(";SY(" + symbol + QString::number(_digit++) + QString::number((int)depth_value % 10) + ")");
    }
    while ((depth_value /= 10) > 1);
    rule_str.append(_depth_str);

    return rule_str;
}

enum {
    quapos_unknown = 0,
    quapos_surveyed = 1,
    quapos_unsurveyed = 2,
    quapos_inadequately_surveyed = 3,
    quapos_approximate = 4,
    quapos_position_doubtful = 5,
    quapos_unreliable = 6,
    quapos_reported_not_surveyed = 7,
    quapos_reported_not_confirmed = 8,
    quapos_estimated = 9,
    quapos_precisely_known = 10,
    quapos_calculated = 11,
    quapos_max
};

enum {
    conrad_unknown = 0,
    conrad_radar_conspicuous = 1,
    conrad_not_radar_conspicuous = 2,
    conrad_radar_conspicuous_has_radar_reflector = 3,
    conrad_max
};

QString QUAPOS(int _quapos, int _qualty, int _conrad, int _geom_type, bool _coalne)
{
    QString rule_str;

    switch (_geom_type)
    {
    case wkbPoint:
    case wkbMultiPoint:
        switch(_qualty)
        {
        case 4:
              rule_str.append((";SY(QUAPOS01)")); break;      // "PA"
        case 5:
              rule_str.append((";SY(QUAPOS02)")); break;      // "PD"
        case 7:
        case 8:
              rule_str.append((";SY(QUAPOS03)")); break;      // "REP"
        default:
              rule_str.append((";SY(LOWACC03)")); break;      // "?"
        }
        break;
    case wkbLineString:
    case wkbMultiLineString:
        if ((quapos_unsurveyed <= _quapos) && (_quapos < quapos_precisely_known))
        {
            rule_str = (";LC(LOWACC21)");
        }
        else
        {
            if (!_coalne)
            {
                if (_conrad == conrad_radar_conspicuous)
                {
                    rule_str = "LS(SOLD,3,CHMGF);LS(SOLD,1,CSTLN)";
                }
                else
                {
                    rule_str = "LS(SOLD,1,CSTLN)";
                }
            }
            else
            {
                rule_str = "LS(SOLD,1,CSTLN)";
            }
        }
        break;
    }

    return (rule_str);
}

enum {
    watlev_unknown = 0,
    watlev_partly_submerged_at_high_water = 1,
    watlev_always_dry = 2,
    watlev_always_under_water_or_submerged = 3,
    watlev_covers_and_uncovers = 4,
    watlev_awash = 5,
    watlev_subject_to_inundation_or_flooding = 6,
    watlev_floating = 7,
    watlev_max
};

enum {
    catslc_unknown = 0,
    catslc_breakwater = 1,
    catslc_groyne_groin	= 2,
    catslc_mole = 3,
    catslc_pier_jetty = 4,
    catslc_promenade_pier = 5,
    catslc_wharf_quay = 6,
    catslc_training_wall = 7,
    catslc_rip_rap = 8,
    catslc_revetment = 9,
    catslc_sea_wall = 10,
    catslc_landing_steps = 11,
    catslc_ramp = 12,
    catslc_slipway = 13,
    catslc_fender = 14,
    catslc_solid_face_wharf	= 15,
    catslc_open_face_wharf = 16,
    catslc_log_ramp	= 17,
    catslc_max
};

enum {
    condtn_unknown = 0,
    condtn_under_construction = 1,
    condtn_ruined = 2,
    condtn_under_reclamation = 3,
    condtn_wingless = 4,
    condtn_planned_construction = 5,
    condtn_max
};

QString SLCONS(int _quapos, int _condtn, int _watlev, int _catslc)
{
    QString rule_str;

    if ((quapos_unsurveyed <= _quapos) && (_quapos < quapos_precisely_known))
    {
        rule_str = "SY(LOWACC01)";
    }
    else
    {
        if ((_condtn == condtn_under_construction) || (_condtn == condtn_ruined))
        {
            rule_str = "LS(DASH,1,CSTLN)";
        }
        else
        {
            if ((catslc_wharf_quay  == _catslc) || (catslc_solid_face_wharf == _catslc) || (catslc_open_face_wharf == _catslc))
            {
                rule_str = "LS(SOLD,4,CSTLN)";
            }
            else
            {
                if (_watlev == watlev_always_dry)
                {
                    rule_str = "LS(SOLD,2,CSTLN)";
                }
                else
                if ((_watlev == watlev_always_under_water_or_submerged) || (_watlev == watlev_covers_and_uncovers))
                {
                    rule_str = "LS(DASH,2,CSTLN)";
                }
                else
                {
                    rule_str = "LS(SOLD,2,CSTLN)";
                }
            }
        }
    }

    return rule_str;
}

enum {
    restrn_unknown = 0,
    restrn_anchoring_prohibited = 1,
    restrn_anchoring_restricted = 2,
    restrn_fishing_prohibited = 3,
    restrn_fishing_restricted = 4,
    restrn_trawling_prohibited = 5,
    restrn_trawling_restricted = 6,
    restrn_entry_prohibited = 7,
    restrn_entry_restricted = 8,
    restrn_dredging_prohibited = 9,
    restrn_dredging_restricted = 10,
    restrn_diving_prohibited = 11,
    restrn_diving_restricted = 12,
    restrn_no_wake = 13,
    restrn_area_to_be_avoided = 14,
    restrn_construction_prohibited = 15,
    restrn_discharging_prohibited = 16,
    restrn_discharging_restricted = 17,
    restrn_industrial_or_mineral_exploration_development_prohibited = 18,
    restrn_industrial_or_mineral_exploration_development_restricted = 19,
    restrn_drilling_prohibited = 20,
    restrn_drilling_restricted = 21,
    restrn_removal_of_historical_artifacts_prohibited = 22,
    restrn_cargo_transhipment_lightering_prohibited = 23,
    restrn_dragging_prohibited = 24,
    restrn_stopping_prohibited = 25,
    restrn_landing_prohibited = 26,
    restrn_speed_restricted = 27,
    restrn_max
};

enum
{
    catrea_unknown = 0,
    catrea_offshore_safety_zone = 1,
    catrea_anchoring_prohibition_area = 2,
    catrea_fishing_prohibition_area = 3,
    catrea_nature_reserve = 4,
    catrea_bird_sanctuary = 5,
    catrea_game_reserve = 6,
    catrea_seal_sanctuary = 7,
    catrea_degaussing_range = 8,
    catrea_military_area = 9,
    catrea_historic_wreck_area = 10,
    catrea_inshore_traffic_zone = 11,
    catrea_navigational_aid_safety_zone = 12,
    catrea_danger_of_stranding_area = 13,
    catrea_minefield = 14,
    catrea_diving_prohibition_area = 15,
    catrea_area_to_be_avoided = 16,
    catrea_prohibited_area = 17,
    catrea_swimming_area = 18,
    catrea_waiting_area = 19,
    catrea_research_area = 20,
    catrea_dredging_area = 21,
    catrea_fish_sanctuary = 22,
    catrea_ecological_reserve = 23,
    catrea_no_wake_area = 24,
    catrea_swinging_area = 25,
    catrea_water_skiing_area = 26,
    catrea_max
};

#define LST_INTS(NAME, FLAGS) lists_intersects<int>(NAME, QList<int>() << FLAGS)

QString RESARE(const QList<int> & _restrn, const QList<int> & _catrea)
{
    QString rule_str;

    if (!_restrn.isEmpty())
    {
        if (LST_INTS(_restrn, restrn_entry_prohibited << restrn_dredging_restricted << restrn_discharging_prohibited))
        {
            if (LST_INTS(_restrn, restrn_anchoring_prohibited << restrn_anchoring_restricted << restrn_fishing_prohibited << restrn_fishing_restricted << restrn_trawling_prohibited << restrn_trawling_restricted))
            {
                rule_str = ";SY(ENTRES61)";
            }
            else
            {
                if (LST_INTS(_catrea, catrea_offshore_safety_zone << catrea_historic_wreck_area << catrea_inshore_traffic_zone << catrea_minefield << catrea_area_to_be_avoided << catrea_ecological_reserve << catrea_swinging_area))
                {
                    rule_str = ";SY(ENTRES61)";
                }
                else
                if (LST_INTS(_restrn, restrn_diving_prohibited << restrn_diving_restricted << restrn_no_wake << restrn_area_to_be_avoided << restrn_construction_prohibited))
                {
                    rule_str = ";SY(ENTRES71)";
                }
                else
                if (LST_INTS(_catrea, catrea_nature_reserve << catrea_bird_sanctuary << catrea_game_reserve << catrea_seal_sanctuary << catrea_navigational_aid_safety_zone << catrea_fish_sanctuary << catrea_no_wake_area << catrea_water_skiing_area))
                {
                    rule_str = ";SY(ENTRES71)";
                }
                else
                {
                    rule_str = ";SY(ENTRES51)";
                }
            }

            if (S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
            {
                rule_str.append(";LC(RESARE51)");
            }
            else
            {
                rule_str.append(";LS(DASH,2,CHMGD)");
            }
        }
        else
        {
            if (LST_INTS(_restrn, restrn_anchoring_prohibited << restrn_anchoring_restricted))
            {
                if (LST_INTS(_restrn, restrn_fishing_prohibited << restrn_fishing_restricted << restrn_trawling_prohibited << restrn_trawling_restricted))
                {
                    rule_str = ";SY(ACHRES61)";
                }
                else
                {
                    if (LST_INTS(_catrea, catrea_offshore_safety_zone << catrea_historic_wreck_area << catrea_inshore_traffic_zone << catrea_minefield << catrea_area_to_be_avoided << catrea_ecological_reserve << catrea_swinging_area))
                    {
                        rule_str = ";SY(ACHRES61)";
                    }
                    else
                    if (LST_INTS(_restrn, restrn_diving_prohibited << restrn_diving_restricted << restrn_no_wake << restrn_area_to_be_avoided << restrn_construction_prohibited))
                    {
                        rule_str = ";SY(ACHRES71)";
                    }
                    else
                    if (LST_INTS(_catrea, catrea_nature_reserve << catrea_bird_sanctuary << catrea_game_reserve << catrea_seal_sanctuary << catrea_navigational_aid_safety_zone << catrea_fish_sanctuary << catrea_no_wake_area << catrea_water_skiing_area))
                    {
                        rule_str = ";SY(ACHRES71)";
                    }
                    else
                    {
                        rule_str = ";SY(ACHRES51)";
                    }
                }

                if (S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
                {
                    rule_str.append(";LC(RESARE51)");
                }
                else
                {
                    rule_str.append(";LS(DASH,2,CHMGD)");
                }
            }
            else
            {
                if (LST_INTS(_restrn, restrn_fishing_prohibited << restrn_fishing_restricted << restrn_trawling_prohibited << restrn_trawling_restricted))
                {
                    if (LST_INTS(_catrea, catrea_offshore_safety_zone << catrea_historic_wreck_area << catrea_inshore_traffic_zone << catrea_minefield << catrea_area_to_be_avoided << catrea_ecological_reserve << catrea_swinging_area))
                    {
                        rule_str = ";SY(FSHRES51)";
                    }
                    else
                    if (LST_INTS(_restrn, restrn_diving_prohibited << restrn_diving_restricted << restrn_no_wake << restrn_area_to_be_avoided << restrn_construction_prohibited))
                    {
                        rule_str = ";SY(FSHRES71)";
                    }
                    else
                    if (LST_INTS(_catrea, catrea_nature_reserve << catrea_bird_sanctuary << catrea_game_reserve << catrea_seal_sanctuary << catrea_navigational_aid_safety_zone << catrea_fish_sanctuary << catrea_no_wake_area << catrea_water_skiing_area))
                    {
                        rule_str = ";SY(FSHRES71)";
                    }
                    else
                    {
                        rule_str = ";SY(FSHRES51)";
                    }

                    if (S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
                    {
                        rule_str.append(";LC(FSHRES51)");
                    }
                    else
                    {
                        rule_str.append(";LS(DASH,2,CHMGD)");
                    }
                }
                else
                {
                    if (LST_INTS(_restrn, restrn_diving_prohibited << restrn_diving_restricted << restrn_no_wake << restrn_area_to_be_avoided << restrn_construction_prohibited))
                    {
                        rule_str = ";SY(INFARE51)";
                    }
                    else
                    {
                        rule_str = ";SY(RSRDEF51)";
                    }

                    if (S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
                    {
                        rule_str.append(";LC(CTYARE51)");
                    }
                    else
                    {
                        rule_str.append(";LS(DASH,2,CHMGD)");
                    }
                }
            }
        }
    }
    else
    {
        if (!_catrea.isEmpty())
        {
            if (LST_INTS(_catrea, catrea_offshore_safety_zone << catrea_historic_wreck_area << catrea_inshore_traffic_zone << catrea_minefield << catrea_area_to_be_avoided << catrea_ecological_reserve << catrea_swinging_area))
            {
                if (LST_INTS(_catrea, catrea_nature_reserve << catrea_bird_sanctuary << catrea_game_reserve << catrea_seal_sanctuary << catrea_navigational_aid_safety_zone << catrea_fish_sanctuary << catrea_no_wake_area << catrea_water_skiing_area))
                {
                    rule_str = ";SY(CTYARE71)";
                }
                else
                {
                    rule_str = ";SY(CTYARE51)";
                }
            }
            else
            {
                if (LST_INTS(_catrea, catrea_nature_reserve << catrea_bird_sanctuary << catrea_game_reserve << catrea_seal_sanctuary << catrea_navigational_aid_safety_zone << catrea_fish_sanctuary << catrea_no_wake_area << catrea_water_skiing_area))
                {
                    rule_str = ";SY(INFARE51)";
                }
                else
                {
                    rule_str = ";SY(RSRDEF51)";
                }
            }
        }
        else
        {
            rule_str = ";SY(RSRDEF51)";
        }

        if (S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
        {
            rule_str.append(";LC(CTYARE51)");
        }
        else
        {
            rule_str.append(";LS(DASH,2,CHMGD)");
        }

    }
    return rule_str;
}

QString RESTRN(const QList<int> & _restrn)
{
    QString rule_str;
    if (LST_INTS(_restrn, restrn_entry_prohibited << restrn_dredging_restricted << restrn_discharging_prohibited))
    {
        if (LST_INTS(_restrn, restrn_anchoring_prohibited << restrn_anchoring_restricted << restrn_fishing_prohibited << restrn_fishing_restricted << restrn_trawling_prohibited << restrn_trawling_restricted))
        {
            rule_str = ";SY(ENTRES61)";
        }
        else
        {
            if (LST_INTS(_restrn, restrn_diving_prohibited << restrn_diving_restricted << restrn_no_wake << restrn_area_to_be_avoided << restrn_construction_prohibited))
            {
                rule_str = ";SY(ENTRES71)";
            }
            else
            {
                rule_str = ";SY(ENTRES51)";
            }
        }

    }
    else
    {
        if (LST_INTS(_restrn, restrn_anchoring_prohibited << restrn_anchoring_restricted))
        {
            if (LST_INTS(_restrn, restrn_fishing_prohibited << restrn_fishing_restricted << restrn_trawling_prohibited << restrn_trawling_restricted))
            {
                rule_str = ";SY(ACHRES61)";
            }
            else
            {
                if (LST_INTS(_restrn, restrn_diving_prohibited << restrn_diving_restricted << restrn_no_wake << restrn_area_to_be_avoided << restrn_construction_prohibited))
                {
                    rule_str = ";SY(ACHRES71)";
                }
                else
                {
                    rule_str = ";SY(ACHRES51)";
                }
            }
        }
        else
        {
            if (LST_INTS(_restrn, restrn_fishing_prohibited << restrn_fishing_restricted << restrn_trawling_prohibited << restrn_trawling_restricted))
            {
                if (LST_INTS(_restrn, restrn_diving_prohibited << restrn_diving_restricted << restrn_no_wake << restrn_area_to_be_avoided << restrn_construction_prohibited))
                {
                    rule_str = ";SY(FSHRES71)";
                }
                else
                {
                    rule_str = ";SY(FSHRES51)";
                }
            }
            else
            {
                if (LST_INTS(_restrn, restrn_diving_prohibited << restrn_diving_restricted << restrn_no_wake << restrn_area_to_be_avoided << restrn_construction_prohibited))
                {
                    rule_str = ";SY(INFARE51)";
                }
                else
                {
                    rule_str = ";SY(RSRDEF51)";
                }
            }
        }
    }

    return rule_str;
}

enum
{
    topshp_unknown = 0,
    topshp_cone_point_up = 1,
    topshp_cone_point_down = 2,
    topshp_sphere = 3,
    topshp_2_spheres = 4,
    topshp_cylinder	= 5,
    topshp_board = 6,
    topshp_x_shape_St_Andrews_cross	= 7,
    topshp_upright_cross_St_Georges_cross = 8,
    topshp_cube_point_up = 9,
    topshp_2_cones_point_to_point = 10,
    topshp_2_cones_base_to_base	= 11,
    topshp_rhombus_diamond = 12,
    topshp_2_cones_points_upward = 13,
    topshp_2_cones_points_downward = 14,
    topshp_besom_point_up_broom_or_perch = 15,
    topshp_besom_point_down_broom_or_perch = 16,
    topshp_flag = 17,
    topshp_sphere_over_rhombus = 18,
    topshp_square = 19,
    topshp_rectangle_horizontal = 20,
    topshp_rectangle_vertical = 21,
    topshp_trapezium_up	= 22,
    topshp_trapezium_down = 23,
    topshp_triangle_point_up = 24,
    topshp_triangle_point_down = 25,
    topshp_circle = 26,
    topshp_two_upright_crosses_one_over_the_other = 27,
    topshp_T_shape = 28,
    topshp_triangle_pointing_up_over_a_circle = 29,
    topshp_upright_cross_over_a_circle = 30,
    topshp_rhombus_over_a_circle = 31,
    topshp_circle_over_a_triangle_pointing_up = 32,
    topshp_over_shape = 33,
    topshp_max
};

QString TOPMAR(int _topshp)
{
    QString rule_str;
    if (_topshp == topshp_unknown)
    {
        rule_str = ";SY(QUESMRK1)";
    }
    else
    {
        switch (_topshp) {
            case topshp_cone_point_up : rule_str = (";SY(TOPMAR02)"); break;
            case topshp_cone_point_down : rule_str = (";SY(TOPMAR04)"); break;
            case topshp_sphere : rule_str = (";SY(TOPMAR10)"); break;
            case topshp_2_spheres : rule_str = (";SY(TOPMAR12)"); break;
            case topshp_cylinder : rule_str = (";SY(TOPMAR13)"); break;
            case topshp_board : rule_str = (";SY(TOPMAR14)"); break;
            case topshp_x_shape_St_Andrews_cross : rule_str = (";SY(TOPMAR65)"); break;
            case topshp_upright_cross_St_Georges_cross : rule_str = (";SY(TOPMAR17)"); break;
            case topshp_cube_point_up : rule_str = (";SY(TOPMAR16)"); break;
            case topshp_2_cones_point_to_point: rule_str = (";SY(TOPMAR08)"); break;
            case topshp_2_cones_base_to_base: rule_str = (";SY(TOPMAR07)"); break;
            case topshp_rhombus_diamond: rule_str = (";SY(TOPMAR14)"); break;
            case topshp_2_cones_points_upward: rule_str = (";SY(TOPMAR05)"); break;
            case topshp_2_cones_points_downward: rule_str = (";SY(TOPMAR06)"); break;
            case topshp_flag: rule_str = (";SY(TMARDEF2)"); break;
            case topshp_sphere_over_rhombus: rule_str = (";SY(TOPMAR10)"); break;
            case topshp_square: rule_str = (";SY(TOPMAR13)"); break;
            case topshp_rectangle_horizontal: rule_str = (";SY(TOPMAR14)"); break;
            case topshp_rectangle_vertical: rule_str = (";SY(TOPMAR13)"); break;
            case topshp_trapezium_up: rule_str = (";SY(TOPMAR14)"); break;
            case topshp_trapezium_down: rule_str = (";SY(TOPMAR14)"); break;
            case topshp_triangle_point_up: rule_str = (";SY(TOPMAR02)"); break;
            case topshp_triangle_point_down: rule_str = (";SY(TOPMAR04)"); break;
            case topshp_circle: rule_str = (";SY(TOPMAR10)"); break;
            case topshp_two_upright_crosses_one_over_the_other: rule_str = (";SY(TOPMAR17)"); break;
            case topshp_T_shape: rule_str = (";SY(TOPMAR18)"); break;
            case topshp_triangle_pointing_up_over_a_circle: rule_str = (";SY(TOPMAR02)"); break;
            case topshp_upright_cross_over_a_circle: rule_str = (";SY(TOPMAR17)"); break;
            case topshp_rhombus_over_a_circle: rule_str = (";SY(TOPMAR14)"); break;
            case topshp_circle_over_a_triangle_pointing_up: rule_str = (";SY(TOPMAR10)"); break;
            case topshp_over_shape: rule_str = (";SY(TMARDEF2)"); break;
            default: rule_str = (";SY(TMARDEF2)"); break;
        }
    }
    return rule_str;
}

enum
{
    catwrk_unknown = 0,
    catwrk_non_dangerous_wreck = 1,
    catwrk_dangerous_wreck = 2,
    catwrk_distributed_remains_of_wreck = 3,
    catwrk_wreck_showing_mast = 4,
    catwrk_wreck_showing_any_portion_of_hull_or_superstructure = 5,
    catwrk_max
};

QString WRECKS(int _watlev, double _valsou, int _quasou, int _catwrk, int _quapos, int _geom_type)
{
    QString rule_str;

    const double _safety_contour = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);


    double _depth_value = 0.0;

    if (_valsou != 0.0)
    {
        rule_str.append(SOUNDG(_quasou, tecsou_unknown, _valsou));
    }
    else
    {
        if (_catwrk != catwrk_unknown)
        {
            switch (_catwrk)
            {
            case catwrk_non_dangerous_wreck:
                _depth_value = 20; break;
            case catwrk_dangerous_wreck:
                _depth_value = 0; break;
            case catwrk_wreck_showing_mast:
            case catwrk_wreck_showing_any_portion_of_hull_or_superstructure:
                _depth_value = -15; break;
            }
        }
        else
        {
            switch (_watlev)
            {
            case watlev_unknown:
            case watlev_partly_submerged_at_high_water:
            case watlev_always_dry:
            case watlev_covers_and_uncovers:
            case watlev_subject_to_inundation_or_flooding:
                _depth_value = -15;
                break;
            case watlev_always_under_water_or_submerged:
                _depth_value = 0.01;
                break;
            case watlev_awash:
                _depth_value = 0;
                break;
            }
        }
    }

    switch (_geom_type)
    {
    case wkbPoint:
    case wkbMultiPoint:
        if (_valsou != 0.0)
        {
            if(_valsou < _safety_contour)    // maybe redundant, seems like wrecks with valsou < 20
            {                                                  // are always coded as "dangerous wrecks"
                                                              // Excluding (2 == catwrk) matches Caris logic
                rule_str.append(";SY(DANGER51)");
            }
            else
            {
                rule_str.append(";SY(DANGER52)");
            }
            rule_str.append(";TX('Wk',2,1,2,'15110',1,0,CHBLK,21)");
            if (_quasou == quasou_least_depth_unknown) //Fixes FS 165
            {
                rule_str.append(";SY(WRECKS07)");
            }
        }
        else
        {
            if (_catwrk != catwrk_unknown && _watlev != watlev_unknown)
            {
                if (_catwrk == catlit_directional_function && _watlev == watlev_always_under_water_or_submerged)
                {
                      rule_str.append(";SY(WRECKS04)");
                }
                else
                {
                    if (_catwrk == catwrk_dangerous_wreck && _watlev == watlev_always_under_water_or_submerged)
                    {
                          rule_str.append(";SY(WRECKS05)");
                    }
                    else
                    {
                        if (_catwrk == catwrk_wreck_showing_mast || _catwrk == catwrk_wreck_showing_any_portion_of_hull_or_superstructure)
                        {
                              rule_str.append(";SY(WRECKS01)");
                        }
                        else
                        {
                            if (_watlev == watlev_partly_submerged_at_high_water ||
                                _watlev == watlev_always_dry ||
                                _watlev == watlev_covers_and_uncovers ||
                                _watlev == watlev_awash)
                            {
                                  rule_str.append(";SY(WRECKS01)");
                            }
                            else
                            {
                                  rule_str.append(";SY(WRECKS05)"); // default
                            }
                        }
                    }
                }
            }
        }
        break;
    case wkbPolygon:
    case wkbMultiPolygon:
        if (2 <= _quapos && _quapos < 10)
        {
              rule_str.append(";LC(LOWACC41)");
        }
        else
        {
            if (_valsou != 0.0){
                if (_valsou <= 20)
                {
                      rule_str.append(";LS(DOTT,2,CHBLK)");
                }
                else
                {
                      rule_str.append(";LS(DASH,2,CHBLK)");
                }
            }
            else
            {
                if (_watlev = watlev_unknown)
                {
                      rule_str.append(";LS(DOTT,2,CSTLN)");
                }
                else
                {
                    switch (_watlev)
                    {
                    case watlev_partly_submerged_at_high_water:
                    case watlev_always_dry:
                        rule_str.append(";LS(SOLD,2,CSTLN)");
                        break;
                    case watlev_covers_and_uncovers:
                        rule_str.append(";LS(DASH,2,CSTLN)");
                        break;
                    case watlev_always_under_water_or_submerged:
                    case watlev_awash:
                    default:
                        rule_str.append(";LS(DOTT,2,CSTLN)");
                        break;
                    }
                }
            }
        }
        if (_valsou == 0.0)
        {
            switch (_watlev)
            {
            case watlev_partly_submerged_at_high_water:
            case watlev_always_dry:
                rule_str.append(";AC(CHBRN)");
                break;
            case watlev_covers_and_uncovers:
                rule_str.append(";AC(DEPIT)");
                break;
            case watlev_always_under_water_or_submerged:
            case watlev_awash:
            default:
                rule_str.append(";AC(DEPVS)");
                break;
            }
        }
        break;
    }

    return rule_str;
}

// ------------------------------------------------------------------------------

QVariant get_attribute(OGRFeature * _feat, const QString & _attr_name)
{
    int _idx = _feat->GetFieldIndex(_attr_name.toUpper().toLocal8Bit());
    if (_idx < 0)
    {
        return (QVariant());
    }
    int _count = 0;
    switch (_feat->GetFieldDefnRef(_idx)->GetType())
    {
    case OFTInteger:
        return (QVariant::fromValue(_feat->GetFieldAsInteger(_idx)));
    case OFTReal:
        return (QVariant::fromValue(_feat->GetFieldAsDouble(_idx)));        
    case OFTString:
    default:
        return (QVariant::fromValue(QString(_feat->GetFieldAsString(_idx))));
    case OFTIntegerList:
        {
            const int * _values = _feat->GetFieldAsIntegerList(_idx, & _count);
            QList<QVariant> _v;
            for (int _val_idx = 0; _val_idx < _count; _val_idx++)
            {
                _v << QVariant::fromValue(_values[_val_idx]);
            }
            return (_v);
        }
    case OFTRealList:
        {
            const double * _values = _feat->GetFieldAsDoubleList(_idx, & _count);
            QList<QVariant> _v;
            for (int _val_idx = 0; _val_idx < _count; _val_idx++)
            {
                _v << QVariant::fromValue(_values[_val_idx]);
            }
            return (_v);
        }
    case OFTStringList:
        {
            char ** _values = _feat->GetFieldAsStringList(_idx);
            QList<QVariant> _v;
            while (_values)
            {
                _v << QVariant::fromValue(QString(* _values++));
            }
            return (_v);
        }
    }
    return (QVariant());
}

template <class T>
QList<T> split_str_attr_as_int(const QByteArray & _str)
{
    QList<T> _l;
    foreach (const QByteArray & _att, _str.split(','))
    {
        _l << _att.toInt();
    }
    return (_l);
}

template <class T>
bool lists_intersects(const QList<T> & _l1, const QList<T> & _l2)
{
    foreach (const T & _el1, _l1)
    {
        if (qFind(_l2, _el1) != _l2.end())
        {
            return true;
        }
    }
    return false;
}

#define CHECK_CS(NAME) _cs.contains(NAME, Qt::CaseInsensitive)
#define CHECK_LY(NAME) _layer_name.contains(NAME, Qt::CaseInsensitive)
#define DBL_ATTR(NAME) get_attribute(_feat, NAME).toDouble()
#define INT_ATTR(NAME) get_attribute(_feat, NAME).toInt()
#define STR_ATTR(NAME) get_attribute(_feat, NAME).toByteArray()
#define LST_ATTR(NAME) get_attribute(_feat, NAME).toList()

#define DSBL(XPR) (false)

namespace S52_CS
{
    QString translate(const QString & _layer_name, const QString & _cs, OGRFeature * _feat, int _collection_idx)
    {
        QString _translated_cs;
        if (_feat == 0)
        {
            return _translated_cs;
        }

        OGRGeometry * _geom = _feat->GetGeometryRef();
        if ((_collection_idx > 0) && (dynamic_cast<OGRGeometryCollection *>(_geom)))
        {
            if (_collection_idx < ((OGRGeometryCollection *)_geom)->getNumGeometries())
            {
                _geom = ((OGRGeometryCollection *)_geom)->getGeometryRef(_collection_idx);
            }
        }
        const int _wkb = wkbFlatten(_geom->getGeometryType());

        if (CHECK_CS("DATCVR"))
        {
            _translated_cs = DATCVR();
        }
        else
        if (CHECK_CS("DEPARE"))
        {
            _translated_cs = DEPARE(
                        DBL_ATTR("DRVAL1"),
                        DBL_ATTR("DRVAL2"),
                        CHECK_LY("DRGARE"));
        }
        else
        if (CHECK_CS("DEPCNT"))
        {
            _translated_cs = DEPCNT(
                        DBL_ATTR("DRVAL1"),
                        DBL_ATTR("DRVAL2"),
                        DBL_ATTR("VALDCO"),
                        INT_ATTR("QUAPOS"));
        }
        else
        if (CHECK_CS("LIGHTS"))
        {
            _translated_cs = LIGHTS(
                        DBL_ATTR("VALNMR"),
                        INT_ATTR("CATLIT"),
                        INT_ATTR("LITVIS"),
                        INT_ATTR("COLOUR"),
                        DBL_ATTR("SECTR1"),
                        DBL_ATTR("SECTR2"));
        }
        else
        if (CHECK_CS("SOUNDG"))
        {
            switch (_wkb)
            {
            case wkbPoint:
            case wkbMultiPoint:
                _translated_cs = SOUNDG(
                            INT_ATTR("QUASOU"),
                            INT_ATTR("TECSOU"),
                            ((OGRPoint *)_geom)->getZ());
                break;
            }
        }
        else
        if (CHECK_CS("QUAPOS"))
        {
            _translated_cs = QUAPOS(
                        INT_ATTR("QUAPOS"),
                        INT_ATTR("QUALTY"),
                        INT_ATTR("CONRAD"),
                        _wkb,
                        CHECK_LY("COALNE"));
        }
        else
        if (CHECK_CS("SLCONS"))
        {
            _translated_cs = SLCONS(
                        INT_ATTR("QUAPOS"),
                        INT_ATTR("CONDTN"),
                        INT_ATTR("WATLEV"),
                        INT_ATTR("CATSLC")
                        );
        }
        else
        if (CHECK_CS("RESARE"))
        {
            _translated_cs = RESARE(
                        split_str_attr_as_int<int>(STR_ATTR("RESTRN")),
                        split_str_attr_as_int<int>(STR_ATTR("CATREA")));
        }
        else
        if (CHECK_CS("RESTRN"))
        {
            _translated_cs = RESTRN(
                        split_str_attr_as_int<int>(STR_ATTR("RESTRN")));
        }
        else
        if (CHECK_CS("TOPMAR"))
        {
            _translated_cs = TOPMAR(
                        INT_ATTR("TOPSHP"));
        }
        else
        if (CHECK_CS("WRECKS"))
        {
            _translated_cs = WRECKS(
                        INT_ATTR("WATLEV"),
                        DBL_ATTR("VALSOU"),
                        INT_ATTR("QUASOU"),
                        INT_ATTR("CATWRK"),
                        INT_ATTR("QUAPOS"),
                        _wkb);
        }
        return _translated_cs;
    }
}
