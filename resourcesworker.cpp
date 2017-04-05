/*
 * Copyright (C) 2017 Lily Rivers (VioletDarkKitty)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "resourcesworker.h"
#include <proc/sysinfo.h>
#include <iostream>
#include <string>
#include "memoryconversion.h"
#include "cputools.h"
using namespace memoryConversion;
using namespace cpuTools;

resourcesWorker::resourcesWorker(QObject *parent)
    : QObject(parent), workerThread()
{
    memoryBar = parent->findChild<QProgressBar*>("memoryBar");
    memoryLabel = parent->findChild<QLabel*>("memoryLabel");
    swapBar = parent->findChild<QProgressBar*>("swapBar");
    swapLabel = parent->findChild<QLabel*>("swapLabel");

    connect(this,SIGNAL(updateMemoryBar(int)),memoryBar,SLOT(setValue(int)));
    connect(this,SIGNAL(updateMemoryText(QString)),memoryLabel,SLOT(setText(QString)));
    connect(this,SIGNAL(updateSwapBar(int)),swapBar,SLOT(setValue(int)));
    connect(this,SIGNAL(updateSwapText(QString)),swapLabel,SLOT(setText(QString)));
    plottingData = nullptr;
}

resourcesWorker::~resourcesWorker()
{
    if (plottingData != nullptr) {
        delete plottingData;
    }
}

void resourcesWorker::updateCpu()
{
    std::vector<cpuStruct> cpuTimes = getCpuTimes();
    if (prevCpuTimes.size() != 0) {
        std::vector<double> cpuPercentages = calculateCpuPercentages(cpuTimes, prevCpuTimes);

        /*for(unsigned int i=0; i<cpuPercentages.size(); i++) {
            std::cout << cpuPercentages.at(i) << std::endl;
        }*/

        if (cpuPlotData.size() == 60) {
            cpuPlotData.pop_front();
        }
        cpuPlotData.push_back(cpuPercentages);
    }
    prevCpuTimes = cpuTimes;

    // construct the qvectors to use to plot
    if (plottingData != nullptr) {
        delete plottingData;
    }
    plottingData = new QVector<QVector<double>>();

    // The data is arranged in vectors but each vector has points that are intended
    // for multiple plots on multi core machines.
    // Seperate out the data by reading the number of doubles in the initial vector
    if (cpuPlotData.size() == 0) {
        return; // obviously we cant read the initial vector if it is blank
    }

    for(unsigned int i=0; i<cpuPlotData.at(0).size(); i++) {
        QVector<double> headingVector;
        headingVector.push_back(cpuPlotData.at(0).at(i));
        plottingData->push_back(headingVector);
    }

    // now that the initial qvectors are filled, we can append the rest of the data
    for(unsigned int i=1; i<cpuPlotData.size(); i++) {
        for(unsigned int j=0; j<cpuPlotData.at(i).size(); j++) {
            (*plottingData)[j].push_back(cpuPlotData.at(i).at(j));
        }
    }

    // there might not be enough data in each array so pad with 0s on the front if so
    for(int i=0; i<plottingData->size(); i++) {
        for(unsigned int j=60 - plottingData->at(i).size(); j>0; j--) {
            (*plottingData)[i].push_front((double)0);
        }
    }

    emit(updateCpuPlotSIG(plottingData));
}

/**
 * @brief resourcesWorker::updateMemory Calculate the used memory and emit signals
 */
void resourcesWorker::updateMemory()
{
    double memory = ((double)kb_main_used / kb_main_total) * 100;
    emit(updateMemoryBar(memory));

    memoryEntry mainUsed = convertMemoryUnit(kb_main_used,memoryUnit::kb);
    memoryEntry mainTotal = convertMemoryUnit(kb_main_total,memoryUnit::kb);

    // cleanup the memory values
    std::string mainUsedValue = dbl2str(truncateDouble(roundDouble(mainUsed.id),1));
    std::string mainTotalValue = dbl2str(truncateDouble(roundDouble(mainTotal.id),1));
    std::string memPercent = dbl2str(truncateDouble(roundDouble(memory),1));

    std::string memoryText = mainUsedValue + unitToString(mainUsed.unit)
            + " (" + memPercent + "%) of " + mainTotalValue + unitToString(mainTotal.unit);
    emit(updateMemoryText(QString::fromStdString(memoryText)));
}

/**
 * @brief resourcesWorker::updateSwap Calculate used swap and emit the appropriate signals
 * If there is no swap, display 0
 */
void resourcesWorker::updateSwap()
{
    if (kb_swap_total > 0.0) {
        // swap is active
        double swap = ((double)kb_swap_used / kb_swap_total) * 100;
        emit(updateSwapBar(swap));
        memoryEntry swapUsed = convertMemoryUnit(kb_swap_used,memoryUnit::kb);
        memoryEntry swapTotal = convertMemoryUnit(kb_swap_total,memoryUnit::kb);

        // cleanup the swap values
        std::string swapUsedValue = dbl2str(truncateDouble(roundDouble(swapUsed.id),1));
        std::string swapTotalValue = dbl2str(truncateDouble(roundDouble(swapTotal.id),1));
        std::string swapPercent = dbl2str(truncateDouble(roundDouble(swap),1));

        std::string swapText = swapUsedValue + unitToString(swapUsed.unit)
                + " (" + swapPercent + "%) of " + swapTotalValue + unitToString(swapTotal.unit);
        emit(updateSwapText(QString::fromStdString(swapText)));
    } else {
        // there is no swap
        emit(updateSwapBar(0));
        emit(updateSwapText("Not Available"));
    }
}

/**
 * @brief resourcesWorker::loop Loop this threads execution
 */
void resourcesWorker::loop()
{
    meminfo(); // have procps read the memory
    //std::cout << kb_main_used << "/" << kb_main_total << std::endl;
    //std::cout << memory << "%" << std::endl;

    updateCpu();
    updateMemory();
    updateSwap();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
