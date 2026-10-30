#include "s52_symbol.h"
#include <QMap>
#include <QByteArray>
#include <QFile>
#include <QString>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <cmath>

#define M 4
#define N 4
#define MOD_REC(str)    if(0==qstrncmp(#str,pBuf,4))
#define ENDLN   "%1024[^\037]"
#define NEWLN  "%1024[^\n]"
#define  CIE_x_r                0.640            // nominal CRT primaries
#define  CIE_y_r                0.330
#define  CIE_x_g                0.290
#define  CIE_y_g                0.600
#define  CIE_x_b                0.150
#define  CIE_y_b                0.060
#define  CIE_x_w                0.295 //0.3333333333          // monitor white point
#define  CIE_y_w                0.315// 0.3333333333
#define CIE_D           (       CIE_x_r*(CIE_y_g - CIE_y_b) + \
    CIE_x_g*(CIE_y_b - CIE_y_r) + \
    CIE_x_b*(CIE_y_r - CIE_y_g)     )
#define CIE_C_rD        (       (1./CIE_y_w) * \
    ( CIE_x_w*(CIE_y_g - CIE_y_b) - \
    CIE_y_w*(CIE_x_g - CIE_x_b) + \
    CIE_x_g*CIE_y_b - CIE_x_b*CIE_y_g     ) )
#define CIE_C_gD        (       (1./CIE_y_w) * \
    ( CIE_x_w*(CIE_y_b - CIE_y_r) - \
    CIE_y_w*(CIE_x_b - CIE_x_r) - \
    CIE_x_r*CIE_y_b + CIE_x_b*CIE_y_r     ) )
#define CIE_C_bD        (       (1./CIE_y_w) * \
    ( CIE_x_w*(CIE_y_r - CIE_y_g) - \
    CIE_y_w*(CIE_x_r - CIE_x_g) + \
    CIE_x_r*CIE_y_g - CIE_x_g*CIE_y_r     ) )
#define CIE_rf          (CIE_y_r*CIE_C_rD/CIE_D)
#define CIE_gf          (CIE_y_g*CIE_C_gD/CIE_D)
#define CIE_bf          (CIE_y_b*CIE_C_bD/CIE_D)

static double tmat[3][3] =       //XYZ to RGB
{
{ ( CIE_y_g - CIE_y_b - CIE_x_b*CIE_y_g + CIE_y_b*CIE_x_g ) /CIE_C_rD,
( CIE_x_b - CIE_x_g - CIE_x_b*CIE_y_g + CIE_x_g*CIE_y_b ) /CIE_C_rD,
( CIE_x_g*CIE_y_b - CIE_x_b*CIE_y_g ) /CIE_C_rD},
{ ( CIE_y_b - CIE_y_r - CIE_y_b*CIE_x_r + CIE_y_r*CIE_x_b ) /CIE_C_gD,
( CIE_x_r - CIE_x_b - CIE_x_r*CIE_y_b + CIE_x_b*CIE_y_r ) /CIE_C_gD,
( CIE_x_b*CIE_y_r - CIE_x_r*CIE_y_b ) /CIE_C_gD},
{ ( CIE_y_r - CIE_y_g - CIE_y_r*CIE_x_g + CIE_y_g*CIE_x_r ) /CIE_C_bD,
( CIE_x_g - CIE_x_r - CIE_x_g*CIE_y_r + CIE_x_r*CIE_y_g ) /CIE_C_bD,
( CIE_x_r*CIE_y_g - CIE_x_g*CIE_y_r ) /CIE_C_bD}
};
static double c_gamma = 2.20;

struct Position
{
    union          {int              dummy1,     PAMI,       dummy2;     } minDist;
    union          {int              dummy1,     PAMA,       dummy2;     } maxDist;
    union          {int              LICL,       PACL,       SYCL;       } pivot_x;
    union          {int              LIRW,       PARW,       SYRW;       } pivot_y;
    union          {int              LIHL,       PAHL,       SYHL;       } bnbox_w;
    union          {int              LIVL,       PAVL,       SYVL;       } bnbox_h;
    union          {int              LBXC,       PBXC,       SBXC;       } bnbox_x; // UpLft crnr
    union          {int              LBXR,       PBXR,       SBXR;       } bnbox_y; // UpLft crnr
};

