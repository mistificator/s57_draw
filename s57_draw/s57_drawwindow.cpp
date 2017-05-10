#include "s57_drawwindow.h"
#include "ui_S57_DrawWindow.h"

#include <QFileDialog>
#include <QGraphicsItem>
#include <QGraphicsSceneEvent>
#include <QFileInfo>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>
#include <QScrollBar>

#include <cmath>
#include "cpl_port.h"
#include "s52_symbol.h"


// -------------------------------------------------------------------------

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
    ui->statusbar->addPermanentWidget(distance_label = new QLabel(), 1);
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
            builder.s52().setColorScheme(_action->text());
        }
    }
    ui->actionLandAndSea->setChecked(QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("land_and_sea", false).toBool());
    ui->actionMouse_dragging->setChecked(QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("drag", true).toBool());
    on_actionMouse_dragging_triggered(ui->actionMouse_dragging->isChecked());
    ui->actionTooltips->setChecked(QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("tooltips", false).toBool());

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

    QFile f(":global/scrollbar.css");
    if (f.exists() && f.open(QIODevice::ReadOnly))
    {
        ui->S57View->setStyleSheet(f.readAll());
        f.close();        
    }
    else
    {
        qDebug() << "No css file";
    } 

    scene = new S57_ExtendedGraphicsScene();
    scene->setAspect(aspect);
    scene->installEventFilter(this);
    connect(scene, SIGNAL(scaleChanged()), this, SLOT(scaleChanged()));
    ui->S57View->setScene(scene);
    builder.setScene(scene);

    builder.fullView();

    radar_sector = new QLabel();
    radar_sector->setVisible(false);
    radar_sector->setWindowTitle(QObject::tr("Radar sector"));
    radar_sector->move(0, 48);
    radar_sector->resize(400, 400);
    radar_sector->setParent(this, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Tool);

    bool show_radar = QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("radar", false).toBool();
    ui->actionRadar->setChecked(show_radar);
    on_actionRadar_triggered(show_radar);

    qDebug() << "initial block size" << radar.blockSize();
    radar_emu.resize(radar.blockSize() * radar.positionsCount());
    for (int i = 0; i < radar.blockSize() * radar.positionsCount(); i++)
    {
        radar_emu[i] = (qrand() & 0x7f);
    }

//    radar.setFlags(S57_RadarModificator::RadarCircle | S57_RadarModificator::RadarSector | S57_RadarModificator::RadarRaycastCoasts); // radar flags example
//    radar.setFlags(S57_RadarModificator::RadarForeground | S57_RadarModificator::RadarCircle | S57_RadarModificator::RadarRaycastCoasts); // radar flags example

    timer_id = startTimer(0);
}

S57_DrawWindow::~S57_DrawWindow()
{
    builder.clear();
    scene->deleteLater();
    delete ui;
}

