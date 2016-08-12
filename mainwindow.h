#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QThread>
#include "processinformationworker.h"
#include "resourcesworker.h"
#include <proc/readproc.h>
#include <vector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    Ui::MainWindow *ui;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void handleTabChange();
    void updateProcessInformation();
    void handleProcessStop();

private:
    QTabWidget* mainTabs;
    QTableWidget* processesTable;
    void createProcessesView();
    processInformationWorker* processesThread;
    resourcesWorker* resourcesThread;
    bool processesThreadStarted;
    void stopRunningProcessesThread();
};

#endif // MAINWINDOW_H
