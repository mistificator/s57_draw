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
    virtual ~S57_BuildScene();
    void setMercatorStretch(double ext_aspect);
	void clear();
   void setScene(QGraphicsScene * _scene);
    QGraphicsScene * scene() const;
    void build(const QString & _file_name);
	void render();
	void resetScale();
	void fullView();
	void updateVisibility(double _scale, bool _exact = false);
    double scale() const;
	int itemsCount() const;
    QList<QGraphicsItem *> items() const;
    QStringList layers() const;

    void setScaleTo(S57_BuildScene * _other);
    S52_Symbol & s52() const;

    double scaleTo1(double _scale = 1.0, double _dot_mm = 0.25, double _km_in_deg = 78.715);

    enum ItemKeys {ItemLayerNameKey = 1};

    template <class T = QGraphicsItem>
    QList<T *> items(QString _layer_name) const
    {
        QList<T *> _items;
        foreach(QGraphicsItem * item, items())
        {
            if (item->data(S57_BuildScene::ItemLayerNameKey).toString().compare(_layer_name, Qt::CaseInsensitive) == 0)
            {
                if (T * _t_item = dynamic_cast<T *>(item))
                {
                    _items << _t_item;
                }
            }
        }
        return _items;
    }

    void setRenderEnabled(bool);
    bool isRenderEnabled() const;
private:
	struct Data;
	Data * d;
};

#endif // S57_BuildScene_H
