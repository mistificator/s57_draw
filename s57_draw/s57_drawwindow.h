#ifndef S57_DrawWindow_H
#define S57_DrawWindow_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QProgressDialog>
#include <QLabel>
#include "s57_buildscene.h"

namespace Ui {
class S57_DrawWindow;
}

class S57_DrawWindow : public QMainWindow
{
	Q_OBJECT
	friend class BuildSceneThread;
public:
    explicit S57_DrawWindow(QWidget *parent = 0);
    ~S57_DrawWindow();
	
private slots:
	void on_actionOpen_triggered();
	void on_actionQuit_triggered();
	void on_actionAdd_triggered();
	void on_actionOpen_folder_triggered();
	void on_actionAdd_folder_triggered();
	void on_action1_5000000_triggered();
	void on_action1_2000000_triggered();
	void on_action1_1000000_triggered();
	void on_action1_500000_triggered();
	void on_action1_200000_triggered();
	void on_action1_100000_triggered();
    void on_action1_50000_triggered();
    void on_action1_20000_triggered();
    void on_action1_10000_triggered();
    void on_action1_5000_triggered();
    void on_actionLandAndSea_triggered();
	void on_actionExport_visible_area_triggered();
	void on_actionExport_whole_chart_triggered();
    void colorSchemeChanged();
    void on_actionMouse_dragging_triggered(bool);
    void on_actionExport_symbols_triggered();
    void on_action_aspect_1_1_triggered();
    void on_action_aspect_1_2_triggered();
private:
    Ui::S57_DrawWindow *ui;
	QGraphicsScene * scene;
	QLabel * position_label, * scale_label;
	void keyPressEvent(QKeyEvent * _ke);
	bool eventFilter(QObject *, QEvent *);
    S57_BuildScene builder;
	QProgressDialog * progress;
	bool cancel;
	qint64 count;
    enum {Aspect11 = 1, Aspect12 = 2};
    int aspect;
	void nextDir(const QString & _dir, bool _scan = false);
	double currentScale(double _dot_mm = 0.25, double _km_in_deg = 78.715) const;
	void updateVisibility();
	void recreateScene();
	void scaleTo1(double _scale, double _dot_mm = 0.25, double _km_in_deg = 78.715);
	void precisionAspect();
};

#endif // S57_DrawWindow_H
