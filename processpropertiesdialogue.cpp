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
#include "processpropertiesdialogue.h"
#include "ui_processpropertiesdialogue.h"

processPropertiesDialogue::processPropertiesDialogue(QWidget *parent, pid_t pid) :
    QDialog(parent),
    ui(new Ui::processPropertiesDialogue)
{
    ui->setupUi(this);

    processID = pid;
    before = NULL;
    cpuTime = 0;

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

void processPropertiesDialogue::updateTableData()
{
    PROCTAB *tab = openproc(PROC_PID | PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLUSR | PROC_FILLNS);
    proc_t* p; // Don't fill the proc_t struct with any info

    tab->pids = (pid_t*)malloc(sizeof(pid_t*)*2); // PROCTAB should clean this up when closeproc is called
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
        for(unsigned int i=0; i<propertiesTableData.size(); i++) {
            processInfoTable->setItem(i,1,propertiesTableData.at(i).second(p));
        }
    }
    processInfoTable->resizeColumnsToContents();
    processInfoTable->setUpdatesEnabled(true);

    closeproc(tab);
}

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

void processPropertiesDialogue::loop()
{
    emit(updateTable());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
