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
#include "memoryconversion.h"

namespace memoryConversion {
    const memoryEntry memoryLookup[memoryLookupLength] = {
      {0, memoryUnit::b},
      {1, memoryUnit::kb},
      {2, memoryUnit::mb},
      {3, memoryUnit::gb},
      {4, memoryUnit::tb}
    };

    /**
     * @brief lookupUnit Return the int value of the unit. A byte is 0, all others are sequential
     * @param unit The unit to lookup
     * @return The position of the unit in the array
     */
    int lookupUnit(memoryUnit unit)
    {
        for(int i = 0; i<memoryLookupLength; i++) {
            if (memoryLookup[i].unit == unit) {
                return i;
            }
        }

        throw std::exception();
    }

    /**
     * @brief unitToString Return a string representation of the unit
     * @param unit The unit to return the string for
     * @return The string representation of the unit
     */
    std::string unitToString(memoryUnit unit)
    {
        std::string strings[] = {
            "b",
            "Kb",
            "Mb",
            "Gb",
            "Tb"
        };
        return strings[lookupUnit(unit)];
    }

    /**
     * @brief truncateDouble Truncate the decimal precision of a double precision floating point number
     * @param input The double to truncate
     * @param prec The precision to use
     * @return The truncated double
     */
    double truncateDouble(double input, unsigned int prec)
    {
        return std::floor(std::pow(10, prec) * input) / std::pow(10, prec);
    }

    /**
     * @brief nextMemoryUnit Find the unit which is next after the given unit
     * @param unit The unit to find the next unit from
     * @return The next unit to the given unit or the given unit if there is no other unit
     */
    memoryUnit nextMemoryUnit(memoryUnit unit)
    {
        for(int i = 0; i<memoryLookupLength; i++) {
            if (memoryLookup[i].unit == unit) {
                if (memoryLookup[i].id+1 > memoryLookupLength-1) {
                    break;
                }
                return memoryLookup[i+1].unit;
            }
        }
        return unit;
    }

    /**
     * @brief prevMemoryUnit Find the previous unit which is before the given unit
     * @param unit The unit to find the previous unit from
     * @return The previous unit or the current unit if there is no previous
     */
    memoryUnit prevMemoryUnit(memoryUnit unit)
    {
        for(int i = 0; i<memoryLookupLength; i++) {
            if (memoryLookup[i].unit == unit) {
                if (memoryLookup[i].id-1 < 0) {
                    break;
                }
                return memoryLookup[i-1].unit;
            }
        }
        return unit;
    }

    /**
     * @brief convertMemoryUnit Convert the given capacity to the most optimal unit for display
     * @param memory The capacity to convert
     * @param unit The current unit that it is in
     * @param kb How big a kb is in bytes. Use either 1024 or 1000. Debian and Ubuntu based distros have
     * @return The most optimal display format as a memory entry struct
     */
    memoryEntry convertMemoryUnit(double memory, memoryUnit unit, int kb)
    {
        if (memory < (1/kb)) {
            // too small for the unit it's in
            while(memory < (1/kb) && unit != memoryUnit::b) {
                memory *= kb;
                unit = prevMemoryUnit(unit);
            }
        }
        else
        {
            while(memory > kb) {
                // too big for the unit it's in
                memory /= kb;
                unit = nextMemoryUnit(unit);
            }
        }
        return {memory, unit};
    }

    // from http://stackoverflow.com/questions/15165502/double-to-string-without-scientific-notation-or-trailing-zeros-efficiently
    /**
     * @brief dbl2str Convert a double to a string while discarding trailing 0's produced from just casting. Taken from SO!
     * @param d The double to convert to a string
     * @return A string representation of the double
     */
    std::string dbl2str(double d)
    {
        size_t len = std::snprintf(0, 0, "%.10f", d);
        std::string s(len+1, 0);
        // technically non-portable, see below
        std::snprintf(&s[0], len+1, "%.10f", d);
        // remove nul terminator
        s.pop_back();
        // remove trailing zeros
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        // remove trailing point
        if(s.back() == '.') {
            s.pop_back();
        }
        return s;
    }
}
