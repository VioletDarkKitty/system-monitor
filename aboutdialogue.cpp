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
#include "aboutdialogue.h"
#include "ui_aboutdialogue.h"
#include <QPushButton>

AboutDialogue::AboutDialogue(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialogue)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::Window);

    QPushButton* closeButton = this->findChild<QPushButton*>("closeButton");
    connect(closeButton,SIGNAL(clicked(bool)),this,SLOT(close()));

    QLabel* versionLabel = this->findChild<QLabel*>("versionLabel");
    QString version = QString::number(VERSION_MAJOR) + "." + QString::number(VERSION_MINOR) + "." + QString::number(VERSION_BUILD);
    versionLabel->setText(version);

    QIcon appIcon = QIcon::fromTheme("utilities-system-monitor",QIcon::fromTheme("application-x-executable"));
    QLabel* programIcon = this->findChild<QLabel*>("programIcon");
    programIcon->setPixmap(appIcon.pixmap(appIcon.actualSize(QSize(128,128))));
}

AboutDialogue::~AboutDialogue()
{
    delete ui;
}
