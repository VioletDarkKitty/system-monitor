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
#include "preferencesdialogue.h"
#include "ui_preferencesdialogue.h"
#include <QCheckBox>

PreferencesDialogue::PreferencesDialogue(QWidget *parent, QSettings *settings) :
    QDialog(parent),
    ui(new Ui::PreferencesDialogue)
{
    ui->setupUi(this);
    this->settings = settings;

    QCheckBox *divideByCpuCheckbox = this->findChild<QCheckBox*>("divideByCpuCheckbox");
    connect(divideByCpuCheckbox,SIGNAL(clicked(bool)),this,SLOT(toggleDivideCpuCheckbox(bool)));
    divideByCpuCheckbox->setChecked(settings->value("divide process cpu by cpu count", false).toBool());

    QDoubleSpinBox *updateIntervalProcessesSpinner = this->findChild<QDoubleSpinBox*>("updateIntervalProcessesSpinner");
    connect(updateIntervalProcessesSpinner,SIGNAL(valueChanged(double)),this,SLOT(updateProcessesIntervalSpinner(double)));
    updateIntervalProcessesSpinner->setValue(settings->value("processes update interval", 1.0).toDouble());
}

PreferencesDialogue::~PreferencesDialogue()
{
    delete ui;
}

void PreferencesDialogue::toggleDivideCpuCheckbox(bool checked)
{

    settings->setValue("divide process cpu by cpu count",checked);
}

void PreferencesDialogue::updateProcessesIntervalSpinner(double value)
{
    settings->setValue("processes update interval", value);
}
