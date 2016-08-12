#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include "processinformationworker.h"
#include <chrono>
#include <thread>
#include <string>
#include <pwd.h>
#include "tablenumberitem.h"
#include <signal.h>
#include <QMessageBox>

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

    resourcesThread = new resourcesWorker(this);

    QAction* actionStop = new QAction("Stop",processesTable);
    connect(actionStop,SIGNAL(triggered(bool)),SLOT(handleProcessStop()));

    QList<QAction*> rightClickActions;
    rightClickActions.push_back(actionStop);
    rightClickActions.push_back(new QAction("Kill",processesTable));
    rightClickActions.push_back(new QAction("Properties",processesTable));

    processesTable->setContextMenuPolicy(Qt::ActionsContextMenu);
    processesTable->addActions(rightClickActions);

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

void MainWindow::handleProcessStop()
{
    int row = processesTable->currentIndex().row();
    std::cout << row << std::endl;
    // get PID
    std::cout << processesTable->item(row,3)->text().toStdString() << std::endl;
    int pid = processesTable->item(row,3)->text().toInt();

    QMessageBox::StandardButton reply;
    std::string info = "Are you sure you want to stop "+processesTable->item(row,0)->text().toStdString();
    reply = QMessageBox::question(this, "Info", info.c_str(),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // attempt to send signal
        if (kill(pid,SIGTERM)!=0) {
            if (errno==EPERM) {
                // show a dialogue that we did not have permission to SIGTERM pid
                QMessageBox::information(this, "Error",
                                "Could not send signal, permission denied!", QMessageBox::Ok);
            }
        }
    }

    std::cout << "Killing " << pid << std::endl;
}

void MainWindow::handleTabChange()
{
    unsigned int index = mainTabs->currentIndex();
    if (resourcesThread->running()){resourcesThread->quit();}
    switch(index) {
        case 1:
            stopRunningProcessesThread();
            resourcesThread->start();
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
    processesTable->setUpdatesEnabled(false); // avoid inconsistant data
    std::vector<proc_t>* processes = processesThread->getProcesses();
    processesTable->setRowCount(processes->size());
    //processesTable->clearContents();
    for(unsigned int i=0; i<processes->size(); i++) {
        proc_t* p = &(processes->at(i));
        QString commandName = p->cmd;
        processesTable->setItem(i,0,new QTableWidgetItem(commandName));
        QString user = p->euser; //getpwuid(p->euid)->pw_name;
        processesTable->setItem(i,1,new QTableWidgetItem(user));
        QString cpu = std::to_string(p->pcpu).c_str();
        processesTable->setItem(i,2,new TableNumberItem(cpu));
        QString id = std::to_string(p->tid).c_str();
        processesTable->setItem(i,3,new TableNumberItem(id));
        QString mem = std::to_string(p->vm_size).c_str();
        processesTable->setItem(i,4,new TableNumberItem(mem));
    }
    processesThread->setPaused(false);
    processesTable->repaint();
    processesTable->setUpdatesEnabled(true);
}
