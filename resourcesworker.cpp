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
#include "cputools.h"
#include "memoryconverter.h"
using namespace cpuTools;

resourcesWorker::resourcesWorker(QObject *parent, QSettings *settings)
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

    this->settings = settings;
}

resourcesWorker::~resourcesWorker()
{

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

    QVector<QVector<double>> plottingData = QVector<QVector<double>>();

    // The data is arranged in vectors but each vector has points that are intended
    // for multiple plots on multi core machines.
    // Seperate out the data by reading the number of doubles in the initial vector
    if (cpuPlotData.size() == 0) {
        return; // obviously we cant read the initial vector if it is blank
    }

    for(unsigned int i=0; i<cpuPlotData.at(0).size(); i++) {
        QVector<double> headingVector;
        headingVector.push_back(cpuPlotData.at(0).at(i));
        plottingData.push_back(headingVector);
    }

    // now that the initial qvectors are filled, we can append the rest of the data
    for(unsigned int i=1; i<cpuPlotData.size(); i++) {
        for(unsigned int j=0; j<cpuPlotData.at(i).size(); j++) {
            plottingData[j].push_back(cpuPlotData.at(i).at(j));
        }
    }

    // there might not be enough data in each array so pad with 0s on the front if so
    for(int i=0; i<plottingData.size(); i++) {
        for(unsigned int j=60 - plottingData.at(i).size(); j>0; j--) {
            plottingData[i].push_front((double)0);
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

    memoryConverter mainUsed = memoryConverter(kb_main_used,memoryUnit::kb,standard);
    memoryConverter mainTotal = memoryConverter(kb_main_total,memoryUnit::kb,standard);

    std::string memPercent = memoryConverter::dbl2str(memoryConverter::truncateDouble(memoryConverter::roundDouble(memory, 1),1));

    std::string memoryText = mainUsed.to_string() + " (" + memPercent + "%) of " + mainTotal.to_string();
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

        memoryConverter swapUsed = memoryConverter(kb_swap_used,memoryUnit::kb,standard);
        memoryConverter swapTotal = memoryConverter(kb_swap_total,memoryUnit::kb,standard);

        // cleanup the swap values
        std::string swapPercent = memoryConverter::dbl2str(memoryConverter::truncateDouble(memoryConverter::roundDouble(swap, 1),1));

        std::string swapText = swapUsed.to_string() + " (" + swapPercent + "%) of " + swapTotal.to_string();
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

    standard = memoryConverter::stringToStandard(settings->value("unit prefix standards", JEDEC).toString().toStdString());

    updateCpu();
    updateMemory();
    updateSwap();

    if (settings->value("resources update interval",1.0).toDouble() < 0.25) {
        settings->setValue("resources update interval",0.25);
    }
    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(
                                    settings->value("resources update interval",1.0).toDouble() * 1000));
}
