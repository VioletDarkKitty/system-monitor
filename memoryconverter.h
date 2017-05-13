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
    //memoryConverter & operator=(const memoryConverter &other);
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
    unitStandard getStandard() const;

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

/*std::string& operator+(std::string& s, const memoryConverter &m)
{
    //std::string cast = m;
    //s += cast;
    return s;
}

std::string operator+(memoryConverter &m, const std::string &s)
{
    std::string cast = m;
    cast += s;
    return cast;
}

std::string operator+(memoryConverter &m, const char *s)
{
    std::string buffer = std::string(s);
    std::string cast = m;
    return cast + buffer;
}*/

inline std::string to_string(memoryConverter m)
{
    std::string temp = m;
    return temp;
}

#endif // MEMORYUNIT_H
