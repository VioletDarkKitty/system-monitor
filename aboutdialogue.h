#ifndef ABOUTDIALOGUE_H
#define ABOUTDIALOGUE_H

#include <QDialog>

namespace Ui {
class AboutDialogue;
}

class AboutDialogue : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialogue(QWidget *parent = 0);
    ~AboutDialogue();

private:
    Ui::AboutDialogue *ui;
};

#endif // ABOUTDIALOGUE_H
