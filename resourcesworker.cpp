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
    cpuPlot = dynamic_cast<QCustomPlot*>(parent->findChild<QWidget*>("cpuPlot"));
    cpuPlot = new QCustomPlot(parent->findChild<QTabWidget*>("tabWidgetMain"));

    connect(this,SIGNAL(updateMemoryBar(int)),memoryBar,SLOT(setValue(int)));
    connect(this,SIGNAL(updateMemoryText(QString)),memoryLabel,SLOT(setText(QString)));
    connect(this,SIGNAL(updateSwapBar(int)),swapBar,SLOT(setValue(int)));
    connect(this,SIGNAL(updateSwapText(QString)),swapLabel,SLOT(setText(QString)));
    connect(this,SIGNAL(updateCpuPlotSIG(QVector<double>)),this,SLOT(updateCpuPlotSLO(QVector<double>)));

    // add 0'd data to the cpuPlotData at (60 seconds / update time) times. The default update time is 1 second
    /// TODO: add way to change update time
    std::vector<double> data;
    data.push_back(0);
    for(unsigned int i=0; i<(60/1); i++) {
        cpuPlotData.push_back(data);
    }
}

void resourcesWorker::updateCpuPlotSLO(QVector<double> values)
{
    QVector<double> x(61); // initialize with entries 60..0
    for (int i=61; i<0; ++i)
    {
      x[i] = i;
    }

    cpuPlot->clearGraphs();
    cpuPlot->addGraph();
    cpuPlot->graph(0)->setData(x, values);

    //customPlot->xAxis->setLabel("x");
    //customPlot->yAxis->setLabel("y");

    cpuPlot->xAxis->setRange(0, 60);
    cpuPlot->yAxis->setRange(0, 100);
}

void resourcesWorker::updateCpu()
{
    std::vector<cpuStruct> cpuTimes = getCpuTimes();
    if (prevCpuTimes.size() != 0) {
        std::vector<double> cpuPercentages = calculateCpuPercentages(cpuTimes, prevCpuTimes);

        /*for(unsigned int i=0; i<cpuPercentages.size(); i++) {
            std::cout << cpuPercentages.at(i) << std::endl;
        }*/

        cpuPlotData.pop_front();
        cpuPlotData.push_back(cpuPercentages);
    }
    prevCpuTimes = cpuTimes;

    // construct the qvectors to use to plot
    QVector<double> plotting;
    for(unsigned int i=0; i<cpuPlotData.size(); i++) {
        for(unsigned int j=0; j<cpuPlotData.at(i).size(); j++) {
            plotting.push_back(cpuPlotData.at(i).at(j));
        }
    }
    emit(updateCpuPlotSLO(plotting));
    //updateCpuPlotSLO(plotting);

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
