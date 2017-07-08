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
#include "preferencesdialogue.h"
#include "cSpline.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings = new QSettings("system-monitor","system-monitor");

    processesThread = new processInformationWorker(this, settings);
    resourcesThread = new resourcesWorker(this, settings);
    filesystemThread = new fileSystemWorker(this, settings);

    processesThread->start();
    resourcesThread->start();
    filesystemThread->start();

    quitAction = this->findChild<QAction*>("actionQuit");
    connect(quitAction,SIGNAL(triggered(bool)),QApplication::instance(),SLOT(quit()));

    aboutAction = this->findChild<QAction*>("actionAbout");
    connect(aboutAction,SIGNAL(triggered(bool)),this,SLOT(showAboutWindow()));

    preferencesAction = this->findChild<QAction*>("actionPreferences");
    connect(preferencesAction,SIGNAL(triggered(bool)),this,SLOT(showPreferencesWindow()));

    mainTabs = findChild<QTabWidget*>("tabWidgetMain");
    connect(mainTabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabChange()));

    qRegisterMetaType<qcustomplotCpuVector>("qcustomplotCpuVector");
    cpuPlot = reinterpret_cast<QCustomPlot*>(ui->tabResources->findChild<QWidget*>("cpuPlot"));
    connect(resourcesThread,SIGNAL(updateCpuPlotSIG(const qcustomplotCpuVector&)),
            this,SLOT(updateCpuPlotSLO(const qcustomplotCpuVector&)));

    qRegisterMetaType<qcustomplotNetworkVector>("qcustomplotNetworkVector");
    networkPlot = reinterpret_cast<QCustomPlot*>(ui->tabResources->findChild<QWidget*>("networkPlot"));
    connect(resourcesThread,SIGNAL(updateNetworkPlotSIG(const qcustomplotNetworkVector&)),
            this,SLOT(updateNetworkPlotSLO(qcustomplotNetworkVector)));

    handleTabChange();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete processesThread;
    delete resourcesThread;
    delete filesystemThread;
    delete mainTabs;
    delete settings;
}

void MainWindow::updateCpuPlotSLO(const qcustomplotCpuVector &input)
{
    QVector<double> x(60); // initialize with entries 60..0
    for (int i=59; i>0; --i)
    {
      x[i] = i;
    }

    const qcustomplotCpuVector *values = &input;

    static bool previouslyPlotted = false;
    int size = values->count();//values->size();
    if (size == 0) {
        return;
    }

    // check if using spline
    QVector<QVector<double>> splineValues;
    QVector<QVector<double>> splineXValues;
    bool smooth = settings->value("smoothGraphs", false).toBool();
    if (smooth) {
        // redo all point positions for y using cSpline
        for(int i=0; i<size; i++) {
            std::vector<double> origX = x.toStdVector();
            std::vector<double> origY = values->at(i).toStdVector();
            raven::cSpline spline(origX,origY);

            if(spline.IsError()!=raven::cSpline::e_error::no_error) {
                std::cerr << "Spline error [cpu] " << spline.IsError() << std::endl;
            }

            QVector<double> splineY, splineX;
            spline.Draw([&splineY, &splineX](double xval, double yval) mutable {
                splineY.append(yval);
                splineX.append(xval);
            },x.length()*5);

            splineValues.push_back(splineY);
            splineXValues.push_back(splineX);
        }

        values = &splineValues;
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
        if (smooth) {
            x = splineXValues[0];
        }
        cpuPlot->graph(i)->setData(x, values->at(i));

        if (settings->value("draw cpu area stacked",false).toBool()) {
            cpuPlot->graph(i)->setBrush(QBrush(QColor(colourNames[i % colourNamesLen])));
        } else {
            cpuPlot->graph(i)->setBrush(QBrush());
        }
    }
    previouslyPlotted = true;

    if (smooth) {
        cpuPlot->xAxis->setRange(0, x.last() + 1);
    } else {
        cpuPlot->xAxis->setRange(0, values->at(0).size());
    }
    cpuPlot->yAxis->setRange(0, 100);
    cpuPlot->replot();
}

