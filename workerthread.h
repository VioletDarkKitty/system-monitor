#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H
#include <thread>

class workerThread
{
public:
    workerThread();
    void quit();
    void start();
    bool running();
    void setPaused(bool pause);
protected:
    bool shouldQuit;
    bool shouldPause;
    void cleanupThread();
private:
    virtual void run(){}
    std::thread* workerThreadThread;
};

#endif // WORKERTHREAD_H
