#include "memoryconversion.h"

namespace memoryConversion {
    const memoryEntry memoryLookup[memoryLookupLength] = {
      {0, memoryUnit::b},
      {1, memoryUnit::kb},
      {2, memoryUnit::mb},
      {3, memoryUnit::gb},
      {4, memoryUnit::tb}
    };

    int lookupUnit(memoryUnit unit) {
        for(int i = 0; i<memoryLookupLength; i++) {
            if (memoryLookup[i].unit == unit) {
                return i;
            }
        }

        throw std::exception();
    }

    std::string unitToString(memoryUnit unit) {
        std::string strings[] = {
            "b",
            "Kb",
            "Mb",
            "Gb",
            "Tb"
        };
        return strings[lookupUnit(unit)];
    }

    double truncateDouble(double input, int prec) {
        return std::floor(std::pow(10, prec) * input) / std::pow(10, prec);
    }

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
}