struct Rule
{
    int             RCID;
    bool            is_pattern;
    struct          {QByteArray       NM;       } name;
    union           {char             dummy1,     PADF,       SYDF;       } definition;
    union           {char             dummy2,     PATP,       dummy3;     } fillType;
    union           {char             dummy4,     PASP,       dummy5;     } spacing;
    union           {Position         line,       patt,       symb;       } pos;
    struct          {QByteArray       XPO;      } exposition;
    struct          {QByteArray       BTM;      } bitmap;
    struct          {QByteArray       CRF;      } colRef;
    struct          {QByteArray       VCT;      } vector;
    Rule(): RCID(0), is_pattern(false) {}
};

struct S52color
{
    S52color(): x(0), y(0), L(0), rgb(0) {}
    QByteArray s52_name, color_name;
    double x;
    double y;
    double L;
    QRgb rgb;
};

struct S52_Symbol::Data
{
    QMap<QString, QMap<QString, QImage> > symbols;
    QMap<QString, QMap<QString, S52color> > colors;
    QMap<QString, Rule> rules;
    QString color_scheme;
    Data(): color_scheme("DAY_BRIGHT")
    {

    }
    ~Data()
    {

    }
    QRgb ccie_to_rgb(double _x, double _y, double _L) const
    {
        double _X,_Y,_Z;
        if ( _y != 0 )
        {
            _X = ( _x * _L ) / _y;
            _Y = _L;
            _Z = ( ( ( 1.0 - _x ) - _y ) * _L ) / _y;
        }
        else
        {
            _X=0;
            _Y=0;
            _Z=0;
        }
        //    Transform CIE _X_Y_Z into RGB
        double _dR = ( _X * tmat[0][0] ) + ( _Y * tmat[0][1] ) + ( _Z * tmat[0][2] );
        double _dG = ( _X * tmat[1][0] ) + ( _Y * tmat[1][1] ) + ( _Z * tmat[1][2] );
        double _dB = ( _X * tmat[2][0] ) + ( _Y * tmat[2][1] ) + ( _Z * tmat[2][2] );
        //       Arbitraril_Y clip the luminance values to 100
        _dR = qBound<double>(0.0, _dR, 100.0) / 100.0;
        _dG = qBound<double>(0.0, _dG, 100.0) / 100.0;
        _dB = qBound<double>(0.0, _dB, 100.0) / 100.0;
        //       And scale
		_dR = ::pow ( _dR, 1.0 / c_gamma );
        _dG = ::pow ( _dG, 1.0 / c_gamma );
        _dB = ::pow ( _dB, 1.0 / c_gamma );

        return (qRgb(_dR * 255, _dG * 255, _dB * 255));
    }
    void parse_cols(QFile & _file)
    {
        QByteArray _line = _file.readLine().trimmed();
        QString _scheme = QString(_line.mid(19).replace(0x1f, "")).toUpper();
        while (((_line = _file.readLine().trimmed()).left(4) != "****") && (!_file.atEnd()))
        {
            S52color _color;
            _color.s52_name = _line.mid(9, 5);
            QList<QByteArray> _pars = _line.mid(14).trimmed().split(0x1f);
            for (int _i = 0; _i < _pars.count(); _i++)
            {
                switch (_i)
                {
                case 0:
                    _color.x = _pars[_i].toDouble();
                    break;
                case 1:
                    _color.y = _pars[_i].toDouble();
                    break;
                case 2:
                    _color.L = _pars[_i].toDouble();
                    break;
                case 3:
                    _color.color_name = _pars[_i];
                    break;
                }
            }
            _color.rgb = ccie_to_rgb(_color.x, _color.y, _color.L);
            colors[_scheme][_color.s52_name] = _color;
            //            qDebug() << _scheme << _color.s52_name << (0x00ffffff & _color.rgb) << _color.x << _color.y << _color.L;
        }
    }
    void parse_symb(QFile & _file)
    {
        QByteArray _line = _file.readLine().trimmed();
        Rule _rule;
        _rule.RCID = _line.mid(11, 5).toInt();
        while (((_line = _file.readLine().trimmed()).left(4) != "****") && (!_file.atEnd()))
        {
            QByteArray _cmd = _line.left(4).toUpper();
            if (_cmd == "SYMD")
            {
                _rule.is_pattern = false;
                _rule.name.NM = _line.mid(9, 8);
                _rule.definition.SYDF = _line[17];
                _rule.pos.symb.pivot_x.SYCL = _line.mid(18, 5).toInt();
                _rule.pos.symb.pivot_y.SYRW = _line.mid(23, 5).toInt();
                _rule.pos.symb.bnbox_w.SYHL = _line.mid(28, 5).toInt();
                _rule.pos.symb.bnbox_h.SYVL = _line.mid(33, 5).toInt();
                _rule.pos.symb.bnbox_x.SBXC = _line.mid(38, 5).toInt();
                _rule.pos.symb.bnbox_y.SBXR = _line.mid(43, 5).toInt();
                continue;
            }
            if (_cmd == "PATD")
            {
                _rule.is_pattern = true;
                _rule.name.NM = _line.mid(9, 8);
                _rule.definition.PADF = _line[17];
                _rule.pos.patt.minDist.PAMI = _line.mid(24, 5).toInt();
                _rule.pos.patt.maxDist.PAMA = _line.mid(29, 5).toInt();
                _rule.pos.patt.pivot_x.PACL = _line.mid(34, 5).toInt();
                _rule.pos.patt.pivot_y.PARW = _line.mid(39, 5).toInt();
                _rule.pos.patt.bnbox_w.PAHL = _line.mid(44, 5).toInt();
                _rule.pos.patt.bnbox_h.PAVL = _line.mid(49, 5).toInt();
                _rule.pos.patt.bnbox_x.PBXC = _line.mid(54, 5).toInt();
                _rule.pos.patt.bnbox_y.PBXR = _line.mid(59, 5).toInt();
                continue;
            }
            if (_cmd == "LIND")
            {
                _rule.name.NM = _line.mid(9, 8);
                _rule.definition.SYDF = 'V';
                _rule.pos.line.pivot_x.LICL = _line.mid(17, 5).toInt();
                _rule.pos.line.pivot_y.LIRW = _line.mid(22, 5).toInt();
                _rule.pos.line.bnbox_w.LIHL = _line.mid(27, 5).toInt();
                _rule.pos.line.bnbox_h.LIVL = _line.mid(32, 5).toInt();
                _rule.pos.line.bnbox_x.LBXC = _line.mid(37, 5).toInt();
                _rule.pos.line.bnbox_y.LBXR = _line.mid(42, 5).toInt();
                continue;
            }
            QByteArray _cmd_r = _line.mid(1, 3).toUpper();
            if (_cmd_r == "XPO")
            {
                _rule.exposition.XPO.append(_line.mid(9).replace(0x1f, ""));
            }
            else
                if (_cmd_r == "BTM")
                {
                    _rule.bitmap.BTM.append(_line.mid(9).replace(0x1f, ""));
                }
                else
                    if (_cmd_r == "CRF")
                    {
                        _rule.colRef.CRF.append(_line.mid(9).replace(0x1f, ""));
                    }
                    else
                        if (_cmd_r == "VCT")
                        {
                            _rule.vector.VCT.append(_line.mid(9).replace(0x1f, ""));
                        }
        }
        rules[_rule.name.NM] = _rule;
    }
    QMap<char, QRgb> create_colors(const QString & _scheme, const Rule & _rule)
    {
        QMap<char, QRgb> _cmap;
        for (int _i = 0; _i < _rule.colRef.CRF.count(); _i += 6)
        {
            _cmap[_rule.colRef.CRF[_i]] = colors[_scheme][_rule.colRef.CRF.mid(_i + 1, 5)].rgb;
        }
        _cmap['@'] = 0;
        return _cmap;
    }

