#include "workerthread.h"

workerThread::workerThread()
{
    shouldQuit = false;
    shouldPause = false;
    workerThreadThread = nullptr;
}

workerThread::~workerThread()
{
    // wait for the thread to exit gracefully!
    quit();
    while(running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
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
    return (workerThreadThread!=nullptr) && !shouldPause;
}

void workerThread::setPaused(bool pause)
{
    shouldPause = pause;
}

void workerThread::run()
{
    try {
        while(1) {
            loop();

            while(shouldPause) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            if (shouldQuit) {
                throw std::exception();
            }
        }
    } catch (std::exception &e) {
        shouldQuit = false;
        delete workerThreadThread;
        workerThreadThread = nullptr;
    }
}
