#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include "processinformationworker.h"
#include <chrono>
#include <thread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mainTabs = findChild<QTabWidget*>("tabWidgetMain");
    connect(mainTabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabChange()));

    processesTable = mainTabs->findChild<QTableWidget*>("tableProcesses");
    processesThreadStarted = false;

    handleTabChange();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::stopRunningProcessesThread()
{
    if (processesThreadStarted) {
        processesThread.quit();
        while(processesThread.running()) {
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

    if (processesThreadStarted) {
        stopRunningProcessesThread();
        processesThread.start();
    } else {
        processesThread.start();
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

