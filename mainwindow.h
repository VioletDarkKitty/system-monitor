#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void updateMainWidgetSelection();

private slots:
    void handleProcessesButton();

private:
    Ui::MainWindow *ui;
    int currentPane;
    QWidget* widgetMainData;
};

#endif // MAINWINDOW_H
