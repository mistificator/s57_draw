#ifndef S52_FIXEDITEM
#define S52_FIXEDITEM

#if defined(_WIN32) && defined(LIB_S57_DLL) && !defined(LIB_S57_EXPORT)
    #define LIB_S57_EXPORT __declspec(dllexport)
#elif !defined(LIB_S57_EXPORT)
    #define LIB_S57_EXPORT
#endif

#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QPixmap>
#include <QSize>
#include <QImage>

class LIB_S57_EXPORT QFixedPixmapItem: public QGraphicsItem
{
public:
    QFixedPixmapItem(QGraphicsItem *parent = 0);
    void setScreenSize(const QSize & _size);
    QSize screenSize() const;
    void setPixmap(const QPixmap & _pixmap);
    void setPixmap(const QPixmap * _pixmap);
    void setImage(const QImage & _image);
    void setImage(const QImage * _image);
    QRectF boundingRect() const;
    void paint(QPainter * _painter, const QStyleOptionGraphicsItem * /*_option*/, QWidget * /*_widget*/);
private:
    QSize screen_size;
    QPixmap pixmap;
    QPixmap * pixmap_ptr;
    QImage image;
    QImage * image_ptr;
    enum {Pixmap = 0, Pixmap_Ptr, Image, Image_Ptr};
    int src_type;
    QRectF bound_rect;
    QRectF calcBoundingRect() const;
    void updateBoundingRect();
};

typedef QFixedPixmapItem S52FixedItem;

#endif // S52_FIXEDITEM

