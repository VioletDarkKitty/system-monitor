#include "filesystemworker.h"
#include <QTabWidget>
#include <vector>
#include <proc/sysinfo.h>
#include <QHeaderView>

fileSystemWorker::fileSystemWorker(QObject *parent)
    : QObject(parent), workerThread()
{
    QTabWidget* mainTabs = parent->findChild<QTabWidget*>("tabWidgetMain");
    diskTable = mainTabs->findChild<QTableWidget*>("tableFilesystem");

    connect(this,SIGNAL(updateTableData()),this,SLOT(updateTable()));

    createFilesystemView();
}

void fileSystemWorker::createFilesystemView()
{
    diskTable->setColumnCount(6);
    diskTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //table->setHorizontalHeaderLabels(QString("HEADER 1;HEADER 2;HEADER 3").split(";"));
    diskTable->setHorizontalHeaderLabels(QString("Device;Directory;Type;Total;Available;Used;").split(";"));
    diskTable->verticalHeader()->setVisible(false);
    diskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    diskTable->resizeColumnsToContents();
}

void fileSystemWorker::updateTable()
{
    static disk_stat *disk_info;
    static partition_stat *part_info;

    int cDisks = getdiskstat(&disk_info,&part_info);

    std::vector<partition_stat> parts;
    unsigned int cPartBegin = 0, cPartEnd = 0;
    for(int i=0; i<cDisks; i++) {
        if (disk_info[i].partitions > 0) { // we only want disks with at least 1 partition
            // this excludes ram etc
            cPartEnd += disk_info[i].partitions;
            for(; cPartBegin < cPartEnd; cPartBegin++) {
                parts.push_back(part_info[cPartBegin]);
            }
        }
    }

    diskTable->setUpdatesEnabled(false); // avoid inconsistant data
    diskTable->setSortingEnabled(false);
    diskTable->setRowCount(parts.size());
    unsigned int index = 0;
    for(auto &i:parts) {
        partition_stat* p = &(i);
        if (p!=nullptr) {
            diskTable->setItem(index,0,new QTableWidgetItem(p->partition_name));

            diskTable->showRow(index);
        }
        index++;
    }
    diskTable->setUpdatesEnabled(true);
    diskTable->setSortingEnabled(true);
}

void fileSystemWorker::loop()
{
    emit(updateTableData());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

