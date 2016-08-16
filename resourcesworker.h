#ifndef RESOURCESWORKER_H
#define RESOURCESWORKER_H
#include <QObject>
#include "workerthread.h"
#include <QProgressBar>
#include <QLabel>

class resourcesWorker : public QObject, public workerThread
{
    Q_OBJECT
public:
    explicit resourcesWorker(QObject *parent = 0);
    virtual ~resourcesWorker();
private:
    void loop();
    QProgressBar *memoryBar, *swapBar;
    QLabel *memoryLabel, *swapLabel;
};

#endif // RESOURCESWORKER_H
