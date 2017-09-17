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
#ifndef TABLEMEMORYITEM_H
#define TABLEMEMORYITEM_H
#include <QString>
#include "memoryconverter.h"

class TableMemoryItem : public QTableWidgetItem
{
public:
    TableMemoryItem(memoryConverter memory)
        :QTableWidgetItem(calculateTxt(&memory))
    {
        this->memory = memory;
    }

    bool operator <(const QTableWidgetItem &other) const
    {
        // return true if other is greater than this
        if (other.text() == "N/A") { return false; }
        const TableMemoryItem& otherMemoryItem = dynamic_cast<const TableMemoryItem&>(other);

        return memory < otherMemoryItem.memory;
    }
protected:
    memoryConverter memory;
private:
    static QString calculateTxt(memoryConverter *memory) {
        if (memory->getValue() > 0.0) {
            return QString::fromStdString(memory->to_string());
        }
        return "N/A";
    }
};

#endif // TABLEMEMORYITEM_H

