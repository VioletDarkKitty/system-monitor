/*
 * Copyright (C) 2017 Lily Rivers (VioletDarkKitty)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
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
#include <QLabel>
#include <map>
#include <QSettings>

class processInformationWorker : public QObject, public workerThread
{
    typedef std::map<int, proc_t> storedProcType;
    Q_OBJECT
public:
    explicit processInformationWorker(QObject *parent, QSettings *settings);

private slots:
    void handleProcessStop();
    void updateTable();
    void filterProcesses(QString filter);
    void changeCurrentTableRowSelection(QModelIndex current);
    void showProcessProperties();

signals:
    void updateTableData();
    void signalFilterProcesses(QString filter);
    void updateLoadAverage(QString value);

private:
    void loop();
    QObject* mainWindow;
    QTableWidget* processesTable;
    void createProcessesView();
    QCheckBox* filterCheckbox;
    QLineEdit* searchField;
    bool shouldHideProcess(unsigned int pid);
    storedProcType prevProcs;
    unsigned long long totalCpuTime;
    int selectedRowInfoID;
    QLabel* loadAverage;
    std::map<QString, QIcon> processIconCache;
    QSettings *settings;
};

#endif // PROCESSINFORMATIONWORKER_H
