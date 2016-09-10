#include "aboutdialogue.h"
#include "ui_aboutdialogue.h"
#include <QPushButton>

AboutDialogue::AboutDialogue(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialogue)
{
    ui->setupUi(this);

    QPushButton* closeButton = this->findChild<QPushButton*>("closeButton");
    connect(closeButton,SIGNAL(clicked(bool)),this,SLOT(close()));
}

AboutDialogue::~AboutDialogue()
{
    delete ui;
}
