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

class QFixedPixmapItemEx: public QGraphicsItem
{
public:
    QFixedPixmapItemEx(QGraphicsItem *parent = 0): QGraphicsItem(parent), pixmap_ptr(0), image_ptr(0), src_type(Pixmap)
    {
        setScreenSize(QSize(10, 10));
    }
    virtual void setScreenSize(const QSize & _size)
    {
        screen_size = _size;
    }
    QSize screenSize() const
    {
        return screen_size;
    }
    void setPixmap(const QPixmap & _pixmap)
    {
        pixmap = _pixmap;
        src_type = Pixmap;
    }
    void setPixmap(const QPixmap * _pixmap)
    {
        pixmap_ptr = (QPixmap *)_pixmap;
        src_type = Pixmap_Ptr;
    }
    void setImage(const QImage & _image)
    {
        image = _image;
        src_type = Image;
    }
    void setImage(const QImage * _image)
    {
        image_ptr = (QImage *)_image;
        src_type = Image_Ptr;
    }
    QRectF boundingRect() const
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
    void paint(QPainter * _painter, const QStyleOptionGraphicsItem * /*_option*/, QWidget * /*_widget*/)
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

private:
    QSize screen_size;
    QPixmap pixmap;
    QPixmap * pixmap_ptr;
    QImage image;
    QImage * image_ptr;
    enum {Pixmap = 0, Pixmap_Ptr, Image, Image_Ptr};
    int src_type;
};

