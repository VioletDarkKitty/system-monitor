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
#ifndef FILESYSTEMWORKER_H
#define FILESYSTEMWORKER_H
#include <QObject>
#include "workerthread.h"
#include <QTableWidget>
#include <QSettings>
#include "memoryconverter.h"
#include <chrono>

class fileSystemWorker : public QObject, public workerThread
{
    Q_OBJECT
public:
    explicit fileSystemWorker(QObject *parent, QSettings *settings);

signals:
    void updateTableData();

private slots:
    void updateTable();

private:
    void loop();
    QTableWidget* diskTable;
    void createFilesystemView();
    struct disk {
        QString name;
        QString mountPoint;
        QString type;
        memoryConverter *freeSize;
        memoryConverter *totalSize;
        memoryConverter *usedSize;
        int usedPercentage;
        double io;
        unsigned long long ioms;
    };
    std::chrono::milliseconds::rep timeSinceLastIOCheck;
    std::chrono::nanoseconds lastEpochCount;
    std::vector<disk> oldDisks;
    std::vector<disk> readMtabDisks();
    void fillDiskStructures(std::vector<disk> &disks);
    QSettings *settings;
    unitStandard standard;
};

#endif // FILESYSTEMWORKER_H
