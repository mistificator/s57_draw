#include <QApplication>
#include <QTranslator>
#include "s57_drawwindow.h"

#define RUSSIAN

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

#ifdef RUSSIAN
    QTranslator translator;
    translator.load(":/global/s57_draw.qm");
    a.installTranslator(&translator);
#endif

    S57_DrawWindow w;
	w.show();
	
	return a.exec();
}
