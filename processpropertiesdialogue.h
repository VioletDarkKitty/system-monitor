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
#ifndef PROCESSPROPERTIESDIALOGUE_H
#define PROCESSPROPERTIESDIALOGUE_H

#include <QDialog>
#include <proc/readproc.h>
#include "workerthread.h"
#include <QTableWidget>
#include <vector>
#include <functional>
#include <utility>
#include "processtools.h"
#include "tablememoryitem.h"
#include "tablenumberitem.h"
#include "memoryconverter.h"
#include <QSettings>
using namespace processTools;

namespace Ui {
class processPropertiesDialogue;
}

class processPropertiesDialogue : public QDialog, public workerThread
{
    Q_OBJECT

public:
    explicit processPropertiesDialogue(QWidget *parent, pid_t pid, QSettings *settings);
    ~processPropertiesDialogue();

signals:
    void updateTable();

private slots:
    void updateTableData();

private:
    Ui::processPropertiesDialogue *ui;
    QSettings *settings;
    pid_t processID;
    void loop();
    QTableWidget* processInfoTable;
    enum {
        processName=0, processUser, processStatus, processMemory, processVirtual,
        processResident, processShared, processCPU, processCPUTime, processStarted,
        processNice, processPriority, processPID, processCmdLine, processLastItem
    } processPropertiesTableValue;
    QString getCpuPercentage(proc_t* p);
    std::vector<std::pair<QString,std::function<QTableWidgetItem*(proc_t*)>>> propertiesTableData;
    proc_t* before;
    double cpuTime;
    unitStandard standard;
};

#endif // PROCESSPROPERTIESDIALOGUE_H
