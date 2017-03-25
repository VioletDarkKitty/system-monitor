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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialogue.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    processesThread = new processInformationWorker(this);
    resourcesThread = new resourcesWorker(this);
    filesystemThread = new fileSystemWorker(this);

    processesThread->start();
    resourcesThread->start();
    filesystemThread->start();

    quitAction = this->findChild<QAction*>("actionQuit");
    connect(quitAction,SIGNAL(triggered(bool)),QApplication::instance(),SLOT(quit()));

    aboutAction = this->findChild<QAction*>("actionAbout");
    connect(aboutAction,SIGNAL(triggered(bool)),this,SLOT(showAboutWindow()));

    mainTabs = findChild<QTabWidget*>("tabWidgetMain");
    connect(mainTabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabChange()));

    cpuPlot = reinterpret_cast<QCustomPlot*>(ui->tabResources->findChild<QWidget*>("cpuPlot"));
    connect(resourcesThread,SIGNAL(updateCpuPlotSIG(const qcustomplotCpuVector*)),
            this,SLOT(updateCpuPlotSLO(const qcustomplotCpuVector*)));

    handleTabChange();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete processesThread;
    delete resourcesThread;
    delete filesystemThread;
    delete mainTabs;
}

void MainWindow::updateCpuPlotSLO(const qcustomplotCpuVector *values)
{
    QVector<double> x(60); // initialize with entries 60..0
    for (int i=59; i>0; --i)
    {
      x[i] = i;
    }

    static bool previouslyPlotted = false;
    int size = values->size();
    if (size == 0) {
        return;
    }

    #define colourNamesLen 4
    const QString colourNames[] = {
        "yellow","red","green","blue"
    };
    for(int i=0; i<size; i++) {
        if (!previouslyPlotted) {
            cpuPlot->addGraph();
        } else {
            cpuPlot->graph(i)->data()->clear();
            cpuPlot->graph(i)->setPen(QPen(QColor(colourNames[i % colourNamesLen])));
        }
        cpuPlot->graph(i)->setData(x, values->at(i));
    }
    previouslyPlotted = true;

    //customPlot->xAxis->setLabel("x");
    //customPlot->yAxis->setLabel("y");

    cpuPlot->xAxis->setRange(0, 60);
    cpuPlot->yAxis->setRange(0, 100);
    cpuPlot->replot();
}

/**
 * @brief MainWindow::handleTabChange Handle when the tab is changed, pause the other threads
 */
void MainWindow::handleTabChange()
{
    unsigned int index = mainTabs->currentIndex();
    processesThread->setPaused(true);
    resourcesThread->setPaused(true);
    filesystemThread->setPaused(true);
    switch(index) {
        case 0:
            processesThread->setPaused(false);
        break;

        case 1:
            resourcesThread->setPaused(false);
        break;

        case 2:
            filesystemThread->setPaused(false);
        break;
    }
}

/**
 * @brief MainWindow::showAboutWindow Show the about dialogue without blocking the main thread
 */
void MainWindow::showAboutWindow()
{
    AboutDialogue* about = new AboutDialogue(this);
    //about->show();
    about->exec();
}
