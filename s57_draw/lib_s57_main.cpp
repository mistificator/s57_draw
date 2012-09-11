#include "lib_s57.h"
#include "s52_symbol.h"
#include "s57_buildscene.h"

#include <QStringList>
#include <QDir>
#include <QGraphicsScene>
#include <QPainter>

void nextDir(S57_BuildScene * _builder, const QString & _dir, bool _scan = false)
{
    QStringList _files = QDir(_dir).entryList(QStringList() << "*.000");
    foreach (const QString & _fn, _files)
    {
        if (!_scan)
        {
            _builder->build(_dir + "/" + _fn);
        }
    }
    QStringList _dirs = QDir(_dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (_dirs.isEmpty())
    {
        return;
    }
    foreach (const QString & _dn, _dirs)
    {
        nextDir(_builder, _dir + "/" + _dn, _scan);
    }
}

QImage render_map_region(const QString & _dir, const QRectF & _map_rgn, int _max_dim, bool _land_and_sea_only)
{
    int _width, _height;
    if (_map_rgn.width() > _map_rgn.height())
    {
        _width = _max_dim;
        _height = _max_dim * _map_rgn.height() / _map_rgn.width();
    }
    else
    {
        _height = _max_dim;
        _width = _max_dim * _map_rgn.width() / _map_rgn.height();
    }
    QImage _image(_width, _height, QImage::Format_RGB32);
    _image.fill(Qt::white);
    QPainter _p(&_image);

    S57_BuildScene _builder;
    QGraphicsScene * _scene = new QGraphicsScene();
    _builder.setScene(_scene);
    nextDir(& _builder, _dir);
    _builder.render();
    if (_land_and_sea_only)
    {
        _builder.updateVisibility(0x7fffffff, true);
    }
    _scene->render(& _p, QRectF(), _map_rgn);
    return _image;
}
