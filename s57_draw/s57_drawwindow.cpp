#include "s57_drawwindow.h"
#include "ui_S57_DrawWindow.h"

#include <QFileDialog>
#include <QGraphicsItem>
#include <QGraphicsSceneEvent>
#include <QFileInfo>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>
#include <qmath.h>

#include "s52_symbol.h"

S57_DrawWindow::S57_DrawWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::S57_DrawWindow), scene(0), progress(0), count(0), aspect(Aspect11)
{
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(position_label = new QLabel(), 1);
    QFrame * _f;
    _f = new QFrame();
    _f->setFrameStyle(QFrame::VLine);
    ui->statusbar->addPermanentWidget(_f);
    ui->statusbar->addPermanentWidget(scale_label = new QLabel(), 1);
    _f = new QFrame();
    _f->setFrameStyle(QFrame::VLine);
    ui->statusbar->addPermanentWidget(_f);

    ui->actionColorScheme->setMenu(new QMenu());
    QActionGroup * _ag = new QActionGroup(this);

    QString _def_scheme = QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("scheme", builder.s52().colorScheme()).toString();
    foreach (const QString & _scheme, builder.s52().colorSchemes())
    {
        QAction * _action = ui->actionColorScheme->menu()->addAction(_scheme, this, SLOT(colorSchemeChanged()));
        _action->setActionGroup(_ag);
        _action->setCheckable(true);
        if (_scheme == _def_scheme)
        {
            _action->setChecked(true);
        }
    }
    ui->actionLandAndSea->setChecked(QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("land_and_sea", false).toBool());
    ui->actionMouse_dragging->setChecked(QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("drag", true).toBool());
    on_actionMouse_dragging_triggered(ui->actionMouse_dragging->isChecked());

    _ag = new QActionGroup(this);
    ui->action_aspect_1_1->setActionGroup(_ag);
    ui->action_aspect_1_2->setActionGroup(_ag);
    aspect = QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("aspect", Aspect12).toInt();
    if (aspect == Aspect11)
    {
        ui->action_aspect_1_1->setChecked(true);
        on_action_aspect_1_1_triggered();
    }
    if (aspect == Aspect12)
    {
        ui->action_aspect_1_2->setChecked(true);
        on_action_aspect_1_2_triggered();
    }

    circle[0] = 0;
    circle[1] = 0;
    circle[2] = 0;
}

S57_DrawWindow::~S57_DrawWindow()
{
    delete ui;
}

void S57_DrawWindow::recreateScene()
{
    if (scene)
    {
        builder.clear();
        scene->deleteLater();
    }
    scene = new QGraphicsScene();
    scene->installEventFilter(this);
    ui->S57View->setScene(scene);
    builder.setScene(scene);
    ui->S57View->resetMatrix();
    ui->S57View->resetTransform();
}

void S57_DrawWindow::on_actionOpen_triggered()
{
    QString _fn = QFileDialog::getOpenFileName(
                this,
                "Open S57 chart",
                QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("path", qApp->applicationDirPath()).toString(),
                "*.000"
                );
    if (_fn.isEmpty())
    {
        return;
    }
    QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("path", QFileInfo(_fn).absolutePath());
    ui->S57View->setUpdatesEnabled(false);
    recreateScene();
    builder.build(_fn);
    builder.render();
    scene->setSceneRect(QRectF());
    double _init_scale = qMin<double>(ui->S57View->viewport()->width() / scene->sceneRect().width(), ui->S57View->viewport()->height() / scene->sceneRect().height());
    ui->S57View->scale(_init_scale, aspect * _init_scale);
    updateVisibility();
    circle[0] = scene->addEllipse(QRectF(), Qt::NoPen, QColor(0, 255, 0, 40));
    circle[0]->setVisible(false);
    circle[1] = scene->addEllipse(QRectF(), Qt::NoPen, QColor(0, 255, 0, 80));
    circle[1]->setVisible(false);
    circle[2] = scene->addEllipse(QRectF(), Qt::NoPen, Qt::red);
    circle[2]->setVisible(false);
    ui->S57View->setUpdatesEnabled(true);
    ui->actionAdd->setEnabled(true);
    ui->actionAdd_folder->setEnabled(true);
}


void S57_DrawWindow::on_actionAdd_triggered()
{
    QString _fn = QFileDialog::getOpenFileName(
                this,
                "Add S57 chart",
                QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("path", qApp->applicationDirPath()).toString(),
                "*.000"
                );
    if (_fn.isEmpty())
    {
        return;
    }
    QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("path", QFileInfo(_fn).absolutePath());
    ui->S57View->setUpdatesEnabled(false);
    builder.build(_fn);
    builder.render();
    updateVisibility();
    ui->S57View->setUpdatesEnabled(true);
}

