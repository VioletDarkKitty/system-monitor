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
double truncateDouble(double input, int prec);
memoryUnit nextMemoryUnit(memoryUnit unit);
memoryUnit prevMemoryUnit(memoryUnit unit);
memoryEntry convertMemoryUnit(double memory, memoryUnit unit, int kb=1024);
std::string dbl2str(double d);
}

#endif // MEMORYCONVERSION

