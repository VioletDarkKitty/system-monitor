#ifndef PROCESSINFORMATIONWORKER_H
#define PROCESSINFORMATIONWORKER_H
#include <QObject>

class processInformationWorker : public QObject
{
    Q_OBJECT
public:
    explicit processInformationWorker(QObject *parent = 0);

public slots:
    void getProcessesInformation();
};

#endif // PROCESSINFORMATIONWORKER_H