    QImage render_raster(const QString & _scheme, const Rule & _rule)
    {
        const QMap<char, QRgb> & _cmap = create_colors(_scheme, _rule);
        const int _w = _rule.pos.symb.bnbox_w.SYHL;
        const int _h = _rule.pos.symb.bnbox_h.SYVL;
        if (_w <= 0 || _h <= 0)
        {
            return QImage();
        }

        if (_rule.is_pattern)
        {
            QImage _image(_w, _h, QImage::Format_ARGB32);
            _image.fill(QColor(0, 0, 0, 0).rgba());
            const int _count = qMin(_rule.bitmap.BTM.count(), _w * _h);
            qint32 * _pixels = reinterpret_cast<qint32 *>(_image.bits());
            for (int _i = 0; _i < _count; _i++)
            {
                _pixels[_i] = _cmap[_rule.bitmap.BTM[_i]];
            }
            return _image;
        }

        const int _px = _rule.pos.symb.pivot_x.SYCL;
        const int _py = _rule.pos.symb.pivot_y.SYRW;
        const int _half_w = qMax(qAbs(_px), qAbs(_w - _px));
        const int _half_h = qMax(qAbs(_py), qAbs(_h - _py));
        const int _out_w = 2 * _half_w;
        const int _out_h = 2 * _half_h;
        if (_out_w <= 0 || _out_h <= 0)
        {
            return QImage();
        }

        QImage _image(_out_w, _out_h, QImage::Format_ARGB32);
        _image.fill(QColor(0, 0, 0, 0).rgba());

        const int _offset_x = _half_w - _px;
        const int _offset_y = _half_h - _py;
        const int _count = _rule.bitmap.BTM.size();

        for (int _row = 0; _row < _h; _row++)
        {
            const int _dst_y = _offset_y + _row;
            if (_dst_y < 0 || _dst_y >= _out_h)
            {
                continue;
            }
            qint32 * _scan = reinterpret_cast<qint32 *>(_image.scanLine(_dst_y));
            for (int _col = 0; _col < _w; _col++)
            {
                const int _src_idx = _row * _w + _col;
                if (_src_idx >= _count)
                {
                    break;
                }
                const int _dst_x = _offset_x + _col;
                if (_dst_x >= 0 && _dst_x < _out_w)
                {
                    _scan[_dst_x] = _cmap[_rule.bitmap.BTM[_src_idx]];
                }
            }
        }
        return _image;
    }