void MainWindow::updateNetworkPlotSLO(const qcustomplotNetworkVector &values)
{
    const memoryConverter *sendingMax = nullptr, *recievingMax = nullptr;

    // find max value to scale all other values by
    for(unsigned int i=0; i<2; i++) {
        for(int j=0; j<values.at(i).size(); j++) {
            switch(i) {
                case 0:
                if (sendingMax == nullptr || (*sendingMax) < values.at(i).at(j)) {
                    sendingMax = &(values.at(i).at(j));
                }
                break;
                case 1:
                if (recievingMax == nullptr || (*recievingMax) < values.at(i).at(j)) {
                    recievingMax = &(values.at(i).at(j));
                }
                break;
            }
        }
    }

    // scale values to the same unit and then plot
    memoryConverter scaler;
    if (sendingMax < recievingMax) {
        scaler = *recievingMax;
    } else {
        scaler = *sendingMax;
    }

    QVector<QVector<double>> scaled, *plotting = &scaled;

    for(unsigned int i=0; i<2; i++) {
        QVector<double> scaledValuesTemp;
        for(int j=0; j<values.at(i).size(); j++) {
            memoryConverter temp = memoryConverter(values.at(i).at(j));
            temp.convertTo(scaler.getUnit());
            scaledValuesTemp.push_back(temp.getValue());
        }
        scaled.push_back(scaledValuesTemp);
    }

    QVector<double> x(60); // initialize with entries 60..0
    for (int i=59; i>0; --i)
    {
      x[i] = i;
    }

    const QString colours[] = {
        "blue", "red"
    };

    // check if using spline
    QVector<QVector<double>> splineValues;
    QVector<QVector<double>> splineXValues;
    bool smooth = settings->value("smoothGraphs", false).toBool();
    if (smooth) {
        // redo all point positions for y using cSpline
        for(int i=0; i<2; i++) {
            if (scaled.at(i).empty()) {
                continue;
            }

            std::vector<double> origX = x.toStdVector();
            std::vector<double> origY = scaled.at(i).toStdVector();
            raven::cSpline spline(origX,origY);

            if(spline.IsError()!=raven::cSpline::e_error::no_error) {
                std::cerr << "Spline error [network] " << spline.IsError() << std::endl;
            }

            QVector<double> splineY, splineX;

            spline.Draw([&splineY, &splineX](double xval, double yval) mutable {
                splineY.append(yval);
                splineX.append(xval);
            },x.length()*5);

            splineValues.push_back(splineY);
            splineXValues.push_back(splineX);
        }

        plotting = &splineValues;
    }

    for(unsigned int i=0; i<2; i++) {
        networkPlot->addGraph();
        networkPlot->graph(i)->setPen(QPen(QColor(colours[i])));
        if (smooth) {
            x = splineXValues[0];
        }
        networkPlot->graph(i)->setData(x, plotting->at(i));
    }

    if (smooth) {
        networkPlot->xAxis->setRange(0, x.last() + 1);
    } else {
        networkPlot->xAxis->setRange(0, plotting->at(0).size());
    }
    networkPlot->yAxis->setRange(0, scaler.getValue() + 1);
    networkPlot->yAxis->setLabel(QString::fromStdString(scaler.getUnitAsString()));
    networkPlot->replot();
}

/**
 * @brief MainWindow::handleTabChange Handle when the tab is changed, pause the other threads
 */
void MainWindow::handleTabChange()
{
    unsigned int index = mainTabs->currentIndex();
    processesThread->setPaused(true);
    if (!settings->value("resourcesKeepThreadRunning", true).toBool()) {
        resourcesThread->setPaused(true);
    } else {
        resourcesThread->setPaused(false);
    }
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

/**
 * @brief MainWindow::showPreferencesWindow Show the preferences dialogue
 */
void MainWindow::showPreferencesWindow()
{
    PreferencesDialogue* pref = new PreferencesDialogue(this, settings);
    pref->exec();
}
