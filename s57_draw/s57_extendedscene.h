#ifndef S57_EXTENDEDSCENE_H
#define S57_EXTENDEDSCENE_H

#if defined(_WIN32) && defined(LIB_S57_EXT_DLL) && !defined(LIB_S57_EXT_EXPORT)
    #define LIB_S57_EXT_EXPORT __declspec(dllexport)
#else
    #define LIB_S57_EXT_EXPORT
#endif

#include "s57_buildscene.h"
#include <QGraphicsItem>

class LIB_S57_EXT_EXPORT S57_ExtendedBuildScene: public S57_BuildScene
{
    S57_ExtendedBuildScene(const S57_ExtendedBuildScene &) {}
    S57_ExtendedBuildScene & operator = (const S57_ExtendedBuildScene &) { return (* this); }
public:
    S57_ExtendedBuildScene();
    S57_ExtendedBuildScene(const QString & _lookups_path, const QString & _symbols_path);
    virtual void setScene(QGraphicsScene * _scene);
};

class S57_ExtendedGraphicsScene;
class LIB_S57_EXT_EXPORT S57_ExtendedGraphicsSceneModificator
{
    S57_ExtendedBuildScene * builder_ptr;
    S57_ExtendedGraphicsScene * scene_ptr;
public:
    void registerBuildScene(S57_ExtendedBuildScene * _builder);
    S57_ExtendedBuildScene * builder() const;
    S57_ExtendedGraphicsScene * scene() const;
    static QGraphicsView * view(QPainter * _painter);
    QGraphicsView * view();
    virtual ~S57_ExtendedGraphicsSceneModificator();
    virtual void onDrawForeground(QPainter *, const QRectF &);
    virtual bool onEvent(QEvent *);
    virtual void onAddMod();
    virtual void onRemoveMod();
};

class LIB_S57_EXT_EXPORT S57_GridModificator: public S57_ExtendedGraphicsSceneModificator
{
public:
    virtual void onDrawForeground(QPainter * _painter, const QRectF & _rect);
};

class LIB_S57_EXT_EXPORT S57_RulerModificator: public S57_ExtendedGraphicsSceneModificator
{
    QGraphicsEllipseItem * circle[3];
    bool in_ruler;
public:
    S57_RulerModificator();
    virtual bool onEvent(QEvent * _ev);
};

class RadarItem;
class LIB_S57_EXT_EXPORT S57_RadarModificator: public QObject, public S57_ExtendedGraphicsSceneModificator
{
    Q_OBJECT
public:
    S57_RadarModificator();

    void setMetrics(int _portion_size, int _positions_count);
    int blockSize() const;
    int positionsCount() const;

    QImage roi(double start_angle, double stop_angle, int start_distance, int stop_distance) const; // angles in radians, 0 <= angle <= 2 * M_PI
    QRectF roiRect() const;
    QImage roi() const;

    QMap<double, QList<QGraphicsLineItem *> > coasts() const;
    QList<QGraphicsLineItem *> visibleCoasts() const;
    QList<QLineF> visibleCoastsLines() const;

    virtual void onAddMod();
    virtual void onRemoveMod();
    virtual bool onEvent(QEvent * _ev);

public slots:
    void viewportMoved();
    void addData(const QVector<short> & block, int position);

private:
    RadarItem * ri;
    QGraphicsPolygonItem * rpi;
    QRectF roi_rect;
    QPointF roi_point;
    QList<QGraphicsPolygonItem *> all_coasts;
    QMap<double, QList<QGraphicsLineItem *> > ri_coasts;
    QList<QGraphicsLineItem *> ri_visible_coasts;
    QList<QLineF> ri_sec_lines;
    void updateRoi(QPointF);
};

class LIB_S57_EXT_EXPORT S57_ExtendedGraphicsScene: public QGraphicsScene
{
    friend class S57_ExtendedBuildScene;
    S57_ExtendedGraphicsScene(const S57_ExtendedGraphicsScene &) {}
    S57_ExtendedGraphicsScene & operator = (const S57_ExtendedGraphicsScene &) { return (* this); }
public:
    S57_ExtendedGraphicsScene(QObject * _parent = 0);
    ~S57_ExtendedGraphicsScene();

    void addMod(S57_ExtendedGraphicsSceneModificator * _mod);
    void removeMod(S57_ExtendedGraphicsSceneModificator * _mod);
    QList<S57_ExtendedGraphicsSceneModificator *> mods() const;

    void setAspect(double);
    double aspect() const;

    S57_ExtendedBuildScene * builder() const;
protected:
    virtual void drawForeground(QPainter * _painter, const QRectF & _rect);
    virtual bool event(QEvent * _ev);
    void registerBuildScene(S57_ExtendedBuildScene *);
private:
    struct Data;
    Data * d;
};

#endif // S57_EXTENDEDSCENE_H