class RadarItem: public QFixedPixmapItemEx
{
public:
    RadarItem(QGraphicsItem *parent = 0): QFixedPixmapItemEx(parent)
    {
        setMetrics(8192, 4096);
    }
    ~RadarItem() {}
    void setMetrics(int _portion_size, int _positions_count)
    {
        block_size = _portion_size;
        positions_count = _positions_count;

        reset();
    }
    void reset()
    {
        createSrcMap();
        createDstMap();
    }
    void adjustSize()
    {
        createDstMap();
    }
    void addData(const QVector<short> & block, int position)
    {
        _addData(block, position);
    }
    void setScreenSize(const QSize & _size)
    {
        QFixedPixmapItemEx::setScreenSize(_size);
        adjustSize();
    }
    QImage roi(double start_angle, double stop_angle, int start_distance, int stop_distance) const // angles in radians, 0 <= angle <= 2 * M_PI
    {
        if (start_distance < 0 || stop_distance > block_size || start_distance > stop_distance)
        {
            return QImage();
        }
        while (start_angle < 0)
        {
            start_angle += 2 * M_PI;
        }
        while (stop_angle < 0)
        {
            stop_angle += 2 * M_PI;
        }
        while (start_angle >= 2 * M_PI)
        {
            start_angle -= 2 * M_PI;
        }
        while (stop_angle >= 2 * M_PI)
        {
            stop_angle -= 2 * M_PI;
        }
        const double angle_step = 2.0 * M_PI / positions_count;
        const int _start_angle = start_angle / angle_step;
        const int _stop_angle1 = start_angle < stop_angle ? stop_angle / angle_step : 2 * M_PI / angle_step;
        const int _stop_angle2 = start_angle < stop_angle ? 0 : stop_angle / angle_step;

        QSize sz(stop_distance - start_distance, start_angle < stop_angle ? _stop_angle1 - _start_angle : positions_count - _start_angle + _stop_angle2);
        if (sec_img.size() != sz)
        {
            sec_img = QImage(sz, QImage::Format_RGB32);
        }
        unsigned int * img_ptr = (unsigned int *)sec_img.bits();
        for (int t = _start_angle; t < _stop_angle1; t++)
        {
            const short * _src = src.constData() + t * block_size + start_distance;
            for (int v = start_distance; v < stop_distance; v++)
            {
                * img_ptr++ = colorConvert(* _src++);
            }
        }
        for (int t = 0; t < _stop_angle2; t++)
        {
            const short * _src = src.constData() + t * block_size + start_distance;
            for (int v = start_distance; v < stop_distance; v++)
            {
                * img_ptr++ = colorConvert(* _src++);
            }
        }
        QPoint center = sec_img.rect().center();
        QMatrix matrix;
        matrix.translate(center.x(), center.y());
        matrix.rotate(-90);
        return sec_img.transformed(matrix);
    }
private:
    int block_size, positions_count;
    QVector<QPointF> map_src;
    QVector<int> map_dst;
    QVector<short> src, dst;
    QImage img;
    mutable QImage sec_img;
    void createSrcMap()
    {
        map_src.resize(block_size * positions_count);
        map_src.fill(QPointF(0, 0));
        src.resize(block_size * positions_count);
        src.fill(0);
        const double angle_step = 2.0 * M_PI / positions_count;
        for (int t = 0; t < positions_count; t++)
        {
            const double angle = t * angle_step;
            for (int v = 0; v < block_size; v++)
            {
                map_src[block_size * t + v] = QPointF(v * ::cosf(angle - M_PI / 2), v * ::sinf(angle - M_PI / 2));
            }
        }
    }
    void createDstMap()
    {
        if (map_src.size() < block_size * positions_count)
        {
            return;
        }
        const int w = screenSize().width();
        const int h = screenSize().height();
        if (dst.size() == w * h)
        {
            return;
        }
        dst.resize(w * h);
        dst.fill(0);

        img = QImage(w, h, QImage::Format_ARGB32);
        img.fill(QColor(0, 0, 0, 0));
        setImage(& img);

        map_dst.resize(block_size * positions_count);
        map_dst.fill(0);

        const double xw = (1.0 * w) / (2 * block_size);
        const double yh = (1.0 * h) / (2 * block_size);

        for (int t = 0; t < positions_count; t++)
        {
            for (int v = 0; v < block_size; v++)
            {
                const int idx = block_size * t + v;
                const QPointF & _map_src = map_src[idx];
                const int x = _map_src.x() * xw + (w / 2);
                const int y = _map_src.y() * yh + (h / 2);
                map_dst[idx] = (int)y * w + (int)x;
            }
        }
    }
    void _addData(const QVector<short> & block, int position)
    {
        if (map_dst.size() < block_size * positions_count)
        {
            return;
        }
        if (position >= positions_count)
        {
            return;
        }
        if (position == 0)
        {
            dst.fill(0);
        }
        ::memcpy(src.data() + block_size * position, block.constData(), block_size * sizeof(src[0]));
        const int * offset_ptr = map_dst.constData() + block_size * position;
        const short * block_ptr = block.constData();
        short * dst_ptr = dst.data();
        unsigned int * img_ptr = (unsigned int *)img.bits();
        for (int v = 0; v < block_size; v++, offset_ptr++, block_ptr++)
        {
            short & _dst = dst_ptr[* offset_ptr];
            if (_dst < * block_ptr)
            {
                _dst = * block_ptr;
            }
            img_ptr[* offset_ptr] = colorConvert(_dst);
        }
    }
    static QRgb colorConvert(short data)
    {
        return qRgba(0, 0xff & data, 0, data > 0 ? 255 : 0);
    }
};

S57_DrawWindow::S57_DrawWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::S57_DrawWindow), scene(0), progress(0), count(0), aspect(Aspect11), ri(0)
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

    scene = new S57_ExtendedScene();
    scene->setAspect(aspect);
    scene->setRulerEnabled(true);
