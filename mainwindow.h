#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void handleTabChange();

private:
    Ui::MainWindow *ui;
    QTabWidget* mainTabs;
    QTableWidget* processesTable;
    void createProcessesView();
    /*QThread* processesThread;
    bool processesThreadStarted;*/
};

#endif // MAINWINDOW_H
