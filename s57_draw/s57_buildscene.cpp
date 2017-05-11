#include "s57_buildscene.h"
#include "ogrsf_frmts.h"
#include "s52_symbol.h"
#include "s52_conditional.h"
#include <QGraphicsItem>
#include <QDir>
#include <QMultiMap>
#include <QStringList>
#include <QRegExp>
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#define DBG_PRINT
#ifdef DBG_PRINT
#include <QMessageBox>
#endif

#include "lib_prti99.h"

double mercator_sretch = 1.0;

class QFixedPixmapItem: public QGraphicsItem
{
public:
    QFixedPixmapItem(QGraphicsItem * parent = 0): QGraphicsItem(parent), pixmap_ptr(0)
    {
        setScreenSize(QSize(10, 10));
    }
    void setScreenSize(const QSize & _size)
    {
        prepareGeometryChange();
        screen_size = _size;
    }
    void setPixmap(const QPixmap & _pixmap)
    {
        prepareGeometryChange();
        pixmap = _pixmap;
    }
    void setPixmap(const QPixmap * _pixmap)
    {
        prepareGeometryChange();
        pixmap_ptr = (QPixmap *)_pixmap;
    }
    QRectF boundingRect() const
    {
        QRectF _r;
        if (scene() && (!scene()->views().isEmpty()))
        {
            QGraphicsView * _view = scene()->views().front();
            QPoint _pt = _view->mapFromScene(scenePos());
            _r = mapFromScene(_view->mapToScene(QRect(QPoint(_pt.x() - screen_size.width() / 2, _pt.y() - screen_size.height() / 2), screen_size))).boundingRect();
            if (_r.size() == screen_size)
            {
                return (QRectF());
            }
        }
        return (_r);
    }
    void paint(QPainter * _painter, const QStyleOptionGraphicsItem * /*_option*/, QWidget * /*_widget*/)
    {
        if (pixmap_ptr)
        {
            _painter->drawPixmap(boundingRect(), * pixmap_ptr, pixmap_ptr->rect());
        }
        else
        {
            _painter->drawPixmap(boundingRect(), pixmap, pixmap.rect());
        }
    }

private:
    QSize screen_size;
    QPixmap pixmap;
    QPixmap * pixmap_ptr;
};

struct S57_BuildScene::Data
{
    QGraphicsScene * scene;
    QMap<QString, QPixmap> symbols;
    S52_Symbol s52;
    struct style_struct
    {
        style_struct(): priority(0), ref_count(new int)
        {
            pen = new QPen(Qt::NoPen);
            brush = new QBrush(Qt::NoBrush);
            attributes = new QStringList();
            name = new QString();
            condition = new QString();
            symbol = new QString();
            category = new QString();
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
            ref_count = _other.ref_count;
            (* ref_count)++;
        }
        style_struct & operator = (const style_struct & _other)
        {
            if (& _other == this)
            {
                return (* this);
            }
            priority = _other.priority;
            pen = _other.pen;
            brush = _other.brush;
            attributes = _other.attributes;
            name = _other.name;
            condition = _other.condition;
            symbol = _other.symbol;
            category = _other.category;
            ref_count = _other.ref_count;
            (* ref_count)++;
            return (* this);
        }
        ~style_struct()
        {
            if (* ref_count == 0)
            {
                delete pen;
                delete brush;
                delete attributes;
                delete name;
                delete condition;
                delete symbol;
                delete category;
                delete ref_count;
            }
            else
            {
                (* ref_count)--;
            }
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
    };
    QMultiMap<QString, style_struct> p_style, l_style, a_style;
    QMap<qint64, QGraphicsItemGroup *> groups;
    QList<QGraphicsItem *> items;
    QStringList layers;
    double scale;
    bool render_enabled;
    struct RegisterS57_Helper
    {
        RegisterS57_Helper()
        {
            RegisterOGRS57();
            RegisterOGRPRTI99();
        }
    };

