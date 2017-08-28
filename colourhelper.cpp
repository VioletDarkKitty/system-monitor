
#include "colourhelper.h"

namespace colourHelper {

    QColor createColourFromSettings(QSettings *settings, QString name, const int defaults[])
    {
        return QColor(settings->value(name+"_R",defaults[0]).toInt(),
                      settings->value(name+"_G",defaults[1]).toInt(),
                      settings->value(name+"_B",defaults[2]).toInt());
    }

    void saveColourToSettings(QSettings *settings, QString name, const QColor &colour)
    {
        settings->setValue(name+"_R", colour.red());
        settings->setValue(name+"_G", colour.green());
        settings->setValue(name+"_B", colour.blue());
    }
}
