#ifndef PROCESSINFORMATIONWORKER_H
#define PROCESSINFORMATIONWORKER_H
#include <QObject>
#include <thread>
#include <proc/readproc.h>
#include <vector>

class processInformationWorker : public QObject
{
    Q_OBJECT
public:
    explicit processInformationWorker(QObject *parent = 0);
    void quit();
    void start();
    bool running();
    void setPaused(bool pause);
    std::vector<proc_t>* getProcesses();

signals:
    void updateProcessUI();

private:
    std::thread* workerThread;
    bool shouldQuit;
    bool shouldPause;
    void getProcessesInformation();
    std::vector<proc_t> procs;
};

#endif // PROCESSINFORMATIONWORKER_H
