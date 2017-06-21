#include "s52_fixeditem.h"
#include <QGraphicsScene>
#include <QGraphicsView>

QFixedPixmapItem::QFixedPixmapItem(QGraphicsItem *parent): QGraphicsItem(parent), pixmap_ptr(0), image_ptr(0), src_type(Pixmap)
{
    setScreenSize(QSize(10, 10));
}

void QFixedPixmapItem::setScreenSize(const QSize & _size)
{
    prepareGeometryChange();
    screen_size = _size;
}

QSize QFixedPixmapItem::screenSize() const
{
    return screen_size;
}

void QFixedPixmapItem::setPixmap(const QPixmap & _pixmap)
{
    prepareGeometryChange();
    pixmap = _pixmap;
    src_type = Pixmap;
}

void QFixedPixmapItem::setPixmap(const QPixmap * _pixmap)
{
    prepareGeometryChange();
    pixmap_ptr = (QPixmap *)_pixmap;
    src_type = Pixmap_Ptr;
}

void QFixedPixmapItem::setImage(const QImage & _image)
{
    prepareGeometryChange();
    image = _image;
    src_type = Image;
}

void QFixedPixmapItem::setImage(const QImage * _image)
{
    prepareGeometryChange();
    image_ptr = (QImage *)_image;
    src_type = Image_Ptr;
}

QRectF QFixedPixmapItem::boundingRect() const
{
    QRectF _r;
    if (scene() && (!scene()->views().isEmpty()))
    {
        if (QGraphicsView * _view = scene()->views().front())
        {
            QPoint _pt = _view->mapFromScene(scenePos());
            _r = mapFromScene(_view->mapToScene(QRect(QPoint(_pt.x() - screen_size.width() / 2, _pt.y() - screen_size.height() / 2), screen_size))).boundingRect();
            if (_r.size() == screen_size)
            {
                return (QRectF());
            }
        }
    }
    return (_r);
}

void QFixedPixmapItem::paint(QPainter * _painter, const QStyleOptionGraphicsItem * /*_option*/, QWidget * /*_widget*/)
{
    switch (src_type)
    {
    case Pixmap:
        if (!pixmap.isNull())
        {
            _painter->drawPixmap(boundingRect(), pixmap, pixmap.rect());
        }
        break;
    case Pixmap_Ptr:
        if (pixmap_ptr && !pixmap_ptr->isNull())
        {
            _painter->drawPixmap(boundingRect(), * pixmap_ptr, pixmap_ptr->rect());
        }
        break;
    case Image:
        if (!image.isNull())
        {
            _painter->drawImage(boundingRect(), image, image.rect());
        }
        break;
    case Image_Ptr:
        if (image_ptr && !image_ptr->isNull())
        {
            _painter->drawImage(boundingRect(), * image_ptr, image_ptr->rect());
        }
        break;
    }
}


