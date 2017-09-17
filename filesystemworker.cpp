/*
 * Copyright (C) 2017 Lily Rivers (VioletDarkKitty)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 3
 * of the License only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
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
#include <cmath>
#include <locale>
#include "tablememoryitem.h"
#include "tablenumberitem.h"

fileSystemWorker::fileSystemWorker(QObject *parent, QSettings *settings)
    : QObject(parent), workerThread()
{
    QTabWidget* mainTabs = parent->findChild<QTabWidget*>("tabWidgetMain");
    diskTable = mainTabs->findChild<QTableWidget*>("tableFilesystem");

    connect(this,SIGNAL(updateTableData()),this,SLOT(updateTable()));

    createFilesystemView();
    timeSinceLastIOCheck = 0;
    lastEpochCount = std::chrono::system_clock::now().time_since_epoch();

    this->settings = settings;
}

/**
 * @brief fileSystemWorker::createFilesystemView Create the filesystem table
 */
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

/**
 * @brief fileSystemWorker::readMtabDisks Read the data from /etc/mtab and check it against
 * @return A vector of physical disks that exist. Only the name, mount point, and type are filled in
 */
std::vector<fileSystemWorker::disk> fileSystemWorker::readMtabDisks()
{
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

    return disks;
}

/**
 * @brief fileSystemWorker::fillDiskStructures Fill in any remaining data in each of the given disks
 * @param disks The list of disks to operate on
 */
void fileSystemWorker::fillDiskStructures(std::vector<disk> &disks)
{
    for(unsigned int i=0; i<disks.size(); i++) {
        struct statvfs device; // explicit struct because statvfs is also a function, grr...
        // use the mount point as the search for the device instead of the dev path as
        // using the dev path just checks the root disk for some reason!
        if(statvfs(qPrintable(disks[i].mountPoint), &device)==0) {
            // f_blocks is the size of the device in f_frsize units
            // f_bavail is the number of free blocks to normal users
            // f_bfree should be used to find the used% however
            // GSM reports these numbers in kb=1000 mode

            disks[i].totalSize = new memoryConverter((double)device.f_blocks*device.f_frsize,memoryUnit::b,standard);
            disks[i].freeSize = new memoryConverter((double)device.f_bavail*device.f_frsize,memoryUnit::b,standard);
            disks[i].usedSize = new memoryConverter((double)(device.f_blocks - device.f_bfree) * device.f_frsize,memoryUnit::b,standard);
            // keep the values the same for display of the percentage used
            memoryConverter totalSizeCopy = memoryConverter(*(disks[i].totalSize));
            totalSizeCopy.convertTo(disks[i].usedSize->getUnit());
            disks[i].usedPercentage = disks[i].usedSize->getValue() / totalSizeCopy.getValue() * 100;
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
        // /sys/block contains dm partitions, these should not be shown
        if (parent.section(QRegExp("\\d"),0,parent.size()) != "" && !parent.contains("dm")) {
            // this disk has a parent
            parent.remove(QRegExp("\\d"));
        } else {
            parent = "";
        }

        if (parent == name) {
            // this is actually a full disk partition, ignore the parent
            parent = "";
        }

        std::vector<long> parts;
        QFile stat("/sys/block/"+(parent==""? "":parent+"/")+name+"/stat");
        if (stat.open(QFile::ReadOnly)) {
            QString s = stat.readLine();
            std::locale loc;
            for (int i = 0; i < s.size(); i++) {
                if (std::isspace(s.at(i).toLatin1(), loc)) {
                    continue;
                }

                QString buf;
                for (int j = i; j < s.size(); j++, i++) {
                    if (std::isspace(s.at(j).toLatin1(), loc)) {
                        break;
                    }

                    buf += s.at(j);
                }

                parts.push_back(buf.toLong());
            }

            if (oldDisks.size() > 0) {
                disks[i].ioms = parts[9];
                if (timeSinceLastIOCheck > 0) {
                    disks[i].io = 100.0L * (parts[9]-oldDisks[i].ioms) / timeSinceLastIOCheck;
                }
                /*if (disks[i].io > 100) {
                    disks[i].io = 100;
                }
                if (disks[i].io < 0) {
                    disks[i].io = 0;
                }*/
            } else {
                disks[i].io = 0;
                disks[i].ioms = parts[9];
            }
        }
    }
}

/**
 * @brief fileSystemWorker::updateTable Update the filesystem table
 */
void fileSystemWorker::updateTable()
{
    timeSinceLastIOCheck = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::system_clock::now().time_since_epoch() - lastEpochCount).count();
    lastEpochCount = std::chrono::system_clock::now().time_since_epoch();

    std::vector<disk> disks = readMtabDisks();

    fillDiskStructures(disks);

    // delete the objects we created with new before we assign to prevent leaks
    while(!oldDisks.empty()) {
        disk *workingDisk = &(oldDisks.back());
        delete workingDisk->freeSize;
        delete workingDisk->totalSize;
        delete workingDisk->usedSize;
        oldDisks.pop_back();
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
            diskTable->setItem(index,3,new TableMemoryItem((*p->totalSize)));
            diskTable->setItem(index,4,new TableMemoryItem((*p->freeSize)));
            diskTable->setItem(index,5,new TableNumberItem(QString::number(p->usedPercentage) + QString::fromStdString("% (" +
                                                            p->usedSize->to_string() + ")")));
            diskTable->setItem(index,6,new TableNumberItem(QString::number(p->io) + "%"));
            diskTable->showRow(index);
        }
        index++;
    }
    diskTable->setUpdatesEnabled(true);
    diskTable->setSortingEnabled(true);
}

/**
 * @brief fileSystemWorker::loop Run the loop for the thread
 */
void fileSystemWorker::loop()
{
    standard = memoryConverter::stringToStandard(settings->value("unit prefix standards", JEDEC).toString().toStdString());
    emit(updateTableData());

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

