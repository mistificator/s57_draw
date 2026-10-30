#include "s57_extendedscene.h"
#include "s57_extendedscenemods.h"
#include "s52_fixeditem.h"
#include <QWidget>
#include <QPainter>
#include <QGraphicsView>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QDebug>
#include <QScrollBar>
#include <QTime>
#include <QPainter>
#include <QElapsedTimer>
#include <algorithm>

float sin_table_deg[10800], cos_table_deg[10800];
float sin_table_rad[(int)(6 * M_PI * 1000)], cos_table_rad[(int)(6 * M_PI * 1000)];

struct InitTables
{
    InitTables()
    {
        for (int i = 0; i < sizeof(sin_table_deg) / sizeof(sin_table_deg[0]); i++)
        {
            sin_table_deg[i] = ::sinf(i * M_PI / 1800.0);
            cos_table_deg[i] = ::cosf(i * M_PI / 1800.0);
        }
        for (int i = 0; i < (int)(6 * M_PI * 1000); i++)
        {
            sin_table_rad[i] = ::sinf(i / 1000.0);
            cos_table_rad[i] = ::cosf(i / 1000.0);
        }
    }
};

static InitTables init_tables;

// -----------------------------------------------------------------------------------------------

S57_ExtendedGraphicsSceneModificator::S57_ExtendedGraphicsSceneModificator(): is_active(false), builder_ptr(0)
{

}

void S57_ExtendedGraphicsSceneModificator::setActive(bool state)
{
    is_active = state;
}

bool S57_ExtendedGraphicsSceneModificator::isActive() const
{
    return is_active;
}

void S57_ExtendedGraphicsSceneModificator::registerBuildScene(S57_BuildScene * _builder)
{
    builder_ptr = _builder;
}

S57_BuildScene * S57_ExtendedGraphicsSceneModificator::builder() const
{
    return builder_ptr;
}

S57_ExtendedGraphicsScene * S57_ExtendedGraphicsSceneModificator::scene() const
{
    return builder_ptr ? dynamic_cast<S57_ExtendedGraphicsScene *>(builder_ptr->scene()) : 0;
}

QGraphicsView * S57_ExtendedGraphicsSceneModificator::view(QPainter * _painter)
{
    return dynamic_cast<QGraphicsView *>(dynamic_cast<QWidget *>(_painter->device())->parent());
}

QGraphicsView * S57_ExtendedGraphicsSceneModificator::view() const
{
    return scene() ? (scene()->views().isEmpty() ? 0 : scene()->views().at(0)) : 0;
}

S57_ExtendedGraphicsSceneModificator::~S57_ExtendedGraphicsSceneModificator() {}

void S57_ExtendedGraphicsSceneModificator::onDrawForeground(QPainter *, const QRectF &) {}

void S57_ExtendedGraphicsSceneModificator::onDrawBackground(QPainter *, const QRectF &) {}

bool S57_ExtendedGraphicsSceneModificator::onEvent(QEvent *) { return false; }

void S57_ExtendedGraphicsSceneModificator::onAddMod() {}

void S57_ExtendedGraphicsSceneModificator::onRemoveMod() {}

// -----------------------------------------------------------------------------------------------

QString S57_GridModificator::xLabel(double val)
{
    QString sX;
    if(val >= 0)
    {
        sX= QString::number(int(qAbs(val)))+"°"+QString::number(int(qAbs((-val+int(val))*100)) * 60 / 100).rightJustified(2, '0')+"E";
    }
    else
    {
        sX= QString::number(int(qAbs(val)))+"°"+QString::number(int(qAbs((-val+int(val))*100)) * 60 / 100).rightJustified(2, '0')+"W";
    }
    return sX;
}

QString S57_GridModificator::yLabel(double val)
{
    QString sY;
    if(val >= 0)
    {
        sY= QString::number(int(qAbs(val)))+"°"+QString::number(int(qAbs((-val+int(val))*100)) * 60 / 100).rightJustified(2, '0')+"S";
    }
    else
    {
        sY= QString::number(int(qAbs(val)))+"°"+QString::number(int(qAbs((-val+int(val))*100)) * 60 / 100).rightJustified(2, '0')+"N";
    }
    return sY;
}

void S57_GridModificator::onDrawForeground(QPainter * _painter, const QRectF & _rect)
{
    QGraphicsView * _view = view(_painter);
    if (_view == 0)
    {
        return;
    }

//    const double aspect = scene()->aspect();

    _painter->setPen(QPen(Qt::black, 0, Qt::DotLine));
    _painter->setFont(QFont("Arial", 12));

    const QPointF _scene_top_left = scene()->sceneRect().topLeft();
    const QPointF PtX1_vector((_rect.height()*0.015),(_rect.height()*0.025));
    const QPointF PtX2_vector((_rect.height()*0.015),-(_rect.height()*0.015));

    const QPointF PtY1_vector((_rect.height()*0.005),-(_rect.height()*0.015));
    const QPointF PtY2_vector(-(_rect.height()*0.12),-(_rect.height()*0.015));

    const int grid_count_hint = 5;
    const float _grid_x_step = ( _rect.bottomRight().x()-_rect.topLeft().x())/float(grid_count_hint);
    const float  _grid_y_step = ( _rect.bottomRight().y()-_rect.topLeft().y())/float(grid_count_hint);

    float dx_origin = qAbs(_scene_top_left.x()-_rect.topLeft().x())/_grid_x_step;
    dx_origin = (1.0 - dx_origin + int(dx_origin))*_grid_x_step+_rect.topLeft().x();
    float dy_origin = qAbs(_scene_top_left.y()-_rect.topLeft().y())/_grid_y_step;
    dy_origin = (1.0 - dy_origin + int(dy_origin))*_grid_y_step+_rect.topLeft().y();

    QPointF PX1 = QPointF(dx_origin,_rect.topLeft().y());
    QPointF PtX1 = PX1+PtX1_vector;
    QPointF PY1 = QPointF(_rect.topLeft().x(),dy_origin);
    QPointF PtY1 = PY1+PtY1_vector;

    QPointF PX2 = QPointF(dx_origin,_rect.bottomRight().y());
    QPointF PtX2 = PX2+PtX2_vector;
    QPointF PY2 = QPointF(_rect.bottomRight().x(),dy_origin);
    QPointF PtY2 = PY2+PtY2_vector;

    const QPointF delta_X(_grid_x_step,0);
    const QPointF delta_Y(0,_grid_y_step);

    QString sX,sY;
    for (int _i = 0; _i < grid_count_hint; _i++)
    {
        _painter->drawLine(PX1, PX2);
        _painter->drawLine(PY1, PY2);

        PX1+=delta_X;
        PX2+=delta_X;
        PY1+=delta_Y;
        PY2+=delta_Y;
    }
//    dy_origin /= aspect;

    _painter->save();
    _painter->resetTransform();

    for (int _i=0; _i<grid_count_hint;_i++)
    {
        sX = xLabel(dx_origin);
        sY = yLabel(dy_origin);

        _painter->drawText(_view->mapFromScene(PtX1),sX);
        _painter->drawText(_view->mapFromScene(PtX2),sX);
        _painter->drawText(_view->mapFromScene(PtY1),sY);

        QPointF PtY2_adj = _view->mapFromScene(PtY2);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        const int _text_w = QFontMetrics(_painter->font()).horizontalAdvance(sY);
#else
        const int _text_w = QFontMetrics(_painter->font()).width(sY);
#endif
        PtY2_adj.setX(PtY2_adj.x() - _text_w / 2);
        _painter->drawText(PtY2_adj,sY);

        dx_origin +=_grid_x_step;
//        dy_origin +=_grid_y_step / aspect;
        dy_origin +=_grid_y_step;

        PtX1+=delta_X;
        PtX2+=delta_X;
        PtY1+=delta_Y;
        PtY2+=delta_Y;

    }
    _painter->restore();
}

