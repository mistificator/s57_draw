#include "lib_s57.h"
#include <QApplication>
#include <QDir>
#include <QDebug>

int main(int _argc, char ** _argv)
{
    QApplication _a(_argc, _argv);

    if (_argc == 1)
    {
        qDebug() << "lib_s57 frontend";
        qDebug() << "(c) 2012 m.p.";
        qDebug() << "usage: lib_s57_frontend.exe [path] [coordinates top-left bottom-right: x1, y1, x2, y2] [side dimension: def. 2048] [simple-complex]";
        qDebug() << "example: lib_s57_frontend c:\\map (10,10,20,20) 8192 complex";
        return 0;
    }

    QString _dir;
    QRectF _rgn;
    int _dim = 2048;
    bool _complex = true;
    for (int _i = 1; _i < _argc; _i++)
    {
        const QString _arg = _argv[_i];
        if (QDir(_arg).exists() && (_arg.count() > _dir.count()))
        {
            _dir = _arg;
            continue;
        }
        QStringList _list = QString(_arg).replace("(", "").replace(")", "").split(",", QString::SkipEmptyParts);
        if (_list.count())
        {
            if (_list.count() >= 4)
            {
                _rgn = QRectF(QPointF(_list[0].toDouble(), _list[1].toDouble()), QPointF(_list[2].toDouble(), _list[3].toDouble()));
            }
            continue;
        }
        bool _ok;
        int _n = _arg.toInt(& _ok);
        if (_ok)
        {
            _dim = _n;
        }
        if (_arg.contains("simple", Qt::CaseInsensitive))
        {
            _complex = false;
        }
    }

    if (QDir(_dir).exists() && (_dim > 0))
    {
        render_map_region(_dir, _rgn, _dim, !_complex).save(QApplication::instance()->applicationDirPath() + "/out.png");
    }

    return 0;
}
