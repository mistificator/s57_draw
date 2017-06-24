#ifndef S57_EXTENDEDSCENEMODS_H
#define S57_EXTENDEDSCENEMODS_H

#if defined(_WIN32) && defined(LIB_S57_EXT_DLL) && !defined(LIB_S57_EXT_EXPORT)
    #define LIB_S57_EXT_EXPORT __declspec(dllexport)
#elif !defined(LIB_S57_EXT_EXPORT)
    #define LIB_S57_EXT_EXPORT
#endif

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QGraphicsSimpleTextItem>
#include <QPixmap>
#include <qmath.h>

extern float sin_table_deg[10800], cos_table_deg[10800];
extern float sin_table_rad[(int)(6 * M_PI * 1000)], cos_table_rad[(int)(6 * M_PI * 1000)];
// angle to table index convert
#define R2T(angle_in_radians) (int)((angle_in_radians + 2 * M_PI) * 1000)
#define D2T(angle_in_degrees) (int)((angle_in_degrees + 360) * 10)

class S57_BuildScene;
class S57_ExtendedGraphicsScene;
class LIB_S57_EXT_EXPORT S57_ExtendedGraphicsSceneModificator
{
    friend class S57_ExtendedGraphicsScene;
public:
    S57_ExtendedGraphicsSceneModificator();
    bool isActive() const;
    S57_BuildScene * builder() const;
    S57_ExtendedGraphicsScene * scene() const;
    static QGraphicsView * view(QPainter * _painter);
    QGraphicsView * view() const;
    virtual ~S57_ExtendedGraphicsSceneModificator();
protected:
    virtual void onDrawForeground(QPainter *, const QRectF &);
    virtual void onDrawBackground(QPainter *, const QRectF &);
    virtual bool onEvent(QEvent *);
    virtual void onAddMod();
    virtual void onRemoveMod();
private:
    S57_BuildScene * builder_ptr;
    S57_ExtendedGraphicsScene * scene_ptr;
    bool is_active;
    friend class S57_ExtendedGraphicsScene;
    void setActive(bool);
    void registerBuildScene(S57_BuildScene * _builder);
};

class LIB_S57_EXT_EXPORT S57_GridModificator: public S57_ExtendedGraphicsSceneModificator
{
public:
    static QString xLabel(double val);
    static QString yLabel(double val);
protected:
    virtual void onDrawForeground(QPainter * _painter, const QRectF & _rect);
};

class LIB_S57_EXT_EXPORT S57_RulerModificator: public S57_ExtendedGraphicsSceneModificator
{
public:
    S57_RulerModificator();
protected:
    virtual bool onEvent(QEvent * _ev);
private:
    QGraphicsEllipseItem * circle[3];
    bool in_ruler;
};

class RadarItem;
class LIB_S57_EXT_EXPORT S57_RadarModificator: public QObject, public S57_ExtendedGraphicsSceneModificator
{
    Q_OBJECT
public:
    S57_RadarModificator();

    int blockSize() const;
    int positionsCount() const;
    int screenRadius() const;

    QImage roi(double start_angle, double stop_angle, int start_distance, int stop_distance) const; // angles in radians, 0 <= angle <= 2 * M_PI
    QRectF roiRect() const;
    QImage roi() const;

    QList<QGraphicsLineItem *> coasts() const;
    QList<QGraphicsLineItem *> raycastCoasts() const;
    QList<QLineF> raycastCoastsLines() const;

    enum RadarFlags {
        RadarCircle =           1,
        RadarSector =           2,
        RadarCoasts =           4,
        RadarRaycastCoasts =    8,
        RadarForeground =       16,
        RadarSectorSelector =   32,
        RadarLimb =             64,
        RadarRay =              128,
        RadarAll = RadarCircle | RadarSector | RadarCoasts | RadarRaycastCoasts | RadarForeground | RadarSectorSelector | RadarLimb | RadarRay};
    void setFlags(int);
    int flags() const;

