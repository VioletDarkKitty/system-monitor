#include "processinformationworker.h"
#include <iostream>

processInformationWorker::processInformationWorker(QObject *parent) :
    QObject(parent)
{

}


void processInformationWorker::getProcessesInformation()
{
    while(1) {
        std::cout << "A" << std::endl;
    }
}