void S57_DrawWindow::nextDir(const QString & _dir, bool _scan)
{
    if (progress && progress->wasCanceled())
    {
        return;
    }
    QStringList _files = QDir(_dir).entryList(QStringList() << "*.000" << "*.000.txt" << "*.000.prti99");
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
        if (progress && progress->wasCanceled())
        {
            return;
        }
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

void S57_DrawWindow::openMaps(OpenMode mode, QString file_or_dir)
{
    if (file_or_dir.isEmpty())
    {
        return;
    }

    if (ui->actionRadar->isChecked())
    {
        scene->removeMod(& radar);
    }

    switch (mode)
    {
    case OpenMapDir:
    case AddMapDir:
        nextDir(file_or_dir, true);
        break;
    }
    progress = new QProgressDialog(file_or_dir, QObject::tr("Stop"), 0, count, this);
    progress->setWindowTitle(QObject::tr("Scanning folder"));
    progress->setWindowModality(Qt::WindowModal);
    progress->show();
    count = 0;
    ui->S57View->setUpdatesEnabled(false);
    switch (mode)
    {
    case OpenMapFile:
    case OpenMapDir:
        builder.clear();
        ui->actionAdd->setEnabled(true);
        ui->actionAdd_folder->setEnabled(true);
        break;
    }
    builder.resetScale();
    switch (mode)
    {
    case OpenMapFile:
    case AddMapFile:
        {
            QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("path", QFileInfo(file_or_dir).absolutePath());
            builder.build(file_or_dir);
        }
        break;
    case OpenMapDir:
    case AddMapDir:
        {
            QDir _dir_up = QDir(file_or_dir);
            _dir_up.cd("..");
            QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("path", _dir_up.absolutePath());
            nextDir(file_or_dir);
        }
        break;
    }
    builder.render();

    if (ui->actionRadar->isChecked())
    {
        scene->addMod(& radar);
    }

    scene->fullView();
    progress->close();
    progress->deleteLater();
    progress = 0; count = 0;
    ui->S57View->setUpdatesEnabled(true);   

    QList<QGraphicsItem *> land_items = builder.items("LNDARE");
    for (int i = 0; i < 15; i++)
    {
        targets.removeTargetByName("Test ship " + QString::number(i + 1));
        S57_TargetModificator::Target & t = targets.addTarget(QPointF(), "Test ship " + QString::number(i + 1));
        t.angle = 359.0 * qrand() / RAND_MAX;
        bool found;
        QPointF pos;
        do
        {
            found = false;
            pos = QPointF(scene->sceneRect().x() + scene->sceneRect().width() * qrand() / RAND_MAX, scene->sceneRect().y() + scene->sceneRect().height() * qrand() / RAND_MAX);
            foreach (QGraphicsItem * item, land_items)
            {
                if (item->contains(pos))
                {
                    found = true;
                    break;
                }
            }
        }
        while (found);
        t.pos = pos;
        t.color = QColor((Qt::GlobalColor)(i / 3 + Qt::red));
    }
    targets.renderTargets();
}

void S57_DrawWindow::on_actionOpen_triggered()
{
    QString _fn = QFileDialog::getOpenFileName(
                this,
                QObject::tr("Open S57 chart"),
                QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("path", qApp->applicationDirPath()).toString(),
                "S57 & PRTI99 (*.000 *.000.prti99 *.000.txt)"
                );
    openMaps(OpenMapFile, _fn);
}


void S57_DrawWindow::on_actionAdd_triggered()
{
    QString _fn = QFileDialog::getOpenFileName(
                this,
                QObject::tr("Add S57 chart"),
                QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("path", qApp->applicationDirPath()).toString(),
                "S57 & PRTI99 (*.000 *.000.prti99 *.000.txt)"
                );
    openMaps(AddMapFile, _fn);
}

void S57_DrawWindow::on_actionOpen_folder_triggered()
{
    QString _dir = QFileDialog::getExistingDirectory(this, QObject::tr("Open folder with S57 charts"), QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("path", qApp->applicationDirPath()).toString());
    if (_dir.isEmpty())
    {
        return;
    }
    openMaps(OpenMapDir, _dir);
}


void S57_DrawWindow::on_actionAdd_folder_triggered()
{
    QString _dir = QFileDialog::getExistingDirectory(this, QObject::tr("Open folder with S57 charts"), QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("path", qApp->applicationDirPath()).toString());
    if (_dir.isEmpty())
    {
        return;
    }
    openMaps(AddMapDir, _dir);
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
        case QEvent::GraphicsSceneMouseMove:
        {
            QGraphicsSceneMouseEvent * _me = (QGraphicsSceneMouseEvent *)_ev;
            const QPointF _pt = _me->scenePos();
            QString _info = "(" +
                    QString::number(_pt.x()) + (_pt.x() > 0 ? "E" : "W") + QString(", ") +
                    QString::number(-_pt.y()) + (_pt.y() > 0 ? "S" : "N") + QString(") ");
            position_label->setText(_info);
            break;
        }
        case QEvent::GraphicsSceneHelp:
            if (ui->actionTooltips->isChecked())
            {
                break;
            }
            return true;
        }
    }
    return (QMainWindow::eventFilter(_obj, _ev));
}

