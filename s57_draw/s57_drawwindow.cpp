#include "s57_drawwindow.h"
#include "ui_S57_DrawWindow.h"

#include <QFileDialog>
#include <QGraphicsItem>
#include <QGraphicsSceneEvent>
#include <QFileInfo>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>
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

    QString _def_scheme = QSettings(qApp->applicationDirPath() + "/s57_draw.ini", QSettings::IniFormat).value("scheme", S52_Symbol::global.colorScheme()).toString();
    foreach (const QString & _scheme, S52_Symbol::global.colorSchemes())
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
		if (_ev->type() == QEvent::GraphicsSceneMouseMove)
		{
			QGraphicsSceneMouseEvent * _me = (QGraphicsSceneMouseEvent *)_ev;
            const QPointF _pt = _me->scenePos();
			QString _info = "(" +
                    QString::number(_pt.x()) + ", " +
                    QString::number(_pt.y()) + "); ";
			position_label->setText(_info);
		}
		if (_ev->type() == QEvent::GraphicsSceneWheel)
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
            ui->S57View->scale(_k, _k);
			qDebug() << currentScale();
			updateVisibility();
			ui->S57View->setUpdatesEnabled(true);
			return (true);
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
    S52_Symbol::global.setColorScheme(_action->text());
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
    foreach (const QString & _scheme, S52_Symbol::global.colorSchemes())
    {
        foreach (const QString & _symbol, S52_Symbol::global.symbols(S52_Symbol::global.colorScheme()))
        {
            _total++;
        }
    }
    progress = new QProgressDialog("", "Stop", 0, _total, this);
    progress->setWindowTitle("Rendering symbols");
    progress->setWindowModality(Qt::WindowModal);

    foreach (const QString & _scheme, S52_Symbol::global.colorSchemes())
    {
        QString _path_scheme = _path + "/" + _scheme;
        QDir().mkpath(_path_scheme);
        foreach (const QString & _symbol, S52_Symbol::global.symbols(S52_Symbol::global.colorScheme()))
        {
            progress->setLabelText(_path_scheme + "/" + _symbol + ".PNG");
            progress->setValue(_count);
            s52_symbol(_symbol).save(_path_scheme + "/" + _symbol + ".PNG");
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
