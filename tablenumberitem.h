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
#ifndef TABLENUMBERITEM_H
#define TABLENUMBERITEM_H

// from http://stackoverflow.com/questions/7848683/how-to-sort-datas-in-qtablewidget

class TableNumberItem : public QTableWidgetItem
{
public:
    TableNumberItem(const QString txt = QString("0"))
        :QTableWidgetItem(txt)
    {
    }
    bool operator <(const QTableWidgetItem &other) const
    {
        QString str1 = text();
        QString str2 = other.text();

        /*if (str1[0] == '$' || str1[0] == '€') {
            str1.remove(0, 1);
            str2.remove(0, 1); // we assume both items have the same format
        }*/

        if (str1[str1.length() - 1] == '%') {
            str1.chop(1);
            str2.chop(1); // this works for "N%" and for "N %" formatted strings
        }

    return str1.toDouble() < str2.toDouble();
    }
};

#endif // TABLENUMBERITEM_H
