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
#include "processpropertiesdialogue.h"
#include "ui_processpropertiesdialogue.h"

processPropertiesDialogue::processPropertiesDialogue(QWidget *parent, pid_t pid, QSettings *settings) :
    QDialog(parent),
    ui(new Ui::processPropertiesDialogue)
{
    ui->setupUi(this);

    this->settings = settings;

    processID = pid;
    before = NULL;
    cpuTime = 0;

    // store the information about constructing the table as a vector of pairs of labels and functions which fill in that label's data
    #define MARKUSED(X)  ((void)(&(X))) // stop g++ complaining
    #define varg(x,y) std::make_pair(x,[this](proc_t* p)->QTableWidgetItem*{MARKUSED(p); y;}) // macro used to reduce the amount of typing in the vector initialisation
    // add the actions to the vector
    propertiesTableData = {
        varg("Process Name",return new QTableWidgetItem(getProcessName(p));),
        varg("User",return new QTableWidgetItem(QString(p->euser) + " (" + QString::number(p->euid) + ")");),
        varg("Status",return new QTableWidgetItem(getProcessStatus(p));),
        varg("Memory",return new TableMemoryItem(memoryConverter((p->resident - p->share)*sysconf(_SC_PAGESIZE),memoryUnit::b,standard));),
        varg("Virtual Memory",return new TableMemoryItem(memoryConverter(p->vm_size,memoryUnit::kb,standard));),
        varg("Resident Memory",return new TableMemoryItem(memoryConverter(p->vm_rss,memoryUnit::kb,standard));),
        varg("Shared Memory",return new TableMemoryItem(memoryConverter(p->share*sysconf(_SC_PAGESIZE),memoryUnit::b,standard));),
        varg("CPU",return new TableNumberItem(processPropertiesDialogue::getCpuPercentage(p));),
        varg("CPU Time",{unsigned long long s = (p->stime + p->cstime + p->utime + p->cutime)/(float)sysconf(_SC_CLK_TCK);
                         unsigned long long m = s/60; s = s%60; return new TableNumberItem((QString::number(m)+":"+QString::number(s)));}),
        varg("Started",return new QTableWidgetItem(getProcessStartDate(p->start_time));),
        varg("Nice",return new TableNumberItem(QString::number(p->nice));),
        varg("Priority",return new TableNumberItem(QString::number(p->priority));),
        varg("PID",return new TableNumberItem(QString::number(p->tgid));),
        varg("Command Line",return new QTableWidgetItem(getProcessCmdline(p->tgid));)
    };

    processInfoTable = this->findChild<QTableWidget*>("processInfoTable");
    processInfoTable->verticalHeader()->setHidden(true);
    processInfoTable->horizontalHeader()->setHidden(true);
    processInfoTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    processInfoTable->setRowCount(propertiesTableData.size());
    processInfoTable->setColumnCount(2);
    // set the labels for the table using the stored data
    for(unsigned int i=0; i<propertiesTableData.size(); i++) {
        processInfoTable->setItem(i,0,new QTableWidgetItem(propertiesTableData.at(i).first));
    }

    connect(this, SIGNAL(updateTable()), this, SLOT(updateTableData()));
    connect(parent,SIGNAL(destroyed(QObject*)),this,SLOT(deleteLater()));

    // hide minimise and maximise buttons
    setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);

    this->start();
}

processPropertiesDialogue::~processPropertiesDialogue()
{
    delete ui;
}

/**
 * @brief processPropertiesDialogue::updateTableData
 * Update the process' properties table
 */
void processPropertiesDialogue::updateTableData()
{
    PROCTAB *tab = openproc(PROC_PID | PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLUSR | PROC_FILLNS);
    proc_t* p; // Don't fill the proc_t struct with any info

    tab->pids = (pid_t*)malloc(sizeof(pid_t)*2); // PROCTAB should clean this up when closeproc is called
    tab->pids[0] = processID;
    tab->pids[1] = 0; // The array should be 0 terminated

    processInfoTable->setUpdatesEnabled(false);
    if ((p=readproc(tab,NULL))==NULL) {
        // The process is dead
        // GSM shows N/A for some fields in a dead process
        processInfoTable->setItem(processStatus,1,new QTableWidgetItem("Dead"));
    } else {
        QString procName = getProcessName(p);
        // set the window title
        QString title = procName + " (" + "PID " + QString::number(p->tid) + ")";
        this->setWindowTitle(title);
        // run the function from the propertiesTableData for each row
        for(unsigned int i=0; i<propertiesTableData.size(); i++) {
            processInfoTable->setItem(i,1,propertiesTableData.at(i).second(p));
        }
    }
    processInfoTable->resizeColumnsToContents();
    processInfoTable->setUpdatesEnabled(true);

    closeproc(tab);
}

/**
 * @brief processPropertiesDialogue::getCpuPercentage Calculate the cpu % for the process
 * @param p The proc_t structure for the process to use
 * @return The string containing the process' cpu %
 */
QString processPropertiesDialogue::getCpuPercentage(proc_t* p)
{
    QString cpuPercentage;
    if (before==NULL) {
        cpuPercentage = "0%";
    } else {
        cpuPercentage = QString::number((unsigned int)calculateCPUPercentage(before,p,cpuTime)) + "%";
        free(before);
    }
    before = p;
    cpuTime = getTotalCpuTime();
    return cpuPercentage;
}

/**
 * @brief processPropertiesDialogue::loop
 * Run the loop for this thread
 */
void processPropertiesDialogue::loop()
{
    standard = memoryConverter::stringToStandard(settings->value("unit prefix standards",JEDEC).toString().toStdString());
    emit(updateTable());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
