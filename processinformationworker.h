#ifndef PROCESSINFORMATIONWORKER_H
#define PROCESSINFORMATIONWORKER_H
#include <QObject>
#include <proc/readproc.h>
#include <vector>
#include "workerthread.h"
#include <QTableWidget>

class processInformationWorker : public QObject, public workerThread
{
    Q_OBJECT
public:
    explicit processInformationWorker(QObject *parent = 0);
    std::vector<proc_t>* getProcesses();

private slots:
    void handleProcessStop();
    void updateTable();

signals:
    void updateTableData();

private:
    void loop();
    QTableWidget* processesTable;
    void createProcessesView();
};

#endif // PROCESSINFORMATIONWORKER_H
