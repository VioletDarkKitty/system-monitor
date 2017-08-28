#ifndef COLOURHELPER_H
#define COLOURHELPER_H

#include <QColor>
#include <QSettings>

namespace colourHelper {
    QColor createColourFromSettings(QSettings *settings, QString name, const int defaults[]);
    void saveColourToSettings(QSettings *settings, QString name, const QColor &colour);
}

#endif // COLOURHELPER_H