void S57_DrawWindow::scaleChanged()
{
    const double _scale = builder.scaleTo1();
    scale_label->setText("1:" + QString::number(_scale, '.', 0));
    const QRectF r = ui->S57View->mapToScene(ui->S57View->viewport()->rect()).boundingRect();
    const double miles = qMin<double>(r.width() * 60, r.height() * 60);
    distance_label->setText(QString::number(miles, 'f', 1) + " " + QObject::tr("miles"));

    int block_size = 8192;
    if (miles <= 0.5)
    {
        block_size = 256;
    }
    else
    if (miles <= 1)
    {
        block_size = 512;
    }
    else
    if (miles <= 2)
    {
        block_size = 1024;
    }
    else
    if (miles <= 4)
    {
        block_size = 2048;
    }
    else
    if (miles <= 8)
    {
        block_size = 4096;
    }
    if (block_size != radar.blockSize())
    {
        radar.setMetrics(block_size, 4096);
//        QMetaObject::invokeMethod(& radar, "setMetrics", Qt::QueuedConnection, Q_ARG(int, block_size), Q_ARG(int, 4096));
        qDebug() << "miles" << miles << "block_size set to" << block_size;
    }
}

void S57_DrawWindow::on_action1_5000000_triggered()
{
    scene->scaleTo1(5000000);
}

void S57_DrawWindow::on_action1_2000000_triggered()
{
    scene->scaleTo1(2000000);
}

void S57_DrawWindow::on_action1_1000000_triggered()
{
    scene->scaleTo1(1000000);
}

void S57_DrawWindow::on_action1_500000_triggered()
{
    scene->scaleTo1(500000);
}

void S57_DrawWindow::on_action1_200000_triggered()
{
    scene->scaleTo1(200000);
}

void S57_DrawWindow::on_action1_100000_triggered()
{
    scene->scaleTo1(100000);
}

void S57_DrawWindow::on_action1_50000_triggered()
{
    scene->scaleTo1(50000);
}

void S57_DrawWindow::on_action1_20000_triggered()
{
    scene->scaleTo1(20000);
}

void S57_DrawWindow::on_action1_10000_triggered()
{
    scene->scaleTo1(10000);
}

void S57_DrawWindow::on_action1_5000_triggered()
{
    scene->scaleTo1(5000);
}

void S57_DrawWindow::on_actionLandAndSea_triggered()
{
    scene->setLandAndSeaOnly(ui->actionLandAndSea->isChecked());
    QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("land_and_sea", ui->actionLandAndSea->isChecked());
}

