#include "processpropertiesdialogue.h"
#include "ui_processpropertiesdialogue.h"
#include "tablememoryitem.h"
#include "tablenumberitem.h"
#include "memoryconversion.h"
#include "processtools.h"
using namespace processTools;

processPropertiesDialogue::processPropertiesDialogue(QWidget *parent, pid_t pid) :
    QDialog(parent),
    ui(new Ui::processPropertiesDialogue)
{
    ui->setupUi(this);

    processID = pid;
    before = NULL;
    cpuTime = 0;

    processInfoTable = this->findChild<QTableWidget*>("processInfoTable");
    processInfoTable->verticalHeader()->setHidden(true);
    processInfoTable->horizontalHeader()->setHidden(true);
    processInfoTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    processInfoTable->setRowCount(8);
    processInfoTable->setColumnCount(2);
    processInfoTable->setItem(processName,0,new QTableWidgetItem("Process Name"));
    processInfoTable->setItem(processUser,0,new QTableWidgetItem("User"));
    processInfoTable->setItem(processStatus,0,new QTableWidgetItem("Status"));
    processInfoTable->setItem(processMemory,0,new QTableWidgetItem("Memory"));
    processInfoTable->setItem(processVirtual,0,new QTableWidgetItem("Virtual Memory"));
    processInfoTable->setItem(processResident,0,new QTableWidgetItem("Resident Memory"));
    processInfoTable->setItem(processShared,0,new QTableWidgetItem("Shared Memory"));
    processInfoTable->setItem(processCPU,0,new QTableWidgetItem("CPU%"));

    connect(this, SIGNAL(updateTable()), this, SLOT(updateTableData()));
    connect(parent,SIGNAL(destroyed(QObject*)),this,SLOT(deleteLater()));

    // hide minimise and maximise buttons
    setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);

    this->start();
}

processPropertiesDialogue::~processPropertiesDialogue()
{
    delete ui;
}

void processPropertiesDialogue::updateTableData()
{
    PROCTAB *tab = openproc(PROC_PID | PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLUSR | PROC_FILLNS);
    proc_t* p; // Don't fill the proc_t struct with any info

    tab->pids = (pid_t*)malloc(sizeof(pid_t*)*2); // PROCTAB should clean this up when closeproc is called
    tab->pids[0] = processID;
    tab->pids[1] = 0; // The array should be 0 terminated

    processInfoTable->setUpdatesEnabled(false);
    if ((p=readproc(tab,NULL))==NULL) {
        // The process is dead
        // GSM shows N/A for some fields in a dead process
        processInfoTable->setItem(processStatus,1,new QTableWidgetItem("Dead"));
    } else {
        QString procName = getProcessName(p);
        // set the window title
        QString title = procName + " (" + "PID " + QString::number(p->tid) + ")";
        this->setWindowTitle(title);
        // set the fields for the process properties
        processInfoTable->setItem(processName,1,new QTableWidgetItem(procName));
        QString user = QString(p->euser) + " (" + QString::number(p->euid) + ")";
        processInfoTable->setItem(processUser,1,new QTableWidgetItem(user));
        QString status;
        switch(p->state) {
            case 'S':
                status = "Sleeping";
            break;

            case 'R':
                status = "Running";
            break;

            case 'Z':
                status = "Zombie";
            break;

            default:
                status = "Unknown state" + QString(p->state);
        }
        processInfoTable->setItem(processStatus,1,new QTableWidgetItem(status));
        memoryEntry memory = convertMemoryUnit((p->resident - p->share)*sysconf(_SC_PAGESIZE),memoryUnit::b);
        processInfoTable->setItem(processMemory,1,new TableMemoryItem(memory.unit,truncateDouble(memory.id,1)));
        memoryEntry virtualMemory = convertMemoryUnit(p->vm_size,memoryUnit::kb);
        processInfoTable->setItem(processVirtual,1,new TableMemoryItem(virtualMemory.unit,truncateDouble(virtualMemory.id,1)));
        memoryEntry residentMemory = convertMemoryUnit(p->vm_rss,memoryUnit::kb);
        processInfoTable->setItem(processResident,1,new TableMemoryItem(residentMemory.unit,truncateDouble(residentMemory.id,1)));
        memoryEntry sharedMemory = convertMemoryUnit(p->share*sysconf(_SC_PAGESIZE),memoryUnit::b);
        processInfoTable->setItem(processShared,1,new TableMemoryItem(sharedMemory.unit,truncateDouble(sharedMemory.id,1)));
        QString cpuPercentage;
        if (before==NULL) {
            cpuPercentage = "0%";
        } else {
            cpuPercentage = QString::number((unsigned int)calculateCPUPercentage(before,p,cpuTime)) + "%";
            free(before);
        }
        before = p;
        processInfoTable->setItem(processCPU,1,new TableNumberItem(cpuPercentage));
    }
    processInfoTable->resizeColumnsToContents();
    processInfoTable->setUpdatesEnabled(true);

    closeproc(tab);
}

void processPropertiesDialogue::loop()
{
    emit(updateTable());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
