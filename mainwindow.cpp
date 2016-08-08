#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mainTabs = findChild<QTabWidget*>("tabWidgetMain");
    connect(mainTabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabChange()));

    processesTable = mainTabs->findChild<QTableWidget*>("tableProcesses");
    /*processesThreadStarted = false;*/

    handleTabChange();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createProcessesView()
{
    processesTable->setColumnCount(5);
    processesTable->setRowCount(10);
    processesTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //table->setHorizontalHeaderLabels(QString("HEADER 1;HEADER 2;HEADER 3").split(";"));
    processesTable->setHorizontalHeaderLabels(QString("Process Name;User;% CPU;PID;Memory;").split(";"));
    processesTable->verticalHeader()->setVisible(false);

    /*if (processesThreadStarted) {
        processesThread->quit();
        processesThread->wait();
    } else {
        processesThread = new QThread();
        processesThreadStarted = true;
    }

    connect(processesThread, SIGNAL(started()), this, SLOT(getProcessesInformation()));
    processesThread->start();*/
}

void MainWindow::handleTabChange()
{
    unsigned int index = mainTabs->currentIndex();
    switch(index) {
        case 0:
        default:
            // switched to processes tab
            createProcessesView();
        break;
    }
}

