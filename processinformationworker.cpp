#include "processinformationworker.h"
#include <iostream>
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
#include <unistd.h>
#include <experimental/filesystem>
#include <fstream>

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

    filterCheckbox = mainTabs->findChild<QCheckBox*>("processesFilterCheckbox");
    searchField = mainTabs->findChild<QLineEdit*>("processesSearchField");

    connect(searchField,SIGNAL(textChanged(QString)),this,SLOT(filterProcesses(QString)));
    connect(this,SIGNAL(signalFilterProcesses(QString)),this,SLOT(filterProcesses(QString)));

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

std::string processInformationWorker::getProcessNameFromPID(unsigned int pid)
{
    std::string temp;
    try {
        std::fstream fs;
        fs.open ("/proc/"+std::to_string(pid)+"/cmdline", std::fstream::in);
        fs >> temp;
        fs.close();
    } catch(std::ifstream::failure e) {
        return "FAILED TO READ PROC";
    }

    if (temp.size()<1) {
        return "";
    }

    return std::experimental::filesystem::path(temp).filename();
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
    for(unsigned int i=0; i<processes.size(); i++) {
        proc_t* p = &(processes.at(i));
        std::string processName = getProcessNameFromPID(p->tid);
        processesTable->setItem(i,0,new QTableWidgetItem(processName!=""? processName.c_str():p->cmd));
        QString user = p->euser;
        processesTable->setItem(i,1,new QTableWidgetItem(user));
        QString cpu = std::to_string(p->pcpu).c_str();
        processesTable->setItem(i,2,new TableNumberItem(cpu));
        QString id = std::to_string(p->tid).c_str();
        processesTable->setItem(i,3,new TableNumberItem(id));
        // gnome-system-monitor measures memory as RSS - shared
        // shared memory is only given as # pages, sysconf(_SC_PAGES) is in bytes
        // so do, (#pages RSS - #pages Share) * _SC_PAGES
        memoryEntry memory = convertMemoryUnit((p->resident - p->share)*sysconf(_SC_PAGESIZE),memoryUnit::b);
        processesTable->setItem(i,4,new TableMemoryItem(memory.unit,truncateDouble(memory.id,1)));
        processesTable->showRow(i);

        if (shouldHideProcess(p->euid)) {
            // hide this row
            processesTable->hideRow(i);
        }
    }
    processesTable->setUpdatesEnabled(true);
    processesTable->setSortingEnabled(true);
    emit(signalFilterProcesses(searchField->text()));
}

void processInformationWorker::loop()
{
    emit(updateTableData());
}

