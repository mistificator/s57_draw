#include <QApplication>
#include <QTranslator>
#include "s57_drawwindow.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

    QTranslator translator;
    translator.load(":/global/s57_draw.qm");
    a.installTranslator(&translator);

    S57_DrawWindow w;
	w.show();
	
	return a.exec();
}
