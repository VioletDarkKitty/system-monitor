#include "memoryconverter.h"
#include <cmath>
#include <assert.h>

memoryConverter::memoryConverter(double value, memoryUnit unit,
                                 unitStandard standard)
{
    memoryConverter::memoryEntry converted = fitMemoryValueToUnit(value, unit, getStandardKbSize(standard));
    this->memoryValue = converted.id;
    this->unit = converted.unit;
    this->standard = standard;
}

memoryConverter::memoryConverter(double value, memoryUnit unit, std::string standard)
{
    memoryConverter(value, unit, memoryConverter::stringToStandard(standard));
}

memoryConverter::memoryConverter(const memoryConverter &other)
{
    this->memoryValue = other.getValue();
    this->unit = other.getUnit();
    this->standard = other.getStandard();
}

memoryConverter& memoryConverter::operator=(const memoryConverter& other)
{
    this->memoryValue = other.getValue();
    this->unit = other.getUnit();
    this->standard = other.getStandard();
    return *this;
}

bool memoryConverter::operator<(const memoryConverter &other) const
{
    memoryUnit otherUnit = other.unit;
    double otherValue = other.memoryValue;

    if (((int)unit) < ((int)otherUnit)) { return true; }
    if (((int)unit) > ((int)otherUnit)) { return false; }
    // must be equal, return the bigger of the 2 values

    return memoryValue < otherValue;
}

double memoryConverter::getValue() const
{
    return memoryValue;
}

memoryUnit memoryConverter::getUnit() const
{
    return unit;
}

unitStandard memoryConverter::getStandard() const
{
    return standard;
}

int memoryConverter::getStandardKbSize(unitStandard standard)
{
    switch(standard) {
    case IEC:
        return 1024;
    case JEDEC:
        return 1024;
    case SI:
        return 1000;
    }
    return 1;
}

std::string memoryConverter::getStandardUnit(unitStandard standard, memoryUnit unit)
{
    static std::string IEC_units[] = {
        "B", "KiB", "MiB", "GiB", "TiB"
    };

    static std::string JEDEC_units[] = {
        "B", "KB", "MB", "GB", "TB"
    };

    static std::string SI_units[] = {
        "B", "kB", "MB", "GB", "TB"
    };

    // converting this enum into an int causes sigsegv so use the lookup map instead
    int pos = 0;
    for(; pos<memoryLookupLength; pos++) {
        if (memoryLookup[pos].unit == unit) {
            break;
        }
    }

    switch(standard) {
    case IEC:
        return IEC_units[pos];
        break;
    case JEDEC:
        return JEDEC_units[pos];
        break;
    case SI:
        return SI_units[pos];
        break;
    default:
        return "Error";
    }
}

/**
 * @brief fitMemoryValueToUnit Convert the given capacity to the most optimal unit for display
 * @param memory The capacity to convert
 * @param unit The current unit that it is in
 * @param kb How big a kb is in bytes. Use either 1024 or 1000. Debian and Ubuntu based distros have a standard
 * that says to use kb=1000 with IEC units, we must support this to be packaged
 * @return The most optimal display format as a memory entry struct
 */
memoryConverter::memoryEntry memoryConverter::fitMemoryValueToUnit(double memory, memoryUnit unit, int kb)
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

/**
 * @brief nextMemoryUnit Find the unit which is next after the given unit
 * @param unit The unit to find the next unit from
 * @return The next unit to the given unit or the given unit if there is no other unit
 */
memoryUnit memoryConverter::nextMemoryUnit(memoryUnit unit)
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
memoryUnit memoryConverter::prevMemoryUnit(memoryUnit unit)
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

void memoryConverter::convertTo(memoryUnit newUnit)
{
    memoryEntry converted = fitMemoryValueToUnit(memoryValue, newUnit, getStandardKbSize(standard));
    this->memoryValue = converted.id;
    this->unit = converted.unit;
}

memoryConverter::operator std::string()
{
    std::string buffer;
    buffer += dbl2str(truncateDouble(this->memoryValue, 2));
    buffer += " " + getStandardUnit(this->standard, this->unit);
    return buffer;
}

/**
 * @brief truncateDouble Truncate the decimal precision of a double precision floating point number
 * @param input The double to truncate
 * @param prec The precision to use
 * @return The truncated double
 */
double memoryConverter::truncateDouble(double input, unsigned int prec)
{
    return std::floor(std::pow(10, prec) * input) / std::pow(10, prec);
}

/**
 * @brief roundDouble Round a double without completely removing trailing decimal places
 * @param input The double to round
 * @param prec The number of places to round to
 * @return The rounded double
 */
double memoryConverter::roundDouble(double input, unsigned int prec)
{
    return std::roundf(std::pow(10, prec) * input) / std::pow(10, prec);
}

// from http://stackoverflow.com/questions/15165502/double-to-string-without-scientific-notation-or-trailing-zeros-efficiently
/**
 * @brief dbl2str Convert a double to a string while discarding trailing 0's produced from just casting. Taken from SO!
 * @param d The double to convert to a string
 * @return A string representation of the double
 */
std::string memoryConverter::dbl2str(double d)
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
