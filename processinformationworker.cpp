#include "processinformationworker.h"
#include <QTabWidget>
#include <QAction>
#include <pwd.h>
#include "tablenumberitem.h"
#include <signal.h>
#include <QMessageBox>
#include <QHeaderView>
#include "tablememoryitem.h"
#include "memoryconversion.h"
#include <unistd.h>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>

processInformationWorker::processInformationWorker(QObject *parent) :
    QObject(parent), workerThread() {
    QTabWidget* mainTabs = parent->findChild<QTabWidget*>("tabWidgetMain");
    processesTable = mainTabs->findChild<QTableWidget*>("tableProcesses");

    total_cpu_time = 0;

    QAction* actionStop = new QAction("Stop",processesTable);
    connect(actionStop,SIGNAL(triggered(bool)),SLOT(handleProcessStop()));

    QList<QAction*> rightClickActions;
    rightClickActions.push_back(actionStop);
    rightClickActions.push_back(new QAction("Kill",processesTable));
    rightClickActions.push_back(new QAction("Properties",processesTable));

    processesTable->setContextMenuPolicy(Qt::ActionsContextMenu);
    processesTable->addActions(rightClickActions);

    filterCheckbox = mainTabs->findChild<QCheckBox*>("processesFilterCheckbox");
    searchField = mainTabs->findChild<QLineEdit*>("processesSearchField");

    connect(searchField,SIGNAL(textChanged(QString)),this,SLOT(filterProcesses(QString)));
    connect(this,SIGNAL(signalFilterProcesses(QString)),this,SLOT(filterProcesses(QString)));

    connect(this,SIGNAL(updateTableData()),SLOT(updateTable()));    
    createProcessesView();
}

void processInformationWorker::createProcessesView()
{
    processesTable->setColumnCount(5);
    processesTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //table->setHorizontalHeaderLabels(QString("HEADER 1;HEADER 2;HEADER 3").split(";"));
    processesTable->setHorizontalHeaderLabels(QString("Process Name;User;% CPU;PID;Memory;").split(";"));
    processesTable->verticalHeader()->setVisible(false);
    processesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    processesTable->resizeColumnsToContents();
}

void processInformationWorker::handleProcessStop()
{
    int row = processesTable->currentIndex().row();
    // get PID
    int pid = processesTable->item(row,3)->text().toInt();

    QMessageBox::StandardButton reply;
    std::string info = "Are you sure you want to stop "+processesTable->item(row,0)->text().toStdString();
    reply = QMessageBox::question(processesTable, "Info", info.c_str(),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // attempt to send signal
        if (kill(pid,SIGTERM)!=0) {
            if (errno==EPERM) {
                // show a dialogue that we did not have permission to SIGTERM pid
                QMessageBox::information(processesTable, "Error",
                                "Could not send signal, permission denied!", QMessageBox::Ok);
            }
        }
    }
}

bool processInformationWorker::shouldHideProcess(unsigned int pid)
{
    if (filterCheckbox->checkState() == Qt::CheckState::Checked) {
        // if the filter checkbox is checked, we need to make sure that the
        // user who owns the process is the same as the user running us
        return (pid != geteuid());
    }
    return false;
}

void processInformationWorker::filterProcesses(QString filter)
{
    // loop over the table and hide items which do not match the search term given
    // only check col 0 (process name)
    for(int i = 0; i < processesTable->rowCount(); i++)
    {
        // check if the row is already hidden, if so, skip it
        /// TODO: find a better fix for this, can't check using QTableWidget
        /// so just check the PID manually
        if (!shouldHideProcess(getpwnam(processesTable->item(i,1)->text().toStdString().c_str())->pw_uid)) {
            QTableWidgetItem* item = processesTable->item(i,0); // process name
            processesTable->setRowHidden(i, !(item->text().contains(filter)));
        }
    }
}

/** exe_of() - Obtain the executable path a process is running
 * @pid: Process ID
 * @sizeptr: If specified, the allocated size is saved here
 * @lenptr: If specified, the path length is saved here
 * Returns the dynamically allocated pointer to the path,
 * or NULL with errno set if an error occurs.
*/
// from http://stackoverflow.com/questions/24581908/c-lstat-on-proc-pid-exe
char* processInformationWorker::exe_of(const pid_t pid, size_t *const sizeptr, size_t *const lenptr)
{
    char   *exe_path = NULL;
    size_t  exe_size = 1024;
    ssize_t exe_used;
    char    path_buf[64];
    unsigned int path_len;

    path_len = snprintf(path_buf, sizeof path_buf, "/proc/%ld/exe", (long)pid);
    if (path_len < 1 || path_len >= sizeof path_buf) {
        errno = ENOMEM;
        return NULL;
    }

    while (1) {

        exe_path = (char*)malloc(exe_size);
        if (!exe_path) {
            errno = ENOMEM;
            return NULL;
        }

        exe_used = readlink(path_buf, exe_path, exe_size - 1);
        if (exe_used == (ssize_t)-1)
            return NULL;

        if (exe_used < (ssize_t)1) {
            /* Race condition? */
            errno = ENOENT;
            return NULL;
        }

        if (exe_used < (ssize_t)(exe_size - 1))
            break;

        free(exe_path);
        exe_size += 1024;
    }

    /* Try reallocating the exe_path to minimum size.
     * This is optional, and can even fail without
     * any bad effects. */
    {
        char *temp;

        temp = (char*)realloc(exe_path, exe_used + 1);
        if (temp) {
            exe_path = temp;
            exe_size = exe_used + 1;
        }
    }

    if (sizeptr)
        *sizeptr = exe_size;

    if (lenptr)
        *lenptr = exe_used;

    exe_path[exe_used] = '\0';
    return exe_path;
}

