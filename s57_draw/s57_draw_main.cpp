#include <QApplication>
#include <QTranslator>
#include <QTextCodec>
#include "s57_drawwindow.h"

#define RUSSIAN

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

#ifdef Q_WS_WIN
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
#endif
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

#ifdef RUSSIAN
    QTranslator translator;
    translator.load(":/global/s57_draw.qm");
    a.installTranslator(&translator);
#endif

    S57_DrawWindow w;
	w.show();
	
	return a.exec();
}
