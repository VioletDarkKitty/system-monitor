#ifndef FILESYSTEMWORKER_H
#define FILESYSTEMWORKER_H
#include <QObject>
#include "workerthread.h"
#include <QTableWidget>

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
};

#endif // FILESYSTEMWORKER_H
