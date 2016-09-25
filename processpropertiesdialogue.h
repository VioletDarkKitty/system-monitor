#ifndef PROCESSPROPERTIESDIALOGUE_H
#define PROCESSPROPERTIESDIALOGUE_H

#include <QDialog>
#include <proc/readproc.h>
#include "workerthread.h"
#include <QTableWidget>
#include <vector>
#include <functional>
#include <utility>
#include "processtools.h"
#include "tablememoryitem.h"
#include "tablenumberitem.h"
#include "memoryconversion.h"
using namespace processTools;

namespace Ui {
class processPropertiesDialogue;
}

class processPropertiesDialogue : public QDialog, public workerThread
{
    Q_OBJECT

public:
    explicit processPropertiesDialogue(QWidget *parent, pid_t pid);
    ~processPropertiesDialogue();

signals:
    void updateTable();

private slots:
    void updateTableData();

private:
    Ui::processPropertiesDialogue *ui;
    pid_t processID;
    void loop();
    QTableWidget* processInfoTable;
    enum {
        processName=0, processUser, processStatus, processMemory, processVirtual,
        processResident, processShared, processCPU, processCPUTime, processStarted,
        processNice, processPriority, processPID, processCmdLine, processLastItem
    } processPropertiesTableValue;
    QString getCpuPercentage(proc_t* p);
    // store the information about constructing the table as a vector of pairs of labels and functions which fill in that label's data
    #define MARKUSED(X)  ((void)(&(X))) // stop g++ complaining
    #define varg(x,y) std::make_pair(x,[this](proc_t* p)->auto{MARKUSED(p); y}) // macro used to reduce the amount of typing in the vector initialisation
    std::vector<std::pair<QString,std::function<QTableWidgetItem*(proc_t*)>>> propertiesTableData = {
        varg("Process Name",return new QTableWidgetItem(getProcessName(p));),
        varg("User",return new QTableWidgetItem(QString(p->euser) + " (" + QString::number(p->euid) + ")");),
        varg("Status",return new QTableWidgetItem(getProcessStatus(p));),
        varg("Memory",memoryEntry memory = convertMemoryUnit((p->resident - p->share)*sysconf(_SC_PAGESIZE),memoryUnit::b);
                        return new TableMemoryItem(memory.unit,truncateDouble(memory.id,1));),
        varg("Virtual Memory",memoryEntry virtualMemory = convertMemoryUnit(p->vm_size,memoryUnit::kb);
                        return new TableMemoryItem(virtualMemory.unit,truncateDouble(virtualMemory.id,1));),
        varg("Resident Memory",memoryEntry residentMemory = convertMemoryUnit(p->vm_rss,memoryUnit::kb);
                        return new TableMemoryItem(residentMemory.unit,truncateDouble(residentMemory.id,1));),
        varg("Shared Memory",memoryEntry sharedMemory = convertMemoryUnit(p->share*sysconf(_SC_PAGESIZE),memoryUnit::b);
                        return new TableMemoryItem(sharedMemory.unit,truncateDouble(sharedMemory.id,1));),
        varg("CPU",return new TableNumberItem(processPropertiesDialogue::getCpuPercentage(p));),
        varg("CPU Time",{unsigned long long s = (p->stime + p->cstime + p->utime + p->cutime)/sysconf(_SC_CLK_TCK);
                         unsigned long long m = s/60; s = s%60;
                        return new TableNumberItem((QString::number(m)+":"+QString::number(s)));}),
        varg("Started",return new QTableWidgetItem(getProcessStartDate(p->start_time));),
        varg("Nice",return new TableNumberItem(QString::number(p->nice));),
        varg("Priority",return new TableNumberItem(QString::number(p->priority));),
        varg("PID",return new TableNumberItem(QString::number(p->tgid));),
        varg("Command Line",return new QTableWidgetItem(getProcessCmdline(p->tgid));)
    };
    proc_t* before;
    unsigned long long cpuTime;
};

#endif // PROCESSPROPERTIESDIALOGUE_H
