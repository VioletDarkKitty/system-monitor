/*
 * Copyright (C) 2017 Lily Rivers (VioletDarkKitty)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 3
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
#include <ctime>
#include "colourhelper.h"
using namespace cpuTools;
using namespace colourHelper;

resourcesWorker::resourcesWorker(QObject *parent, QSettings *settings)
    : QObject(parent), workerThread()
{
    memoryBar = parent->findChild<QProgressBar*>("memoryBar");
    memoryLabel = parent->findChild<QLabel*>("memoryLabel");
    swapBar = parent->findChild<QProgressBar*>("swapBar");
    swapLabel = parent->findChild<QLabel*>("swapLabel");

    networkRecievingLabel = parent->findChild<QLabel*>("networkRecievingLabel");
    networkRecievingTotalLabel = parent->findChild<QLabel*>("networkTotalRecievedLabel");
    networkRecievingColourButton = parent->findChild<QPushButton*>("networkRecievingColourButton");
    networkSendingLabel = parent->findChild<QLabel*>("networkSendingLabel");
    networkSendingTotalLabel = parent->findChild<QLabel*>("networkTotalSentLabel");
    networkSendingColourButton = parent->findChild<QPushButton*>("networkSendingColourButton");

    connect(this,SIGNAL(updateMemoryBar(int)),memoryBar,SLOT(setValue(int)));
    connect(this,SIGNAL(updateMemoryText(QString)),memoryLabel,SLOT(setText(QString)));
    connect(this,SIGNAL(updateSwapBar(int)),swapBar,SLOT(setValue(int)));
    connect(this,SIGNAL(updateSwapText(QString)),swapLabel,SLOT(setText(QString)));

    connect(this,SIGNAL(updateNetworkRecieving(QString)),networkRecievingLabel,SLOT(setText(QString)));
    connect(this,SIGNAL(updateNetworkRecievingTotal(QString)),networkRecievingTotalLabel,SLOT(setText(QString)));
    connect(networkRecievingColourButton, SIGNAL(clicked(bool)), this, SLOT(createColourDialogue()), Qt::UniqueConnection);
    connect(this,SIGNAL(updateNetworkSending(QString)),networkSendingLabel,SLOT(setText(QString)));
    connect(this,SIGNAL(updateNetworkSendingTotal(QString)),networkSendingTotalLabel,SLOT(setText(QString)));
    connect(networkSendingColourButton, SIGNAL(clicked(bool)), this, SLOT(createColourDialogue()), Qt::UniqueConnection);

    /// TODO: this is horrible
    struct__intArrayHolder defaultRecieving;
    memset(&defaultRecieving, 0, sizeof(struct__intArrayHolder));
    defaultRecieving.array[2] = 255;
    struct__intArrayHolder defaultSending;
    memset(&defaultSending, 0, sizeof(struct__intArrayHolder));
    defaultSending.array[0] = 255;
    defaultColours[networkRecievingColourButton->objectName()] = defaultRecieving;
    defaultColours[networkSendingColourButton->objectName()] = defaultSending;

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

void resourcesWorker::createColourDialogue()
{
    QColor defaultColour = colourHelper::createColourFromSettings(settings, sender()->objectName(), defaultColours[sender()->objectName()].array);
    QColor newColour = QColorDialog::getColor(defaultColour, (QWidget*) this->parent());
    if (newColour.isValid()) {
        colourHelper::saveColourToSettings(settings, sender()->objectName(), newColour);
    }
}

/**
 * @brief resourcesWorker::updateMemory Calculate the used memory and emit signals
 */
