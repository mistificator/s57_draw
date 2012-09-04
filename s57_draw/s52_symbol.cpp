#include "s52_symbol.h"
#include <QMap>
#include <QByteArray>
#include <QFile>
#include <QString>
#include <QDebug>
#include <QPainter>
#include <qmath.h>

S52_Symbol S52_Symbol::global;

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
    struct          {QByteArray       NM;       } name;
    union           {char             dummy,      PADF,       SYDF;       } definition;
    union           {char             dummy1,     PATP,       dummy2;     } fillType;
    union           {char             dummy1,     PASP,       dummy2;     } spacing;
    union           {Position         line,       patt,       symb;       } pos;
    struct          {QByteArray       XPO;      } exposition;
    struct          {QByteArray       BTM;      } bitmap;
    struct          {QByteArray       CRF;      } colRef;
    struct          {QByteArray       VCT;      } vector;
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
        _dR = qMin<double>(_dR, 100) / 100;
        _dG = qMin<double>(_dG, 100) / 100;
        _dB = qMin<double>(_dB, 100) / 100;
        //       And scale
        _dR = qPow ( _dR, 1.0 / c_gamma );
        _dG = qPow ( _dG, 1.0 / c_gamma );
        _dB = qPow ( _dB, 1.0 / c_gamma );

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
        QImage _image(_rule.pos.symb.bnbox_w.SYHL, _rule.pos.symb.bnbox_h.SYVL, QImage::Format_ARGB32);
        _image.fill(QColor(0, 0, 0, 0).rgba());
        const int _count = _rule.bitmap.BTM.count();
        qint32 * _pixels = reinterpret_cast<qint32 *>(_image.bits());
        for (int _i = 0; _i < _count; _i++)
        {
            _pixels[_i] = _cmap[_rule.bitmap.BTM[_i]];
        }
        return (_image);
    }
    void draw_element(QPainter & _p, QPolygonF & _poly, bool & _ci, bool & _fp, bool & _pm)
    {
        if (_poly.isEmpty())
        {
            return;
        }
        if (_fp)
        {
            _p.setBrush(_p.pen().color());
            _fp = false;
        }
        if (_ci)
        {
            if (_poly.count() > 1)
            {
                const double _r = _poly[1].x();
                _p.drawEllipse(_poly[0], _r, _r);
            }
            _ci = false;
            _pm = false;
        }
        else
        if (_pm)
        {
            _p.drawPolygon(_poly);
            _pm = false;
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
        int _max_width = 0, _width = 0;
        const double _extra = 10.0;
        const double _w = _rule.pos.symb.bnbox_w.SYHL * _scale_factor;
        const double _h = _rule.pos.symb.bnbox_h.SYVL * _scale_factor;
        const double _x = _rule.pos.symb.bnbox_x.SBXC * _scale_factor;
        const double _y = _rule.pos.symb.bnbox_y.SBXR * _scale_factor;
        QImage _image(_w + _extra, _h + _extra, QImage::Format_ARGB32);
        _image.fill(QColor(0, 0, 0, 0).rgba());
        QPainter _p(& _image);
        QPolygonF _poly;
        bool _ci = false, _fp = false, _pm = false;
        foreach (const QByteArray & _par, _rule.vector.VCT.split(';'))
        {
            const QByteArray _cmd = _par.left(2).toUpper();
            if (_cmd == "SP")
            {
                draw_element(_p, _poly, _ci, _fp, _pm);
                _p.setPen(QColor(_cmap[_par[2]]));
            }
            else
            if (_cmd == "SW")
            {
                _width = _par.mid(2, 1).toInt();
                _p.setPen(QPen(_p.pen().color(), _width));
                if (_width > _max_width)
                {
                    _max_width = _width;
                }
            }
            else
            if (_cmd == "PU")
            {
                draw_element(_p, _poly, _ci, _fp, _pm);
                QList<QByteArray> _pt = _par.mid(2).split(',');
                for (int _i = 0; _i < _pt.count(); _i +=2 )
                {
                    _poly.append(QPointF(_pt[_i].toInt() * _scale_factor - _x, _pt[_i + 1].toInt() * _scale_factor - _y));
                }
            }
            else
            if (_cmd == "PD")
            {
                QList<QByteArray> _pt = _par.mid(2).split(',');
                for (int _i = 0; _i < _pt.count(); _i +=2 )
                {
                    _poly.append(QPointF(_pt[_i].toInt() * _scale_factor - _x, _pt[_i + 1].toInt() * _scale_factor - _y));
                }
            }
            else
            if (_cmd == "CI")
            {
                _ci = true;
                _poly.append(QPointF(_par.mid(2).toInt() * _scale_factor, 0));
            }
            else
            if (_cmd == "FP")
            {
                _fp = true;
            }
            else
            if (_cmd == "PM")
            {
                _pm = (_par[2] == '2');
            }
        }
        draw_element(_p, _poly, _ci, _fp, _pm);
        return (_image.copy(0, 0, _w + 2 * _max_width, _h + 2 * _max_width));
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

S52_Symbol::S52_Symbol(): d(new Data())
{
}

S52_Symbol::~S52_Symbol()
{
    delete d;
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
    if (!d->symbols.keys().contains(QString(_scheme).toUpper()))
    {
        return (QStringList());
    }
    return (d->symbols[_scheme].keys());
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
