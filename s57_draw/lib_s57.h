#ifndef LIB_S57_MAIN_H
#define LIB_S57_MAIN_H

#ifdef LIB_S57_DLL
    #define LIB_S57_EXPORT __declspec(dllexport)
#else
    #define LIB_S57_EXPORT
#endif

#include <QString>
#include <QRectF>
#include <QImage>
#include <QSize>

QImage LIB_S57_EXPORT render_map_region(const QString & _dir, const QRectF & _map_rgn, int _max_dim = 2048, bool _land_and_sea_only = true);

#endif // LIB_S57_MAIN_H
