#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "processinformationworker.h"
#include "resourcesworker.h"
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

private:
    processInformationWorker* processesThread;
    resourcesWorker* resourcesThread;
    QTabWidget* mainTabs;
};

#endif // MAINWINDOW_H
