#ifndef PROCESSINFORMATIONWORKER_H
#define PROCESSINFORMATIONWORKER_H
#include <QObject>
#include <thread>

class processInformationWorker : public QObject
{
    Q_OBJECT
public:
    explicit processInformationWorker(QObject *parent = 0);
    void quit();
    void start();
    bool running();

private:
    std::thread* workerThread;
    bool shouldQuit;
    void getProcessesInformation();
};

#endif // PROCESSINFORMATIONWORKER_H
