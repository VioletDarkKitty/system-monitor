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
#ifndef COLOURHELPER_H
#define COLOURHELPER_H

#include <QColor>
#include <QSettings>

namespace colourHelper {
    QColor createColourFromSettings(QSettings *settings, QString name, const int defaults[]);
    void saveColourToSettings(QSettings *settings, QString name, const QColor &colour);
}

#endif // COLOURHELPER_H
