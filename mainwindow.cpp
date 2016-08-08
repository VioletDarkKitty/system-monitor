#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "processeswidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    currentPane = 0;

    QPushButton* buttonProcesses = findChild<QPushButton*>("buttonProcesses");
    connect(buttonProcesses, SIGNAL (released()), this, SLOT (handleProcessesButton()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateMainWidgetSelection()
{
    widgetMainData = findChild<QWidget*>("widgetMainData");
    switch(currentPane) {
        case 1:
        default:
            widgetMainData = new ProcessesWidget(this);
        break;
    }
}

void MainWindow::handleProcessesButton()
{
    currentPane = 1;
    updateMainWidgetSelection();
}
