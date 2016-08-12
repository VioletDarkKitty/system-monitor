#ifndef PROCESSINFORMATIONWORKER_H
#define PROCESSINFORMATIONWORKER_H
#include <QObject>
#include <proc/readproc.h>
#include <vector>
#include "workerthread.h"

class processInformationWorker : public QObject, public workerThread
{
    Q_OBJECT
public:
    explicit processInformationWorker(QObject *parent = 0);
    std::vector<proc_t>* getProcesses();

signals:
    void updateProcessUI();

private:
    void run();
    std::vector<proc_t> procs;
};

#endif // PROCESSINFORMATIONWORKER_H