std::string processInformationWorker::getProcessNameFromPID(unsigned int pid)
{
    std::string temp;
    try {
        std::fstream fs;
        fs.open ("/proc/"+std::to_string(pid)+"/cmdline", std::fstream::in);
        fs >> temp;
        fs.close();
    } catch(std::ifstream::failure e) {
        return "FAILED TO READ PROC";
    }

    if (temp.size()<1) {
        return "";
    }

    return std::experimental::filesystem::path(explode(temp,'\0')[0]).filename();
}

unsigned long long processInformationWorker::getTotalCpuTime()
{
    // from https://github.com/scaidermern/top-processes/blob/master/top_proc.c#L54
    FILE* file = fopen("/proc/stat", "r");
    if (file == NULL) {
        perror("Could not open stat file");
        return 0;
    }

    char buffer[1024];
    unsigned long long user = 0, nice = 0, system = 0, idle = 0;
    // added between Linux 2.5.41 and 2.6.33, see man proc(5)
    unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guestnice = 0;

    char* ret = fgets(buffer, sizeof(buffer) - 1, file);
    if (ret == NULL) {
        perror("Could not read stat file");
        fclose(file);
        return 0;
    }
    fclose(file);

    sscanf(buffer,
           "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestnice);

    // sum everything up (except guest and guestnice since they are already included
    // in user and nice, see http://unix.stackexchange.com/q/178045/20626)
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

void processInformationWorker::updateTable() {
    // from http://codingrelic.geekhold.com/2011/02/listing-processes-with-libproc.html
    PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLUSR | PROC_FILLCOM);
    // proc_info must be static! https://gitlab.com/procps-ng/procps/issues/33
    static proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_t));

    total_cpu_time = getTotalCpuTime() - total_cpu_time;

    storedProcType processes;
    while (readproc(proc, &proc_info) != NULL) {
        processes[proc_info.tid]=proc_info;
    }
    closeproc(proc);

    // fill in cpu%
    if (prevProcs.size()>0) {
        // we have previous proc info
        for(auto &newItr:processes) {
            for(auto &prevItr:prevProcs) {
                if (newItr.first == prevItr.first) {
                    // PID matches, calculate the cpu
                    unsigned long long cpuTime = ((newItr.second.utime + newItr.second.stime)
                            - (prevItr.second.utime + prevItr.second.stime));
                    /// TODO: GSM has an option to divide by # cpus
                    newItr.second.pcpu = (cpuTime / (float)total_cpu_time) * 100.0 * sysconf(_SC_NPROCESSORS_CONF);
                }
            }
        }
    }

    processesTable->setUpdatesEnabled(false); // avoid inconsistant data
    processesTable->setSortingEnabled(false);
    processesTable->setRowCount(processes.size());
    unsigned int index = 0;
    for(auto &i:processes) {
        proc_t* p = (&i.second);
        std::string processName = "ERROR";
        char* temp = exe_of(p->tid,NULL,NULL);
        // if exe_of fails here, it will be because permission is denied
        if (temp!=NULL) {
            processName = std::experimental::filesystem::path(temp).filename();
            free(temp);
        } else {
            // next try to read from /proc/*/cmdline
            processName = getProcessNameFromPID(p->tid);
            if (processName=="") {
                // fallback on /proc/*/stat program name value
                // bad because it is limited to 16 chars
                processName = p->cmd;
            }
        }
        processesTable->setItem(index,0,new QTableWidgetItem(processName.c_str()));
        QString user = p->euser;
        processesTable->setItem(index,1,new QTableWidgetItem(user));
        //std::cout << p->pcpu << std::endl;
        QString cpu = std::to_string(p->pcpu).c_str();
        processesTable->setItem(index,2,new TableNumberItem(cpu));
        QString id = std::to_string(p->tid).c_str();
        processesTable->setItem(index,3,new TableNumberItem(id));
        // gnome-system-monitor measures memory as RSS - shared
        // shared memory is only given as # pages, sysconf(_SC_PAGES) is in bytes
        // so do, (#pages RSS - #pages Share) * _SC_PAGES
        memoryEntry memory = convertMemoryUnit((p->resident - p->share)*sysconf(_SC_PAGESIZE),memoryUnit::b);
        processesTable->setItem(index,4,new TableMemoryItem(memory.unit,truncateDouble(memory.id,1)));
        processesTable->showRow(index);

        if (shouldHideProcess(p->euid)) {
            // hide this row
            processesTable->hideRow(index);
        }
        index++;
    }
    processesTable->setUpdatesEnabled(true);
    processesTable->setSortingEnabled(true);
    emit(signalFilterProcesses(searchField->text()));

    // keep processes we've read for cpu calculations next cycle
    prevProcs = processes;
}

void processInformationWorker::loop()
{
    emit(updateTableData());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

