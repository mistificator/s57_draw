#include "s57_buildscene.h"
#include "ogrsf_frmts.h"
#include "s52_symbol.h"
#include "s52_conditional.h"
#include "s52_fixeditem.h"
#include "s52_utils.h"
#include <QGraphicsItem>
#include <QDir>
#include <QMultiMap>
#include <QStringList>
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QRegularExpression>

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define S57_SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#else
#define S57_SKIP_EMPTY_PARTS QString::SkipEmptyParts
#endif

#define DBG_PRINT
#ifdef DBG_PRINT
#include <QMessageBox>
#endif

double mercator_sretch = 1.0;

struct S57_BuildScene::Data
{
    QGraphicsScene * scene;
    QMap<QString, QPixmap> symbols;
    S52_Symbol s52;
    struct style_struct
    {
        style_struct(): ref_count(new int), priority(0)
        {
            pen = new QPen(Qt::NoPen);
            brush = new QBrush(Qt::NoBrush);
            attributes = new QStringList();
            name = new QString();
            condition = new QString();
            symbol = new QString();
            category = new QString();
            texts = new QStringList();
            * ref_count = 0;
        }
        style_struct(const style_struct & _other)
        {
            priority = _other.priority;
            pen = _other.pen;
            brush = _other.brush;
            attributes = _other.attributes;
            name = _other.name;
            condition = _other.condition;
            symbol = _other.symbol;
            category = _other.category;
            texts = _other.texts;
            ref_count = _other.ref_count;
            (* ref_count)++;
        }
        void release()
        {
            if (*ref_count == 0)
            {
                delete pen;
                delete brush;
                delete attributes;
                delete name;
                delete condition;
                delete symbol;
                delete category;
                delete texts;
                delete ref_count;
            }
            else
            {
                (*ref_count)--;
            }
        }
        style_struct & operator = (const style_struct & _other)
        {
            if (&_other == this)
            {
                return (*this);
            }
            release();
            priority = _other.priority;
            pen = _other.pen;
            brush = _other.brush;
            attributes = _other.attributes;
            name = _other.name;
            condition = _other.condition;
            symbol = _other.symbol;
            category = _other.category;
            texts = _other.texts;
            ref_count = _other.ref_count;
            (*ref_count)++;
            return (*this);
        }
        ~style_struct()
        {
            release();
        }
        style_struct deep_copy() const
        {
            style_struct _s;
            * _s.attributes =  * attributes;
            * _s.brush =       * brush;
            * _s.category =    * category;
            * _s.condition =   * condition;
            * _s.name =        * name;
            * _s.pen =         * pen;
            _s.priority =      priority;
            * _s.symbol =      * symbol;
            * _s.texts =       * texts;
            return (_s);
        }
        mutable int * ref_count;
        int priority;
        QPen * pen;
        QBrush * brush;
        QStringList * attributes;
        QString * name;
        QString * condition;
        QString * symbol;
        QString * category;
        QStringList * texts;
    };
    QMultiMap<QString, style_struct> p_style, l_style, a_style;
    QMap<qint64, QMap<QString, QGraphicsItemGroup *> > super_groups;
    QList<QGraphicsItem *> items;
    QStringList layers;
    double scale;
    bool render_enabled;
    bool br_visible;
    QColor br_color;
    struct RegisterS57_Helper
    {
        RegisterS57_Helper()
        {
            RegisterOGRS57();
        }
    };

    Data(const QString & _lookups_path = QString(), const QString & _symbols_path = QString()): scene(0), scale(0), render_enabled(true), br_visible(false)
    {
        static RegisterS57_Helper reg_s57;
        parse_styles(_lookups_path, _symbols_path);
    }
    ~Data()
    {
    }
#ifdef DBG_PRINT
    void debug_print_attr(const style_struct & _s, const QString & _if)
    {
        if (*_s.name != _if.toUpper())
        {
            return;
        }
        QString _str;
        foreach (const QString & _a, *_s.attributes)
        {
            _str += _a + " ";
        }
        QMessageBox::information(0, *_s.name, _str);
    }
    void debug_print_all(const QString & _if)
    {
        QString _str;
        QList<style_struct> _list;
        _list = p_style.values(_if);
        foreach (const style_struct & _s, _list)
        {
            _str += *_s.name + " " + *_s.category + "\n";
        }
        _list = l_style.values(_if);
        foreach (const style_struct & _s, _list)
        {
            _str += *_s.name + " " + *_s.category + "\n";
        }
        _list = a_style.values(_if);
        foreach (const style_struct & _s, _list)
        {
            _str += *_s.name + " " + *_s.category + "\n";
        }
        QMessageBox::information(0, _if, _str);
    }
#endif

    void apply_draw_style(const QString & _draw_str, style_struct * _s)
    {
        QStringList _draw = _draw_str.split(";", S57_SKIP_EMPTY_PARTS);
        for (int _i = 0; _i < _draw.count(); _i++)
        {
            const QString _raw_cmd = _draw[_i].trimmed();
            if (_raw_cmd.startsWith("TX(", Qt::CaseInsensitive) ||
                _raw_cmd.startsWith("TE(", Qt::CaseInsensitive))
            {
                if (!_s->texts->contains(_raw_cmd))
                {
                    _s->texts->append(_raw_cmd);
                }
                continue;
            }
            QStringList _tok = _raw_cmd.split(QRegularExpression("(\\W)"), S57_SKIP_EMPTY_PARTS);
            if (_tok.isEmpty())
            {
                continue;
            }
            const QString _cmd = _tok[0].toUpper();
            if (_cmd == "LS")
            {
                if (_tok.count() > 3)
                {
                    const QRgb _rgb = s52.color(_tok[3]);
                    QPen _pen(QColor(_rgb ? _rgb : qRgb(0, 0, 0)));
                    _pen.setCosmetic(true);
                    _pen.setWidth(qBound(1, _tok[2].toInt(), 2));
                    const QString _style = _tok[1].toUpper();
                    if (_style == "DASH")
                    {
                        _pen.setDashPattern(QVector<qreal>() << 6.0 << 4.0);
                    }
                    else if (_style == "DOTT")
                    {
                        _pen.setDashPattern(QVector<qreal>() << 1.5 << 3.5);
                    }
                    else
                    {
                        _pen.setStyle(Qt::SolidLine);
                    }
                    *_s->pen = _pen;
                }
            }
            else if (_cmd == "AC")
            {
                if (_tok.count() > 1)
                {
                    const QString _col_code = _tok[1].toUpper();
                    const QRgb _rgb = s52.color(_col_code);
                    if (_rgb != 0)
                    {
                        QColor _col(_rgb);
                        if (_tok.count() > 2)
                        {
                            const int _st = _tok[2].toInt();
                            if (_st > 0)
                            {
                                _col.setAlpha(qBound(0, 255 - _st * 64, 255));
                            }
                        }
                        *_s->brush = QBrush(_col);
                    }
                    if (_col_code == "DEPDW")      _s->priority = 1;
                    else if (_col_code == "DEPMD") _s->priority = 2;
                    else if (_col_code == "DEPMS") _s->priority = 3;
                    else if (_col_code == "DEPVS") _s->priority = 4;
                    else if (_col_code == "DEPIT") _s->priority = 5;
                }
            }
            else if (_cmd == "CS")
            {
                if (_tok.count() > 1)
                {
                    *_s->condition = QString(_tok[1]).toUpper();
                }
            }
            else if (_cmd == "SY")
            {
                if (_tok.count() > 1)
                {
                    const QString _symbol = QString(_tok[1]).toUpper();
                    if (!_s->symbol->contains(_symbol))
                    {
                        if (!_s->symbol->isEmpty())
                        {
                            _s->symbol->append(" ");
                        }
                        _s->symbol->append(_symbol);
                    }
                }
            }
            else if (_cmd == "AP")
            {
                // nothing to do
            }
            else if (_cmd == "LC")
            {
                if (_tok.count() > 1)
                {
                    const QString _lc_name = _tok[1].toUpper();
                    QRgb _rgb = s52.symbol_color(_lc_name);
                    if (_rgb == 0)
                    {
                        _rgb = s52.color("CHMGD");
                    }
                    QPen _pen(QColor(_rgb ? _rgb : qRgb(192, 0, 192)));
                    _pen.setCosmetic(true);
                    _pen.setWidth(1);
                    if (_lc_name.startsWith("LOWACC"))
                    {
                        _pen.setDashPattern(QVector<qreal>() << 2.0 << 3.0);
                    }
                    else if (!_lc_name.startsWith("HODATA"))
                    {
                        _pen.setDashPattern(QVector<qreal>() << 6.0 << 4.0);
                    }
                    *_s->pen = _pen;
                }
            }
        }
    }

