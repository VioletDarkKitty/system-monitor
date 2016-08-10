#include "processinformationworker.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <exception>

processInformationWorker::processInformationWorker(QObject *parent) :
    QObject(parent)
{
    shouldQuit = false;
    shouldPause = false;
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
    shouldPause = false;
}

bool processInformationWorker::running()
{
    return (workerThread!=nullptr);
}

void processInformationWorker::getProcessesInformation()
{
    try {
        while(1) {
            //std::cout << "A" << std::endl;

            // from http://codingrelic.geekhold.com/2011/02/listing-processes-with-libproc.html
            PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS);
            // proc_info must be static! https://gitlab.com/procps-ng/procps/issues/33
            static proc_t proc_info;
            memset(&proc_info, 0, sizeof(proc_t));
            procs.clear();

            while (readproc(proc, &proc_info) != NULL) {
                /*printf("%20s:\t%5ld\t%5lld\t%5lld\n",
                proc_info.cmd, proc_info.resident,
                proc_info.utime, proc_info.stime);*/
                procs.push_back(proc_info);
            }
            closeproc(proc);
            emit(updateProcessUI());

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            while(shouldPause) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

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

void processInformationWorker::setPaused(bool pause)
{
    shouldPause = pause;
}

std::vector<proc_t>* processInformationWorker::getProcesses()
{
    return &procs;
}
