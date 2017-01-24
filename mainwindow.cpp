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
