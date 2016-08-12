#ifndef RESOURCESWORKER_H
#define RESOURCESWORKER_H
#include <QObject>
#include "workerthread.h"
#include <QProgressBar>
#include <QLabel>
#include <string>
#include <exception>
#include <cmath>

class resourcesWorker : public QObject, public workerThread
{
    Q_OBJECT
public:
    explicit resourcesWorker(QObject *parent = 0);
    enum memoryUnit {b = 0, kb, mb, gb, tb};
    struct memoryEntry {
        double id;
        memoryUnit unit;
    };
    const memoryEntry memoryLookup[5] = {
      {0, b},
      {1, kb},
      {2, mb},
      {3, gb},
      {4, tb}
    };

signals:
    void updateResourcesUI();

private:
    void run();
    QProgressBar* memoryBar, swapBar;
    QLabel* memoryLabel;
    int memoryLookupLength = sizeof(memoryLookup) / sizeof(memoryEntry);
    resourcesWorker::memoryEntry convertMemoryUnit(double memory, resourcesWorker::memoryUnit unit, int kb = 1024);
    resourcesWorker::memoryUnit nextMemoryUnit(resourcesWorker::memoryUnit unit);
    resourcesWorker::memoryUnit prevMemoryUnit(resourcesWorker::memoryUnit unit);
    int lookupUnit(resourcesWorker::memoryUnit unit) {
        for(int i = 0; i<memoryLookupLength; i++) {
            if (memoryLookup[i].unit == unit) {
                return i;
            }
        }

        throw std::exception();
    }
    std::string unitToString(resourcesWorker::memoryUnit unit) {
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
};

#endif // RESOURCESWORKER_H