    Data(const QString & _lookups_path = QString(), const QString & _symbols_path = QString()): scene(0), scale(0), render_enabled(true)
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
        QStringList _draw = _draw_str.split(";", QString::SkipEmptyParts);
        for (int _i = 0; _i < _draw.count(); _i++)
        {
            QStringList _tok = _draw[_i].split(QRegExp("(\\W)"), QString::SkipEmptyParts);
            //            qDebug() << "apply" << _tok;
            if (_tok.isEmpty())
            {
                continue;
            }
            if ((_tok[0].toUpper() == "LS"))
            {
                if (_tok.count() > 3)
                {
                    *_s->pen = QColor(s52.color(_tok[3]));
                    if (_tok[1].toUpper() == "DASH")
                    {
                        _s->pen->setStyle(Qt::DashLine);
                    }
                    else
                    if (_tok[1].toUpper() == "DOTT")
                    {
                        _s->pen->setStyle(Qt::DotLine);
                    }
                    _s->pen->setWidth(0);
                }
            }
            else
                if (_tok[0].toUpper() == "AC")
                {
                    if (_tok.count() > 1)
                    {
                        *_s->brush = QColor(s52.color(_tok[1]));
                    }
                }
                else
                if (_tok[0].toUpper() == "CS")
                {
                    if (_tok.count() > 1)
                    {
                        *_s->condition = QString(_tok[1]).toUpper();
                    }
                }
                else
                if (_tok[0].toUpper() == "SY")
                {
                    if (_tok.count() > 1)
                    {
                        const QString & _symbol = QString(_tok[1]).toUpper();
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
                else
                if (_tok[0].toUpper() == "AP")
                {
                    if (_tok.count() > 1)
                    {
                        _s->brush->setTextureImage(s52.image(_tok[1]));
                    }
                }
                else
                if (_tok[0].toUpper() == "LC")
                {
                    if (_tok.count() > 1)
                    {
                        if (* _s->name != "CBLSUB")
                        {
                            *_s->pen = QColor(s52.symbol_color(_tok[1]));
                            _s->pen->setWidth(0);
                        }
                    }
                }
        }
    }

    style_struct parse_lookup_line(QString _line, QMultiMap<QString, style_struct> * _style)
    {
        _line = _line.replace(' ', "");
        if (_line[0] == '*')
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
            _p[_i] = _p[_i].replace("\"", "");
        }
        QStringList _attr = QString(_p[1])./*replace("?", "").*/split("|", QString::SkipEmptyParts);
        QString _key = QString(_p[0]).toUpper();
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
//                QVariant _attr_val = get_attribute(_feat, _attr_name);
//				if (!_attr_val.isValid())
                QVariant _attr_val = get_attribute(_feat, _attr_name.left(6));
				if (_attr_val.toInt() == 0 || _attr_val.toInt() != _attr_name.mid(6).toInt())
                {
                    _ok = false;
                    break;
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
        foreach (const QString & _symbol, _complex_symbol.split(" "))
        {
            if (symbols.contains(_symbol))
            {
                const QPixmap & _px_ref = symbols[_symbol];
                _w += _px_ref.width();
                _h = qMax<int>(_h, _px_ref.height());
                _list << & _px_ref;
            }
        }
        QPixmap _px(_w, _h);
        _px.fill(QColor(0, 0, 0, 0));
        QPainter _painter(& _px);
        _w = 0;
        foreach (const QPixmap * _px_ptr, _list)
        {
            _painter.drawPixmap(_w, 0, _px_ptr->width(), _px_ptr->height(), * _px_ptr);
            _w += _px_ptr->width();
        }
        symbols[_complex_symbol] = _px;
        return (& symbols[_complex_symbol]);
    }

    QGraphicsItem * addToGroup(qint64 _scamin, QGraphicsItem * _item, QList<QGraphicsItem *> & _items)
    {
        if (_scamin == 0)
        {
            _scamin = 0x7fffffff;
        }
        QGraphicsItemGroup * & _g = groups[_scamin];
        if (!_g)
        {
            _g = new QGraphicsItemGroup();
            _items << _g;
            _g->setZValue(-_scamin);
            _g->setFlag(QGraphicsItem::ItemHasNoContents);
        }
        _item->setGroup(_g);
        return (_g);
    }

    void build_s57(OGRDataSource * _ds)
    {
        QList<QGraphicsItem *> _items;
//        qDebug() << "We're in build_s57!";
        if (!_ds)
        {
            return;
        }
        qDebug() << "Datasource has" << _ds->GetLayerCount() << "layers";
        qint64 _z_offset = _ds->GetLayerByName("DSID") ? get_attribute(_ds->GetLayerByName("DSID")->GetNextFeature(), "DSPM_CSCL").toLongLong() : 0;
		if (_z_offset == 0)
		{
			_z_offset = _ds->GetLayerByName("M_CSCL") ? get_attribute(_ds->GetLayerByName("M_CSCL")->GetNextFeature(), "CSCALE").toLongLong() : 0;
		}
        qDebug() << "z_offset = " << _z_offset;
        for (int _idx = 0; _idx < _ds->GetLayerCount(); _idx++)
        {
            OGRLayer * _layer = _ds->GetLayer(_idx);
            _layer->ResetReading();
            QString _layer_name = QString(_layer->GetLayerDefn()->GetName()).toUpper();
            if (!layers.contains(_layer_name))
            {
                layers << _layer_name;
            }
            qDebug() << _idx << _layer_name;
            bool _ignore_group = (_layer_name == "DEPARE") || (_layer_name == "LNDARE");
            QGraphicsItem * _item;
            while(OGRFeature * _feat = _layer->GetNextFeature())
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
                    const qint64 _scamin = get_attribute(_feat, "SCAMIN").toLongLong();
					const QString _tooltip = _layer_name + "\nSCAMIN " + QString::number(_scamin) + "\nCSCL " + QString::number(_z_offset);
                    if (((_scamin > 0 && _scamin < 0x7fffffff) || _ignore_group) && ((*_s.pen != Qt::NoPen) || (*_s.brush != Qt::NoBrush) || (!_s.symbol->isNull())))
                    {
                        switch (_wkb)
                        {
                        case wkbPoint:
                        {
                            if (!_s.symbol->isEmpty())
                            {
                                _item = new QFixedPixmapItem();
                                addToGroup(_scamin, _item, _items);
                                _item->setPos(((OGRPoint *)_geom)->getX(), -((OGRPoint *)_geom)->getY()*mercator_sretch);
                                _pixmap = check_complex_symbol(* _s.symbol);
                                ((QFixedPixmapItem *)_item)->setScreenSize(_pixmap->size());
                                ((QFixedPixmapItem *)_item)->setPixmap(_pixmap);
                                _item->setZValue(20 + _s.priority - _z_offset);
								_item->setToolTip(_tooltip);
                            }
                            break;
                        }
                        case wkbMultiPoint:
                        {
                            for (int _idx = 0; _idx < ((OGRMultiPoint *)_geom)->getNumGeometries(); _idx++)
                            {
                                if (!_s.symbol->isEmpty())
                                {
                                    _p = (OGRPoint *)((OGRMultiPoint *)_geom)->getGeometryRef(_idx);
                                    _item = new QFixedPixmapItem();
                                    addToGroup(_scamin, _item, _items);
                                    _item->setPos(_p->getX(), -_p->getY()*mercator_sretch);
                                    _pixmap = check_complex_symbol(* _s.symbol);
                                    ((QFixedPixmapItem *)_item)->setScreenSize(_pixmap->size());
                                    ((QFixedPixmapItem *)_item)->setPixmap(_pixmap);
                                    _item->setZValue(20 + _s.priority - _z_offset);
									_item->setToolTip(_tooltip);
                                }
                                _s = apply_rule(_feat, _layer_name, _idx);
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
							addToGroup(_scamin, _item, _items);
                            ((QGraphicsPathItem *)_item)->setPen(*_s.pen);
                            _item->setZValue(10 + _s.priority - _z_offset);
							_item->setToolTip(_tooltip);
                            if (!_s.symbol->isEmpty())
                            {
                                _item = new QFixedPixmapItem();
                                addToGroup(_scamin, _item, _items);
                                _item->setPos(path.pointAtPercent(0.5));
                                _pixmap = check_complex_symbol(* _s.symbol);
                                ((QFixedPixmapItem *)_item)->setScreenSize(_pixmap->size());
                                ((QFixedPixmapItem *)_item)->setPixmap(_pixmap);
                                _item->setZValue(20 + _s.priority - _z_offset);
								_item->setToolTip(_tooltip);
                            }
                            break;
                        }
                        case wkbMultiLineString:
                        {
                            for (int _idx = 0; _idx < ((OGRMultiLineString *)_geom)->getNumGeometries(); _idx++)
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
                                addToGroup(_scamin, _item, _items);
                                ((QGraphicsPathItem *)_item)->setPen(*_s.pen);
                                _item->setZValue(10 + _s.priority - _z_offset);
								_item->setToolTip(_tooltip);
                                if (!_s.symbol->isEmpty())
                                {
                                    _item = new QFixedPixmapItem();
                                    addToGroup(_scamin, _item, _items);
                                    _item->setPos(path.pointAtPercent(0.5));
                                    _pixmap = check_complex_symbol(* _s.symbol);
                                    ((QFixedPixmapItem *)_item)->setScreenSize(_pixmap->size());
                                    ((QFixedPixmapItem *)_item)->setPixmap(_pixmap);
                                    _item->setZValue(20 + _s.priority - _z_offset);
									_item->setToolTip(_tooltip);
                                }
                                _s = apply_rule(_feat, _layer_name, _idx);
                            }
                            break;
                        }
                        case wkbPolygon:
                        {
                            _poly.clear();
                            _r = ((OGRPolygon *)_geom)->getExteriorRing();
                            _sum_x = 0; _sum_y = 0; _sum_count = 0;
                            for (int _idx = 0; _idx < _r->getNumPoints(); _idx++)
                            {
                                _r->getPoint(_idx, & _p1);
                                _poly.append(QPointF(_p1.getX(), -_p1.getY()*mercator_sretch));
                                _sum_x += _p1.getX();
                                _sum_y += _p1.getY()*mercator_sretch;
								_sum_count++;
                            }
//	                        _poly.append(_poly[0]);
							_item = new QGraphicsPolygonItem(_poly);
							addToGroup(_scamin, _item, _items);
							((QGraphicsPolygonItem *)_item)->setPen(*_s.pen);
							if (_poly.front() == _poly.back())
							{
								((QGraphicsPolygonItem *)_item)->setBrush(*_s.brush);
							}
							_item->setZValue(_s.priority - _z_offset);
							_item->setToolTip(_tooltip);
                            if (!_s.symbol->isEmpty())
                            {
                                _item = new QFixedPixmapItem();
                                addToGroup(_scamin, _item, _items);
                                _item->setPos(_sum_x / _sum_count, -_sum_y / _sum_count);
                                _pixmap = check_complex_symbol(* _s.symbol);
                                ((QFixedPixmapItem *)_item)->setScreenSize(_pixmap->size());
                                ((QFixedPixmapItem *)_item)->setPixmap(_pixmap);
                                _item->setZValue(20 + _s.priority - _z_offset);
								_item->setToolTip(_tooltip);
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
									addToGroup(_scamin, _item, _items);
									((QGraphicsPolygonItem *)_item)->setPen(*_s.pen);
//									((QGraphicsPolygonItem *)_item)->setPen(QColor(Qt::red));								
									((QGraphicsPolygonItem *)_item)->setBrush(*_s.brush);
									_item->setZValue(_s.priority - _z_offset);
									_item->setToolTip(_tooltip);

									_contigous = _idx + 1;
								}
							}

                            if (!_s.symbol->isEmpty())
                            {
                                _item = new QFixedPixmapItem();
                                addToGroup(_scamin, _item, _items);
                                _item->setPos(_sum_x / _sum_count, -_sum_y / _sum_count);
                                _pixmap = check_complex_symbol(* _s.symbol);
                                ((QFixedPixmapItem *)_item)->setScreenSize(_pixmap->size());
                                ((QFixedPixmapItem *)_item)->setPixmap(_pixmap);
                                _item->setZValue(20 + _s.priority - _z_offset);
								_item->setToolTip(_tooltip);
                            }
                            _s = apply_rule(_feat, _layer_name/*, _idx*/);                           
                            break;
                        }
                        default:
                            continue;
                            break;
                        }
                        _item->setData(S57_BuildScene::ItemLayerNameKey, _layer_name);
                    }
                }
				if (_feat != 0)
				{
	                OGRFeature::DestroyFeature(_feat);
				}
            }
        }
        items << _items;
    }
    void updateVisibility(double _scale, bool _exact)
    {
        scale = _scale;
        if (!scene)
        {
            return;
        }
        const qint64 _i_scale = _scale;
        if (!_exact)
        {
            for (QMap<qint64, QGraphicsItemGroup *>::iterator _i = groups.begin(); _i != groups.end(); _i++)
            {
                _i.value()->setVisible(_i_scale <= _i.key() && render_enabled);
            }
        }
        else
        {
            for (QMap<qint64, QGraphicsItemGroup *>::iterator _i = groups.begin(); _i != groups.end(); _i++)
            {
                _i.value()->setVisible(_i_scale == _i.key() && render_enabled);
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
    d->groups.clear();
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

void S57_BuildScene::build(const QString & _file_name)
{
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
        d->build_s57(_ds);
        delete _ds;
    }
    else
    {
        qDebug() << _file_name << "NOT opened";
    }
    QDir::setCurrent(_saved_current);
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
		view->resetMatrix();
		view->resetTransform();
	}
}

void S57_BuildScene::fullView()
{
	if (!d->scene)
	{
		return;
	}
	foreach (QGraphicsView * view, d->scene->views())
	{
		view->resetMatrix();
		view->resetTransform();
	}
    d->scene->setSceneRect(d->scene->itemsBoundingRect());
    foreach (QGraphicsView * view, d->scene->views())
	{
		const double _init_scale = qMin<double>(view->viewport()->width() / d->scene->sceneRect().width(), view->viewport()->height() / d->scene->sceneRect().height());
		view->scale(_init_scale, _init_scale);
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
    foreach (QGraphicsItem * _item, d->items)
    {
        _count += _item->childItems().count();
    }
    return (_count);
}

QList<QGraphicsItem *> S57_BuildScene::items() const
{
    QList<QGraphicsItem *> _items;
    foreach (QGraphicsItem * _item, d->items)
    {
        _items << _item->childItems();
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
    const QPointF _top_left = view->mapToScene(0, 0);
    const QPointF _bottom_right = view->mapToScene(view->viewport()->width(), view->viewport()->height());
    const double _mm = qMax(view->viewport()->width(), view->viewport()->height()) * _dot_mm;
    const double _degs = qMax(qAbs(_bottom_right.x() - _top_left.x()), qAbs(_bottom_right.y() - _top_left.y()));
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
