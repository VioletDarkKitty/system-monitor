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