void S57_DrawWindow::nextDir(const QString & _dir, bool _scan)
{
    if (progress && progress->wasCanceled())
    {
        return;
    }
    QStringList _files = QDir(_dir).entryList(QStringList() << "*.000");
    foreach (const QString & _fn, _files)
    {
        if (!_scan)
        {
            builder.build(_dir + "/" + _fn);
            if (progress)
            {
                progress->setLabelText(_dir);
                progress->setValue(count);
            }
            qApp->processEvents();
        }
        count++;
    }
    QStringList _dirs = QDir(_dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (_dirs.isEmpty())
    {
        return;
    }
    foreach (const QString & _dn, _dirs)
    {
        nextDir(_dir + "/" + _dn, _scan);
    }
}

void S57_DrawWindow::on_actionOpen_folder_triggered()
{
    QString _dir = QFileDialog::getExistingDirectory(this, "Open folder with S57 charts", QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("path", qApp->applicationDirPath()).toString());
    if (_dir.isEmpty())
    {
        return;
    }
    QDir _dir_up = QDir(_dir);
    _dir_up.cd("..");
    QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("path", _dir_up.absolutePath());
    nextDir(_dir, true);
    ui->S57View->setUpdatesEnabled(false);
    recreateScene();
    progress = new QProgressDialog(_dir, "Stop", 0, count, this);
    progress->setWindowTitle("Scanning folder");
    progress->setWindowModality(Qt::WindowModal);
    count = 0;
    nextDir(_dir);
    builder.render();
    scene->setSceneRect(QRectF());
    double _init_scale = qMin<double>(ui->S57View->viewport()->width() / scene->sceneRect().width(), ui->S57View->viewport()->height() / scene->sceneRect().height());
    ui->S57View->scale(_init_scale, aspect * _init_scale);
    updateVisibility();
    circle[0] = scene->addEllipse(QRectF(), Qt::NoPen, QColor(0, 255, 0, 40));
    circle[0]->setVisible(false);
    circle[1] = scene->addEllipse(QRectF(), Qt::NoPen, QColor(0, 255, 0, 80));
    circle[1]->setVisible(false);
    circle[2] = scene->addEllipse(QRectF(), Qt::NoPen, Qt::red);
    circle[2]->setVisible(false);
    progress->close();
    progress->deleteLater();
    progress = 0; count = 0;
    ui->S57View->setUpdatesEnabled(true);
    ui->actionAdd->setEnabled(true);
    ui->actionAdd_folder->setEnabled(true);
}


void S57_DrawWindow::on_actionAdd_folder_triggered()
{
    QString _dir = QFileDialog::getExistingDirectory(this, "Open folder with S57 charts", QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("path", qApp->applicationDirPath()).toString());
    if (_dir.isEmpty())
    {
        return;
    }
    QDir _dir_up = QDir(_dir);
    _dir_up.cd("..");
    QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("path", _dir_up.absolutePath());
    nextDir(_dir, true);
    ui->S57View->setUpdatesEnabled(false);
    progress = new QProgressDialog(_dir, "Stop", 0, count, this);
    progress->setWindowTitle("Scanning folder");
    progress->setWindowModality(Qt::WindowModal);
    count = 0;
    nextDir(_dir);
    builder.render();
    updateVisibility();
    progress->close();
    progress->deleteLater();
    progress = 0; count = 0;
    ui->S57View->setUpdatesEnabled(true);
}

void S57_DrawWindow::keyPressEvent(QKeyEvent * _ke)
{
    ui->S57View->setUpdatesEnabled(false);
    if (_ke->key() == Qt::Key_Plus)
    {
        ui->S57View->scale(2, 2);
        qDebug() << currentScale();
        updateVisibility();
    }
    if (_ke->key() == Qt::Key_Minus)
    {
        ui->S57View->scale(0.5, 0.5);
        qDebug() << currentScale();
        updateVisibility();
    }
    ui->S57View->setUpdatesEnabled(true);
}

void S57_DrawWindow::on_actionQuit_triggered()
{
    close();
}

bool S57_DrawWindow::eventFilter(QObject * _obj, QEvent * _ev)
{
    if (_obj == scene)
    {
        switch (_ev->type())
        {
        case QEvent::GraphicsSceneMousePress:
        {
            QGraphicsSceneMouseEvent * _me = (QGraphicsSceneMouseEvent *)_ev;
            if (_me->button() == Qt::RightButton && circle[0])
            {
                circle[0]->setVisible(!circle[0]->isVisible());
                circle[1]->setVisible(circle[0]->isVisible());
                circle[2]->setVisible(circle[0]->isVisible());
                if (circle[0]->isVisible())
                {
                    circle[0]->setRect(_me->scenePos().x(), _me->scenePos().y(), 0, 0);
                    circle[1]->setRect(circle[0]->rect());
                    circle[2]->setRect(circle[0]->rect());
                }
            }
            break;
        }
        case QEvent::GraphicsSceneMouseMove:
        {
            QGraphicsSceneMouseEvent * _me = (QGraphicsSceneMouseEvent *)_ev;
            const QPointF _pt = _me->scenePos();
            QString _info = "(" +
                    QString::number(_pt.x()) + ", " +
                    QString::number(_pt.y()) + "); ";
            position_label->setText(_info);
            if (circle[0] && circle[0]->isVisible())
            {
                const QPointF _center = circle[0]->rect().center();
                const double _new_radius = qSqrt(1e-9 + (_pt.x() - _center.x()) * (_pt.x() - _center.x()) + aspect * aspect * (_pt.y() - _center.y()) * (_pt.y() - _center.y()));
                circle[0]->setRect(_center.x() - _new_radius, _center.y() - _new_radius / aspect, 2 * _new_radius, 2 * _new_radius / aspect);
                circle[0]->setStartAngle(((qAtan2(_pt.x() - _center.x(), aspect * (_pt.y() - _center.y())) - M_PI / 2) * 180 / M_PI + 2.5) * 16);
                circle[0]->setSpanAngle(355 * 16);

                circle[1]->setRect(circle[0]->rect());
                circle[1]->setStartAngle(circle[0]->startAngle() - 7.5 * 16);
                circle[1]->setSpanAngle(10 * 16);

                const double _scale = builder.scale() / 200000000.0;
                circle[2]->setRect(_center.x() - _scale, _center.y() - _scale / aspect, _scale * 2, _scale * 2 / aspect);
            }
            break;
        }
        case QEvent::GraphicsSceneWheel:
        {
            QGraphicsSceneWheelEvent * _we = (QGraphicsSceneWheelEvent *)_ev;
            ui->S57View->setUpdatesEnabled(false);
            double _k;
            const int _d = _we->delta();
            if (_d < 0)
            {
                _k = - 100.0 / _d;
            }
            if (_d > 0)
            {
                _k = _d / 100.0;
            }
            if (_d != 0)
            {
                ui->S57View->scale(_k, _k);
                qDebug() << currentScale();
                updateVisibility();
            }
            ui->S57View->setUpdatesEnabled(true);
            return (true);
        }
        }
    }
    return (QMainWindow::eventFilter(_obj, _ev));
}

double S57_DrawWindow::currentScale(double _dot_mm, double _km_in_deg) const
{
    if (!scene)
    {
        return (1);
    }
    const QPointF _top_left = ui->S57View->mapToScene(0, 0);
    const QPointF _bottom_right = ui->S57View->mapToScene(ui->S57View->viewport()->width(), ui->S57View->viewport()->height());
    const double _mm = qMax(ui->S57View->viewport()->width(), ui->S57View->viewport()->height()) * _dot_mm;
    const double _degs = qMax(qAbs(_bottom_right.x() - _top_left.x()), qAbs(_bottom_right.y() - _top_left.y()));
    return (_km_in_deg * _degs * 1000000.0 / _mm);
}

void S57_DrawWindow::updateVisibility()
{
    const double _scale = currentScale();
    if (ui->actionLandAndSea->isChecked())
    {
        builder.updateVisibility(0x7fffffff, true);
    }
    else
    {
        builder.updateVisibility(_scale);
    }
    scale_label->setText("1:" + QString::number(_scale, '.', 0));
    QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("land_and_sea", ui->actionLandAndSea->isChecked());
}

void S57_DrawWindow::scaleTo1(double _scale, double _dot_mm, double _km_in_deg)
{
    if (!scene)
    {
        return;
    }
    const QPointF _top_left = ui->S57View->mapToScene(0, 0);
    const QPointF _bottom_right = ui->S57View->mapToScene(ui->S57View->viewport()->width(), ui->S57View->viewport()->height());
    const double _mm = qMax(ui->S57View->viewport()->width(), ui->S57View->viewport()->height()) * _dot_mm;
    const double _degs = qMax(qAbs(_bottom_right.x() - _top_left.x()), qAbs(_bottom_right.y() - _top_left.y()));
    const double _new_degs = _scale * _mm / (_km_in_deg * 1000000.0);
    ui->S57View->setUpdatesEnabled(false);
    ui->S57View->scale(_degs / _new_degs, _degs / _new_degs);
    updateVisibility();
    ui->S57View->setUpdatesEnabled(true);
}


void S57_DrawWindow::on_action1_5000000_triggered()
{
    scaleTo1(5000000);
}

void S57_DrawWindow::on_action1_2000000_triggered()
{
    scaleTo1(2000000);
}

void S57_DrawWindow::on_action1_1000000_triggered()
{
    scaleTo1(1000000);
}

void S57_DrawWindow::on_action1_500000_triggered()
{
    scaleTo1(500000);
}

void S57_DrawWindow::on_action1_200000_triggered()
{
    scaleTo1(200000);
}

void S57_DrawWindow::on_action1_100000_triggered()
{
    scaleTo1(100000);
}

void S57_DrawWindow::on_action1_50000_triggered()
{
    scaleTo1(50000);
}

void S57_DrawWindow::on_action1_20000_triggered()
{
    scaleTo1(20000);
}

void S57_DrawWindow::on_action1_10000_triggered()
{
    scaleTo1(10000);
}

void S57_DrawWindow::on_action1_5000_triggered()
{
    scaleTo1(5000);
}

void S57_DrawWindow::on_actionLandAndSea_triggered()
{
    updateVisibility();
}

void S57_DrawWindow::on_actionExport_visible_area_triggered()
{
    QString _file_name = QFileDialog::getSaveFileName(this, "Save visible area", qApp->applicationDirPath(), "*.png");
    if (_file_name.isEmpty())
    {
        return;
    }
    if (!scene)
    {
        QMessageBox::information(this, "Nothing to export", "Chart is empty");
        return;
    }
    QImage _img(8192, 8192, QImage::Format_ARGB32);
    _img.fill(QColor(0, 0, 0, 0).rgba());
    QPainter _p(&_img);
    scene->render(&_p, QRectF(), ui->S57View->mapToScene(ui->S57View->viewport()->rect()).boundingRect());
    _img.save(_file_name);
}

void S57_DrawWindow::on_actionExport_whole_chart_triggered()
{
    QString _file_name = QFileDialog::getSaveFileName(this, "Save whole chart", qApp->applicationDirPath(), "*.png");
    if (_file_name.isEmpty())
    {
        return;
    }
    if (!scene)
    {
        QMessageBox::information(this, "Nothing to export", "Chart is empty");
        return;
    }
    QImage _img(8192, 8192, QImage::Format_ARGB32);
    _img.fill(QColor(0, 0, 0, 0).rgba());
    QPainter _p(&_img);
    scene->render(&_p);
    _img.save(_file_name);
}

void S57_DrawWindow::colorSchemeChanged()
{
    QAction * _action = dynamic_cast<QAction *>(sender());
    if (_action == 0)
    {
        return;
    }
    builder.s52().setColorScheme(_action->text());
    QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("scheme", _action->text());
}

void S57_DrawWindow::on_actionMouse_dragging_triggered(bool _state)
{
    ui->S57View->setDragMode(_state ? QGraphicsView::ScrollHandDrag : QGraphicsView::NoDrag);
    ui->S57View->setCursor(_state ? Qt::ArrowCursor : Qt::CrossCursor);
    QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("drag", _state);
}

void S57_DrawWindow::on_actionExport_symbols_triggered()
{
    QString _path = qApp->applicationDirPath() + "/render/symbols";
    QDir().mkpath(_path);

    int _total = 0, _count = 0;
    foreach (const QString & _scheme, builder.s52().colorSchemes())
    {
        foreach (const QString & _symbol, builder.s52().symbols(builder.s52().colorScheme()))
        {
            _total++;
        }
    }
    progress = new QProgressDialog("", "Stop", 0, _total, this);
    progress->setWindowTitle("Rendering symbols");
    progress->setWindowModality(Qt::WindowModal);

    foreach (const QString & _scheme, builder.s52().colorSchemes())
    {
        QString _path_scheme = _path + "/" + _scheme;
        QDir().mkpath(_path_scheme);
        foreach (const QString & _symbol, builder.s52().symbols(builder.s52().colorScheme()))
        {
            progress->setLabelText(_path_scheme + "/" + _symbol + ".PNG");
            progress->setValue(_count);
            builder.s52().image(_symbol).save(_path_scheme + "/" + _symbol + ".PNG");
            _count++;
            qApp->processEvents();
            if (progress->wasCanceled())
            {
                break;
            }
        }
        if (progress->wasCanceled())
        {
            break;
        }
    }

    progress->close();
    progress->deleteLater();
    progress = 0;

}

void S57_DrawWindow::on_action_aspect_1_1_triggered()
{
    if (aspect == Aspect12)
    {
        ui->S57View->scale(1.0, 0.5);
        QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("aspect", aspect = Aspect11);
    }
}

void S57_DrawWindow::on_action_aspect_1_2_triggered()
{
    if (aspect == Aspect11)
    {
        ui->S57View->scale(1.0, 2.0);
        QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("aspect", aspect = Aspect12);
    }
}
