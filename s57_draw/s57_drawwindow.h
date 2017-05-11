#ifndef S57_DrawWindow_H
#define S57_DrawWindow_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QProgressDialog>
#include <QLabel>
#include <QActionGroup>
#include "s57_extendedscene.h"

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
    void scaleChanged();
    void colorSchemeChanged();

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
    void on_actionMouse_dragging_triggered(bool);
    void on_actionExport_symbols_triggered();
    void on_action_aspect_1_1_triggered();
    void on_action_aspect_1_2_triggered();
    void on_actionFullview_triggered();
    void on_actionTooltips_triggered(bool);
    void on_actionRadar_triggered(bool);
    void on_actionRadarCircle_triggered();
    void on_actionRadarSector_triggered();
    void on_actionRadarCoasts_triggered();
    void on_actionRadarRaycastCoasts_triggered();
    void on_actionRadarForeground_triggered();
    void on_actionRadarHideMap_triggered(bool checked);
    void on_actionTargets_triggered(bool checked);

    void on_actionReset_targets_triggered();

private:
    Ui::S57_DrawWindow * ui;
    S57_ExtendedGraphicsScene * scene;
    QLabel * position_label, * scale_label, * distance_label;
    bool eventFilter(QObject *, QEvent *);
    S57_BuildScene builder;
    S57_RulerModificator ruler;
    S57_GridModificator grid;
    S57_RadarModificator radar;
    S57_TargetModificator targets;
    QLabel * radar_sector;
    int radar_timer_id, target_timer_id;
    int target_timer_period;
    QVector<short> radar_emu;
    QProgressDialog * progress;
    bool cancel;
    qint64 count;
    enum {Aspect11 = 1, Aspect12 = 2};
    int aspect;
    void nextDir(const QString & _dir, bool _scan = false);
    enum OpenMode {OpenMapFile = 0, AddMapFile, OpenMapDir, AddMapDir};
    void openMaps(OpenMode mode, QString file_or_dir);

    double precisionAspect();
    void setRadarFlags();

    QList<QGraphicsItem *> land_items, depth_items;
    QActionGroup * knots_group;
    void createTargets();
    static double knotToScene(double knots, int msecs);
    void moveTargets();
    QMap<QString, QGraphicsItem *> targets_depth_cache, targets_land_cache;

    void timerEvent(QTimerEvent *);
    void closeEvent(QCloseEvent *);
};

#endif // S57_DrawWindow_H
