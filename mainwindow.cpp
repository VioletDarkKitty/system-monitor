#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    processesThread = new processInformationWorker(this);
    resourcesThread = new resourcesWorker(this);

    processesThread->start();
    resourcesThread->start();

    mainTabs = findChild<QTabWidget*>("tabWidgetMain");
    connect(mainTabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabChange()));

    handleTabChange();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete processesThread;
    delete resourcesThread;
    delete mainTabs;
}

void MainWindow::handleTabChange()
{
    unsigned int index = mainTabs->currentIndex();
    processesThread->setPaused(true);
    resourcesThread->setPaused(true);
    switch(index) {
        case 0:
            processesThread->setPaused(false);
        break;

        case 1:
            resourcesThread->setPaused(false);
        break;

        case 2:
        break;
    }
}
