/*
 * Copyright (C) 2017 Lily Rivers (VioletDarkKitty)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
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
#ifndef MEMORYCONVERSION_H
#define MEMORYCONVERSION_H
#include <string>
#include <exception>
#include <cmath>

namespace memoryConversion {
//#define stripTrailing0s(x) x.erase((x.find_last_not_of('0')==std::string::npos? x.find_last_not_of('0')+2:x.find_last_not_of('0')+1),std::string::npos);
#define memoryLookupLength 5
enum class memoryUnit {b = 0, kb, mb, gb, tb};
struct memoryEntry {
    double id;
    memoryUnit unit;
};
extern const memoryEntry memoryLookup[memoryLookupLength];
extern const memoryEntry memoryLookup[5];
int lookupUnit(memoryUnit unit);
std::string unitToString(memoryUnit unit);
double truncateDouble(double input, unsigned int prec);
memoryUnit nextMemoryUnit(memoryUnit unit);
memoryUnit prevMemoryUnit(memoryUnit unit);
memoryEntry convertMemoryUnit(double memory, memoryUnit unit, int kb=1024);
std::string dbl2str(double d);
}

#endif // MEMORYCONVERSION