    style_struct parse_lookup_line(QString _line, QMultiMap<QString, style_struct> * _style)
    {
        _line = _line.trimmed();
        if (_line.isEmpty() || _line[0] == '*')
        {
            return style_struct();
        }
        QStringList _p = _line.split("\",\"");
        if (_p.count() < 7)
        {
            return style_struct();
        }
        for (int _i = 0; _i < _p.count(); _i++)
        {
            _p[_i] = _p[_i].replace("\"", "").trimmed();
        }
        QStringList _attr = QString(_p[1]).replace(' ', "").split("|", S57_SKIP_EMPTY_PARTS);
        QString _key = QString(_p[0]).replace(' ', "").toUpper();
        style_struct * _s = 0;
        for (QMultiMap<QString, style_struct>::iterator _i = _style->begin(); _i != _style->end(); _i++)
        {
            if ((_i.key() == _key) && (*_i.value().attributes == _attr))
            {
                _s = & _i.value();
                break;
            }
        }
        if (!_s)
        {
            _s = & _style->insert(_key, style_struct()).value();
            *_s->attributes = _attr;
            *_s->name = _key;
        }
        _s->priority = _p[3].toInt();
        * _s->category = _p[5].toUpper();
        apply_draw_style(_p[2], _s);
        return (* _s);
    }

    void parse_lookup_file(const QString & _name, QMultiMap<QString, style_struct> & _style)
    {
        QFile _dic(_name);
        if (!_dic.open(QFile::ReadOnly | QFile::Text))
        {
            return;
        }
        while (!_dic.atEnd())
        {
            parse_lookup_line(_dic.readLine(), & _style);
        }
        _dic.close();
    }

    void parse_lookups(QString _lookups_path = QString())
    {
        QString _saved_current = QDir::currentPath();
        if (_lookups_path.isEmpty())
        {
			_lookups_path = QApplication::instance()->applicationDirPath() + "/s52/lookups";
        }
        QDir::setCurrent(_lookups_path + "/points");
        foreach(const QString & _name, QDir().entryList(QStringList() << "*.dic"))
        {
            parse_lookup_file(_name, p_style);
        }
        QDir::setCurrent(_lookups_path + "/lines");
        foreach(const QString & _name, QDir().entryList(QStringList() << "*.dic"))
        {
            parse_lookup_file(_name, l_style);
        }
        QDir::setCurrent(_lookups_path + "/areas");
        foreach(const QString & _name, QDir().entryList(QStringList() << "*.dic"))
        {
            parse_lookup_file(_name, a_style);
        }
        QDir::setCurrent(_saved_current);
    }

    void parse_styles(QString _lookups_path = QString(), QString _symbols_path = QString())
    {
        if (_symbols_path.isEmpty())
        {
            _symbols_path = qApp->applicationDirPath() + "/s52/symbols";
        }
        s52.setSources(QStringList() << _symbols_path + "/S52RAZDS.RLE");
        symbols = s52.pixmaps();
        parse_lookups(_lookups_path);
    }

