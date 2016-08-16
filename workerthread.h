#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H
#include <thread>

class workerThread
{
public:
    workerThread();
    virtual ~workerThread();
    void quit();
    void start();
    bool running();
    void setPaused(bool pause);    
private:
    bool shouldQuit;
    bool shouldPause;
    void run();
    virtual void loop(){}
    std::thread* workerThreadThread;
};

#endif // WORKERTHREAD_H
