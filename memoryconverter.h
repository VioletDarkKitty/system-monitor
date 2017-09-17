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
#ifndef MEMORYUNIT_H
#define MEMORYUNIT_H

#include <string>

enum memoryUnit {b = 0, kb, mb, gb, tb};
enum unitStandard {
    IEC, JEDEC, SI
};

class memoryConverter
{
public:
    memoryConverter(double value, memoryUnit unit, unitStandard standard);
    memoryConverter(double value, memoryUnit unit, std::string standard);
    memoryConverter() {
        memoryValue = 0;
        unit = b;
        standard = JEDEC;
    }
    memoryConverter(const memoryConverter &other);
    memoryConverter& operator=(const memoryConverter& other);
    bool operator <(const memoryConverter &other) const;
    operator std::string();
    void convertTo(memoryUnit newUnit);
    static unitStandard stringToStandard(std::string standard) {
        if (standard == "IEC") {
            return IEC;
        } else if (standard == "JEDEC") {
            return JEDEC;
        } else if (standard == "SI") {
            return SI;
        }
        return JEDEC;
    }
    static double truncateDouble(double input, unsigned int prec);
    static double roundDouble(double input, unsigned int prec);
    static std::string dbl2str(double d);
    double getValue() const;
    memoryUnit getUnit() const;
    std::string getUnitAsString();
    unitStandard getStandard() const;
    std::string to_string();

private:
    double memoryValue;
    memoryUnit unit;
    unitStandard standard;
    struct memoryEntry {
        double id;
        memoryUnit unit;
    };
    int getStandardKbSize(unitStandard standard);
    std::string getStandardUnit(unitStandard standard, memoryUnit unit);
    memoryEntry fitMemoryValueToUnit(double memory, memoryUnit unit, int kb);
    #define memoryLookupLength 5
    const memoryEntry memoryLookup[memoryLookupLength] = {
      {0, memoryUnit::b},
      {1, memoryUnit::kb},
      {2, memoryUnit::mb},
      {3, memoryUnit::gb},
      {4, memoryUnit::tb}
    };
    memoryUnit nextMemoryUnit(memoryUnit unit);
    memoryUnit prevMemoryUnit(memoryUnit unit);
};

#endif // MEMORYUNIT_H
