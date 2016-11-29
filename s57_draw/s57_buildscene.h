#ifndef S57_BuildScene_H
#define S57_BuildScene_H

#if defined(_WIN32) && defined(LIB_S57_DLL) && !defined(LIB_S57_EXPORT)
    #define LIB_S57_EXPORT __declspec(dllexport)
#else
    #define LIB_S57_EXPORT
#endif

#include <QGraphicsScene>

class QPainter;
class QRectF;
class QString;

class S52_Symbol;
class LIB_S57_EXPORT S57_BuildScene
{
    S57_BuildScene(const S57_BuildScene &) {}
    S57_BuildScene & operator = (const S57_BuildScene &) { return (* this); }
public:
    S57_BuildScene();
    S57_BuildScene(const QString & _lookups_path, const QString & _symbols_path);
    ~S57_BuildScene();
    void setMercatorStretch(double ext_aspect);
	void clear();
	void setScene(QGraphicsScene * _scene);
    void build(const QString & _file_name);
	void render();
	void resetScale();
	void fullView();
	void updateVisibility(double _scale, bool _exact = false);
    double scale() const;
	int itemsCount() const;

    void setScaleTo(S57_BuildScene * _other);
    S52_Symbol & s52() const;
private:
	struct Data;
	Data * d;
};

class LIB_S57_EXPORT S57_ExtendedScene: public QGraphicsScene
{
    friend class S57_BuildScene;
    S57_ExtendedScene(const S57_ExtendedScene &) {}
    S57_ExtendedScene & operator = (const S57_ExtendedScene &) { return (* this); }
public:
    S57_ExtendedScene(QObject * _parent = 0);
    ~S57_ExtendedScene();
    void setAspect(double);

    void setRadarHintEnabled(bool);
    bool isRadarHintEnabled() const;
protected:
    void drawForeground(QPainter * _painter, const QRectF & _rect);
    bool event(QEvent * _ev);
    void registerBuildScene(S57_BuildScene *);
private:
    struct Data;
    Data * d;
};

#endif // S57_BuildScene_H
