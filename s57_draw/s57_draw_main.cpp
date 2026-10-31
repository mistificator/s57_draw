#include <QApplication>
#include <QTranslator>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif
#include "s57_drawwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#ifdef Q_WS_WIN
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
#endif
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
#endif
#ifdef RUSSIAN
    QTranslator translator;
    if (translator.load(":/global/s57_draw.qm"))
    {
        a.installTranslator(&translator);
    }
#endif
    S57_DrawWindow w;
    w.show();
    return a.exec();
}
