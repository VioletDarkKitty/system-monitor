#include "processinformationworker.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <exception>
#include <QTabWidget>
#include <QAction>
#include <string>
#include <pwd.h>
#include "tablenumberitem.h"
#include <signal.h>
#include <QMessageBox>
#include <QHeaderView>
#include "tablememoryitem.h"
#include "memoryconversion.h"

processInformationWorker::processInformationWorker(QObject *parent) :
    QObject(parent), workerThread() {
    QTabWidget* mainTabs = parent->findChild<QTabWidget*>("tabWidgetMain");
    processesTable = mainTabs->findChild<QTableWidget*>("tableProcesses");

    QAction* actionStop = new QAction("Stop",processesTable);
    connect(actionStop,SIGNAL(triggered(bool)),SLOT(handleProcessStop()));

    QList<QAction*> rightClickActions;
    rightClickActions.push_back(actionStop);
    rightClickActions.push_back(new QAction("Kill",processesTable));
    rightClickActions.push_back(new QAction("Properties",processesTable));

    processesTable->setContextMenuPolicy(Qt::ActionsContextMenu);
    processesTable->addActions(rightClickActions);

    connect(this,SIGNAL(updateTableData()),SLOT(updateTable()));

    createProcessesView();
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
    std::cout << row << std::endl;
    // get PID
    std::cout << processesTable->item(row,3)->text().toStdString() << std::endl;
    int pid = processesTable->item(row,3)->text().toInt();

    QMessageBox::StandardButton reply;
    std::string info = "Are you sure you want to stop "+processesTable->item(row,0)->text().toStdString();
    reply = QMessageBox::question(processesTable, "Info", info.c_str(),
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
        std::cout << "Killing " << pid << std::endl;
    }
}

void processInformationWorker::updateTable() {
    // from http://codingrelic.geekhold.com/2011/02/listing-processes-with-libproc.html
    PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLUSR);
    // proc_info must be static! https://gitlab.com/procps-ng/procps/issues/33
    static proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_t));

    std::vector<proc_t> processes;
    while (readproc(proc, &proc_info) != NULL) {
        /*printf("%20s:\t%5ld\t%5lld\t%5lld\n",
        proc_info.cmd, proc_info.resident,
        proc_info.utime, proc_info.stime);*/
        processes.push_back(proc_info);
    }
    closeproc(proc);

    processesTable->setUpdatesEnabled(false); // avoid inconsistant data
    processesTable->setSortingEnabled(false);
    processesTable->setRowCount(processes.size());
    //processesTable->clearContents();
    for(unsigned int i=0; i<processes.size(); i++) {
        proc_t* p = &(processes.at(i));
        QString commandName = p->cmd;
        processesTable->setItem(i,0,new QTableWidgetItem(commandName));
        QString user = p->euser;
        processesTable->setItem(i,1,new QTableWidgetItem(user));
        QString cpu = std::to_string(p->pcpu).c_str();
        processesTable->setItem(i,2,new TableNumberItem(cpu));
        QString id = std::to_string(p->tid).c_str();
        processesTable->setItem(i,3,new TableNumberItem(id));
        memoryEntry memory = convertMemoryUnit(p->vm_rss,kb);
        processesTable->setItem(i,4,new TableMemoryItem(memory.unit,truncateDouble(memory.id,1)));
    }
    processesTable->setUpdatesEnabled(true);
    processesTable->setSortingEnabled(true);
}

void processInformationWorker::loop()
{
    emit(updateTableData());
}

