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
#include "processinformationworker.h"
#include <QTabWidget>
#include <pwd.h>
#include "tablenumberitem.h"
#include <signal.h>
#include <QMessageBox>
#include <QHeaderView>
#include "tablememoryitem.h"
#include "memoryconversion.h"
#include "processpropertiesdialogue.h"
#include "processtools.h"
#include <proc/sysinfo.h>
#include <QAction>
using namespace processTools;

processInformationWorker::processInformationWorker(QObject *parent) :
    QObject(parent), workerThread() {
    mainWindow = parent;
    QTabWidget* mainTabs = parent->findChild<QTabWidget*>("tabWidgetMain");
    processesTable = mainTabs->findChild<QTableWidget*>("tableProcesses");

    loadAverage = parent->findChild<QLabel*>("loadAvgLabel");
    connect(this,SIGNAL(updateLoadAverage(QString)),loadAverage,SLOT(setText(QString)));

    totalCpuTime = 0;
    selectedRowInfoID = 0;

    QAction* actionStop = new QAction("Stop",processesTable);
    connect(actionStop,SIGNAL(triggered(bool)),SLOT(handleProcessStop()));

    QAction* actionProperties = new QAction("Properties",processesTable);
    connect(actionProperties,SIGNAL(triggered(bool)),this,SLOT(showProcessProperties()));

    QList<QAction*> rightClickActions;
    rightClickActions.push_back(actionStop);
    rightClickActions.push_back(new QAction("Kill",processesTable));
    rightClickActions.push_back(actionProperties);

    processesTable->setContextMenuPolicy(Qt::ActionsContextMenu);
    processesTable->addActions(rightClickActions);

    filterCheckbox = mainTabs->findChild<QCheckBox*>("processesFilterCheckbox");
    searchField = mainTabs->findChild<QLineEdit*>("processesSearchField");

    connect(searchField,SIGNAL(textChanged(QString)),this,SLOT(filterProcesses(QString)));
    connect(this,SIGNAL(signalFilterProcesses(QString)),this,SLOT(filterProcesses(QString)));
    connect(processesTable->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this,SLOT(changeCurrentTableRowSelection(QModelIndex)));

    connect(this,SIGNAL(updateTableData()),SLOT(updateTable()));    
    createProcessesView();
}

void processInformationWorker::showProcessProperties()
{
    processPropertiesDialogue* properties = new processPropertiesDialogue((QWidget*)mainWindow, selectedRowInfoID);
    properties->show();
    properties->exec();
}

void processInformationWorker::changeCurrentTableRowSelection(QModelIndex current)
{
    // remember the tid so we can get the correct row again when we update the table
    selectedRowInfoID = processesTable->item(current.row(),3)->text().toInt();
}

void processInformationWorker::createProcessesView()
{
    processesTable->setColumnCount(5);
    processesTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //table->setHorizontalHeaderLabels(QString("HEADER 1;HEADER 2;HEADER 3").split(";"));
    processesTable->setHorizontalHeaderLabels(QString("Process Name;User;% CPU;PID;Memory;").split(";"));
    processesTable->verticalHeader()->setVisible(false);
    processesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    processesTable->resizeColumnsToContents();
}

void processInformationWorker::handleProcessStop()
{
    int row = processesTable->currentIndex().row();
    // get PID
    int pid = processesTable->item(row,3)->text().toInt();

    QMessageBox::StandardButton reply;
    std::string info = "Are you sure you want to stop "+processesTable->item(row,0)->text().toStdString();
    reply = QMessageBox::question(processesTable, "Info", QString::fromStdString(info),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // attempt to send signal
        if (kill(pid,SIGTERM)!=0) {
            if (errno==EPERM) {
                // show a dialogue that we did not have permission to SIGTERM pid
                QMessageBox::information(processesTable, "Error",
                                "Could not send signal, permission denied!", QMessageBox::Ok);
            }
        }
    }
}

bool processInformationWorker::shouldHideProcess(unsigned int pid)
{
    if (filterCheckbox->checkState() == Qt::CheckState::Checked) {
        // if the filter checkbox is checked, we need to make sure that the
        // user who owns the process is the same as the user running us
        return (pid != geteuid());
    }
    return false;
}

