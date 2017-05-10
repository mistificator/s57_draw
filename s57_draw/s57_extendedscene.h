#ifndef S57_EXTENDEDSCENE_H
#define S57_EXTENDEDSCENE_H

#if defined(_WIN32) && defined(LIB_S57_EXT_DLL) && !defined(LIB_S57_EXT_EXPORT)
    #define LIB_S57_EXT_EXPORT __declspec(dllexport)
#else
    #define LIB_S57_EXT_EXPORT
#endif

#include "s57_buildscene.h"
#include "s57_extendedscenemods.h"

class LIB_S57_EXT_EXPORT S57_ExtendedGraphicsScene: public QGraphicsScene
{
    Q_OBJECT
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

    void setBuilder(S57_BuildScene * _builder);
    S57_BuildScene * builder() const;
    QGraphicsView * view() const;

    void setLandAndSeaOnly(bool);
    bool isLandAndSeaOnly() const;

    void scale(double _scale);
    void scaleTo1(double _scale);
public slots:
    void fullView();
signals:
    void scaleChanged();
protected:
    virtual void drawForeground(QPainter * _painter, const QRectF & _rect);
    virtual void drawBackground(QPainter * _painter, const QRectF & _rect);
    virtual bool eventFilter(QObject * _obj, QEvent * _ev);
    void updateVisibility();
private slots:
    void registerBuildScene(S57_BuildScene *);
    void sceneChanged();
private:
    struct Data;
    Data * d;
};

#endif // S57_EXTENDEDSCENE_H