    QVariant get_attribute(OGRFeature * _feat, const QString & _attr_name)
    {
        if (_feat == 0)
        {
            return QVariant();
        }
        int _idx = _feat->GetFieldIndex(_attr_name.toUpper().toLocal8Bit());
        if (_idx < 0 || !_feat->IsFieldSet(_idx))
        {
            return QVariant();
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

    style_struct apply_cs(OGRFeature * _feat, const QString & _layer_name, const style_struct & _s, int _collection_idx = 0)
    {
        if (!_s.condition->isEmpty())
        {
            QString _translated_cs = S52_CS::translate(_layer_name, * _s.condition, _feat, _collection_idx);
            if (_translated_cs.isEmpty())
            {
                qDebug() << "no conditional symbology for" << _layer_name << * _s.condition;
            }
            else
            {
                //                qDebug() << "prepare to apply" << _translated_cs;
                style_struct _s1 = _s.deep_copy();
                _s1.condition->clear();
                apply_draw_style(_translated_cs, & _s1);
                return (_s1);
            }
        }
        return (_s);
    }

    style_struct apply_rule(OGRFeature * _feat, const QString & _layer_name, int _collection_idx = 0)
    {
        QList<style_struct> _list;
        switch (wkbFlatten(_feat->GetGeometryRef()->getGeometryType()))
        {
        case wkbPoint:
        case wkbMultiPoint:
            _list = p_style.values(_layer_name);
            break;
        case wkbLineString:
        case wkbMultiLineString:
            _list = l_style.values(_layer_name);
            break;
        case wkbPolygon:
        case wkbMultiPolygon:
            _list = a_style.values(_layer_name);
            break;
        default:
            return (style_struct());
        }
        if (_list.isEmpty())
        {
            return (style_struct());
        }
        foreach (const style_struct & _s, _list)
        {
            bool _ok = true;
            foreach (const QString & _attr_name, *_s.attributes)
            {
                const QString _attr_code = _attr_name.left(6);
                const QString _expected_val = _attr_name.mid(6);
                const QVariant _attr_val = get_attribute(_feat, _attr_code);
                if (!_attr_val.isValid())
                {
                    _ok = false;
                    break;
                }
                if (!_expected_val.isEmpty() && _expected_val != "?")
                {
                    if (_attr_val.toInt() != _expected_val.toInt())
                    {
                        _ok = false;
                        break;
                    }
                }
            }
            if (_ok)
            {
                return (apply_cs(_feat, _layer_name, _s, _collection_idx));
            }
        }
        foreach (const style_struct & _s, _list)
        {
            if (_s.attributes->isEmpty())
            {
                continue;
            }
            bool _ok = true;
            foreach (const QString & _attr_name, *_s.attributes)
            {
                const QString _attr_code = _attr_name.left(6);
                const QString _expected_val = _attr_name.mid(6);
                const QVariant _attr_val = get_attribute(_feat, _attr_code);
                if (!_attr_val.isValid())
                {
                    _ok = false;
                    break;
                }
                if (!_expected_val.isEmpty() && _expected_val != "?")
                {
                    if (_attr_val.toInt() != _expected_val.toInt())
                    {
                        _ok = false;
                        break;
                    }
                }
            }
            if (_ok)
            {
                return (apply_cs(_feat, _layer_name, _s, _collection_idx));
            }
        }
        return (style_struct());
    }

    QPixmap * check_complex_symbol(const QString & _complex_symbol)
    {
        if (symbols.contains(_complex_symbol))
        {
            return (& symbols[_complex_symbol]);
        }
        QList<const QPixmap *> _list;
        int _w = 0, _h = 0;
        foreach (const QString & _symbol, _complex_symbol.split(" ", S57_SKIP_EMPTY_PARTS))
        {
            if (symbols.contains(_symbol))
            {
                const QPixmap & _px_ref = symbols[_symbol];
                if (!_px_ref.isNull())
                {
                    _w = qMax<int>(_w, _px_ref.width());
                    _h = qMax<int>(_h, _px_ref.height());
                    _list << & _px_ref;
                }
            }
        }
        if (_w <= 0 || _h <= 0)
        {
            symbols[_complex_symbol] = QPixmap();
            return (& symbols[_complex_symbol]);
        }
        QPixmap _px(_w, _h);
        _px.fill(QColor(0, 0, 0, 0));
        QPainter _painter(& _px);
        foreach (const QPixmap * _px_ptr, _list)
        {
            _painter.drawPixmap((_w - _px_ptr->width()) / 2,
                                (_h - _px_ptr->height()) / 2,
                                _px_ptr->width(),
                                _px_ptr->height(),
                                * _px_ptr);
        }
        symbols[_complex_symbol] = _px;
        return (& symbols[_complex_symbol]);
    }

    QGraphicsItem * addToGroup(qint64 _scamin, QGraphicsItem * _item, QMap<qint64, QGraphicsItemGroup *> & groups)
    {
        if (_scamin <= 0)
        {
            _scamin = 0x7fffffff;
        }
        _item->setData(S57_BuildScene::ItemScaminKey, _scamin);
        QGraphicsItemGroup * & _g = groups[0];
        if (!_g)
        {
            _g = new QGraphicsItemGroup();
            _g->setZValue(0);
            _g->setFlag(QGraphicsItem::ItemHasNoContents);
        }
        _item->setGroup(_g);
        return (_g);
    }

    void addTextItems(OGRFeature * _feat, const style_struct & _s, const QPointF & _pos,
                      qint64 _scamin, qint64 _z_offset, const QString & _tooltip,
                      const QString & _layer_name, const QString & _map_name,
                      QMap<qint64, QGraphicsItemGroup *> & groups)
    {
        if (_s.texts->isEmpty() || !S52_getMarinerParam(S52_MAR_SHOW_TEXT))
        {
            return;
        }
        const bool _is_geo_name = (_layer_name == "LNDARE" || _layer_name == "LNDRGN" ||
                                   _layer_name == "SEAARE" || _layer_name == "BUAARE");
        qint64 _text_scamin = _scamin;
        if (_text_scamin <= 0 || _text_scamin >= 0x7fffffff)
        {
            if (_is_geo_name)
            {
                _text_scamin = (_z_offset > 0) ? (_z_offset * 3) : 0x7fffffff;
            }
            else
            {
                _text_scamin = (_z_offset > 0) ? _z_offset : 500000;
            }
        }

        foreach (const QString & _cmd, *_s.texts)
        {
            const bool _is_te = _cmd.startsWith("TE(", Qt::CaseInsensitive);
            const int _open = _cmd.indexOf('(');
            const int _close = _cmd.lastIndexOf(')');
            if (_open < 0 || _close <= _open)
            {
                continue;
            }
            const QString _inner = _cmd.mid(_open + 1, _close - _open - 1);
            QStringList _args;
            QString _cur;
            bool _in_quotes = false;
            for (int _ci = 0; _ci < _inner.length(); ++_ci)
            {
                const QChar _ch = _inner[_ci];
                if (_ch == '\'')
                {
                    _in_quotes = !_in_quotes;
                    _cur.append(_ch);
                }
                else if (_ch == ',' && !_in_quotes)
                {
                    _args.append(_cur.trimmed());
                    _cur.clear();
                }
                else
                {
                    _cur.append(_ch);
                }
            }
            _args.append(_cur.trimmed());

            QString _text;
            int _base_idx = 0;
            if (_is_te)
            {
                if (_args.count() < 9)
                {
                    continue;
                }
                QString _fmt = _args[0];
                if (_fmt.startsWith('\'') && _fmt.endsWith('\''))
                {
                    _fmt = _fmt.mid(1, _fmt.length() - 2);
                }
                if (_fmt.startsWith("by ", Qt::CaseInsensitive) ||
                    _fmt.startsWith("bn ", Qt::CaseInsensitive))
                {
                    continue;
                }
                QString _attr_str = _args[1];
                if (_attr_str.startsWith('\'') && _attr_str.endsWith('\''))
                {
                    _attr_str = _attr_str.mid(1, _attr_str.length() - 2);
                }
                const QStringList _attr_names = _attr_str.split(',', S57_SKIP_EMPTY_PARTS);
                if (_attr_names.isEmpty())
                {
                    continue;
                }
                const QString _first_attr = _attr_names[0].trimmed();
                const QVariant _val = get_attribute(_feat, _first_attr);
                if (!_val.isValid() || _val.toString().isEmpty())
                {
                    continue;
                }
                QRegularExpression _re("%[^a-zA-Z]*([diouxXfFeEgGaAs])");
                QRegularExpressionMatch _m = _re.match(_fmt);
                const QChar _spec = _m.hasMatch() ? _m.captured(1)[0].toLower() : QChar();
                if (_spec == 'f' || _spec == 'g' || _spec == 'e' || _spec == 'a')
                {
                    _text = QString::asprintf(_fmt.toUtf8().constData(), _val.toDouble());
                }
                else if (_spec == 'd' || _spec == 'i' || _spec == 'u' || _spec == 'x' || _spec == 'o')
                {
                    _text = QString::asprintf(_fmt.toUtf8().constData(), _val.toInt());
                }
                else if (_spec == 's')
                {
                    _text = QString::asprintf(_fmt.toUtf8().constData(), _val.toString().toUtf8().constData());
                }
                else
                {
                    _text = _val.toString();
                }
                _base_idx = 2;
            }
            else
            {
                if (_args.count() < 8)
                {
                    continue;
                }
                const QString _atisp = _args[0];
                if (_atisp.startsWith('\'') && _atisp.endsWith('\''))
                {
                    _text = _atisp.mid(1, _atisp.length() - 2);
                }
                else
                {
                    const QVariant _val = get_attribute(_feat, _atisp);
                    if (!_val.isValid())
                    {
                        continue;
                    }
                    _text = _val.toString().trimmed();
                }
                _base_idx = 1;
            }

            if (_text.isEmpty())
            {
                continue;
            }

            const int _hjust = _args[_base_idx].toInt();
            const int _vjust = _args[_base_idx + 1].toInt();
            QString _chars = _args[_base_idx + 3];
            _chars.remove('\'');
            const double _xoffs = _args[_base_idx + 4].toDouble();
            const double _yoffs = _args[_base_idx + 5].toDouble();
            const QString _col_name = _args[_base_idx + 6];

            int _pt_size = (_chars.length() >= 5) ? _chars.mid(3, 2).toInt() : 10;
            if (_pt_size <= 0) _pt_size = 10;
            const bool _bold = (_chars.length() >= 2 && _chars[1].digitValue() >= 6);

            QFont _font("Arial", _pt_size, _bold ? QFont::Bold : QFont::Normal);
            QFontMetrics _fm(_font);
            const QRect _br = _fm.boundingRect(_text);
            const int _tw = qMax(1, _br.width() + 4);
            const int _th = qMax(1, _fm.height());
            const double _char_w = _fm.averageCharWidth();
            const double _char_h = _fm.capHeight() > 0 ? _fm.capHeight() : _fm.ascent();

            double _tx = _xoffs * _char_w;
            double _ty = _yoffs * _char_h;
            if (_hjust == 1)      _tx -= _tw / 2.0;
            else if (_hjust == 2) _tx -= _tw;
            if (_vjust == 1)      _ty -= _th;
            else if (_vjust == 2) _ty -= _th / 2.0;

            const QString _cache_key = QString("TXT_%1_%2_%3_%4_%5_%6_%7")
                                           .arg(_text).arg(_hjust).arg(_vjust).arg(_chars).arg(_xoffs).arg(_yoffs).arg(_col_name);

            if (!symbols.contains(_cache_key))
            {
                const int _half_w = qCeil(qMax(qAbs(_tx), qAbs(_tx + _tw))) + 2;
                const int _half_h = qCeil(qMax(qAbs(_ty), qAbs(_ty + _th))) + 2;

                QPixmap _px(2 * _half_w, 2 * _half_h);
                _px.fill(QColor(0, 0, 0, 0));
                QPainter _p(&_px);
                _p.setFont(_font);
                const QRgb _rgb = s52.color(_col_name);
                _p.setPen(QColor(_rgb ? _rgb : qRgb(0, 0, 0)));
                _p.drawText(QRectF(_half_w + _tx, _half_h + _ty, _tw, _th),
                            Qt::AlignCenter | Qt::TextDontClip, _text);
                _p.end();
                symbols[_cache_key] = _px;
            }

            QPixmap * _px_ptr = &symbols[_cache_key];
            QFixedPixmapItem * _t_item = new QFixedPixmapItem();
            addToGroup(_text_scamin, _t_item, groups);
            _t_item->setPos(_pos);
            _t_item->setScreenSize(_px_ptr->size());
            _t_item->setPixmap(_px_ptr);
            _t_item->setZValue((_is_geo_name ? 35 : 30) + _s.priority);
            _t_item->setToolTip(_tooltip);
            _t_item->setData(S57_BuildScene::ItemLayerNameKey, _layer_name);
            _t_item->setData(S57_BuildScene::ItemMapNameKey, _map_name);
            _t_item->setData(S57_BuildScene::ItemIsTextKey, true);
            _t_item->setData(S57_BuildScene::ItemTextRectKey, QRectF(_tx, _ty, _tw, _th));
        }
    }

    S57_BuildScene::MetaInfo build_s57(OGRDataSource * _ds)
    {
         S57_BuildScene::MetaInfo info;
//        qDebug() << "We're in build_s57!";
        if (!_ds)
        {
            return info;
        }
        info = metaInfo(_ds);
        QMap<qint64, QGraphicsItemGroup *> groups;
        const QString _map_name = QFileInfo(_ds->GetName()).absoluteFilePath();
        qDebug() << "Datasource has" << _ds->GetLayerCount() << "layers";
        for (int _idx = 0; _idx < _ds->GetLayerCount(); _idx++)
        {
            OGRLayer * _layer = _ds->GetLayer(_idx);
            _layer->ResetReading();
            QString _layer_name = QString(_layer->GetLayerDefn()->GetName()).toUpper();
            if (_layer_name.startsWith("M_") || _layer_name == "DSID")
            {
                continue;
            }
            if (!layers.contains(_layer_name))
            {
                layers << _layer_name;
            }
            qDebug() << _idx << _layer_name;
            bool _ignore_group = (_layer_name == "DEPARE") || (_layer_name == "LNDARE") ||
                                 (_layer_name == "DRGARE") || (_layer_name == "LAKARE") ||
                                 (_layer_name == "COALNE");
            QGraphicsItem * _item;
            while (OGRFeature * _feat = _layer->GetNextFeature())
            {
                if (OGRGeometry * _geom = _feat->GetGeometryRef())
                {
                    OGRPoint _p1, _p2, pt;
                    OGRPoint * _p;
                    OGRLineString * _l;
                    QPolygonF _poly;
                    OGRLinearRing * _r;
                    QPixmap * _pixmap;
                    QPainterPath path;
                    QVector<QPolygonF> _mpoly;
                    double _sum_x, _sum_y;
                    int _sum_count;
                    const int _wkb = wkbFlatten(_geom->getGeometryType());
                    style_struct _s = apply_rule(_feat, _layer_name);
                    double _z_prio = _s.priority;
                    if (_layer_name == "DEPARE" || _layer_name == "DRGARE")
                    {
                        const double _d1 = get_attribute(_feat, "DRVAL1").toDouble();
                        _z_prio = 1.0 + qBound(0.0, (1000.0 - _d1) / 2000.0, 0.9);
                    }
                    else if (_layer_name == "LNDARE")
                    {
                        _z_prio = qMax<double>(_s.priority, 2.0);
                    }
                    const qint64 _raw_scamin = get_attribute(_feat, "SCAMIN").toLongLong();
                    const qint64 _scamin = _ignore_group ? 0 : _raw_scamin;
                    const QString _objnam = get_attribute(_feat, "OBJNAM").toString();
                    const QString _tooltip = _layer_name +
                                             (_objnam.isEmpty() ? QString() : ("\n" + _objnam)) +
                                             "\nSCAMIN " + QString::number(_scamin) +
                                             "\nCSCL " + QString::number(info.scale);
                    if ((((_scamin >= 0 && _scamin < 0x7fffffff) || _ignore_group) &&
                         ((*_s.pen != Qt::NoPen) || (*_s.brush != Qt::NoBrush) || (!_s.symbol->isNull()) || (!_s.texts->isEmpty()))))
                    {
                        _item = 0;
                        switch (_wkb)
                        {
                        case wkbPoint:
                        {
                            if (!_s.symbol->isEmpty())
                            {
                                _item = new QFixedPixmapItem();
                                addToGroup(_scamin, _item, groups);
                                _item->setPos(((OGRPoint *)_geom)->getX(), -((OGRPoint *)_geom)->getY()*mercator_sretch);
                                _pixmap = check_complex_symbol(* _s.symbol);
                                ((QFixedPixmapItem *)_item)->setScreenSize(_pixmap->size());
                                ((QFixedPixmapItem *)_item)->setPixmap(_pixmap);
                                _item->setZValue(20.0 + _z_prio);
								_item->setToolTip(_tooltip);
                            }
                            addTextItems(_feat, _s, QPointF(((OGRPoint *)_geom)->getX(), -((OGRPoint *)_geom)->getY()*mercator_sretch), _scamin, info.scale, _tooltip, _layer_name, _map_name, groups);
                            break;
                        }
                        case wkbMultiPoint:
                        {
                            const int _num_geoms = ((OGRMultiPoint *)_geom)->getNumGeometries();
                            for (int _idx = 0; _idx < _num_geoms; _idx++)
                            {
                                if (!_s.symbol->isEmpty())
                                {
                                    _p = (OGRPoint *)((OGRMultiPoint *)_geom)->getGeometryRef(_idx);
                                    _item = new QFixedPixmapItem();
                                    addToGroup(_scamin, _item, groups);
                                    _item->setPos(_p->getX(), -_p->getY()*mercator_sretch);
                                    _pixmap = check_complex_symbol(* _s.symbol);
                                    ((QFixedPixmapItem *)_item)->setScreenSize(_pixmap->size());
                                    ((QFixedPixmapItem *)_item)->setPixmap(_pixmap);
                                    _item->setZValue(20.0 + _z_prio);
                                    _item->setToolTip(_tooltip);
                                }
                                addTextItems(_feat, _s, QPointF(((OGRPoint *)((OGRMultiPoint *)_geom)->getGeometryRef(_idx))->getX(), -((OGRPoint *)((OGRMultiPoint *)_geom)->getGeometryRef(_idx))->getY()*mercator_sretch), _scamin, info.scale, _tooltip, _layer_name, _map_name, groups);
                                if (_idx + 1 < _num_geoms)
                                {
                                    _s = apply_rule(_feat, _layer_name, _idx + 1);
                                }
                            }
                            break;
                        }
                        case wkbLineString:
                        {
                            _l = ((OGRLineString *)_geom);
                            _l->StartPoint(&_p1);
                            _l->EndPoint(& _p2);
							path = QPainterPath();
							_l->getPoint(0, & pt);
							path.moveTo(pt.getX(), -pt.getY()*mercator_sretch);
							for (int p_i = 1; p_i < _l->getNumPoints(); p_i++)
							{
								_l->getPoint(p_i, & pt);
								path.lineTo(pt.getX(), -pt.getY()*mercator_sretch);
							}
                            _item = new QGraphicsPathItem(path);
                            addToGroup(_scamin, _item, groups);
                            ((QGraphicsPathItem *)_item)->setPen(*_s.pen);
                            _item->setZValue(10.0 + _z_prio);
							_item->setToolTip(_tooltip);
                            if (!_s.symbol->isEmpty())
                            {
                                _item = new QFixedPixmapItem();
                                addToGroup(_scamin, _item, groups);
                                _item->setPos(path.pointAtPercent(0.5));
                                _pixmap = check_complex_symbol(* _s.symbol);
                                ((QFixedPixmapItem *)_item)->setScreenSize(_pixmap->size());
                                ((QFixedPixmapItem *)_item)->setPixmap(_pixmap);
                                _item->setZValue(20.0 + _z_prio);
								_item->setToolTip(_tooltip);
                            }
                            addTextItems(_feat, _s, path.pointAtPercent(0.5), _scamin, info.scale, _tooltip, _layer_name, _map_name, groups);
                            break;
                        }
                        case wkbMultiLineString:
                        {
                            const int _num_geoms = ((OGRMultiLineString *)_geom)->getNumGeometries();
                            for (int _idx = 0; _idx < _num_geoms; _idx++)
                            {
                                _l = (OGRLineString *)((OGRMultiLineString *)_geom)->getGeometryRef(_idx);
                                _l->StartPoint(&_p1);
                                _l->EndPoint(& _p2);
								path = QPainterPath();
								_l->getPoint(0, & pt);
								path.moveTo(pt.getX(), -pt.getY()*mercator_sretch);
								for (int p_i = 1; p_i < _l->getNumPoints(); p_i++)
								{
									_l->getPoint(p_i, & pt);
									path.lineTo(pt.getX(), -pt.getY()*mercator_sretch);
								}
                                _item = new QGraphicsPathItem(path);
                                addToGroup(_scamin, _item, groups);
                                ((QGraphicsPathItem *)_item)->setPen(*_s.pen);
                                _item->setZValue(10.0 + _z_prio);
								_item->setToolTip(_tooltip);
                                if (!_s.symbol->isEmpty())
                                {
                                    _item = new QFixedPixmapItem();
                                    addToGroup(_scamin, _item, groups);
                                    _item->setPos(path.pointAtPercent(0.5));
                                    _pixmap = check_complex_symbol(* _s.symbol);
                                    ((QFixedPixmapItem *)_item)->setScreenSize(_pixmap->size());
                                    ((QFixedPixmapItem *)_item)->setPixmap(_pixmap);
                                   _item->setZValue(20.0 + _z_prio);
									_item->setToolTip(_tooltip);
                                }
                                addTextItems(_feat, _s, path.pointAtPercent(0.5), _scamin, info.scale, _tooltip, _layer_name, _map_name, groups);
                                if (_idx + 1 < _num_geoms)
                                {
                                    _s = apply_rule(_feat, _layer_name, _idx + 1);
                                }
                            }
                            break;
                        }
                        case wkbPolygon:
                        {
                            _poly.clear();
                            OGRPolygon * _ogr_poly = (OGRPolygon *)_geom;
                            _r = _ogr_poly->getExteriorRing();
                            if (!_r || _r->getNumPoints() < 3)
                            {
                                break;
                            }
                            _sum_x = 0; _sum_y = 0; _sum_count = 0;
                            for (int _idx = 0; _idx < _r->getNumPoints(); _idx++)
                            {
                                _r->getPoint(_idx, & _p1);
                                _poly.append(QPointF(_p1.getX(), -_p1.getY()*mercator_sretch));
                                _sum_x += _p1.getX();
                                _sum_y += _p1.getY()*mercator_sretch;
                                _sum_count++;
                            }
                            // _poly.append(_poly[0]);
                            if (_layer_name == "DEPARE" && _ogr_poly->getNumInteriorRings() > 0)
                            {
                                for (int _ir = 0; _ir < _ogr_poly->getNumInteriorRings(); ++_ir)
                                {
                                    OGRLinearRing * _inner = _ogr_poly->getInteriorRing(_ir);
                                    if (!_inner || _inner->getNumPoints() < 3) continue;
                                    QPolygonF _hole;
                                    for (int _p_idx = 0; _p_idx < _inner->getNumPoints(); ++_p_idx)
                                    {
                                        _inner->getPoint(_p_idx, &_p1);
                                        _hole.append(QPointF(_p1.getX(), -_p1.getY()*mercator_sretch));
                                    }
                                    _poly = _poly.subtracted(_hole);
                                }
                            }
                            _item = new QGraphicsPolygonItem(_poly);
                            addToGroup(_scamin, _item, groups);
                            ((QGraphicsPolygonItem *)_item)->setPen(*_s.pen);
                            ((QGraphicsPolygonItem *)_item)->setBrush(*_s.brush);
                            _item->setZValue(_z_prio);
                            _item->setToolTip(_tooltip);
                            if (!_s.symbol->isEmpty())
                            {
                                _item = new QFixedPixmapItem();
                                addToGroup(_scamin, _item, groups);
                                _item->setPos(_sum_x / _sum_count, -_sum_y / _sum_count);
                                _pixmap = check_complex_symbol(* _s.symbol);
                                ((QFixedPixmapItem *)_item)->setScreenSize(_pixmap->size());
                                ((QFixedPixmapItem *)_item)->setPixmap(_pixmap);
                                _item->setZValue(20.0 + _z_prio);
                                _item->setToolTip(_tooltip);
                            }
                            if (_sum_count > 0)
                            {
                                addTextItems(_feat, _s, QPointF(_sum_x / _sum_count, -_sum_y / _sum_count), _scamin, info.scale, _tooltip, _layer_name, _map_name, groups);
                            }
                            break;
                        }
                        case wkbMultiPolygon:
                        {
                            _sum_x = 0; _sum_y = 0; _sum_count = 0;
							_mpoly.clear();
							_mpoly.resize(((OGRMultiPolygon *)_geom)->getNumGeometries());
                            for (int _idx = 0; _idx < ((OGRMultiPolygon *)_geom)->getNumGeometries(); _idx++)
                            {
								_poly.clear();
                                _r = ((OGRPolygon *)((OGRMultiPolygon *)_geom)->getGeometryRef(_idx))->getExteriorRing();
                                for (int _sub_idx = 0; _sub_idx < _r->getNumPoints(); _sub_idx++)
                                {
                                    _r->getPoint(_sub_idx, & _p1);
                                    _poly.append(QPointF(_p1.getX(), -_p1.getY()*mercator_sretch));
                                    _sum_x += _p1.getX();
                                    _sum_y += _p1.getY()*mercator_sretch;
									_sum_count++;
                                }
								_mpoly[_idx] = _poly;
							}
							int _contigous = 0;
							const int _mpcount = _mpoly.count();
							for (int _idx = 0; _idx < _mpcount; _idx++)
							{
								if (_idx == _mpcount - 1 || _mpoly[_idx].back() != _mpoly[_idx + 1].front())
								{
									_poly.clear();
									for (int _mpi = _contigous; _mpi <= _idx; _mpi++)
									{
										_poly << _mpoly[_mpi];
									}
									_poly << _poly.front();
									_item = new QGraphicsPolygonItem(_poly);
                                    addToGroup(_scamin, _item, groups);
									((QGraphicsPolygonItem *)_item)->setPen(*_s.pen);
//									((QGraphicsPolygonItem *)_item)->setPen(QColor(Qt::red));								
									((QGraphicsPolygonItem *)_item)->setBrush(*_s.brush);
                                    _item->setZValue(_z_prio);
									_item->setToolTip(_tooltip);

									_contigous = _idx + 1;
								}
							}

                            if (!_s.symbol->isEmpty())
                            {
                                _item = new QFixedPixmapItem();
                                addToGroup(_scamin, _item, groups);
                                _item->setPos(_sum_x / _sum_count, -_sum_y / _sum_count);
                                _pixmap = check_complex_symbol(* _s.symbol);
                                ((QFixedPixmapItem *)_item)->setScreenSize(_pixmap->size());
                                ((QFixedPixmapItem *)_item)->setPixmap(_pixmap);
                                _item->setZValue(20.0 + _z_prio);
								_item->setToolTip(_tooltip);
                            }
                            if (_sum_count > 0)
                            {
                                addTextItems(_feat, _s, QPointF(_sum_x / _sum_count, -_sum_y / _sum_count), _scamin, info.scale, _tooltip, _layer_name, _map_name, groups);
                            }
                            _s = apply_rule(_feat, _layer_name/*, _idx*/);                           
                            break;
                        }
                        default:
                            continue;
                            break;
                        }
                        if (_item)
                        {
                            _item->setData(S57_BuildScene::ItemLayerNameKey, _layer_name);
                            _item->setData(S57_BuildScene::ItemMapNameKey, _map_name);
                        }
                    }
                }
				if (_feat != 0)
				{
	                OGRFeature::DestroyFeature(_feat);
				}
            }
        }
        qint64 _z_offset = info.scale;
        if (_z_offset <= 0)
        {
            _z_offset = 10000000;
        }
        QGraphicsItemGroup * & _g = super_groups[_z_offset][info.name];
        if (!_g)
        {
            _g = new QGraphicsItemGroup();
            _g->setZValue(-_z_offset);
            _g->setFlag(QGraphicsItem::ItemHasNoContents);
        }
        if (br_visible)
        {
            QPen br_pen(br_color, 0);
            QGraphicsRectItem * br = new QGraphicsRectItem(info.boundingRect);
            br->setPen(br_pen);
            items << br;
        }
        foreach (QGraphicsItemGroup * _group, groups.values())
        {
            _group->setGroup(_g);
        }
        if (!_g->childItems().isEmpty())
        {
            items << _g;
        }
        else
        {
            delete _g;
            super_groups[_z_offset].remove(info.name);
            if (super_groups[_z_offset].isEmpty())
            {
                super_groups.remove(_z_offset);
            }
        }
        return info;
    }
    S57_BuildScene::MetaInfo metaInfo(OGRDataSource * _ds)
    {
//        qDebug() << "We're in build_s57!";
        S57_BuildScene::MetaInfo info;
        if (!_ds)
        {
            return info;
        }
        info.name = _ds->GetName();
        qint64 _z_offset = 0;
        if (OGRLayer * _dsid = _ds->GetLayerByName("DSID"))
        {
            _dsid->ResetReading();
            if (OGRFeature * _f = _dsid->GetNextFeature())
            {
                _z_offset = get_attribute(_f, "DSPM_CSCL").toLongLong();
                OGRFeature::DestroyFeature(_f);
            }
        }
        if (_z_offset == 0)
        {
            if (OGRLayer * _mcscl = _ds->GetLayerByName("M_CSCL"))
            {
                _mcscl->ResetReading();
                if (OGRFeature * _f = _mcscl->GetNextFeature())
                {
                    _z_offset = get_attribute(_f, "CSCALE").toLongLong();
                    OGRFeature::DestroyFeature(_f);
                }
            }
        }
        info.scale = _z_offset;
        if (OGRLayer * _layer = _ds->GetLayerByName("M_NSYS"))
        {
            _layer->ResetReading();
            while (OGRFeature * _feat = _layer->GetNextFeature())
            {
                if (OGRGeometry * _geom = _feat->GetGeometryRef())
                {
                    if (wkbFlatten(_geom->getGeometryType()) == wkbPolygon)
                    {
                        OGRLinearRing * _r = ((OGRPolygon *)_geom)->getExteriorRing();
                        if (_r)
                        {
                            OGRPoint _p1;
                            QPolygonF _poly;
                            for (int _idx = 0; _idx < _r->getNumPoints(); _idx++)
                            {
                                _r->getPoint(_idx, &_p1);
                                _poly.append(QPointF(_p1.getX(), -_p1.getY()*mercator_sretch));
                            }
                            info.boundingRect = info.boundingRect.isNull()
                                                    ? _poly.boundingRect()
                                                    : info.boundingRect.united(_poly.boundingRect());
                            qDebug() << "M_NSYS" << info.boundingRect;
                        }
                    }
                }
                OGRFeature::DestroyFeature(_feat);
            }
        }
        return info;
    }
    static QGraphicsItem * topParent(QGraphicsItem * _item)
    {
        QGraphicsItem * _parent = _item;
        while (_parent->parentItem())
        {
            _parent = _parent->parentItem();
        }
        return _parent;
    }

    void updateVisibility(double _scale, bool _exact)
    {
        scale = _scale;
        if (!scene || super_groups.isEmpty())
        {
            return;
        }
        const qint64 _i_scale = static_cast<qint64>(_scale);
        QList<QGraphicsItem *> _visible_texts;

        const qint64 _max_cscl = super_groups.lastKey();
        for (QMap<qint64, QMap<QString, QGraphicsItemGroup *> >::iterator _i = super_groups.begin(); _i != super_groups.end(); ++_i)
        {
            const qint64 _map_cscl = _i.key();
            const bool _map_vis = render_enabled && (_map_cscl == _max_cscl || _i_scale <= _map_cscl * 4);
            foreach (QGraphicsItemGroup * _g, _i.value())
            {
                _g->setVisible(_map_vis);
                if (!_map_vis)
                {
                    continue;
                }
                foreach (QGraphicsItem * _group_item, _g->childItems())
                {
                    _group_item->setVisible(true);
                    foreach (QGraphicsItem * _leaf, _group_item->childItems())
                    {
                        const qint64 _item_scamin = _leaf->data(S57_BuildScene::ItemScaminKey).toLongLong();
                        const bool _vis = _exact ? (_item_scamin == _i_scale) : (_i_scale <= _item_scamin);
                        _leaf->setVisible(_vis);
                        if (_vis && !_exact && _leaf->data(S57_BuildScene::ItemIsTextKey).toBool())
                        {
                            _visible_texts.append(_leaf);
                        }
                    }
                }
            }
        }

        if (!_visible_texts.isEmpty() && !scene->views().isEmpty())
        {
            QGraphicsView * _view = scene->views().first();
            const double _m11 = _view->transform().m11();
            const double _m22 = _view->transform().m22();
            if (_m11 > 0.0 && _m22 > 0.0)
            {
                std::sort(_visible_texts.begin(), _visible_texts.end(),
                          [](const QGraphicsItem * a, const QGraphicsItem * b) {
                              return a->zValue() > b->zValue();
                          });
                QVector<QRectF> _occupied;
                _occupied.reserve(_visible_texts.size());
                foreach (QGraphicsItem * _t_item, _visible_texts)
                {
                    const QPointF _px_pos(_t_item->pos().x() * _m11, _t_item->pos().y() * _m22);
                    const QRectF _box = _t_item->data(S57_BuildScene::ItemTextRectKey).toRectF()
                                            .translated(_px_pos)
                                            .adjusted(-2.0, -2.0, 2.0, 2.0);
                    bool _collision = false;
                    for (int _k = 0; _k < _occupied.size(); ++_k)
                    {
                        if (_occupied[_k].intersects(_box))
                        {
                            _collision = true;
                            break;
                        }
                    }
                    if (_collision)
                    {
                        _t_item->setVisible(false);
                    }
                    else
                    {
                        _occupied.append(_box);
                    }
                }
            }
        }
    }
};

S57_BuildScene::S57_BuildScene(): d(new Data())
{

}

S57_BuildScene::S57_BuildScene(const QString & _lookups_path, const QString & _symbols_path): d(new Data(_lookups_path, _symbols_path))
{

}

S57_BuildScene::~S57_BuildScene()
{
    delete d;
}

void S57_BuildScene::setMercatorStretch(double ext_aspect)
{
	mercator_sretch = ext_aspect;
}

void S57_BuildScene::clear()
{
    if (d->scene)
    {
//		foreach (QGraphicsItem * _item, d->scene->items()) d->scene->removeItem(_item);
        foreach (QGraphicsItem * _item, d->items) d->scene->removeItem(_item);
//        d->scene->clear();
        d->scene->setSceneRect(QRectF());
    }
    d->super_groups.clear();
    d->items.clear();
    d->layers.clear();
}

void S57_BuildScene::setScene(QGraphicsScene * _scene)
{
    d->scene = _scene;
    if (d->scene)
    {
        QMetaObject::invokeMethod(d->scene, "registerBuildScene", Q_ARG(S57_BuildScene *, this));
        d->scene->setSceneRect(d->scene->itemsBoundingRect());
    }
}

QGraphicsScene * S57_BuildScene::scene() const
{
    return d->scene;
}

S57_BuildScene::MetaInfo S57_BuildScene::metaInfo(const QString & _file_name) const
{
    S57_BuildScene::MetaInfo info;
    QString _saved_current = QDir::currentPath();

    OGRDataSource * _ds = 0;

#if (GDAL_VERSION_MAJOR > 1)
    if (GDALDataset * _gds = (GDALDataset *)GDALOpenEx(_file_name.toLocal8Bit(), GDAL_OF_VECTOR | GDAL_OF_READONLY, 0, 0, 0))
    {
        _ds = dynamic_cast<OGRDataSource * >(_gds);
    }
#else
    _ds = OGRSFDriverRegistrar::Open(_file_name.toLocal8Bit());
#endif
    if (_ds)
    {
#if (GDAL_VERSION_MAJOR > 1)
        qDebug() << _file_name << "opened with" << _ds->GetDriverName();
#else
        qDebug() << _file_name << "opened with" << _ds->GetDriver()->GetName();
#endif
        info = d->metaInfo(_ds);
        delete _ds;
    }
    else
    {
        qDebug() << _file_name << "NOT opened";
    }
    QDir::setCurrent(_saved_current);

    return info;
}

S57_BuildScene::MetaInfo S57_BuildScene::build(const QString & _file_name)
{
    S57_BuildScene::MetaInfo info;

    QString _saved_current = QDir::currentPath();
    QDir::setCurrent(qApp->applicationDirPath() + "/s57/classes");

    OGRDataSource * _ds = 0;

#if (GDAL_VERSION_MAJOR > 1)
    if (GDALDataset * _gds = (GDALDataset *)GDALOpenEx(_file_name.toLocal8Bit(), GDAL_OF_VECTOR | GDAL_OF_READONLY, 0, 0, 0))
    {
        _ds = dynamic_cast<OGRDataSource * >(_gds);
    }
#else
    _ds = OGRSFDriverRegistrar::Open(_file_name.toLocal8Bit());
#endif
    if (_ds)
    {
#if (GDAL_VERSION_MAJOR > 1)
        qDebug() << _file_name << "opened with" << _ds->GetDriverName();
#else
        qDebug() << _file_name << "opened with" << _ds->GetDriver()->GetName();
#endif
        info = d->build_s57(_ds);
        delete _ds;
    }
    else
    {
        qDebug() << _file_name << "NOT opened";
    }
    QDir::setCurrent(_saved_current);

    return info;
}

void S57_BuildScene::unbuild(const QString & _file_name)
{
    const QString _fname_adj = QFileInfo(_file_name).absoluteFilePath();
    for (QMap<qint64, QMap<QString, QGraphicsItemGroup *> >::iterator _i = d->super_groups.begin(); _i != d->super_groups.end();)
    {
        for (QMap<QString, QGraphicsItemGroup *>::iterator _k = _i->begin(); _k != _i->end(); )
        {
            if (_k.key() == _fname_adj)
            {
                d->scene->removeItem(* _k);
                _k = _i->erase(_k);
            }
            else
            {
                _k++;
            }
        }
        if (_i->isEmpty())
        {
            _i = d->super_groups.erase(_i);
        }
        else
        {
            _i++;
        }
    }
    for (QList<QGraphicsItem *>::iterator i = d->items.begin(); i != d->items.end();)
    {
        if ((* i)->data(S57_BuildScene::ItemMapNameKey).toString() == _fname_adj)
        {
            d->scene->removeItem(* i);
            d->items.removeOne(* i);
            continue;
        }
        ++i;
    }
}

void S57_BuildScene::render()
{
    if (!d->scene)
    {
        return;
    }
    foreach (QGraphicsItem * _item, d->items)
    {
		if (_item->scene() == 0)
		{
	        d->scene->addItem(_item);
		}
	}
	fullView();
}

void S57_BuildScene::resetScale()
{
	if (!d->scene)
	{
		return;
	}
    foreach (QGraphicsView * view, d->scene->views())
    {
        view->resetTransform();
    }
}

void S57_BuildScene::fullView()
{
    if (!d->scene) return;
    foreach (QGraphicsView * view, d->scene->views())
    {
        view->resetTransform();
    }
    d->scene->setSceneRect(d->scene->itemsBoundingRect());
    if (d->scene->sceneRect().width() <= 0.0 || d->scene->sceneRect().height() <= 0.0)
    {
        return;
    }
    foreach (QGraphicsView * view, d->scene->views())
    {
        const double _init_scale = qMin<double>(
            view->viewport()->width() / d->scene->sceneRect().width(),
            view->viewport()->height() / d->scene->sceneRect().height());
        if (_init_scale > 0.0)
        {
            view->scale(_init_scale, _init_scale);
        }
    }
}

void S57_BuildScene::updateVisibility(double _scale, bool _exact)
{
    d->updateVisibility(_scale, _exact);
}

double S57_BuildScene::scale() const
{
    return d->scale;
}

int S57_BuildScene::itemsCount() const
{
    int _count = 0;
    foreach (QGraphicsItem * super_group_item, d->items)
    {
        foreach (QGraphicsItem * group_item, super_group_item->childItems())
        {
            _count += group_item->childItems().count();
        }
    }
    return (_count);
}

QList<QGraphicsItem *> S57_BuildScene::items() const
{
    QList<QGraphicsItem *> _items;
    foreach (QGraphicsItem * super_group_item, d->items)
    {
        foreach (QGraphicsItem * group_item, super_group_item->childItems())
        {
            _items << group_item->childItems();
        }
    }
    return _items;
}

QStringList S57_BuildScene::layers() const
{
    return d->layers;
}

void S57_BuildScene::setScaleTo(S57_BuildScene * _other)
{
    if (_other->d->scene && d->scene)
    {
        _other->d->updateVisibility(d->scale, false);
    }
}

S52_Symbol & S57_BuildScene::s52() const
{
    return d->s52;
}

double S57_BuildScene::scaleTo1(double _scale, double _dot_mm, double _km_in_deg)
{
    if (!d->scene || d->scene->views().isEmpty())
    {
        return 1.0;
    }
    QGraphicsView * view = d->scene->views().at(0);
    const int vp_h = view->viewport()->height();
    if (vp_h <= 0)
    {
        return 1.0;
    }
    const QPointF _top_left = view->mapToScene(0, 0);
    const QPointF _bottom_right = view->mapToScene(view->viewport()->width(), vp_h);

    const double dot_mm = (_dot_mm > 0.0)
                              ? _dot_mm
                              : (view->logicalDpiY() > 0 ? (25.4 / view->logicalDpiY()) : 0.25);

    const double _mm = vp_h * dot_mm;
    const double _degs = qAbs(_bottom_right.y() - _top_left.y());
    const double _new_degs = _scale * _mm / (_km_in_deg * 1000000.0);
    return _degs / _new_degs;
}

void S57_BuildScene::setRenderEnabled(bool state)
{
    d->render_enabled = state;
    updateVisibility(d->scale);
}

bool S57_BuildScene::isRenderEnabled() const
{
    return d->render_enabled;
}

void S57_BuildScene::setBoundingRectVisible(bool state, QColor c)
{
    d->br_visible = state;
    d->br_color = c;
}

bool S57_BuildScene::isBoundingRectVisible() const
{
    return d->br_visible;
}
