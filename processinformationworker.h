#ifndef PROCESSINFORMATIONWORKER_H
#define PROCESSINFORMATIONWORKER_H
#include <QObject>
#include <proc/readproc.h>
#include <vector>
#include "workerthread.h"
#include <QTableWidget>
#include <QCheckBox>
#include <QLineEdit>

class processInformationWorker : public QObject, public workerThread
{
    Q_OBJECT
public:
    explicit processInformationWorker(QObject *parent = 0);

private slots:
    void handleProcessStop();
    void updateTable();
    void filterProcesses(QString filter);

signals:
    void updateTableData();
    void signalFilterProcesses(QString filter);

private:
    void loop();
    QTableWidget* processesTable;
    void createProcessesView();
    QCheckBox* filterCheckbox;
    QLineEdit* searchField;
    bool shouldHideProcess(unsigned int pid);
    std::string getProcessNameFromPID(unsigned int pid);
};

#endif // PROCESSINFORMATIONWORKER_H