//    scene->setGridEnabled(false);
    scene->installEventFilter(this);
    ui->S57View->setScene(scene);
    builder.setScene(scene);
    builder.fullView();

    radar_sector = new QLabel();
    radar_sector->setVisible(false);
    radar_sector->setWindowTitle(QObject::tr("Radar sector"));
    radar_sector->resize(400, 400);

    ri = new RadarItem();
    ri->setZValue(1000000);
    scene->addItem(ri);

    rpi = new QGraphicsPolygonItem();
    QPen rpi_pen(Qt::red, 5);
    rpi_pen.setCosmetic(true);
    rpi->setPen(rpi_pen);
    rpi->setZValue(ri->zValue() + 1);
    scene->addItem(rpi);

    bool show_radar = QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("radar", false).toBool();
    ui->actionRadar->setChecked(show_radar);
    on_actionRadar_triggered(show_radar);

    radar_emu.resize(8192 * 4096);
    for (int i = 0; i < 8192 * 4096; i++)
    {
        radar_emu[i] = qrand();
    }

    connect(ui->S57View->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(viewportMoved()));
    connect(ui->S57View->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(viewportMoved()));
//    connect(ui->S57View->horizontalScrollBar(), SIGNAL(sliderMoved(int)), this, SLOT(viewportMoved()));
//    connect(ui->S57View->verticalScrollBar(), SIGNAL(sliderMoved(int)), this, SLOT(viewportMoved()));

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

    coasts.clear();
    scene->removeItem(ri);
    scene->removeItem(rpi);

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
    ui->S57View->scale(1, aspect);
    updateVisibility();
    progress->close();
    progress->deleteLater();
    progress = 0; count = 0;
    ui->S57View->setUpdatesEnabled(true);

    scene->addItem(ri);
    scene->addItem(rpi);
    viewportMoved();

    foreach(QGraphicsItem * item, scene->items())
    {
        if (item->data(S57_BuildScene::ItemLayerNameKey).toString().compare("LNDARE", Qt::CaseInsensitive) == 0)
        {
            if (QGraphicsPolygonItem * coast_item = dynamic_cast<QGraphicsPolygonItem *>(item))
            {
                coasts << coast_item;
            }
        }
    }
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