    void flush_polyline(QPainter & _p, QPolygonF & _poly)
    {
        if (_poly.isEmpty())
        {
            return;
        }
        _p.setBrush(Qt::NoBrush);
        if (_poly.count() == 1)
        {
            _p.drawPoint(_poly[0]);
        }
        else
        {
            _p.drawPolyline(_poly);
        }
        _poly.clear();
    }

    QImage render_vector(const QString & _scheme, const Rule & _rule, double _scale_factor)
    {
        const QMap<char, QRgb> & _cmap = create_colors(_scheme, _rule);
        const double _extra = 6.0;
        const double _w = _rule.pos.symb.bnbox_w.SYHL * _scale_factor;
        const double _h = _rule.pos.symb.bnbox_h.SYVL * _scale_factor;
        const double _x = _rule.pos.symb.bnbox_x.SBXC * _scale_factor;
        const double _y = _rule.pos.symb.bnbox_y.SBXR * _scale_factor;
        const double _px = _rule.pos.symb.pivot_x.SYCL * _scale_factor;
        const double _py = _rule.pos.symb.pivot_y.SYRW * _scale_factor;

        const double _half_w = qMax(qAbs(_px - _x), qAbs((_x + _w) - _px)) + _extra;
        const double _half_h = qMax(qAbs(_py - _y), qAbs((_y + _h) - _py)) + _extra;

        QImage _image(qCeil(2.0 * _half_w), qCeil(2.0 * _half_h), QImage::Format_ARGB32);
        _image.fill(QColor(0, 0, 0, 0).rgba());
        QPainter _p(&_image);
        _p.setRenderHint(QPainter::Antialiasing, true);

        const double _offset_x = _px - _half_w;
        const double _offset_y = _py - _half_h;

        QPolygonF _poly;
        QPainterPath _pm_path;
        QPointF _cur_pt(0.0, 0.0);
        QColor _cur_color(Qt::black);
        int _alpha = 255;
        int _width = 1;
        bool _in_pm = false;

        foreach (const QByteArray & _par_raw, _rule.vector.VCT.split(';'))
        {
            const QByteArray _par = _par_raw.trimmed();
            if (_par.count() < 2)
            {
                continue;
            }
            const QByteArray _cmd = _par.left(2).toUpper();
            if (_cmd == "SP")
            {
                if (!_in_pm)
                {
                    flush_polyline(_p, _poly);
                }
                if (_par.count() > 2)
                {
                    _cur_color = QColor(_cmap[_par[2]]);
                    _cur_color.setAlpha(_alpha);
                    _p.setPen(QPen(_cur_color, _width));
                }
            }
            else if (_cmd == "SW")
            {
                if (!_in_pm)
                {
                    flush_polyline(_p, _poly);
                }
                if (_par.count() > 2)
                {
                    _width = qMax(1, _par.mid(2).toInt());
                    _p.setPen(QPen(_cur_color, _width));
                }
            }
            else if (_cmd == "ST")
            {
                if (_par.count() > 2)
                {
                    const int _st = _par.mid(2).toInt();
                    _alpha = qBound(0, 255 - _st * 64, 255);
                    _cur_color.setAlpha(_alpha);
                    _p.setPen(QPen(_cur_color, _width));
                }
            }
            else if (_cmd == "PU")
            {
                if (_in_pm)
                {
                    if (_poly.count() > 1)
                    {
                        _pm_path.addPolygon(_poly);
                    }
                    _poly.clear();
                }
                else
                {
                    flush_polyline(_p, _poly);
                }
                QList<QByteArray> _pt = _par.mid(2).split(',');
                for (int _i = 1; _i < _pt.count(); _i += 2)
                {
                    _cur_pt = QPointF(_pt[_i - 1].toInt() * _scale_factor - _offset_x,
                                      _pt[_i].toInt() * _scale_factor - _offset_y);
                    _poly.append(_cur_pt);
                }
            }
            else if (_cmd == "PD")
            {
                const QByteArray _coords = _par.mid(2);
                if (_coords.isEmpty())
                {
                    _poly.append(_cur_pt);
                }
                else
                {
                    QList<QByteArray> _pt = _coords.split(',');
                    for (int _i = 1; _i < _pt.count(); _i += 2)
                    {
                        _cur_pt = QPointF(_pt[_i - 1].toInt() * _scale_factor - _offset_x,
                                          _pt[_i].toInt() * _scale_factor - _offset_y);
                        _poly.append(_cur_pt);
                    }
                }
            }
            else if (_cmd == "CI")
            {
                const double _r = _par.mid(2).toInt() * _scale_factor;
                if (_in_pm)
                {
                    _pm_path.addEllipse(_cur_pt, _r, _r);
                }
                else
                {
                    flush_polyline(_p, _poly);
                    _pm_path = QPainterPath();
                    _pm_path.addEllipse(_cur_pt, _r, _r);
                    _p.setBrush(Qt::NoBrush);
                    _p.drawEllipse(_cur_pt, _r, _r);
                }
            }
            else if (_cmd == "PM")
            {
                const char _mode = (_par.count() > 2) ? _par[2] : '0';
                if (_mode == '0')
                {
                    flush_polyline(_p, _poly);
                    _in_pm = true;
                    _pm_path = QPainterPath();
                    _pm_path.setFillRule(Qt::OddEvenFill);
                    _poly.clear();
                    _poly.append(_cur_pt);
                }
                else if (_mode == '1')
                {
                    if (_poly.count() > 1)
                    {
                        _pm_path.addPolygon(_poly);
                        _pm_path.closeSubpath();
                    }
                    _poly.clear();
                }
                else if (_mode == '2')
                {
                    if (_poly.count() > 1)
                    {
                        _pm_path.addPolygon(_poly);
                        _pm_path.closeSubpath();
                    }
                    _poly.clear();
                    _in_pm = false;
                }
            }
            else if (_cmd == "FP")
            {
                if (!_pm_path.isEmpty())
                {
                    _p.fillPath(_pm_path, QBrush(_cur_color));
                }
            }
            else if (_cmd == "EP")
            {
                if (!_pm_path.isEmpty())
                {
                    _p.strokePath(_pm_path, _p.pen());
                }
            }
        }
        if (!_in_pm)
        {
            flush_polyline(_p, _poly);
        }
        return _image;
    }