    void setCustomRadarCenter(QPointF);
    QPointF radarCenter() const;
    double radarRadius() const;
public slots:
    void setMetrics(int _block_size, int _positions_count);
    void setRoiSize(int _d_radius, int _d_angle); // d_radius in 0 .. blockSize(), d_angle in 0 .. positionsCount()
    int roiRadius() const;
    int roiAngle() const;
    void setRoiPos(QPointF);
    QPointF roiPos() const;
    QPolygonF roiSelector() const;
    void viewportMoved();
    void addData(QVector<short> & block, int position, const unsigned char coast_color = 0xaf); // block size == blockSize()
    void addData(short * block, int position, const unsigned char coast_color = 0xaf); // block size == blockSize()
    void updateRoi();
protected:
    virtual void onAddMod();
    virtual void onRemoveMod();
    virtual bool onEvent(QEvent * _ev);
    virtual void onDrawForeground(QPainter * _painter, const QRectF & _rect);
    virtual void onDrawBackground(QPainter * _painter, const QRectF & _rect);
private:
    RadarItem * ri;
    QGraphicsPolygonItem * rpi;
    QGraphicsLineItem * rli;
    QRectF roi_rect;
    QPointF roi_point;
    QList<QGraphicsPolygonItem *> all_coasts;
    QMap<double, QList<QGraphicsLineItem *> > ri_sorted_coasts;
    QList<QGraphicsLineItem *> ri_unsorted_coasts, ri_visible_coasts;
    QList<QLineF> ri_sec_lines;
    QPointF custom_ct;
    int radar_flags;
    int d_radius, d_angle;
    void updateRoi(QPointF &);
    void polarLinePointPrepare(const QLineF & line, float & q, float & r);
    float polarLinePoint(float q, float r, float t);
    float polarLinePoint(const QLineF & line, float t);
    virtual bool eventFilter(QObject *, QEvent *);
};

#include <QUuid>
class QFixedPixmapItem;
class LIB_S57_EXT_EXPORT S57_TargetModificator: public QObject, public S57_ExtendedGraphicsSceneModificator
{
    Q_OBJECT
public:
    struct Target
    {
        QString name;
        QUuid uuid;
        QColor color;
        QPolygonF geometry;
        QList<QPointF> path;
        QPointF pos;
        float angle; // degrees, ccw
        bool visible;
        int numberValue;
        QFont numberFont;
        QColor numberColor;
        QSize numberSize;
        bool numberVisible;
        Target(): angle(0), numberVisible(false), numberColor(Qt::white), numberValue(1), visible(true) {}
    private:
        friend class S57_TargetModificator;
        QFixedPixmapItem * pixmap;
        QFixedPixmapItem * number;
        mutable QPixmap number_px;
    };
    S57_TargetModificator();
    ~S57_TargetModificator();
    Target & addTarget(QPointF pos, QString name, const QUuid & uuid = QUuid::createUuid());
    Target * targetByName(QString name);
    Target * targetByUuid(QUuid uuid);
    void removeTarget(const Target * target);
    void removeTargetByName(QString name);
    void removeTargetByUuid(QUuid uuid);
    void clear();
    QList<Target> & targets();
    const QList<Target> & targets() const;
public slots:
    void renderTarget(const Target * target);
    void renderTargets();
protected:
    virtual void onAddMod();
    virtual void onRemoveMod();
private:
    QList<Target> _targets;
};

class LIB_S57_EXT_EXPORT S57_DirectionArrowModificator: public S57_ExtendedGraphicsSceneModificator
{
public:
    S57_DirectionArrowModificator();
    void setDirection(double); // degree, cw
    double direction() const;
    void setColor(QColor);
    QColor color() const;
protected:
    virtual void onDrawForeground(QPainter *, const QRectF &);
private:
    double dir;
    QColor c;
};

class LIB_S57_EXT_EXPORT S57_SimpleItemModificator: public S57_ExtendedGraphicsSceneModificator
{
public:
    S57_SimpleItemModificator();
    void setItem(QGraphicsItem & _item); // user is resposible for _item destiny, modificator doesn't own it
    QGraphicsItem & item() const;
protected:
    virtual void onAddMod();
    virtual void onRemoveMod();
private:
    QGraphicsItem * p_item;
};

#endif // S57_EXTENDEDSCENEMODS_H

