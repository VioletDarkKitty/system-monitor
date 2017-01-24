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
using namespace memoryConversion;

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
    std::string mainUsedValue = dbl2str(truncateDouble(mainUsed.id,1));
    std::string mainTotalValue = dbl2str(truncateDouble(mainTotal.id,1));
    std::string memPercent = dbl2str(truncateDouble(memory,1));

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
        std::string swapUsedValue = dbl2str(truncateDouble(swapUsed.id,1));
        std::string swapTotalValue = dbl2str(truncateDouble(swapTotal.id,1));
        std::string swapPercent = dbl2str(truncateDouble(swap,1));

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

    updateMemory();
    updateSwap();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
