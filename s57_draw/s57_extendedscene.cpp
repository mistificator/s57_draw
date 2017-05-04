#include "s57_extendedscene.h"
#include <QWidget>
#include <QPainter>
#include <QGraphicsView>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QDebug>
#include <cmath>
#include "cpl_port.h"

S57_ExtendedBuildScene::S57_ExtendedBuildScene(): S57_BuildScene()
{
}

S57_ExtendedBuildScene::S57_ExtendedBuildScene(const QString & _lookups_path, const QString & _symbols_path): S57_BuildScene(_lookups_path, _symbols_path)
{
}

void S57_ExtendedBuildScene::setScene(QGraphicsScene * _scene)
{
    S57_BuildScene::setScene(_scene);
    if (S57_ExtendedGraphicsScene * ext_scene = dynamic_cast<S57_ExtendedGraphicsScene *>(_scene))
    {
        ext_scene->registerBuildScene(this);
    }
}

// -----------------------------------------------------------------------------------------------

struct S57_ExtendedGraphicsScene::Data
{
    S57_ExtendedGraphicsScene * scene;
    S57_ExtendedBuildScene * builder;
    double aspect;
    QList<S57_ExtendedGraphicsSceneModificator *> mods;
    Data(S57_ExtendedGraphicsScene * _scene): scene(_scene), builder(0), aspect(1.0)
    {
    }
    ~Data()
    {
    }
};

S57_ExtendedGraphicsScene::S57_ExtendedGraphicsScene(QObject * _parent): QGraphicsScene(_parent), d(new Data(this))
{

}

S57_ExtendedGraphicsScene::~S57_ExtendedGraphicsScene()
{
    delete d;
}

void S57_ExtendedGraphicsScene::addMod(S57_ExtendedGraphicsSceneModificator * _mod)
{
    if (!d->mods.contains(_mod))
    {
        if (d->builder)
        {
            _mod->registerBuildScene(d->builder);
        }
        _mod->onAddMod();
        d->mods << _mod;
    }
    invalidate();
    update();
}

void S57_ExtendedGraphicsScene::removeMod(S57_ExtendedGraphicsSceneModificator * _mod)
{
    if (d->mods.removeOne(_mod))
    {
        _mod->onRemoveMod();
    }
    invalidate();
    update();
}

QList<S57_ExtendedGraphicsSceneModificator *> S57_ExtendedGraphicsScene::mods() const
{
    return d->mods;
}

void S57_ExtendedGraphicsScene::setAspect(double aspect)
{
    d->aspect = aspect;
}

double S57_ExtendedGraphicsScene::aspect() const
{
    return d->aspect;
}

void S57_ExtendedGraphicsScene::drawForeground(QPainter * _painter, const QRectF & _rect)
{
    foreach (S57_ExtendedGraphicsSceneModificator * _mod, d->mods)
    {
        _mod->onDrawForeground(_painter, _rect);
    }
}

bool S57_ExtendedGraphicsScene::event(QEvent * _ev)
{
    foreach (S57_ExtendedGraphicsSceneModificator * _mod, d->mods)
    {
        _mod->onEvent(_ev);
    }
    return QGraphicsScene::event(_ev);
}

void S57_ExtendedGraphicsScene::registerBuildScene(S57_ExtendedBuildScene * _builder)
{
    d->builder = _builder;
    if (d->builder)
    {
        foreach (S57_ExtendedGraphicsSceneModificator * _mod, d->mods)
        {
            _mod->registerBuildScene(d->builder);
        }
    }
}

S57_ExtendedBuildScene * S57_ExtendedGraphicsScene::builder() const
{
    return d->builder;
}
