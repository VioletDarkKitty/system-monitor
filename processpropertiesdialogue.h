#ifndef PROCESSPROPERTIESDIALOGUE_H
#define PROCESSPROPERTIESDIALOGUE_H

#include <QDialog>
#include <proc/readproc.h>
#include "workerthread.h"
#include <QTableWidget>

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
        processResident, processShared, processCPU, processStarted
    };
    /*const int processName = 0, processUser = 1, processStatus = 2, processMemory = 3,
              processVirtual = 4, processResident = 5, processShared = 6, processCPU = 7,
              processStarted = 8;*/
};

#endif // PROCESSPROPERTIESDIALOGUE_H
