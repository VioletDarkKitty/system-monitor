#ifndef PROCESSINFORMATIONWORKER_H
#define PROCESSINFORMATIONWORKER_H
#include <QObject>
#include <proc/readproc.h>
#include <map>
#include "workerthread.h"
#include <QTableWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <string>
#include <vector>

class processInformationWorker : public QObject, public workerThread
{
    typedef std::map<int, proc_t> storedProcType;
    Q_OBJECT
public:
    explicit processInformationWorker(QObject *parent = 0);

private slots:
    void handleProcessStop();
    void updateTable();
    void filterProcesses(QString filter);
    void changeCurrentTableRowSelection(QModelIndex current);
    void showProcessProperties();

signals:
    void updateTableData();
    void signalFilterProcesses(QString filter);

private:
    void loop();
    QObject* mainWindow;
    QTableWidget* processesTable;
    void createProcessesView();
    QCheckBox* filterCheckbox;
    QLineEdit* searchField;
    bool shouldHideProcess(unsigned int pid);
    storedProcType prevProcs;
    unsigned long long total_cpu_time;
    int selectedRowInfoID;
};

#endif // PROCESSINFORMATIONWORKER_H
