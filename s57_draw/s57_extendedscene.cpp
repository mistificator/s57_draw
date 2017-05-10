#include "s57_extendedscene.h"
#include <QWidget>
#include <QPainter>
#include <QGraphicsView>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsItem>
#include <QDebug>
#include <cmath>
#include "cpl_port.h"

// -----------------------------------------------------------------------------------------------

struct S57_ExtendedGraphicsScene::Data
{
    S57_ExtendedGraphicsScene * scene;
    S57_BuildScene * builder;
    double aspect;
    QList<S57_ExtendedGraphicsSceneModificator *> mods;
    bool land_and_sea;
    Data(S57_ExtendedGraphicsScene * _scene): scene(_scene), builder(0), aspect(1.0), land_and_sea(false)
    {
    }
    ~Data()
    {
    }
};

S57_ExtendedGraphicsScene::S57_ExtendedGraphicsScene(QObject * _parent): QGraphicsScene(_parent), d(new Data(this))
{
    installEventFilter(this);
    connect(this, SIGNAL(changed(QList<QRectF>)), this, SIGNAL(scaleChanged()));
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
            _mod->setActive(true);
            _mod->onAddMod();
        }
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
        _mod->setActive(false);
    }
    invalidate();
    update();
}

QList<S57_ExtendedGraphicsSceneModificator *> S57_ExtendedGraphicsScene::mods() const
{
    return d->mods;
}

void S57_ExtendedGraphicsScene::drawForeground(QPainter * _painter, const QRectF & _rect)
{
    foreach (S57_ExtendedGraphicsSceneModificator * _mod, d->mods)
    {
        _mod->onDrawForeground(_painter, _rect);
    }
}

void S57_ExtendedGraphicsScene::drawBackground(QPainter * _painter, const QRectF & _rect)
{
    foreach (S57_ExtendedGraphicsSceneModificator * _mod, d->mods)
    {
        _mod->onDrawBackground(_painter, _rect);
    }
}

bool S57_ExtendedGraphicsScene::eventFilter(QObject * _obj, QEvent * _ev)
{
    switch (_ev->type())
    {
        case QEvent::KeyPress:
        {
            QKeyEvent * _ke = (QKeyEvent *)_ev;
            if (view())
            {
                view()->setUpdatesEnabled(false);
                if (_ke->key() == Qt::Key_Plus)
                {
                    view()->scale(1.25, 1.25);
                    //        qDebug() << currentScale();
                    updateVisibility();
                }
                if (_ke->key() == Qt::Key_Minus)
                {
                    view()->scale(0.8, 0.8);
                    //        qDebug() << currentScale();
                    updateVisibility();
                }
                view()->setUpdatesEnabled(true);
            }
        }
        break;
        case QEvent::GraphicsSceneWheel:
        {
            QGraphicsSceneWheelEvent * _we = (QGraphicsSceneWheelEvent *)_ev;
           if (view())
           {
                view()->setUpdatesEnabled(false);
                double _k = 0;
                const int _d = _we->delta();
                if (_d < 0)
                {
                    _k = 0.8;
                }
                if (_d > 0)
                {
                    _k = 1.25;
                }
                if (_d != 0)
                {
                    view()->scale(_k, _k);
                    //                qDebug() << currentScale();
                    updateVisibility();
                }
                view()->setUpdatesEnabled(true);
           }
        }
        break;
    }

    foreach (S57_ExtendedGraphicsSceneModificator * _mod, d->mods)
    {
        _mod->onEvent(_ev);
    }

    return QGraphicsScene::event(_ev);
}

void S57_ExtendedGraphicsScene::registerBuildScene(S57_BuildScene * _builder)
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

void S57_ExtendedGraphicsScene::setBuilder(S57_BuildScene * _builder)
{
    registerBuildScene(_builder);
    d->builder->setScene(this);
}

S57_BuildScene * S57_ExtendedGraphicsScene::builder() const
{
    return d->builder;
}

QGraphicsView * S57_ExtendedGraphicsScene::view() const
{
    return (views().isEmpty() ? 0 : views().at(0));
}

void S57_ExtendedGraphicsScene::setLandAndSeaOnly(bool state)
{
    d->land_and_sea = state;
    updateVisibility();
}

bool S57_ExtendedGraphicsScene::isLandAndSeaOnly() const
{
    return d->land_and_sea;
}

void S57_ExtendedGraphicsScene::updateVisibility()
{
    if (d->builder == 0)
    {
        return;
    }
    if (d->land_and_sea)
    {
        d->builder->updateVisibility(0x7fffffff, true);
    }
    else
    {
        d->builder->updateVisibility(builder()->scaleTo1());
    }
    emit scaleChanged();
}

void S57_ExtendedGraphicsScene::scale(double _scale)
{
    if (view() == 0)
    {
        return;
    }
    view()->setUpdatesEnabled(false);
    view()->scale(_scale, _scale);
    updateVisibility();
    view()->setUpdatesEnabled(true);
}

void S57_ExtendedGraphicsScene::scaleTo1(double _scale)
{
     scale(d->builder->scaleTo1(_scale));
}

void S57_ExtendedGraphicsScene::fullView()
{
    if (view() == 0)
    {
        return;
    }
    view()->setUpdatesEnabled(false);
    d->builder->fullView();
    view()->scale(1, d->aspect);
    updateVisibility();
    view()->setUpdatesEnabled(true);
}

void S57_ExtendedGraphicsScene::setAspect(double aspect)
{
    const double _scale = aspect / d->aspect;
    d->aspect = aspect;
    if (view() == 0)
    {
        return;
    }
    view()->setUpdatesEnabled(false);
    view()->scale(1, _scale);
    updateVisibility();
    view()->setUpdatesEnabled(true);
}

double S57_ExtendedGraphicsScene::aspect() const
{
    return d->aspect;
}