// -----------------------------------------------------------------------------------------------

S57_RulerModificator::S57_RulerModificator(): S57_ExtendedGraphicsSceneModificator(), in_ruler(false)
{
    circle[0] = 0;
    circle[1] = 0;
    circle[2] = 0;
}

bool S57_RulerModificator::onEvent(QEvent *_ev)
{
    const double aspect = scene()->aspect();
    switch (_ev->type())
    {
        case QEvent::GraphicsSceneMousePress:
        {
            QGraphicsSceneMouseEvent * _me = (QGraphicsSceneMouseEvent *)_ev;
            if (_me->button() == Qt::RightButton)
            {
                in_ruler = !in_ruler;
                if (in_ruler)
                {
                    //circle[0] = scene->addEllipse(QRectF(_me->scenePos(), QSizeF(1e-9, 1e-9)), Qt::NoPen, QColor(0, 255, 0, 40));
                    circle[0] = scene()->addEllipse(QRectF(_me->scenePos(), QSizeF(1e-9, 1e-9)),QPen( QColor(50, 15, 0, 250), 0) , Qt::NoBrush);

                    circle[1] = scene()->addEllipse(circle[0]->rect(), Qt::NoPen, QColor(0, 255, 0, 80));
                    circle[2] = scene()->addEllipse(circle[0]->rect(), Qt::NoPen, Qt::red);
                }
                else
                {
                    scene()->removeItem(circle[0]);
                    scene()->removeItem(circle[1]);
                    scene()->removeItem(circle[2]);
                }
            }
            break;
        }
        case QEvent::GraphicsSceneMouseMove:
        {
            if (in_ruler)
            {
                QGraphicsSceneMouseEvent * _me = (QGraphicsSceneMouseEvent *)_ev;
                const QPointF _pt = _me->scenePos();
                const QPointF _center = circle[0]->rect().center();
                const double _new_radius = ::sqrt(1e-9 + (_pt.x() - _center.x()) * (_pt.x() - _center.x()) + aspect * aspect * (_pt.y() - _center.y()) * (_pt.y() - _center.y()));
                circle[0]->setRect(_center.x() - _new_radius, _center.y() - _new_radius / aspect, 2 * _new_radius, 2 * _new_radius / aspect);
                circle[0]->setStartAngle(((::atan2(_pt.x() - _center.x(), aspect * (_pt.y() - _center.y())) - M_PI / 2) * 180 / M_PI + 2.5) * 16);
             //   circle[0]->setSpanAngle(355 * 16);
                circle[0]->setSpanAngle(359.9375 * 16);

                circle[1]->setRect(circle[0]->rect());
            //    circle[1]->setStartAngle(circle[0]->startAngle() - 7.5 * 16);
                circle[1]->setStartAngle(circle[0]->startAngle() - 0.5 * 16);

                //circle[1]->setSpanAngle(10 * 16);
                circle[1]->setSpanAngle(1.0 * 16);

                const double _scale = builder()->scaleTo1() / 200000000.0;
                circle[2]->setRect(_center.x() - _scale, _center.y() - _scale / aspect, _scale * 2, _scale * 2 / aspect);
            }
            break;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------------------------

class RadarItem: public QFixedPixmapItem
{
public:
    RadarItem(QGraphicsItem *parent = 0): QFixedPixmapItem(parent)
    {
        roi_enabled = true;
        src.reserve(8192 * 4096);
        map_src.reserve(8192 * 4096);
        map_dst.reserve(8192 * 4096);
        setMetrics(8192, 4096);
        colors[0] = 0;
        prev_position = 0;
        for (int c = 1; c < 192; c++)
        {
            colors[c] = QColor::fromHsv(120, 255, c * 255 / 192).rgba();
        }
        for (int c = 192; c < 256; c++)
        {
            colors[c] = QColor::fromHsv(120, (256 - c) * 255 / 64, 255).rgba();
        }
    }
    ~RadarItem() {}
    void setMetrics(int _block_size, int _positions_count)
    {
        block_size = _block_size;
        positions_count = _positions_count;

        reset();
    }
    int blockSize() const
    {
        return block_size;
    }
    int positionsCount() const
    {
        return positions_count;
    }
    void reset()
    {
        createSrcMap();
        createDstMap();
    }
    void adjustSize()
    {
        createDstMap();
    }
    void addData(const short * block, int position)
    {
        if (map_dst.size() < block_size * positions_count)
        {
            return;
        }
        if (position >= positions_count)
        {
            return;
        }
        if (position - prev_position < 0 && position < positions_count / 2 && prev_position > positions_count / 2)
        {
            dst.fill(0);
        }
        prev_position = position;
        ::memcpy(src.data() + block_size * position, block, block_size * sizeof(src.data()[0]));
        if (roi_enabled)
        {
            const int * offset_ptr = map_dst.constData() + block_size * position;
            short * dst_ptr = dst.data();
            unsigned int * img_ptr = (unsigned int *)img.bits();
            for (int v = 0; v < block_size; v++, offset_ptr++, block++)
            {
                short & _dst = dst_ptr[* offset_ptr];
                if (_dst < * block)
                {
                    _dst = * block;
                }
                img_ptr[* offset_ptr] = colors[_dst & 0xff];
            }
        }
    }
    void setScreenSize(const QSize & _size)
    {
        QFixedPixmapItem::setScreenSize(_size);
        adjustSize();
    }
    QImage roi(double start_angle, double stop_angle, int start_distance, int stop_distance) const // angles in radians, 0 <= angle <= 2 * M_PI
    {
        if (start_distance < 0 || stop_distance > block_size || start_distance > stop_distance)
        {
            return QImage();
        }
        while (start_angle < 0)
        {
            start_angle += 2 * M_PI;
        }
        while (stop_angle < 0)
        {
            stop_angle += 2 * M_PI;
        }
        while (start_angle >= 2 * M_PI)
        {
            start_angle -= 2 * M_PI;
        }
        while (stop_angle >= 2 * M_PI)
        {
            stop_angle -= 2 * M_PI;
        }
        const double angle_step = 2.0 * M_PI / positions_count;
        const int _start_angle = start_angle / angle_step;
        const int _stop_angle1 = start_angle < stop_angle ? stop_angle / angle_step : 2 * M_PI / angle_step;
        const int _stop_angle2 = start_angle < stop_angle ? 0 : stop_angle / angle_step;

        QSize sz(stop_distance - start_distance, start_angle < stop_angle ? _stop_angle1 - _start_angle : positions_count - _start_angle + _stop_angle2);
        if (sec_img.size() != sz)
        {
            sec_img = QImage(sz, QImage::Format_RGB32);
        }
        if (roi_enabled)
        {
            unsigned int * img_ptr = (unsigned int *)sec_img.bits();
            for (int t = _start_angle; t < _stop_angle1; t++)
            {
                const short * _src = src.constData() + t * block_size + start_distance;
                for (int v = start_distance; v < stop_distance; v++)
                {
                    * img_ptr++ = colors[* _src++ & 0xff];
                }
            }
            for (int t = 0; t < _stop_angle2; t++)
            {
                const short * _src = src.constData() + t * block_size + start_distance;
                for (int v = start_distance; v < stop_distance; v++)
                {
                    * img_ptr++ = colors[* _src++ & 0xff];
                }
            }
        }
        QPoint center = sec_img.rect().center();
        QTransform matrix;
        matrix.translate(center.x(), center.y());
        matrix.rotate(-90);
        return sec_img.transformed(matrix);
    }
    QLineF radius(int position_number) const
    {
        return QLineF(QPointF(), map_src[block_size * position_number + block_size - 1]);
    }
    void setRoiEnabled(bool state)
    {
        roi_enabled = state;
    }
    bool roiEnabled() const
    {
        return roi_enabled;
    }
private:
    int block_size, positions_count;
    QVector<QPointF> map_src;
    QVector<int> map_dst;
    QVector<short> src, dst;
    QImage img;
    mutable QImage sec_img;
    QRgb colors[256];
    bool roi_enabled;
    int prev_position;
    void createSrcMap()
    {
        qDebug() << "createSrcMap" << block_size << positions_count;
        map_src.resize(block_size * positions_count);
        src.resize(block_size * positions_count);
        src.fill(0);
        const double angle_step = 2.0 * M_PI / positions_count;
        QPointF * map_src_ptr = map_src.data();
        double angle = - M_PI / 2 + angle_step;
        for (int t = 0; t < positions_count; t++, angle += angle_step)
        {
            const int index = R2T(angle);
            for (int v = 0; v < block_size; v++, map_src_ptr++)
            {
                * map_src_ptr = QPointF(v * cos_table_rad[index], v * sin_table_rad[index]);
            }
        }
    }
    void createDstMap()
    {
        if (map_src.size() != block_size * positions_count)
        {
            return;
        }
        const int w = screenSize().width();
        const int h = screenSize().height();
        if (dst.size() == w * h && map_dst.size() == block_size * positions_count)
        {
            return;
        }
        qDebug() << "createDstMap" << w << h;

        if (dst.count() != w * h)
        {
            dst.resize(w * h);
            dst.fill(0);
        }

        if (img.isNull() || img.size() != QSize(w, h))
        {
            img = QImage(w, h, QImage::Format_ARGB32);
            img.fill(QColor(0, 0, 0, 0).rgba());
            setImage(& img);
        }

        map_dst.resize(block_size * positions_count);

        const double xw = (1.0 * w) / (2 * block_size);
        const double yh = (1.0 * h) / (2 * block_size);

        const int w_2 = w /2;
        const int h_2 = h /2;

        const qreal * map_src_ptr = (const qreal *)map_src.constData();
        int * map_dst_ptr = map_dst.data();
        for (int t = 0; t < positions_count; t++)
        {
            for (int v = 0; v < block_size; v++)
            {
                const int x = * map_src_ptr++ * xw + w_2;
                const int y = * map_src_ptr++ * yh + h_2;
                * map_dst_ptr++ = (y * w) + x;
            }
        }
    }
};

S57_RadarModificator::S57_RadarModificator(): S57_ExtendedGraphicsSceneModificator(), QObject(), radar_flags(RadarAll)
{
    ri = new RadarItem();
    ri->setZValue(1000000);

    rli = new QGraphicsLineItem();
    QPen rli_pen(Qt::yellow, 1);
    rli_pen.setCosmetic(true);
    rli->setPen(rli_pen);
    rli->setZValue(ri->zValue() + 1);

    rpi = new QGraphicsPolygonItem();
    QPen rpi_pen(Qt::red, 5);
    rpi_pen.setCosmetic(true);
    rpi->setPen(rpi_pen);
    rpi->setZValue(ri->zValue() + 2);

    setRoiSize(0.2 * blockSize(), 0.2 * positionsCount());
}

void S57_RadarModificator::setMetrics(int _block_size, int _positions_count)
{
    ri->setMetrics(_block_size, _positions_count);
    setRoiSize(d_radius, d_angle);
}

void S57_RadarModificator::setRoiSize(int _d_radius, int _d_angle)
{
    d_radius = qMin(_d_radius, blockSize());
    d_angle = qMin(_d_angle, positionsCount());
    updateRoi();
}

int S57_RadarModificator::roiRadius() const
{
    return d_radius;
}

int S57_RadarModificator::roiAngle() const
{
    return  d_angle;
}

void S57_RadarModificator::setRoiPos(QPointF pt)
{
    if (view())
    {
        roi_point = pt;
        updateRoi(roi_point);
    }
}

QPointF S57_RadarModificator::roiPos() const
{
    return roi_point;
}

QPolygonF S57_RadarModificator::roiSelector() const
{
    QPolygonF poly = rpi->polygon();
    const QPointF pos = rpi->pos();
    for (int i = 0; i < poly.count(); i++)
    {
        poly[i].setX(poly[i].x() + pos.x());
        poly[i].setY(poly[i].y() + pos.y());
    }
    return poly;
}

int S57_RadarModificator::blockSize() const
{
    return ri->blockSize();
}
int S57_RadarModificator::positionsCount() const
{
    return ri->positionsCount();
}

int S57_RadarModificator::screenRadius() const
{
    return ri->screenSize().width() / 2;
}

QList<QGraphicsLineItem *> S57_RadarModificator::coasts() const
{
    return ri_unsorted_coasts;
}

QList<QGraphicsLineItem *> S57_RadarModificator::raycastCoasts() const
{
    return ri_visible_coasts;
}

QList<QLineF> S57_RadarModificator::raycastCoastsLines() const
{
    return ri_sec_lines;
}

void S57_RadarModificator::onAddMod()
{
    view()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    roi_point = scene()->sceneRect().center();

    scene()->addItem(ri);
    scene()->addItem(rpi);
    scene()->addItem(rli);

    ri->setVisible((radar_flags & RadarCircle) == RadarCircle);
    rpi->setVisible((radar_flags & RadarSectorSelector) == RadarSectorSelector);
    rli->setVisible((radar_flags & RadarRay) == RadarRay);

    all_coasts = builder()->items<QGraphicsPolygonItem>("LNDARE");

    connect(view()->horizontalScrollBar(), SIGNAL(valueChanged(int)), dynamic_cast<QObject *>(this), SLOT(viewportMoved()), Qt::DirectConnection);
    connect(view()->verticalScrollBar(), SIGNAL(valueChanged(int)), dynamic_cast<QObject *>(this), SLOT(viewportMoved()), Qt::DirectConnection);
    connect(scene(), SIGNAL(scaleChanged()), dynamic_cast<QObject *>(this), SLOT(viewportMoved()), Qt::DirectConnection);

    viewportMoved();

    view()->installEventFilter(this);
}

void S57_RadarModificator::onRemoveMod()
{
    view()->removeEventFilter(this);

    disconnect(view()->horizontalScrollBar(), SIGNAL(valueChanged(int)), dynamic_cast<QObject *>(this), SLOT(viewportMoved()));
    disconnect(view()->verticalScrollBar(), SIGNAL(valueChanged(int)), dynamic_cast<QObject *>(this), SLOT(viewportMoved()));
    disconnect(scene(), SIGNAL(scaleChanged()), dynamic_cast<QObject *>(this), SLOT(viewportMoved()));

    foreach (QGraphicsLineItem * ri_coast_item, ri_unsorted_coasts)
    {
        if (ri_coast_item->scene())
        {
            scene()->removeItem(ri_coast_item);
        }
        delete ri_coast_item;
    }
    ri_sorted_coasts.clear();
    ri_unsorted_coasts.clear();

    foreach (QGraphicsLineItem * ri_visible_coast_item, ri_visible_coasts)
    {
        if (ri_visible_coast_item->scene())
        {
            scene()->removeItem(ri_visible_coast_item);
        }
        delete ri_visible_coast_item;
    }
    ri_visible_coasts.clear();
    ri_sec_lines.clear();

    ri->setVisible(false);
    rpi->setVisible(false);
    rli->setVisible(false);

    scene()->removeItem(ri);
    scene()->removeItem(rpi);
    scene()->removeItem(rli);

    all_coasts.clear();
}

void S57_RadarModificator::polarLinePointPrepare(const QLineF & line, float & q, float & r)
{
    const float t0 = line.p1().y() * 2 * M_PI / ri->positionsCount(), t1 = line.p2().y() * 2 * M_PI / ri->positionsCount();
    const float r0 = line.p1().x(), r1 = line.p2().x();

    q = (r1 * sin_table_rad[R2T(t1)] - r0 * sin_table_rad[R2T(t0)]) / (r1 * cos_table_rad[R2T(t1)] - r0 * cos_table_rad[R2T(t0)]);
    r = r0 * (sin_table_rad[R2T(t0)] - q * cos_table_rad[R2T(t0)]);
}

float S57_RadarModificator::polarLinePoint(float q, float r, float t)
{
    t = t * 2 * M_PI / ri->positionsCount();
    return r / (sin_table_rad[R2T(t)] - q * cos_table_rad[R2T(t)]);
}

float S57_RadarModificator::polarLinePoint(const QLineF & line, float t)
{
    float q, r;
    polarLinePointPrepare(line, q, r);
    return polarLinePoint(q, r, t);
}

void S57_RadarModificator::addData(QVector<short> & block, int position, const unsigned char coast_color)
{
    addData(block.data(), position, coast_color);
}

void S57_RadarModificator::addData(short * block, int position, const unsigned char coast_color)
{
    if (!isActive())
    {
        return;
    }
    if ((radar_flags & RadarRay) == RadarRay)
    {
        const float radius = radarRadius();
        const float angle = 2 * M_PI * position / positionsCount() - M_PI / 2;
        const QPointF ct = radarCenter();
        rli->setLine(QLineF(ct, QPointF(ct.x() + radius * ::cos(angle) * scene()->aspect(), ct.y() + radius * ::sin(angle))));
    }
    const int block_size = ri->blockSize();
    const QLineF ray(QPoint(1, position), QPointF(block_size, position));
    foreach (QLineF line, ri_sec_lines)
    {
        QPointF pt;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        if (line.intersects(ray, &pt) == QLineF::BoundedIntersection)
#else
        if (line.intersect(ray, &pt) == QLineF::BoundedIntersection)
#endif
        {
            float q, r;
            polarLinePointPrepare(line, q, r);
            for (float sub_pos = 0.0f; sub_pos < 1.0f; sub_pos += 0.1f) // super resolution
            {
                const int plp = polarLinePoint(q, r, position + sub_pos);
                block[qMin<int>(block_size - 1, qMax<int>(0.0f, plp))] = coast_color;
                block[qMin<int>(block_size - 1, qMax<int>(0.0f, plp + 1))] = coast_color;
                block[qMin<int>(block_size - 1, qMax<int>(0.0f, plp - 1))] = coast_color;
            }
        }
    }

    if (ri->isVisible())
    {
        ri->addData(block, position);
    }
}

void S57_RadarModificator::updateRoi()
{
    updateRoi(roi_point);
}

class WidgetUpdatesDisabler
{
public:
    WidgetUpdatesDisabler(QWidget * _w)
    {
        w = _w;
        saved_state = w->updatesEnabled();
        if (saved_state)
        {
            w->setUpdatesEnabled(false);
        }
    }
    ~WidgetUpdatesDisabler()
    {
        if (saved_state != w->updatesEnabled())
        {
            w->setUpdatesEnabled(saved_state);
        }
    }

private:
    QWidget * w;
    bool saved_state;
};

void S57_RadarModificator::viewportMoved()
{
    if (!isActive())
    {
        return;
    }

    WidgetUpdatesDisabler wud(view());

    S57_ExtendedGraphicsScene * _scene = scene();

    if ((radar_flags & RadarRay) == RadarRay)
    {
        const QPointF ct = radarCenter();
        const QLineF line = rli->line();
        double dx = ct.x() - line.x1();
        double dy = ct.y() - line.y1();
        rli->setLine(line.x1() + dx, line.y1() + dy, line.x2() + dx, line.y2() + dy);
    }

    QStringList log;
    log << QString(__FUNCTION__) + " start";
    QElapsedTimer t;
    t.start();

    foreach (QGraphicsLineItem * ri_coast_item, ri_unsorted_coasts)
    {
        if (ri_coast_item->scene())
        {
            ri_coast_item->hide();
//            _scene->removeItem(ri_coast_item);
        }
        delete ri_coast_item;
    }
    ri_sorted_coasts.clear();
    ri_unsorted_coasts.clear();

    log << QString::number(t.elapsed()) + " cleanup coasts";

    foreach (QGraphicsLineItem * ri_visible_coast_item, ri_visible_coasts)
    {
        if (ri_visible_coast_item->scene())
        {
            ri_visible_coast_item->hide();
//            _scene->removeItem(ri_visible_coast_item); // stupid monstrous instruction
        }
        delete ri_visible_coast_item;
    }
    ri_visible_coasts.clear();
    ri_sec_lines.clear();

    log << QString::number(t.elapsed()) + " cleanup raycast coasts";

    const double aspect = _scene->aspect();

/*
    QPointF tl = view()->mapToScene(QRect(QPoint(), view()->viewport()->size()).topLeft());
    QPointF br = view()->mapToScene(QRect(QPoint(), view()->viewport()->size()).bottomRight());
    QPointF ct = view()->mapToScene(QRect(QPoint(), view()->viewport()->size()).center());

    const double radius = qMin(qAbs(tl.x() - br.x()), qAbs(tl.y() - br.y())) / 2.0;
*/

//    const QRectF r = view()->mapToScene(view()->viewport()->rect()).boundingRect();
//    const float radius = qMin((r.width() / aspect) / 2, r.height() / 2);
    const float radius = radarRadius();

    const QPointF ct = radarCenter();

    const int sz = qMin(view()->viewport()->size().width(), view()->viewport()->size().height());
    ri->setScreenSize(QSize(sz, sz));
    ri->setPos(ct); // anchor center
    rpi->setPos(ct);

    if ((radar_flags & RadarSector) == RadarSector)
    {
        updateRoi(roi_point);
        log << QString::number(t.elapsed()) + " update roi";
    }

    if ((radar_flags & RadarCoasts) != RadarCoasts && (radar_flags & RadarRaycastCoasts) != RadarRaycastCoasts && (radar_flags & RadarSector) != RadarSector)
    {
        return;
    }

    const int pos_count = ri->positionsCount();
    QList<int> angles;
    QPolygonF pr1(QRectF(ct.x() - radius * aspect, ct.y() - radius, 2 * radius * aspect, 2 * radius));
    QPen pen(Qt::yellow, 5);
    pen.setCosmetic(true);
    const double r2 = radius * radius * aspect * aspect;
    foreach (QGraphicsPolygonItem * coast_item, all_coasts)
    {
        const QPolygonF & poly = coast_item->polygon();
        if (!poly.intersected(pr1).isEmpty())
        {
            QPointF prev_pt = poly[0];
            int prev_index = 0;
            for (int i = 1; i < poly.count(); i++)
            {
                const QPointF & pt = poly[i];
                const QPointF mid_pt((pt.x() + prev_pt.x()) / 2, (pt.y() + prev_pt.y()) / 2);
                const float dist_1 = (mid_pt.x() - ct.x()) * (mid_pt.x() - ct.x()) + (mid_pt.y() - ct.y()) * (mid_pt.y() - ct.y());
                const float dist_2 = (pt.x() - ct.x()) * (pt.x() - ct.x()) + (pt.y() - ct.y()) * (pt.y() - ct.y());
                const float dist_3 = (prev_pt.x() - ct.x()) * (prev_pt.x() - ct.x()) + (prev_pt.y() - ct.y()) * (prev_pt.y() - ct.y());
                if (dist_1 < r2 || dist_2 < r2 || dist_3 < r2)
                {
                    if (i - prev_index == 1)
                    {
                        QLineF coast_line(prev_pt.x(), prev_pt.y(), pt.x(), pt.y());
                        if (coast_line.length() < 0.16)
                        {
                            QGraphicsLineItem * ri_coast_item = new QGraphicsLineItem(coast_line);
                            ri_coast_item->setPen(pen);
                            if ((radar_flags & RadarCoasts) == RadarCoasts)
                            {
                                _scene->addItem(ri_coast_item);
                            }
                            ri_sorted_coasts[dist_1] << ri_coast_item;
                            ri_unsorted_coasts << ri_coast_item;
                            double angle = ::atan2((mid_pt.y() - ct.y()), (mid_pt.x() - ct.x()));
                            angle = M_PI / 2 + angle;
                            if (angle < 0)
                            {
                                angle += 2 * M_PI;
                            }
                            if (angle >= 2 * M_PI)
                            {
                                angle -= 2 * M_PI;
                            }
                            const int i_angle = angle * pos_count / (2 * M_PI);
                            if (!angles.contains(i_angle))
                            {
                                angles << i_angle;
                            }
                        }
                    }
                }
                prev_pt = pt;
                prev_index = i;
            }
        }
    }
    log << QString::number(t.elapsed()) + " coasts gen";

    if ((radar_flags & RadarRaycastCoasts) != RadarRaycastCoasts)
    {
        return;
    }

    pen.setColor(Qt::green);
    pen.setWidth(5);

//    const int t_step = 1; // raycasting granularity (1 - max resolution)
    const int t_step = qMax(1, 1 << (int)(0.5 * ::log10f(builder()->scaleTo1())));
    int t_next = 0;
    std::sort(angles.begin(), angles.end());
//    for (int t = 0; t < pos_count; t+=t_step)
    foreach (int t, angles)
    {
        if (t_next > t)
        {
            continue;
        }
        t_next = t + t_step;
        QLineF lr = ri->radius(t);
        lr.setP2(QPointF(lr.x2() * aspect, lr.y2()));
        const QLineF & line = lr.translated(ct.x(), ct.y());
        bool stop = false;
        foreach (QList<QGraphicsLineItem *> list, ri_sorted_coasts)
        {
            foreach (QGraphicsLineItem * ri_coast_item, list)
            {
                QPointF pt;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                if (line.intersects(ri_coast_item->line(), & pt) == QLineF::BoundedIntersection)
#else
                if (line.intersect(ri_coast_item->line(), & pt) == QLineF::BoundedIntersection)
#endif
                {
                    const QLineF & coast_line = ri_coast_item->line();
                    QGraphicsLineItem * ri_visible_coast_item = new QGraphicsLineItem(coast_line);
                    ri_visible_coast_item->setPen(pen);
                    ri_visible_coast_item->setZValue(1000000);
                    if ((radar_flags & RadarRaycastCoasts) == RadarRaycastCoasts)
                    {
                        _scene->addItem(ri_visible_coast_item);
                    }
                    ri_visible_coasts << ri_visible_coast_item;

                    stop = true;
                    if ((radar_flags & RadarSector) != RadarSector)
                    {
                        break;
                    }

                    QLineF test_line1(ct, coast_line.p1()), test_line2(ct, coast_line.p2());

                    test_line1.setP1(QPointF(test_line1.x1() / aspect, test_line1.y1()));
                    test_line1.setP2(QPointF(test_line1.x2() / aspect, test_line1.y2()));

                    test_line2.setP1(QPointF(test_line2.x1() / aspect, test_line2.y1()));
                    test_line2.setP2(QPointF(test_line2.x2() / aspect, test_line2.y2()));

                    double angle1 = 90.0 - test_line1.angle();
                    double angle2 = 90.0 - test_line2.angle();
                    if (angle1 < 0)
                    {
                        angle1 += 360.0;
                    }
                    angle1 = angle1 * ri->positionsCount() / 360.0;
                    if (angle2 < 0)
                    {
                        angle2 += 360.0;
                    }
                    angle2 = angle2 * ri->positionsCount() / 360.0;
                    double angle = test_line1.angle() > test_line2.angle() ? angle2 : angle1;
//                    qDebug() << t << angle;
                    if (t < angle - t_step + 1)
                    {
                        t = angle - t_step + 1;
                    }

                    const float len_1 = test_line1.length() * ri->blockSize() / radius;
                    const float len_2 = test_line2.length() * ri->blockSize() / radius;

//                    const float len_1 = ::sqrt((test_line1.p2().x() - test_line1.p1().x()) * (test_line1.p2().x() - test_line1.p1().x()) / (aspect * aspect) + (test_line1.p2().y() - test_line1.p1().y()) * (test_line1.p2().y() - test_line1.p1().y())) * ri->blockSize() / radius;
//                    const float len_2 = ::sqrt((test_line1.p2().x() - test_line1.p1().x()) * (test_line1.p2().x() - test_line1.p1().x()) / (aspect * aspect) + (test_line1.p2().y() - test_line1.p1().y()) * (test_line1.p2().y() - test_line1.p1().y())) * ri->blockSize() / radius;

                    if (angle1 > angle2 && angle1 - angle2 > ri->positionsCount() / 2)
                    {
                        ri_sec_lines << QLineF(len_1, angle1, len_2, angle2 + ri->positionsCount());
                        ri_sec_lines << QLineF(len_1, angle1 - ri->positionsCount(), len_2, angle2);
                    }
                    else
                    if (angle2 > angle1 && angle2 - angle1 > ri->positionsCount() / 2)
                    {
                        ri_sec_lines << QLineF(len_2, angle2, len_1, angle1 + ri->positionsCount());
                        ri_sec_lines << QLineF(len_2, angle2 - ri->positionsCount(), len_1, angle1);
                    }
                    else
                    {
                        ri_sec_lines << QLineF(len_1, angle1, len_2, angle2);
                    }
                    break;
                }
            }
            if (stop)
            {
                break;
            }
        }
    }

    log << QString::number(t.elapsed()) + " raycast coasts gen";

    qDebug() << log.join("\n");
}

void S57_RadarModificator::updateRoi(QPointF & _pt)
{
    QGraphicsView * _view = view();

    if (!_view)
    {
        return;
    }

    const double aspect = scene()->aspect();

/*
    QPointF tl = _view->mapToScene(QRect(QPoint(), _view->viewport()->size()).topLeft());
    QPointF br = _view->mapToScene(QRect(QPoint(), _view->viewport()->size()).bottomRight());
    QPointF ct = _view->mapToScene(QRect(QPoint(), _view->viewport()->size()).center());

    const double radius = qMin(qAbs(tl.x() - br.x()), qAbs(tl.y() - br.y())) / 2.0;
*/
//    QRectF r = _view->mapToScene(_view->viewport()->rect()).boundingRect();
//    const float radius = qMin((r.width() / aspect) / 2, r.height() / 2);
    const float radius = radarRadius();

    QPointF ct = radarCenter();

    double angle = ::atan2(_pt.y() - ct.y(), (_pt.x() - ct.x()) / aspect);
    double length = ::sqrtf((_pt.y() - ct.y()) * (_pt.y() - ct.y()) + (_pt.x() - ct.x()) * (_pt.x() - ct.x()) / (aspect * aspect) + 1e-15);

    const double r_step = 0.5 * d_radius / blockSize();
    const double a_step = M_PI * d_angle / positionsCount();

    if (length < r_step * radius)
    {
        length = r_step * radius;
    }
    if (length > (1.0 - r_step) * radius)
    {
        length = (1.0 - r_step) * radius;
    }

    const double l1 = length - r_step * radius;
    const double l2 = length + r_step * radius;
    const double a1 = angle - a_step;
    const double a2 = angle + a_step;

    _pt = QPointF(ct.x() + ((l1 + l2) / 2) * ::cos((a1 + a2) / 2) * aspect, ct.y() + ((l1 + l2) / 2) * ::sin((a1 + a2) / 2));

    QPolygonF rpi_poly;


    rpi_poly << QPointF(l1 * cos_table_rad[R2T(a1)] * aspect, l1 * sin_table_rad[R2T(a1)]);
    rpi_poly << QPointF(l2 * cos_table_rad[R2T(a1)] * aspect, l2 * sin_table_rad[R2T(a1)]);
    for (double a = a1; a <= a2; a += 0.05)
    {
        rpi_poly << QPointF(l2 * cos_table_rad[R2T(a)] * aspect, l2 * sin_table_rad[R2T(a)]);
    }
    rpi_poly << QPointF(l2 * cos_table_rad[R2T(a2)] * aspect, l2 * sin_table_rad[R2T(a2)]);
    rpi_poly << QPointF(l1 * cos_table_rad[R2T(a2)] * aspect, l1 * sin_table_rad[R2T(a2)]);
    for (double a = a2; a >= a1; a -= 0.05)
    {
        rpi_poly << QPointF(l1 * cos_table_rad[R2T(a)] * aspect, l1 * sin_table_rad[R2T(a)]);
    }

    /*
    rpi_poly << QPointF(l1 * ::cos(a1) * aspect, l1 * ::sin(a1));
    rpi_poly << QPointF(l2 * ::cos(a1)  * aspect, l2 * ::sin(a1));
    for (double a = a1; a <= a2; a += 0.05)
    {
        rpi_poly << QPointF(l2 * ::cos(a) * aspect, l2 * ::sin(a));
    }
    rpi_poly << QPointF(l2 * ::cos(a2) * aspect, l2 * ::sin(a2));
    rpi_poly << QPointF(l1 * ::cos(a2) * aspect, l1 * ::sin(a2));
    for (double a = a2; a >= a1; a -= 0.05)
    {
        rpi_poly << QPointF(l1 * ::cos(a) * aspect, l1 * ::sin(a));
    }
    */

    rpi->setPolygon(rpi_poly);

    rpi->setPos(ct);

    roi_rect.setLeft(a1 + M_PI / 2);
    roi_rect.setRight(a2 + M_PI / 2);
    roi_rect.setTop(l1 * ri->blockSize() / radius);
    roi_rect.setBottom(l2 * ri->blockSize() / radius);
}

bool S57_RadarModificator::onEvent(QEvent * _ev)
{
    switch (_ev->type())
    {
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseMove:
    {
        QGraphicsSceneMouseEvent * _me = (QGraphicsSceneMouseEvent *)_ev;
        const QPointF _pt = _me->scenePos();
        if ((_me->buttons() & Qt::LeftButton) == Qt::LeftButton)
        {
            if ((radar_flags & RadarSector) == RadarSector)
            {
                roi_point = _pt;
                updateRoi(roi_point);
            }
        }
        break;
    }
    }
    return false;
}

void S57_RadarModificator::onDrawForeground(QPainter * _painter, const QRectF &)
{
    _painter->save();
    _painter->resetTransform();
    QRect r = view(_painter)->viewport()->rect();
    const float radius = qMin(r.width() / 2, r.height() / 2);
    const float center_x = r.width() / 2;
    const float center_y = r.height() / 2;
    if ((radar_flags & RadarForeground) == RadarForeground)
    {

        _painter->setPen(QPen(Qt::gray));
        _painter->setBrush(QBrush(Qt::gray));

        QPainterPath lpath;
        lpath.moveTo(0, 0);
        lpath.lineTo(center_x, 0);
        lpath.lineTo(center_x, center_y - radius);
        lpath.arcTo(center_x - radius, center_y - radius, 2 * radius, 2 * radius, 90, 180);
        lpath.lineTo(center_x, r.height());
        lpath.lineTo(0, r.height());
        lpath.lineTo(0, 0);

        QPainterPath rpath;
        rpath.moveTo(r.width(), 0);
        rpath.lineTo(center_x, 0);
        rpath.lineTo(center_x, center_y - radius);
        rpath.arcTo(center_x - radius, center_y - radius, 2 * radius, 2 * radius, 90, -180);
        rpath.lineTo(center_x, r.height());
        rpath.lineTo(r.width(), r.height());
        rpath.lineTo(r.width(), 0);

        _painter->drawPath(lpath);
        _painter->drawPath(rpath);
    }

    if ((radar_flags & RadarLimb) == RadarLimb)
    {
        QPen p;
        p.setWidth(1);
        p.setCosmetic(true);
        _painter->setBrush(QBrush());
        QFont f("Arial");
        f.setPixelSize(10);
        _painter->setFont(f);
        const int count = 36;
        const float step = 360.0 / count;
        p.setStyle(Qt::DotLine);
        p.setColor(Qt::darkGray);
        _painter->setPen(p);
        for (int i = 0; i < count / 3; i++)
        {
            const float angle = i * step * 3 * M_PI / 180.0;
            const float _sin = sin_table_rad[R2T(angle - M_PI / 2)];
            const float _cos = cos_table_rad[R2T(angle - M_PI / 2)];
            _painter->drawLine(QPoint(center_x, center_y), QPoint(center_x + radius * _cos, center_y + radius * _sin));
        }
        p.setStyle(Qt::SolidLine);
        p.setColor(Qt::white);
        _painter->setPen(p);
        _painter->setBrush(QBrush(qRgba(0, 0, 0, 80)));
        const float text_offset_x = -0.02 * radius;
        const float text_offset_y = 0.01 * radius;
        const float tick_offset = 1.015 * radius;
        const float label_offset = 1.045 * radius;
        for (int i = 0; i < count; i++)
        {
            const float angle = i * step * M_PI / 180.0;
            const float _sin = sin_table_rad[R2T(angle - M_PI / 2)];
            const float _cos = cos_table_rad[R2T(angle - M_PI / 2)];
            _painter->drawLine(QPoint(center_x + tick_offset * _cos, center_y + tick_offset * _sin), QPoint(center_x + radius * _cos, center_y + radius * _sin));
            _painter->drawText(QPoint(center_x + label_offset * _cos + text_offset_x, center_y + label_offset * _sin + text_offset_y), QString::number(i * step));
        }
    }

    _painter->restore();
}

void S57_RadarModificator::onDrawBackground(QPainter * _painter, const QRectF & _rect)
{
    _painter->fillRect(_rect, Qt::black);
}

QImage S57_RadarModificator::roi(double start_angle, double stop_angle, int start_distance, int stop_distance) const // angles in radians, 0 <= angle <= 2 * M_PI
{
    return ri->roi(start_angle, stop_angle, start_distance, stop_distance);
}

QRectF S57_RadarModificator::roiRect() const
{
    return roi_rect;
}

QImage S57_RadarModificator::roi() const
{
    return ri->roi(roi_rect.left(), roi_rect.right(), roi_rect.top(), roi_rect.bottom());
}

bool S57_RadarModificator::eventFilter(QObject * _obj, QEvent * _ev)
{
    if (_obj == view())
    {
        switch (_ev->type())
        {
        case QEvent::Show:
        case QEvent::Resize:
            QMetaObject::invokeMethod(this, "viewportMoved", Qt::QueuedConnection);
            break;
        }
    }
    return false;
}

void S57_RadarModificator::setFlags(int f)
{
    radar_flags = f;
    ri->setVisible((radar_flags & RadarCircle) == RadarCircle);
    rpi->setVisible((radar_flags & RadarSectorSelector) == RadarSectorSelector);
    rli->setVisible((radar_flags & RadarRay) == RadarRay);
    ri->setRoiEnabled((radar_flags & RadarSector) == RadarSector);
    viewportMoved();
}

int S57_RadarModificator::flags() const
{
    return radar_flags;
}

void S57_RadarModificator::setCustomRadarCenter(QPointF ct)
{
    custom_ct = ct;
}

QPointF S57_RadarModificator::radarCenter() const
{
    QGraphicsView * _view = view();
    if (!_view)
    {
        return custom_ct;
    }
    return custom_ct.isNull() ? _view->mapToScene(_view->viewport()->rect()).boundingRect().center() : custom_ct;
}

double S57_RadarModificator::radarRadius() const
{
    if (!view() || !scene())
    {
        return -1;
    }
    const QRectF r = view()->mapToScene(view()->viewport()->rect()).boundingRect();
    return  qMin((r.width() / scene()->aspect()) / 2, r.height() / 2);
}
// ---------------------------------------------------------------------------------------------------------

S57_TargetModificator::S57_TargetModificator(): QObject(), S57_ExtendedGraphicsSceneModificator()
{
}

S57_TargetModificator::~S57_TargetModificator()
{
    clear();
}

S57_TargetModificator::Target & S57_TargetModificator::addTarget(QPointF pos, QString name, const QUuid & uuid)
{
    Target target;
    target.name = name;
    target.uuid = uuid;
    target.pos = pos;
    target.color = Qt::red;
    target.geometry = QPolygonF()
            << QPointF(0, -15)
            << QPointF(5, -5)
            << QPointF(5, 15)
            << QPointF(0, 10)
            << QPointF(-5, 15)
            << QPointF(-5, -5)
            << QPointF(0, -15);
    target.pixmap = new QFixedPixmapItem();
    target.pixmap->setToolTip(name);
    target.pixmap->setZValue(1000000);

    target.numberSize = QSize(12, 12);
    target.numberFont = QFont("Arial", target.numberSize.height() * 3 / 4);
    target.number = new QFixedPixmapItem();
    target.number->setZValue(target.pixmap->zValue() + 1);


    _targets << target;
    return _targets.back();
}

S57_TargetModificator::Target * S57_TargetModificator::targetByName(QString name)
{
    for (QList<Target>::iterator i = _targets.begin(); i != _targets.end(); ++i)
    {
        if (i->name == name)
        {
            return & * i;
        }
    }
    return 0;
}

S57_TargetModificator::Target * S57_TargetModificator::targetByUuid(QUuid uuid)
{
    for (QList<Target>::iterator i = _targets.begin(); i != _targets.end(); ++i)
    {
        if (i->uuid == uuid)
        {
            return & * i;
        }
    }
    return 0;
}

void S57_TargetModificator::removeTarget(const Target * target)
{
    if (!target)
    {
        return;
    }
    for (QList<Target>::iterator i = _targets.begin(); i != _targets.end(); ++i)
    {
        if (& * i == target)
        {
            if (i->pixmap)
            {
                if (i->pixmap->scene())
                {
                    i->pixmap->scene()->removeItem(i->pixmap);
                }
                delete i->pixmap;
                i->pixmap = nullptr;
            }
            if (i->number)
            {
                if (i->number->scene())
                {
                    i->number->scene()->removeItem(i->number);
                }
                delete i->number;
                i->number = nullptr;
            }
            _targets.erase(i);
            return;
        }
    }
}

void S57_TargetModificator::removeTargetByName(QString name)
{
    for (QList<Target>::iterator i = _targets.begin(); i != _targets.end(); ++i)
    {
        if (i->name == name)
        {
            removeTarget(& * i);
            return;
        }
    }
}

void S57_TargetModificator::removeTargetByUuid(QUuid uuid)
{
    for (QList<Target>::iterator i = _targets.begin(); i != _targets.end(); ++i)
    {
        if (i->uuid == uuid)
        {
            removeTarget(& * i);
            return;
        }
    }
}

void S57_TargetModificator::clear()
{
    foreach (Target target, _targets)
    {
        if (target.pixmap)
        {
            if (target.pixmap->scene())
            {
                target.pixmap->scene()->removeItem(target.pixmap);
            }
            delete target.pixmap;
            target.pixmap = 0;
        }
        if (target.number)
        {
            if (target.number->scene())
            {
                scene()->removeItem(target.number);
            }
            delete target.number;
            target.number = 0;
        }
    }
    _targets.clear();
}

QList<S57_TargetModificator::Target> & S57_TargetModificator::targets()
{
    return _targets;
}

const QList<S57_TargetModificator::Target> & S57_TargetModificator::targets() const
{
    return _targets;
}

void S57_TargetModificator::onAddMod()
{
    renderTargets();
}

void S57_TargetModificator::onRemoveMod()
{
    foreach (Target target, _targets)
    {
        if (target.pixmap)
        {
            if (target.pixmap->scene())
            {
                scene()->removeItem(target.pixmap);
            }
        }
        if (target.number)
        {
            if (target.number->scene())
            {
                scene()->removeItem(target.number);
            }
        }
    }
}

void S57_TargetModificator::renderTarget(const Target * target)
{
    if (!isActive())
    {
        return;
    }
    if (!target)
    {
        return;
    }
    if (target->pixmap)
    {
        QPen pen(Qt::black, 1);
        pen.setCosmetic(true);
        QPolygonF geometry = target->geometry;
        QTransform t;
        t.rotate(target->angle);
        geometry = t.map(geometry);
        QRectF r = geometry.boundingRect();
        geometry = geometry.translated(QPointF(- r.x(), - r.y()));
        r = geometry.boundingRect();
        QPixmap px(::ceilf(r.width()) + pen.width(), ::ceilf(r.height()) + pen.width());
        px.fill(QColor(0, 0, 0, 0));
        QPainter p(& px);
        p.setPen(pen);
        p.setBrush(QBrush(target->color));
        p.drawPolygon(geometry);
        if (isActive() && !target->pixmap->scene())
        {
            scene()->addItem(target->pixmap);
        }
        target->pixmap->setPos(target->pos);
        target->pixmap->setScreenSize(px.size());
        target->pixmap->setPixmap(px);
        target->pixmap->setVisible(target->visible);
    }
    if (target->number)
    {
        if (target->number_px.size().width() != (int)(target->numberSize.width() / 0.375) || target->number_px.size().height() != (int)(target->numberSize.height() / 0.375))
        {
            target->number_px = QPixmap(target->numberSize.width() / 0.375, target->numberSize.height() / 0.375);
            target->number->setPixmap(target->number_px);
            target->number->setScreenSize(target->number_px.size());
            const_cast<Target *>(target)->numberFont.setPointSize(target->numberSize.height() * 3 / 4);
        }
        target->number_px.fill(QColor(0, 0, 0, 0));
        QPainter p(& target->number_px);
        p.setFont(target->numberFont);
        p.setPen(target->numberColor);
        QRect frame_rect(QPoint(0, 0), target->numberSize);
        QRect text_rect(1, 1, target->numberSize.width() - 2, target->numberSize.height() - 2);
        const float angle = (target->angle + 90) * M_PI / 180;
        frame_rect.moveCenter(QPoint((target->number_px.width() / 2) + (target->number_px.width() / 4) * ::cos(angle), (target->number_px.height() / 2) + (target->number_px.height() / 4) * ::sin(angle)));
        text_rect.moveCenter(QPoint((target->number_px.width() / 2) + (target->number_px.width() / 4) * ::cos(angle), (target->number_px.height() / 2) + (target->number_px.height() / 4) * ::sin(angle)));
//        p.setBrush(QColor(target->numberColor.red(), target->numberColor.green(), target->numberColor.blue(), 80));
        p.setBrush(QColor(0, 0, 0, 80));
        p.drawRect(frame_rect);
        p.drawText(text_rect, Qt::AlignCenter, QString::number(target->numberValue));
        if (isActive() && !target->number->scene())
        {
            scene()->addItem(target->number);
        }
        target->number->setPixmap(target->number_px);
        target->number->setPos(target->pos);
        target->number->setVisible(target->numberVisible);
    }
}

void S57_TargetModificator::renderTargets()
{
    if (!isActive())
    {
        return;
    }
    for (QList<Target>::const_iterator i = _targets.constBegin(); i != _targets.constEnd(); ++i)
    {
        renderTarget(&(*i));
    }
}



// ---------------------------------------------------------------------------------------------------------




S57_DirectionArrowModificator::S57_DirectionArrowModificator(): S57_ExtendedGraphicsSceneModificator(), dir(0), c(Qt::yellow)
{

}

void S57_DirectionArrowModificator::setDirection(double _d)
{
    dir = _d;
}

double S57_DirectionArrowModificator::direction() const
{
    return dir;
}

void S57_DirectionArrowModificator::setColor(QColor _c)
{
    c = _c;
}

QColor S57_DirectionArrowModificator::color() const
{
    return c;
}

void S57_DirectionArrowModificator::onDrawForeground(QPainter * _painter, const QRectF &)
{
    _painter->save();
    _painter->resetTransform();
    const QRect vrect = view(_painter)->viewport()->rect();
    const double r = qMin(vrect.width() / 2, vrect.height() / 2);
    const QPointF ct = vrect.center();

    QPolygonF poly;
    const int sz = 7;
    poly
            << QPointF(ct.x() + (r - sz) * ::cos(dir * M_PI / 180.0 - M_PI / 2), ct.y() + (r - sz) * ::sin(dir * M_PI / 180.0 - M_PI / 2))
            << QPointF(ct.x() + (r + sz) * ::cos((dir + 2) * M_PI / 180.0 - M_PI / 2), ct.y() + (r + sz) * ::sin((dir + 2) * M_PI / 180.0 - M_PI / 2))
            << QPointF(ct.x() + (r + sz) * ::cos((dir - 2) * M_PI / 180.0 - M_PI / 2), ct.y() + (r + sz) * ::sin((dir - 2) * M_PI / 180.0 - M_PI / 2));

    QPen pen(c);
    pen.setCosmetic(true);
    _painter->setPen(pen);
    _painter->setBrush(c);
    _painter->drawPolygon(poly);

    _painter->restore();
}

// ---------------------------------------------------------------------------------------------------------

S57_SimpleItemModificator::S57_SimpleItemModificator(): S57_ExtendedGraphicsSceneModificator()
{
    p_item = 0;
}

void S57_SimpleItemModificator::setItem(QGraphicsItem & _item)
{
    p_item = & _item;
}

QGraphicsItem & S57_SimpleItemModificator::item() const
{
    return * p_item;
}

void S57_SimpleItemModificator::onAddMod()
{
    if (p_item)
    {
        scene()->addItem(p_item);
    }
}


void S57_SimpleItemModificator::onRemoveMod()
{
    if (p_item)
    {
        scene()->removeItem(p_item);
    }
}
