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
    char* exe_of(const pid_t pid, size_t *const sizeptr, size_t *const lenptr);
    std::string getProcessNameFromPID(unsigned int pid);
    storedProcType prevProcs;
    unsigned long long total_cpu_time;
    unsigned long long getTotalCpuTime();
    const std::vector<std::string> explode(const std::string& s, const char& c)
    {
        std::string buff{""};
        std::vector<std::string> v;

        for(auto n:s)
        {
            if(n != c) buff+=n; else
            if(n == c && buff != "") { v.push_back(buff); buff = ""; }
        }
        if(buff != "") v.push_back(buff);

        return v;
    }
};

#endif // PROCESSINFORMATIONWORKER_H
