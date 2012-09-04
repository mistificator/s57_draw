#ifndef S52_SYMBOL_H
#define S52_SYMBOL_H

#include <QImage>
#include <QPixmap>
#include <QStringList>
#include <QMap>

class S52_Symbol
{
public:
    ~S52_Symbol();
    void setSources(const QStringList & _list);
    QImage image (const QString & _name) const;
    QPixmap pixmap (const QString & _name) const;
    QRgb color(const QString & _name) const;
    QRgb symbol_color(const QString & _name) const;

    bool setColorScheme(const QString & _scheme);
    QString colorScheme() const;

    QStringList symbols(const QString & _scheme) const;
    QStringList colorSchemes() const;

    QMap<QString, QImage> images() const;
    QMap<QString, QPixmap> pixmaps() const;

    static S52_Symbol global;
private:
    S52_Symbol();
    struct Data;
    Data * d;
};

QImage s52_symbol(const QString & _name);
QRgb s52_color(const QString & _name);
QRgb s52_symbol_color(const QString & _name);

#endif // S52_SYMBOL_H
