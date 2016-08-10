#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include "processinformationworker.h"
#include <chrono>
#include <thread>
#include <string>
#include <pwd.h>
#include "tablenumberitem.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mainTabs = findChild<QTabWidget*>("tabWidgetMain");
    connect(mainTabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabChange()));

    processesTable = mainTabs->findChild<QTableWidget*>("tableProcesses");
    processesThreadStarted = false;
    processesThread = new processInformationWorker(this);
    connect(processesThread, SIGNAL(updateProcessUI()), this,
            SLOT(updateProcessInformation()));

    handleTabChange();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete processesThread;
    delete processesTable;
    delete mainTabs;
}

void MainWindow::stopRunningProcessesThread()
{
    if (processesThreadStarted) {
        processesThread->quit();
        while(processesThread->running()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        processesThreadStarted = false;
    }
}

void MainWindow::createProcessesView()
{
    processesTable->setColumnCount(5);
    processesTable->setRowCount(10);
    processesTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //table->setHorizontalHeaderLabels(QString("HEADER 1;HEADER 2;HEADER 3").split(";"));
    processesTable->setHorizontalHeaderLabels(QString("Process Name;User;% CPU;PID;Memory;").split(";"));
    processesTable->verticalHeader()->setVisible(false);
    processesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    processesTable->resizeColumnsToContents();

    if (processesThreadStarted) {
        stopRunningProcessesThread();
        processesThread->start();
    } else {
        processesThread->start();
        processesThreadStarted = true;
    }
}

void MainWindow::handleTabChange()
{
    unsigned int index = mainTabs->currentIndex();
    switch(index) {
        case 1:
            stopRunningProcessesThread();
        break;

        case 2:
            stopRunningProcessesThread();
        break;

        case 0:
        default:
            // switched to processes tab
            createProcessesView();
        break;
    }
}

void MainWindow::updateProcessInformation()
{
    processesThread->setPaused(true);
    std::vector<proc_t>* processes = processesThread->getProcesses();
    processesTable->setRowCount(processes->size());
    //processesTable->clearContents();
    for(unsigned int i=0; i<processes->size(); i++) {
        proc_t* p = &(processes->at(i));
        QString commandName = p->cmd;
        processesTable->setItem(i,0,new QTableWidgetItem(commandName));
        QString user = getpwuid(p->ruid)->pw_name;
        processesTable->setItem(i,1,new QTableWidgetItem(user));
        QString cpu = std::to_string(p->pcpu).c_str();
        processesTable->setItem(i,2,new TableNumberItem(cpu));
        QString id = std::to_string(p->tid).c_str();
        processesTable->setItem(i,3,new TableNumberItem(id));
        QString mem = std::to_string(p->vm_size).c_str();
        processesTable->setItem(i,4,new TableNumberItem(mem));
    }
    processesTable->repaint();
    processesThread->setPaused(false);
}
