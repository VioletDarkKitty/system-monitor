/*
 * Copyright (C) 2017 Lily Rivers (VioletDarkKitty)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 3
 * of the License only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
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
