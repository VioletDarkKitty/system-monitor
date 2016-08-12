#include "workerthread.h"

workerThread::workerThread()
{
    shouldQuit = false;
    shouldPause = false;
    workerThreadThread = nullptr;
}

void workerThread::start()
{
    if (workerThreadThread!=nullptr) {
        throw std::exception();
    }

    workerThreadThread = new std::thread(
                &workerThread::run,this);
    workerThreadThread->detach();
}

void workerThread::quit()
{
    shouldQuit = true;
    shouldPause = false;
}

bool workerThread::running()
{
    return (workerThreadThread!=nullptr);
}

void workerThread::setPaused(bool pause)
{
    shouldPause = pause;
}

void workerThread::cleanupThread()
{
    shouldQuit = false;
    delete workerThreadThread;
    workerThreadThread = nullptr;
}
