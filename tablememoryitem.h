#ifndef TABLEMEMORYITEM_H
#define TABLEMEMORYITEM_H
#include "memoryconversion.h"
#include <string>
using namespace memoryConversion;

class TableMemoryItem : public QTableWidgetItem
{
public:
    TableMemoryItem(memoryUnit u=b, double v=0)
        :QTableWidgetItem(calculateTxt(u,v))
    {
        if (v==0) {
            // if we set the string as N/A using calculateTxt, set the unit
            // to be as low as possible
            unit = b; // the lowest enum value
        }
        value = v;
    }
    bool operator <(const TableMemoryItem &other) const
    {
        // return true if other is greater than this
        //TableMemoryItem& otherMemoryItem = dynamic_cast<TableMemoryItem&>(other);
        memoryUnit otherUnit = other.unit;
        double otherValue = other.value;

        if (unit < otherUnit) { return true; }
        if (unit > otherUnit) { return false; }
        // must be equal, return the bigger of the 2 values

        return value < otherValue;
    }
protected:
    memoryUnit unit;
    double value;
private:
    static QString calculateTxt(memoryUnit u=b, double v=0) {
        // used for setting the string shown in the table
        // a value of 0 means we can't read it so show N/A like gnome does
        if (v>0.0) {
            std::string temp = std::to_string(v);
            stripTrailing0s(temp);
            temp += " " + unitToString(u);
            return temp.c_str();
        }
        return "N/A";
    }
};

#endif // TABLEMEMORYITEM_H

