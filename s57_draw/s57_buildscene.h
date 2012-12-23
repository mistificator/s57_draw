#ifndef S57_BuildScene_H
#define S57_BuildScene_H

#include <QGraphicsScene>

#ifdef LIB_S57_DLL
    #define LIB_S57_EXPORT __declspec(dllexport)
#else
    #define LIB_S57_EXPORT
#endif

class LIB_S57_EXPORT S57_BuildScene
{
    S57_BuildScene(const S57_BuildScene &) {}
    S57_BuildScene & operator = (const S57_BuildScene &) { return (* this); }
public:
    S57_BuildScene();
    S57_BuildScene(const QString & _lookups_path, const QString & _symbols_path);
    ~S57_BuildScene();
	void clear();
	void setScene(QGraphicsScene * _scene);
	void build(const QString & _file_name);
	void render();
	void updateVisibility(double _scale, bool _exact = false);
    double scale() const;
	int itemsCount() const;

    void setScaleTo(S57_BuildScene * _other);
private:
	struct Data;
	Data * d;
};

#endif // S57_BuildScene_H
