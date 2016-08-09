#include "processinformationworker.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <exception>

processInformationWorker::processInformationWorker(QObject *parent) :
    QObject(parent)
{
    shouldQuit = false;
    workerThread = nullptr;
}

void processInformationWorker::start()
{
    if (workerThread!=nullptr) {
        throw std::exception();
    }

    workerThread = new std::thread(
                &processInformationWorker::getProcessesInformation,this);
    workerThread->detach();
}

void processInformationWorker::quit()
{
    shouldQuit = true;
}

bool processInformationWorker::running()
{
    return (workerThread!=nullptr);
}

void processInformationWorker::getProcessesInformation()
{
    try {
        while(1) {
            std::cout << "A" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            if (shouldQuit) {
                throw std::exception();
            }
        }
    } catch (std::exception &e) {
        shouldQuit = false;
        delete workerThread;
        workerThread = nullptr;
    }
}
