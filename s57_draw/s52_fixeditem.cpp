#include "s52_fixeditem.h"

#include <QGraphicsScene>
#include <QGraphicsView>

QFixedPixmapItem::QFixedPixmapItem(QGraphicsItem *parent):
    QGraphicsItem(parent), pixmap_ptr(0), image_ptr(0), src_type(Pixmap)
{
    setScreenSize(QSize(10, 10));
}

QRectF QFixedPixmapItem::calcBoundingRect() const
{
    if (scene() && (!scene()->views().isEmpty()))
    {
        if (QGraphicsView * _view = scene()->views().front())
        {
            // Вычисляем габариты в единицах сцены без погрешности округления scenePos() до QPoint
            const QRectF _scene_r = _view->mapToScene(QRect(QPoint(0, 0), screen_size)).boundingRect();
            if (_scene_r.size() == QSizeF(screen_size))
            {
                return QRectF();
            }
            const double _w = _scene_r.width();
            const double _h = _scene_r.height();
            return QRectF(-_w / 2.0, -_h / 2.0, _w, _h);
        }
    }
    return QRectF();
}

void QFixedPixmapItem::updateBoundingRect()
{
    const QRectF _new_r = calcBoundingRect();
    if (_new_r != bound_rect)
    {
        // На момент вызова prepareGeometryChange() метод boundingRect() еще возвращает
        // старый bound_rect, поэтому BSP-дерево корректно находит и отвязывает старые узлы
        prepareGeometryChange();
        bound_rect = _new_r;
    }
}

void QFixedPixmapItem::setScreenSize(const QSize & _size)
{
    screen_size = _size;
    updateBoundingRect();
}

QSize QFixedPixmapItem::screenSize() const
{
    return screen_size;
}

void QFixedPixmapItem::setPixmap(const QPixmap & _pixmap)
{
    pixmap = _pixmap;
    src_type = Pixmap;
    updateBoundingRect();
}

void QFixedPixmapItem::setPixmap(const QPixmap * _pixmap)
{
    pixmap_ptr = (QPixmap *)_pixmap;
    src_type = Pixmap_Ptr;
    updateBoundingRect();
}

void QFixedPixmapItem::setImage(const QImage & _image)
{
    image = _image;
    src_type = Image;
    updateBoundingRect();
}

void QFixedPixmapItem::setImage(const QImage * _image)
{
    image_ptr = (QImage *)_image;
    src_type = Image_Ptr;
    updateBoundingRect();
}

QRectF QFixedPixmapItem::boundingRect() const
{
    return bound_rect;
}

void QFixedPixmapItem::paint(QPainter * _painter, const QStyleOptionGraphicsItem * /*_option*/, QWidget * /*_widget*/)
{
    const QRectF _r = calcBoundingRect();
    if (_r.isEmpty())
    {
        return;
    }
    switch (src_type)
    {
    case Pixmap:
        if (!pixmap.isNull())
        {
            _painter->drawPixmap(_r, pixmap, pixmap.rect());
        }
        break;
    case Pixmap_Ptr:
        if (pixmap_ptr && !pixmap_ptr->isNull())
        {
            _painter->drawPixmap(_r, *pixmap_ptr, pixmap_ptr->rect());
        }
        break;
    case Image:
        if (!image.isNull())
        {
            _painter->drawImage(_r, image, image.rect());
        }
        break;
    case Image_Ptr:
        if (image_ptr && !image_ptr->isNull())
        {
            _painter->drawImage(_r, *image_ptr, image_ptr->rect());
        }
        break;
    }
}
