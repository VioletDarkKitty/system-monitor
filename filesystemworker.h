/*
 * Copyright (C) 2017 Lily Rivers (VioletDarkKitty)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
#include "memoryconversion.h"
using namespace memoryConversion;

class fileSystemWorker : public QObject, public workerThread
{
    Q_OBJECT
public:
    explicit fileSystemWorker(QObject *parent = 0);

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
        memoryEntry freeSize;
        memoryEntry totalSize;
        memoryEntry usedSize;
        int usedPercentage;
        float io;
        long ioms;
    };
    float timeSinceLastIOCheck;
    std::vector<disk> oldDisks;
};

#endif // FILESYSTEMWORKER_H