void S57_DrawWindow::keyPressEvent(QKeyEvent * _ke)
{
    ui->S57View->setUpdatesEnabled(false);
    if (_ke->key() == Qt::Key_Plus)
    {
        ui->S57View->scale(1.25, 1.25);
        //        qDebug() << currentScale();
        updateVisibility();
    }
    if (_ke->key() == Qt::Key_Minus)
    {
        ui->S57View->scale(0.8, 0.8);
        //        qDebug() << currentScale();
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
        case QEvent::GraphicsSceneMouseMove:
        {
            QGraphicsSceneMouseEvent * _me = (QGraphicsSceneMouseEvent *)_ev;
            const QPointF _pt = _me->scenePos();
            QString _info = "(" +
                    QString::number(_pt.x()) + (_pt.x() > 0 ? "E" : "W") + QString(", ") +
                    QString::number(-_pt.y()) + (_pt.y() > 0 ? "S" : "N") + QString(") ");
            position_label->setText(_info);

            if (radar_sector->isVisible() && (_me->buttons() & Qt::LeftButton) == Qt::LeftButton)
            {
                QPointF tl = ui->S57View->mapToScene(QRect(QPoint(), ui->S57View->viewport()->size()).topLeft());
                QPointF br = ui->S57View->mapToScene(QRect(QPoint(), ui->S57View->viewport()->size()).bottomRight());
                QPointF ct = ui->S57View->mapToScene(QRect(QPoint(), ui->S57View->viewport()->size()).center());

                const double radius = qMin(qAbs(tl.x() - br.x()), qAbs(tl.y() - br.y())) / 2.0;

                double angle = ::atan2(_pt.y() - ct.y(), _pt.x() - ct.x());
                double length = ::sqrtf((_pt.y() - ct.y()) * (_pt.y() - ct.y()) + (_pt.x() - ct.x()) * (_pt.x() - ct.x()) + 1e-15);

                if (length < 0.1 * radius)
                {
                    length = 0.1 * radius;
                }
                if (length > 0.9 * radius)
                {
                    length = 0.9 * radius;
                }

                const double l1 = length - 0.1 * radius;
                const double l2 = length + 0.1 * radius;
                const double a1 = angle - 0.1;
                const double a2 = angle + 0.1;
                QPolygonF rpi_poly;
                rpi_poly << QPointF(l1 * ::cos(a1) * aspect, l1 * ::sin(a1));
                rpi_poly << QPointF(l1 * ::cos(a2) * aspect, l1 * ::sin(a2));
                rpi_poly << QPointF(l2 * ::cos(a2) * aspect, l2 * ::sin(a2));
                rpi_poly << QPointF(l2 * ::cos(a1) * aspect, l2 * ::sin(a1));
                rpi_poly << QPointF(l1 * ::cos(a1) * aspect, l1 * ::sin(a1));

                rpi->setPolygon(rpi_poly);

                rpi->setPos(ct);

                roi_rect.setLeft(a1 + M_PI / 2);
                roi_rect.setTop(a2 + M_PI / 2);
                roi_rect.setBottom(l1 * 8192 / radius);
                roi_rect.setRight(l2 * 8192 / radius);
            }
            break;
        }
        case QEvent::GraphicsSceneWheel:
        {
            QGraphicsSceneWheelEvent * _we = (QGraphicsSceneWheelEvent *)_ev;
            ui->S57View->setUpdatesEnabled(false);
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
                ui->S57View->scale(_k, _k);
                //                qDebug() << currentScale();
                updateVisibility();
            }
            ui->S57View->setUpdatesEnabled(true);
            return (true);
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

void S57_DrawWindow::updateVisibility()
{
    const double _scale = builder.scaleTo1();
    if (ui->actionLandAndSea->isChecked())
    {
        builder.updateVisibility(0x7fffffff, true);
    }
    else
    {
        builder.updateVisibility(_scale);
    }
    scale_label->setText("1:" + QString::number(_scale, '.', 0));
    viewportMoved();
}

void S57_DrawWindow::scaleTo1(double _scale)
{
    if (!scene)
    {
        return;
    }
    ui->S57View->setUpdatesEnabled(false);
    const double scale = builder.scaleTo1(_scale);
    ui->S57View->scale(scale, scale);
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
        ui->S57View->setUpdatesEnabled(false);
        ui->S57View->scale(1.0, 0.5);
        if (scene)
        {
            scene->setAspect(aspect);
        }
        updateVisibility();
        ui->S57View->setUpdatesEnabled(true);
        QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("aspect", aspect);
    }
}

void S57_DrawWindow::on_action_aspect_1_2_triggered()
{
    if (aspect == Aspect11)
    {
        aspect = Aspect12;
        ui->S57View->setUpdatesEnabled(false);
        ui->S57View->scale(1.0, 2.0);
        if (scene)
        {
            scene->setAspect(aspect);
        }
        updateVisibility();
        ui->S57View->setUpdatesEnabled(true);
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
    builder.fullView();
    ui->S57View->scale(1, aspect);
    updateVisibility();
}

void S57_DrawWindow::on_actionTooltips_triggered(bool t)
{
    QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("tooltips", t);
}

void S57_DrawWindow::showEvent(QShowEvent *)
{
    viewportMoved();
}

void S57_DrawWindow::resizeEvent(QResizeEvent *)
{
    viewportMoved();
}

void S57_DrawWindow::timerEvent(QTimerEvent *)
{
    static int pos = -1;
    for (int k = 0; k < 128; k++)
    {
        pos++;
        if (pos >= 4096)
        {
            pos = 0;
        }
        QVector<short> v(8192);
        v.fill(0);
        for (int i = 0; i < v.count(); i += 327.0  * qrand() / RAND_MAX)
        {
            v[i] = radar_emu[8192 * pos + i];
        }
        if (pos < 3072)
        {
            for (int i = 4111; i < 4211; i++)
            {
                v[i] = 0xff;
            }
        }
        else
        {
            for (int i = 7111; i < 7211; i++)
            {
                v[i] = 0xff;
            }
        }
        ri->addData(v, pos);
    }
    if (radar_sector->isVisible())
    {
        radar_sector->setPixmap(QPixmap::fromImage(ri->roi(roi_rect.left(), roi_rect.top(), roi_rect.bottom(), roi_rect.right()).scaled(radar_sector->size())));
    }
}

void S57_DrawWindow::viewportMoved()
{
    QPointF tl = ui->S57View->mapToScene(QRect(QPoint(), ui->S57View->viewport()->size()).topLeft());
    QPointF br = ui->S57View->mapToScene(QRect(QPoint(), ui->S57View->viewport()->size()).bottomRight());
    QPointF ct = ui->S57View->mapToScene(QRect(QPoint(), ui->S57View->viewport()->size()).center());

    const double radius = qMin(qAbs(tl.x() - br.x()), qAbs(tl.y() - br.y())) / 2.0;

    const int sz = qMin(ui->S57View->viewport()->size().width(), ui->S57View->viewport()->size().height());
    ri->setScreenSize(QSize(sz, sz));
    ri->setPos(ct); // anchor center
    rpi->setPos(ct);

    foreach (QGraphicsLineItem * ri_coast_item, ri_coasts)
    {
        scene->removeItem(ri_coast_item);
        delete ri_coast_item;
    }
    ri_coasts.clear();
    QPainterPath p1;
    p1.addEllipse(ct, radius, radius);
    QPen pen(Qt::green, 5);
    pen.setCosmetic(true);
    const double r2 = radius * radius;
    foreach (QGraphicsPolygonItem * coast_item, coasts)
    {
        QPainterPath p2;
        p2.addPolygon(coast_item->polygon());
        if (p1.intersects(p2))            
        {
            const QPolygonF & poly = coast_item->polygon();
            QPointF prev_pt = poly[0];
            int prev_index = 0;
            for (int i = 1; i < poly.count(); i++)
            {
                const QPointF & pt = poly[i];
                const QPointF mid_pt((pt.x() + prev_pt.x()) / 2, (pt.y() + prev_pt.y()) / 2);
                const float dist_1 = (mid_pt.x() - ct.x()) * (mid_pt.x() - ct.x()) + (mid_pt.y() - ct.y()) * (mid_pt.y() - ct.y());
                const float dist_2 = (pt.x() - ct.x()) * (pt.x() - ct.x()) + (pt.y() - ct.y()) * (pt.y() - ct.y());
                const float dist_3 = (prev_pt.x() - ct.x()) * (prev_pt.x() - ct.x()) + (prev_pt.y() - ct.y()) * (prev_pt.y() - ct.y());
                if (dist_1 < r2 || dist_2 < r2 || dist_3 < r2)
                {
                    if (i - prev_index == 1)
                    {
                        QGraphicsLineItem * ri_coast_item = new QGraphicsLineItem(prev_pt.x(), prev_pt.y(), pt.x(), pt.y());
                        ri_coast_item->setPen(pen);
                        scene->addItem(ri_coast_item);
                        ri_coasts << ri_coast_item;

                    }
                }
                prev_pt = pt;
                prev_index = i;
            }
        }
    }
}


void S57_DrawWindow::on_actionRadar_triggered(bool checked)
{
    ri->setVisible(checked);
    rpi->setVisible(checked);
    QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).setValue("radar", checked);
    if (checked)
    {
        radar_sector->show();
        ui->actionMouse_dragging->setChecked(false);
        on_actionMouse_dragging_triggered(false);
        ui->actionMouse_dragging->setEnabled(false);
    }
    else
    {
        radar_sector->hide();
        ui->actionMouse_dragging->setEnabled(true);
    }
}

void S57_DrawWindow::closeEvent(QCloseEvent *)
{
    radar_sector->close();
}