void S57_DrawWindow::on_actionExport_visible_area_triggered()
{
    QString _file_name = QFileDialog::getSaveFileName(this, QObject::tr("Save visible area"), qApp->applicationDirPath(), "*.png");
    if (_file_name.isEmpty())
    {
        return;
    }
    if (!scene)
    {
        QMessageBox::information(this, QObject::tr("Nothing to export"), QObject::tr("Chart is empty"));
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
    QString _file_name = QFileDialog::getSaveFileName(this, QObject::tr("Save whole chart"), qApp->applicationDirPath(), "*.png");
    if (_file_name.isEmpty())
    {
        return;
    }
    if (!scene)
    {
        QMessageBox::information(this, QObject::tr("Nothing to export"), QObject::tr("Chart is empty"));
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
    if (scene && !scene->items().isEmpty() && QMessageBox::question(this, tr("Question"), tr("All maps will be closed. Continue?"), tr("Yes"), tr("No")) != 0)
    {
        return;
    }
    builder.clear();
    builder.resetScale();
    ui->actionAdd->setEnabled(false);
    ui->actionAdd_folder->setEnabled(false);
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
    progress = new QProgressDialog("", QObject::tr("Stop"), 0, _total, this);
    progress->setWindowTitle(QObject::tr("Rendering symbols"));
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
        aspect = Aspect11;
        scene->setAspect(aspect);
        QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("aspect", aspect);
    }
}

void S57_DrawWindow::on_action_aspect_1_2_triggered()
{
    if (aspect == Aspect11)
    {
        aspect = Aspect12;
        scene->setAspect(aspect);
        QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("aspect", aspect);
    }
}

double S57_DrawWindow::precisionAspect()
{
    QPointF worldcenter = scene->sceneRect().center();
    return qAbs<double>(1.0 / cos(worldcenter.y()/180.0*M_PI));
}

void S57_DrawWindow::on_actionFullview_triggered()
{
    scene->fullView();
}

void S57_DrawWindow::on_actionTooltips_triggered(bool t)
{
    QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("tooltips", t);
}

void S57_DrawWindow::timerEvent(QTimerEvent *)
{
    QPointF ct = ui->S57View->viewport()->rect().center();

    static int pos = -1;
    const int block_size = radar.blockSize();
//    qDebug() << "block_size" << block_size << "pos" << pos;
    for (int k = 0; k < 128; k++)
    {
        pos++;
        if (pos >= radar.positionsCount())
        {
            pos = 0;
        }
        QVector<short> v(block_size);
        v.fill(0);
        for (int i = 0; i < v.count(); i += 327.0  * qrand() / RAND_MAX)
        {
            v[i] = radar_emu[block_size * pos + i];
        }

        const int angle = pos * 360.0 / radar.positionsCount();
        foreach (S57_TargetModificator::Target target, targets.targets())
        {
            QLineF line(ct, ui->S57View->mapFromScene(target.pos));
            if (((int)(90 - line.angle()) + 360) % 360 == angle)
            {
                const int len = line.length() * block_size / radar.screenRadius();
                if (len + 2 < radar.blockSize())
                {
                    v[len] = 0xff;
                    v[len + 1] = 0xff;
                    v[len + 2] = 0xff;
                }
            }
        }

        radar.addData(v, pos);
    }
    if (radar_sector->isVisible())
    {
        const QImage & img = radar.roi();
        if (!img.size().isNull())
        {
            radar_sector->setPixmap(QPixmap::fromImage(img.scaled(radar_sector->size())));
        }
    }

    ui->S57View->viewport()->update();
}

void S57_DrawWindow::on_actionRadar_triggered(bool checked)
{
    QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("radar", checked);
    if (checked)
    {
        builder.setRenderEnabled(false);
        radar_sector->show();
        activateWindow();
        ui->actionMouse_dragging->setChecked(false);
        on_actionMouse_dragging_triggered(false);
        ui->actionMouse_dragging->setEnabled(false);
        scene->removeMod(& ruler);
        scene->removeMod(& grid);
        scene->removeMod(& targets);
        scene->addMod(& radar);
    }
    else
    {
        radar_sector->hide();
        ui->actionMouse_dragging->setEnabled(true);
        scene->addMod(& ruler);
        scene->addMod(& grid);
        scene->addMod(& targets);
        scene->removeMod(& radar);
        builder.setRenderEnabled(true);
    }
}

void S57_DrawWindow::closeEvent(QCloseEvent *)
{
    radar_sector->close();
}

  void S57_DrawWindow::setRadarFlags()
  {
      radar.setFlags((ui->actionRadarCircle->isChecked() ? S57_RadarModificator::RadarCircle:0) |
                     (ui->actionRadarSector->isChecked() ? S57_RadarModificator::RadarSector:0) |
                     (ui->actionRadarCoasts->isChecked() ? S57_RadarModificator::RadarCoasts:0 )|
                     (ui->actionRadarRaycastCoasts->isChecked() ? S57_RadarModificator::RadarRaycastCoasts:0)   |
                     (ui->actionRadarForeground->isChecked() ? S57_RadarModificator::RadarForeground:0)
                     ); // radar flags example

  }

void S57_DrawWindow::on_actionRadarCircle_triggered()
{
    setRadarFlags();
}

void S57_DrawWindow::on_actionRadarSector_triggered()
{
     setRadarFlags();
}

void S57_DrawWindow::on_actionRadarCoasts_triggered()
{
    setRadarFlags();
}

void S57_DrawWindow::on_actionRadarRaycastCoasts_triggered()
{
    setRadarFlags();
}

void S57_DrawWindow::on_actionRadarForeground_triggered()
{
    setRadarFlags();
}
