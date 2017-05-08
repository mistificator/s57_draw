#ifndef S57_EXTENDEDSCENEMODS_H
#define S57_EXTENDEDSCENEMODS_H

#if defined(_WIN32) && defined(LIB_S57_EXT_DLL) && !defined(LIB_S57_EXT_EXPORT)
    #define LIB_S57_EXT_EXPORT __declspec(dllexport)
#else
    #define LIB_S57_EXT_EXPORT
#endif

#include <QGraphicsItem>

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
    QGraphicsView * view();
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

    void setMetrics(int _block_size, int _positions_count);
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
        RadarAll = RadarCircle | RadarSector | RadarCoasts | RadarRaycastCoasts | RadarForeground};
    void setFlags(int);
    int flags() const;
public slots:
    void viewportMoved();
    void addData(QVector<short> & block, int position);

protected:
    virtual void onAddMod();
    virtual void onRemoveMod();
    virtual bool onEvent(QEvent * _ev);
    virtual void onDrawForeground(QPainter * _painter, const QRectF & _rect);
    virtual void onDrawBackground(QPainter * _painter, const QRectF & _rect);
private:
    RadarItem * ri;
    QGraphicsPolygonItem * rpi;
    QRectF roi_rect;
    QPointF roi_point;
    QList<QGraphicsPolygonItem *> all_coasts;
    QMap<double, QList<QGraphicsLineItem *> > ri_sorted_coasts;
    QList<QGraphicsLineItem *> ri_unsorted_coasts, ri_visible_coasts;
    QList<QLineF> ri_sec_lines;
    int radar_flags;
    void updateRoi(QPointF);
    void polarLinePointPrepare(const QLineF & line, float & q, float & r);
    float polarLinePoint(float q, float r, float t);
    float polarLinePoint(const QLineF & line, float t);
    virtual bool eventFilter(QObject *, QEvent *);
};

#include <QUuid>
class QFixedPixmapItemEx;
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
        Target(): angle(0) {}
    private:
        friend class S57_TargetModificator;
        QFixedPixmapItemEx * pixmap;
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

#endif // S57_EXTENDEDSCENEMODS_H