void processInformationWorker::filterProcesses(QString filter)
{
    // loop over the table and hide items which do not match the search term given
    // only check col 0 (process name)
    for(int i = 0; i < processesTable->rowCount(); i++)
    {
        // check if the row is already hidden, if so, skip it
        /// TODO: find a better fix for this, can't check using QTableWidget
        /// so just check the PID manually
        if (!shouldHideProcess(getpwnam(processesTable->item(i,1)->text().toStdString().c_str())->pw_uid)) {
            QTableWidgetItem* item = processesTable->item(i,0); // process name
            processesTable->setRowHidden(i, !(item->text().contains(filter)));
        }
    }
}

void processInformationWorker::updateTable() {
    // from http://codingrelic.geekhold.com/2011/02/listing-processes-with-libproc.html
    PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLUSR | PROC_FILLCOM);
    // proc_info must be static! https://gitlab.com/procps-ng/procps/issues/33
    static proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_t));

    storedProcType processes;
    while (readproc(proc, &proc_info) != NULL) {
        processes[proc_info.tid]=proc_info;
    }
    closeproc(proc);

    // fill in cpu%
    if (prevProcs.size()>0) {
        // we have previous proc info
        for(auto &newItr:processes) {
            for(auto &prevItr:prevProcs) {
                if (newItr.first == prevItr.first) {
                    // PID matches, calculate the cpu
                    newItr.second.pcpu = (unsigned int)calculateCPUPercentage(&prevItr.second,&newItr.second,totalCpuTime);
                    break;
                }
            }
        }
    }
    // update the cpu time for next loop
    totalCpuTime = getTotalCpuTime();

    processesTable->setUpdatesEnabled(false); // avoid inconsistant data
    processesTable->setSortingEnabled(false);
    processesTable->setRowCount(processes.size());
    unsigned int index = 0;
    for(auto &i:processes) {
        proc_t* p = (&i.second);
        processesTable->setItem(index,0,new QTableWidgetItem(getProcessName(p)));
        QString user = p->euser;
        processesTable->setItem(index,1,new QTableWidgetItem(user));
        //std::cout << p->pcpu << std::endl;
        QString cpu = QString::number(p->pcpu);
        processesTable->setItem(index,2,new TableNumberItem(cpu));
        QString id = QString::number(p->tid);
        processesTable->setItem(index,3,new TableNumberItem(id));
        // gnome-system-monitor measures memory as RSS - shared
        // shared memory is only given as # pages, sysconf(_SC_PAGES) is in bytes
        // so do, (#pages RSS - #pages Share) * _SC_PAGES
        memoryEntry memory = convertMemoryUnit((p->resident - p->share)*sysconf(_SC_PAGESIZE),memoryUnit::b);
        processesTable->setItem(index,4,new TableMemoryItem(memory.unit,truncateDouble(memory.id,1)));
        processesTable->showRow(index);

        if (shouldHideProcess(p->euid)) {
            // hide this row
            processesTable->hideRow(index);
        }

        // update the row selection to reflect the row that was previously selected
        if (selectedRowInfoID>0) {
            if (selectedRowInfoID == p->tid) {
                // this is the same process
                processesTable->selectRow(index);
            }
        }

        index++;
    }

    processesTable->setUpdatesEnabled(true);
    processesTable->setSortingEnabled(true);
    emit(signalFilterProcesses(searchField->text()));

    // keep processes we've read for cpu calculations next cycle
    prevProcs = processes;
}

void processInformationWorker::loop()
{
    emit(updateTableData());
    // get the load average
    double load[3];
    getloadavg(load,3);
    // work out how overloaded the system was in the last minute based on CPU alone
    // yes, linux uses more than just CPU in the load calculations
    double overload = load[0] - smp_num_cpus;
    QString avg = "Load averages for the last 1, 5, 15 minutes: " + QString::number(load[0])
            + ", " + QString::number(load[1]) + ", " + QString::number(load[2])
            + (overload<0? "":"\nOverloaded by " + QString::number(overload*100) + "% in the last minute!");
    emit(updateLoadAverage(avg));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

