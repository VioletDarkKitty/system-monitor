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
#ifndef RESOURCESWORKER_H
#define RESOURCESWORKER_H
#include <QObject>
#include "workerthread.h"
#include <QProgressBar>
#include <QLabel>
#include "qcustomplot.h"
#include <vector>
#include <QVector>
#include "cputools.h"
#include <deque>
#include <QSettings>
#include "memoryconverter.h"
#include <QHash>

typedef QVector<QVector<double>> qcustomplotCpuVector;
typedef QVector<QVector<memoryConverter>> qcustomplotNetworkVector;

// qhash wont take an array as a value for some reason, ptr not working
struct struct__intArrayHolder {
    int array[3];
};

class resourcesWorker : public QObject, public workerThread
{
    Q_OBJECT
public:
    explicit resourcesWorker(QObject *parent, QSettings *settings);
    const QHash<QString, struct__intArrayHolder> getColourDefaults() {
        return defaultColours;
    }

    ~resourcesWorker();
signals:
    void updateMemoryBar(int value);
    void updateMemoryText(QString value);
    void updateSwapBar(int value);
    void updateSwapText(QString value);
    void updateCpuPlotSIG(const qcustomplotCpuVector &values);
    void updateNetworkRecieving(QString value);
    void updateNetworkRecievingTotal(QString value);
    void updateNetworkSending(QString value);
    void updateNetworkSendingTotal(QString value);
    void updateNetworkPlotSIG(const qcustomplotNetworkVector &values);
private:
    void loop();
    QProgressBar *memoryBar, *swapBar;
    QLabel *memoryLabel, *swapLabel;
    void updateMemory();
    void updateSwap();
    void updateCpu();
    std::vector<cpuTools::cpuStruct> prevCpuTimes;
    std::deque<std::vector<double>> cpuPlotData;
    QSettings *settings;
    unitStandard standard;
    QLabel *networkRecievingLabel, *networkRecievingTotalLabel, *networkSendingLabel, *networkSendingTotalLabel;
    void updateNetwork();
    QPushButton *networkRecievingColourButton, *networkSendingColourButton;
    QHash<QString, struct__intArrayHolder> defaultColours;
public slots:
    void createColourDialogue();
};

#endif // RESOURCESWORKER_H