    void set_sources(const QStringList & _list)
    {
        colors.clear();
        rules.clear();
        symbols.clear();
        foreach (const QString & _src, _list)
        {
            QFile _file(_src);
            if (!_file.open(QFile::ReadOnly | QFile::Text))
            {
                continue;
            }
            while (!_file.atEnd())
            {
                QByteArray _line = _file.peek(4).toUpper();
                if ((_line.count()) < 4 || (_line[0] == ';'))
                {
                    _file.readLine();
                    continue;
                }
                if (_line == "COLS")
                {
                    parse_cols(_file);
                }
                else
                    if ((_line == "SYMB") || (_line == "LNST") || (_line == "PATT"))
                    {
                        parse_symb(_file);
                    }
                    else
                    {
                        _file.readLine();
                    }
            }
            _file.close();
        }
        foreach (const Rule & _rule, rules)
        {
            foreach (const QString & _scheme, colors.keys())
            {
                if (QString(_rule.definition.SYDF).compare("R", Qt::CaseInsensitive) == 0)
                {
                    symbols[_scheme][_rule.name.NM] = render_raster(_scheme, _rule);
                }
                else
                    if (QString(_rule.definition.SYDF).compare("V", Qt::CaseInsensitive) == 0)
                    {
                        symbols[_scheme][_rule.name.NM] = render_vector(_scheme, _rule, 0.025);
                    }
            }
        }
    }
};