void resourcesWorker::updateMemory()
{
    memoryConverter mainUsed;
    // GSM includes cached/buffered memory in the used memory but excludes shared memory
    if (settings->value("cachedMemoryIsUsed", false).toBool()) {
        mainUsed = memoryConverter(kb_main_used + kb_main_buffers + kb_main_cached - kb_main_shared,memoryUnit::kb,standard);
    } else {
        mainUsed = memoryConverter(kb_main_used,memoryUnit::kb,standard);
    }

    memoryConverter mainTotal = memoryConverter(kb_main_total,memoryUnit::kb,standard);

    memoryConverter mainUsedCopy = mainUsed;
    mainUsedCopy.convertTo(mainTotal.getUnit());
    double memory = (mainUsedCopy.getValue() / mainTotal.getValue()) * 100;
    emit(updateMemoryBar(memory));
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
 * @brief resourcesWorker::updateNetwork Search for network interfaces and total their sending and recieving bytes for display
 * and then display them
 */
void resourcesWorker::updateNetwork()
{
    static long long lastSentBytes = 0, lastRecievedBytes = 0;
    long long totalSentBytes = 0, totalRecievedBytes = 0;

    // search in /sys/class/net for interfaces and total their bytes
    // ignore lo as this could look like an idle disconnected system is still sending data
    /// TODO: have an option to include lo traffic

    QDirIterator dirIt("/sys/class/net",QDirIterator::NoIteratorFlags);
    while (dirIt.hasNext()) {
        dirIt.next();

        if (dirIt.fileName() == "." || dirIt.fileName() == "..") {
            continue;
        }

        // follow symbolic links to check for folders that link to ../../devices/virtual/[name] as these are not true interfaces, lo is also virtual
        QFileInfo fInfo = QFileInfo(dirIt.filePath());
        QString path = fInfo.canonicalFilePath();

        if (path.contains("/devices/virtual/net/")) {
            continue;
        }

        // total bytes from this folder
        QFile sending(QString(path + "/statistics/tx_bytes"));
        sending.open(QIODevice::ReadOnly);
        //qDebug() << "tx " << dirIt.fileName() << sending.readLine();
        totalSentBytes += QString(sending.readLine()).toLongLong();
        QFile recieving(QString(path + "/statistics/rx_bytes"));
        recieving.open(QIODevice::ReadOnly);
        totalRecievedBytes += QString(recieving.readLine()).toLongLong();
    }

    long long sentSinceLastCheck = totalSentBytes - lastSentBytes;
    long long recievedSinceLastCheck = totalRecievedBytes - lastRecievedBytes;

    // used to scale the per second measurements to actually be for one second
    double waitDuration = settings->value("resources update interval",1.0).toDouble();

    memoryConverter sending = memoryConverter(sentSinceLastCheck / waitDuration,memoryUnit::b,standard);
    emit(updateNetworkSending(QString::fromStdString(sending.to_string())+"/s"));

    memoryConverter recieving = memoryConverter(recievedSinceLastCheck / waitDuration,memoryUnit::b,standard);
    emit(updateNetworkRecieving(QString::fromStdString(recieving.to_string())+"/s"));

    memoryConverter sent = memoryConverter(totalSentBytes,memoryUnit::b,standard);
    emit(updateNetworkSendingTotal(QString::fromStdString(sent.to_string())));

    memoryConverter recieved = memoryConverter(totalRecievedBytes,memoryUnit::b,standard);
    emit(updateNetworkRecievingTotal(QString::fromStdString(recieved.to_string())));

    // prepare the data for plotting
    // this qvector is of 2 elements, the first being recieving and the second sending
    static QVector<QVector<memoryConverter>> plottingData = QVector<QVector<memoryConverter>>();

    if (plottingData.empty()) {
        // fill with initial data
        for(int i=0; i<2; i++) {
            plottingData.push_back(QVector<memoryConverter>());
            for(unsigned int j=0; j<60; j++) {
                plottingData[i].push_back(memoryConverter(0,memoryUnit::b,standard));
            }
        }
    }

    if (lastSentBytes != 0) {
        plottingData[0].pop_front();
        plottingData[0].push_back(recieving);
        plottingData[1].pop_front();
        plottingData[1].push_back(sending);

        emit(updateNetworkPlotSIG(plottingData));
    }

    lastSentBytes = totalSentBytes;
    lastRecievedBytes = totalRecievedBytes;

    QPixmap recievingColour = QPixmap(networkRecievingColourButton->width(), networkRecievingColourButton->height());
    recievingColour.fill(createColourFromSettings(settings, networkRecievingColourButton->objectName(),
                                                  defaultColours[networkRecievingColourButton->objectName()].array));
    networkRecievingColourButton->setIcon(QIcon(recievingColour));

    QPixmap sendingColour = QPixmap(networkSendingColourButton->width(), networkSendingColourButton->height());
    sendingColour.fill(createColourFromSettings(settings, networkSendingColourButton->objectName(),
                                                defaultColours[networkSendingColourButton->objectName()].array));
    networkSendingColourButton->setIcon(QIcon(sendingColour));
}

/**
 * @brief resourcesWorker::loop Loop this threads execution
 */
void resourcesWorker::loop()
{
    clock_t begin = clock();

    if (settings->value("resources update interval",1.0).toDouble() < 0.25) {
        settings->setValue("resources update interval",0.25);
    }

    meminfo(); // have procps read the memory
    //std::cout << kb_main_used << "/" << kb_main_total << std::endl;
    //std::cout << memory << "%" << std::endl;

    standard = memoryConverter::stringToStandard(settings->value("unit prefix standards", JEDEC).toString().toStdString());

    updateCpu();
    updateMemory();
    updateSwap();
    updateNetwork();

    // only wait for enough time to fill the interval as some time has been used for calculations
    clock_t end = clock();
    double elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;

    double waitTime = settings->value("resources update interval",1.0).toDouble() - elapsedSecs;

    if (waitTime >= 0) {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(waitTime* 1000));
    }
}
