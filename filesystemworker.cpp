#include "filesystemworker.h"
#include <QTabWidget>
#include <vector>
#include <proc/sysinfo.h>
#include <QHeaderView>
#include <QFile>
#include <sys/statvfs.h>
#include <iostream>
#include <stdexcept>
#include <time.h>
#include <unistd.h>
#include <QFileInfo>

fileSystemWorker::fileSystemWorker(QObject *parent)
    : QObject(parent), workerThread()
{
    QTabWidget* mainTabs = parent->findChild<QTabWidget*>("tabWidgetMain");
    diskTable = mainTabs->findChild<QTableWidget*>("tableFilesystem");

    connect(this,SIGNAL(updateTableData()),this,SLOT(updateTable()));

    createFilesystemView();
    timeSinceLastIOCheck = 0;
}

void fileSystemWorker::createFilesystemView()
{
    diskTable->setColumnCount(7);
    diskTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //table->setHorizontalHeaderLabels(QString("HEADER 1;HEADER 2;HEADER 3").split(";"));
    diskTable->setHorizontalHeaderLabels(QString("Device;Directory;Type;Total;Available;Used;IO").split(";"));
    diskTable->verticalHeader()->setVisible(false);
    diskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    diskTable->resizeColumnsToContents();
}

void fileSystemWorker::updateTable()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    timeSinceLastIOCheck = ts.tv_sec*1000 - timeSinceLastIOCheck;

    // mtab lists all of the mounted disks on the machine, need to check that they exist in /dev and are real devices to avoid listing
    // things like the proc and udev file systems
    QFile file("/etc/mtab");
    std::vector<disk> disks;
    if (file.open(QFile::ReadOnly)) {
        while(true) {
            QStringList tabParts = QString::fromLocal8Bit(file.readLine()).trimmed().split(" "); // read the next line
            if (tabParts.count() > 1) {
                // still some of the file left to read
                if (tabParts[0].contains("/dev/")) {
                    // http://serverfault.com/questions/267609/how-to-understand-etc-mtab
                    disk d;
                    d.name=tabParts[0];
                    d.mountPoint=tabParts[1];
                    d.type=tabParts[2];
                    disks.push_back(d);
                }
            } else {
                // EOF
                break;
            }
        }
    }


    // http://stackoverflow.com/questions/4965355/converting-statvfs-to-percentage-free-correctly
    for(unsigned int i=0; i<disks.size(); i++) {        
        struct statvfs device; // explicit struct because statvfs is also a function, grr...
        // use the mount point as the search for the device instead of the dev path as
        // using the dev path just checks the root disk for some reason!
        if(statvfs(qPrintable(disks[i].mountPoint), &device)==0) {
            // f_blocks is the size of the device in f_frsize units
            // f_bavail is the number of free blocks to normal users
            // f_bfree should be used to find the used% however
            // GSM reports these numbers in kb=1000 mode
            /// TODO: add an option to swap this mode
            disks[i].totalSize = convertMemoryUnit((double)device.f_blocks*device.f_frsize,memoryUnit::b);
            disks[i].freeSize = convertMemoryUnit((double)device.f_bavail*device.f_frsize,memoryUnit::b);
            disks[i].usedSize = convertMemoryUnit((double)(device.f_blocks - device.f_bfree) * device.f_frsize,memoryUnit::b);
            disks[i].usedPercentage = disks[i].usedSize.id / disks[i].totalSize.id * 100;
        } else {
            throw std::runtime_error(qPrintable("'" + disks[i].mountPoint + "'' failed statvfs!"));
        }

        // https://www.kernel.org/doc/Documentation/iostats.txt
        // check /sys/block/<disk>/<partition> for the disks we found in mtab and then grab some extra data about them
        // some disks have are logical and have /dev/mapper/<disk> names, these should be shown using /dev/mapper names as in GSM but the names
        // need to be resolved to be used as diskstats uses dm type names. The disks are symlinked to the correct paths
        QString name = disks[i].name;
        if (disks[i].name.contains("/dev/mapper")) {
            char buf[1024];
            size_t len;
            if ((len = readlink(qPrintable(name), buf, sizeof(buf)-1))) {
                buf[len] = '\0';
            }
            name = QString(buf);
        }
        name = QFileInfo(name).fileName();

        QString parent = QFileInfo(name).fileName();
        if (parent.section(QRegExp("\\d"),0,parent.size()) != "" && !parent.contains("dm")) {
            // this disk has a parent
            parent.remove(QRegExp("\\d"));
        } else {
            parent = "";
        }

        #define NUMBER_OF_CHARS_PER_COL_IOSTAT 9
        std::vector<long> parts;
        QFile stat("/sys/block/"+(parent==""? "":parent+"/")+name+"/stat");
        if (stat.open(QFile::ReadOnly)) {
            QString s = stat.readLine();
            for (int i = 0; i < s.size(); i += NUMBER_OF_CHARS_PER_COL_IOSTAT) {
                parts.push_back(QString(QString::fromStdString(s.toStdString().substr(i, NUMBER_OF_CHARS_PER_COL_IOSTAT))).toLong());
            }

            if (oldDisks.size() > 0) {
                disks[i].ioms = parts[9];
                disks[i].io = round(100 * (parts[9]-oldDisks[i].ioms) / timeSinceLastIOCheck);
                if (disks[i].io > 100) {
                    disks[i].io = 100;
                }
                if (disks[i].io < 0) {
                    disks[i].io = 0;
                }
            } else {
                disks[i].io = 0;
                disks[i].ioms = parts[9];
            }
        }
    }

    oldDisks = disks;

    diskTable->setUpdatesEnabled(false); // avoid inconsistant data
    diskTable->setSortingEnabled(false);
    diskTable->setRowCount(disks.size());
    unsigned int index = 0;
    for(auto &i:disks) {
        disk* p = &(i);
        if (p!=nullptr) {
            diskTable->setItem(index,0,new QTableWidgetItem(p->name));
            diskTable->setItem(index,1,new QTableWidgetItem(p->mountPoint));
            diskTable->setItem(index,2,new QTableWidgetItem(p->type));
            diskTable->setItem(index,3,new QTableWidgetItem(QString::fromStdString(dbl2str(truncateDouble(p->totalSize.id,1)))
                                                            + QString::fromStdString(unitToString(p->totalSize.unit))));
            diskTable->setItem(index,4,new QTableWidgetItem(QString::fromStdString(dbl2str(truncateDouble(p->freeSize.id,1)))
                                                            + QString::fromStdString(unitToString(p->freeSize.unit))));
            diskTable->setItem(index,5,new QTableWidgetItem(QString::number(p->usedPercentage) + QString::fromStdString("% (" +
                                                            dbl2str(truncateDouble(p->usedSize.id,1)) + unitToString(p->usedSize.unit)) + ")"));
            diskTable->setItem(index,6,new QTableWidgetItem(QString::number(p->io) + "%"));
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