S52_Symbol::S52_Symbol(): d(QSharedPointer<Data>::create())
{
}

S52_Symbol::S52_Symbol(const S52_Symbol & _other): d(QSharedPointer<Data>::create(*_other.d))
{
}

S52_Symbol & S52_Symbol::operator = (const S52_Symbol & _other)
{
    if (&_other != this)
    {
        d = QSharedPointer<Data>::create(*_other.d);
    }
    return *this;
}

S52_Symbol::~S52_Symbol()
{
}

void S52_Symbol::setSources(const QStringList & _list)
{
    d->set_sources(_list);
    //    qDebug() << d->symbols.begin()->count() << d->colors.begin()->count();
}

QImage S52_Symbol::image (const QString & _name) const
{    
    if (d->symbols[d->color_scheme].contains(_name))
    {
        return (d->symbols[d->color_scheme][_name]);
    }
    qDebug() << "not found" << d->color_scheme << _name;
    return (QImage());
}

QPixmap S52_Symbol::pixmap (const QString & _name) const
{
    if (d->symbols[d->color_scheme].contains(_name))
    {
        return QPixmap::fromImage(d->symbols[d->color_scheme][_name]);
    }
    qDebug() << "not found" << d->color_scheme << _name;
    return (QPixmap());
}

QRgb S52_Symbol::color(const QString & _name) const
{
    if (d->colors[d->color_scheme].contains(_name))
    {
        return (d->colors[d->color_scheme][_name].rgb);
    }
    return (0);
}

QRgb S52_Symbol::symbol_color(const QString & _name) const
{
    if (d->rules.contains(_name))
    {
        const QMap<char, QRgb> & _cmap = d->create_colors(d->color_scheme, d->rules[_name]);
        if (_cmap.contains('A'))
        {
            return (_cmap['A']);
        }
    }
    return (0);
}

bool S52_Symbol::setColorScheme(const QString & _scheme)
{
    if (d->colors.keys().contains(QString(_scheme).toUpper()))
    {
        d->color_scheme = _scheme;
        return (true);
    }
    return (false);
}

QString S52_Symbol::colorScheme() const
{
    return (d->color_scheme);
}

QStringList S52_Symbol::colorSchemes() const
{
    return (d->colors.keys());
}

QStringList S52_Symbol::symbols(const QString & _scheme) const
{
    const QString _up = _scheme.toUpper();
    if (!d->symbols.contains(_up))
    {
        return QStringList();
    }
    return d->symbols[_up].keys();
}

QMap<QString, QImage> S52_Symbol::images () const
{
    QMap<QString, QImage> _images;
    foreach (const QString _name, d->symbols[d->color_scheme].keys())
    {
        _images[_name] = d->symbols[d->color_scheme][_name];
    }
    return (_images);
}

QMap<QString, QPixmap> S52_Symbol::pixmaps () const
{
    QMap<QString, QPixmap> _pixmaps;
    foreach (const QString _name, d->symbols[d->color_scheme].keys())
    {
        _pixmaps[_name] = QPixmap::fromImage(d->symbols[d->color_scheme][_name]);
    }
    return (_pixmaps);
}

/*
QImage s52_symbol(const QString & _name)
{    
    return (S52_Symbol::global.image(_name));
}

QRgb s52_color(const QString & _name)
{
    return (S52_Symbol::global.color(_name));
}

QRgb s52_symbol_color(const QString & _name)
{
    return (S52_Symbol::global.symbol_color(_name));
}
*/
