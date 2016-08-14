#include "resourcesworker.h"
#include <proc/sysinfo.h>
#include <iostream>
#include <string>

resourcesWorker::resourcesWorker(QObject *parent)
    : QObject(parent), workerThread()
{
    memoryBar = parent->findChild<QProgressBar*>("memoryBar");
    memoryLabel = parent->findChild<QLabel*>("memoryLabel");
}

resourcesWorker::memoryUnit resourcesWorker::nextMemoryUnit(resourcesWorker::memoryUnit unit)
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

resourcesWorker::memoryUnit resourcesWorker::prevMemoryUnit(resourcesWorker::memoryUnit unit)
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

resourcesWorker::memoryEntry resourcesWorker::convertMemoryUnit(double memory,
                            resourcesWorker::memoryUnit unit, int kb)
{
    if (memory < (1/kb)) {
        // too small for the unit it's in
        while(memory < 0.001 && unit != b) {
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

void resourcesWorker::loop()
{
    meminfo();
    std::cout << kb_main_used << "/" << kb_main_total << std::endl;
    double memory = ((double)kb_main_used / kb_main_total) * 100;
    std::cout << memory << "%" << std::endl;
    memoryBar->setValue(memory);
    memoryEntry mainUsed = convertMemoryUnit(kb_main_used,kb);
    memoryEntry mainTotal = convertMemoryUnit(kb_main_total,kb);

    #define stripTrailing0s(x) x.erase(x.find_last_not_of('0')+1,std::string::npos);
    std::string mainUsedValue = std::to_string(truncateDouble(mainUsed.id,1));
    stripTrailing0s(mainUsedValue);
    std::string mainTotalValue = std::to_string(truncateDouble(mainTotal.id,1));
    stripTrailing0s(mainTotalValue);
    std::string memPercent = std::to_string(truncateDouble(memory,1));
    stripTrailing0s(memPercent);

    std::string memoryText = mainUsedValue + unitToString(mainUsed.unit)
            + " (" + memPercent + "%) of " + mainTotalValue + unitToString(mainTotal.unit);
    memoryLabel->setText(memoryText.c_str());
}
