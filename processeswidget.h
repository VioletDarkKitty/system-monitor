#ifndef PROCESSESWIDGET_H
#define PROCESSESWIDGET_H

#include <QWidget>

namespace Ui {
class ProcessesWidget;
}

class ProcessesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProcessesWidget(QWidget *parent = 0);
    ~ProcessesWidget();

private:
    Ui::ProcessesWidget *ui;
};

#endif // PROCESSESWIDGET_H
