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
#include "preferencesdialogue.h"
#include "ui_preferencesdialogue.h"
#include <QCheckBox>
#include <QSignalMapper>

PreferencesDialogue::PreferencesDialogue(QWidget *parent, QSettings *settings) :
    QDialog(parent),
    ui(new Ui::PreferencesDialogue)
{
    ui->setupUi(this);
    this->settings = settings;

    this->setWindowFlags(Qt::Window);

    QCheckBox *divideByCpuCheckbox = this->findChild<QCheckBox*>("divideByCpuCheckbox");
    connect(divideByCpuCheckbox,SIGNAL(clicked(bool)),this,SLOT(toggleDivideCpuCheckbox(bool)));
    divideByCpuCheckbox->setChecked(settings->value("divide process cpu by cpu count", false).toBool());

    QDoubleSpinBox *updateIntervalProcessesSpinner = this->findChild<QDoubleSpinBox*>("updateIntervalProcessesSpinner");
    connect(updateIntervalProcessesSpinner,SIGNAL(valueChanged(double)),this,SLOT(updateProcessesIntervalSpinner(double)));
    updateIntervalProcessesSpinner->setValue(settings->value("processes update interval", 1.0).toDouble());

    QDoubleSpinBox *updateIntervalResourcesSpinner = this->findChild<QDoubleSpinBox*>("updateIntervalResourcesSpinner");
    connect(updateIntervalResourcesSpinner,SIGNAL(valueChanged(double)),this,SLOT(updateResourcesIntervalSpinner(double)));
    updateIntervalResourcesSpinner->setValue(settings->value("resources update interval", 1.0).toDouble());

    IECStandard = this->findChild<QRadioButton*>("IECButton");
    connect(IECStandard,SIGNAL(toggled(bool)),this,SLOT(updateStandardsRadioButton()));
    JEDECStandard = this->findChild<QRadioButton*>("JEDECButton");
    connect(JEDECStandard,SIGNAL(toggled(bool)),this,SLOT(updateStandardsRadioButton()));
    SIStandard = this->findChild<QRadioButton*>("SIButton");
    connect(SIStandard,SIGNAL(toggled(bool)),this,SLOT(updateStandardsRadioButton()));
    checkStandardsRadioButtonBasedOnSettingValue();

    QCheckBox* stackedCpuCheckbox = this->findChild<QCheckBox*>("stackedCpuCheckbox");
    connect(stackedCpuCheckbox,SIGNAL(clicked(bool)),this,SLOT(toggleStackedCpuCheckbox(bool)));
    stackedCpuCheckbox->setChecked(settings->value("draw cpu area stacked", false).toBool());

    std::vector<QCheckBox*> processColShowCheckboxes = {
        this->findChild<QCheckBox*>("processSharedCheckbox"),
        this->findChild<QCheckBox*>("processNiceCheckbox"),
        this->findChild<QCheckBox*>("processStartedCheckbox"),
        this->findChild<QCheckBox*>("processUserCheckbox"),
        this->findChild<QCheckBox*>("processPidCheckbox"),
        this->findChild<QCheckBox*>("processResidentMemoryCheckbox"),
        this->findChild<QCheckBox*>("processCpuTimeCheckbox"),
        this->findChild<QCheckBox*>("processMemoryCheckbox"),
        this->findChild<QCheckBox*>("processCpuCheckbox"),
        this->findChild<QCheckBox*>("processCommandlineCheckbox"),
        this->findChild<QCheckBox*>("processStatusCheckbox"),
        this->findChild<QCheckBox*>("processVirtualMemoryCheckbox"),
        this->findChild<QCheckBox*>("processNameCheckbox")
    };

    QSignalMapper *signalMapper = new QSignalMapper(this);
    connect(signalMapper,SIGNAL(mapped(QWidget*)),this,SLOT(toggleProcessCheckbox(QWidget*)));

    for(unsigned int i=0; i<processColShowCheckboxes.size(); i++) {
        QCheckBox* currentCheckbox = processColShowCheckboxes.at(i);

        bool isDefaulted = false;
        static std::vector<QString> defaultCheckboxes = {
            "processNameCheckbox", "processUserCheckbox", "processCpuCheckbox",
            "processPidCheckbox", "processMemoryCheckbox"
        };
        for(unsigned int j=0; j<defaultCheckboxes.size(); j++) {
            if (defaultCheckboxes.at(j) == currentCheckbox->property("objectName").toString()) {
                isDefaulted = true;
                break;
            }
        }

        currentCheckbox->setChecked(settings->value(currentCheckbox->property("objectName").toString(),isDefaulted).toBool());
        signalMapper->setMapping(currentCheckbox, currentCheckbox);
        connect(currentCheckbox, SIGNAL(clicked()), signalMapper, SLOT(map()));
    }

    QCheckBox* runResourcesThreadInBackgroundCheckbox = this->findChild<QCheckBox*>("resourcesKeepThreadRunning");
    connect(runResourcesThreadInBackgroundCheckbox,SIGNAL(clicked(bool)),this,SLOT(toggleResourcesBackgroundCheckbox(bool)));
    runResourcesThreadInBackgroundCheckbox->setChecked(settings->value("resourcesKeepThreadRunning", true).toBool());

    QCheckBox* cachedMemoryIsUsed = this->findChild<QCheckBox*>("cachedMemoryIsUsed");
    connect(cachedMemoryIsUsed,SIGNAL(clicked(bool)),this,SLOT(toggleCachedIsUsedCheckbox(bool)));
    cachedMemoryIsUsed->setChecked(settings->value("cachedMemoryIsUsed", false).toBool());

    QCheckBox* smoothGraphs = this->findChild<QCheckBox*>("smoothGraphs");
    connect(smoothGraphs,SIGNAL(clicked(bool)),this,SLOT(toggleSmoothGraphsCheckbox(bool)));
    smoothGraphs->setChecked(settings->value("smoothGraphs", false).toBool());
}

PreferencesDialogue::~PreferencesDialogue()
{
    delete ui;
}

void PreferencesDialogue::toggleDivideCpuCheckbox(bool checked)
{
    settings->setValue("divide process cpu by cpu count",checked);
}

void PreferencesDialogue::toggleStackedCpuCheckbox(bool checked)
{
    settings->setValue("draw cpu area stacked",checked);
}

void PreferencesDialogue::updateProcessesIntervalSpinner(double value)
{
    settings->setValue("processes update interval", value);
}

void PreferencesDialogue::updateResourcesIntervalSpinner(double value)
{
    settings->setValue("resources update interval", value);
}

void PreferencesDialogue::updateStandardsRadioButton()
{
    const QString settingName = "unit prefix standards";
    // find out which radio button is pressed and write the setting to the file
    if (IECStandard->isChecked()) {
        settings->setValue(settingName, "IEC");
    } else if (JEDECStandard->isChecked()) {
        settings->setValue(settingName, "JEDEC");
    } else if (SIStandard->isChecked()) { // check even though there are only 3!
        settings->setValue(settingName, "SI");
    }
}

void PreferencesDialogue::checkStandardsRadioButtonBasedOnSettingValue()
{
    const QString settingName = "unit prefix standards";
    // find if the setting is set to the correct name and then set the radio buttons
    const QString value = settings->value(settingName, "JEDEC").toString();
    if (value == "IEC") {
        IECStandard->setChecked(true);
    } else if (value == "JEDEC") {
        JEDECStandard->setChecked(true);
    } else if (value == "SI") {
        SIStandard->setChecked(true);
    }
}

void PreferencesDialogue::toggleProcessCheckbox(QWidget* checkbox)
{
    QCheckBox* changingCheckbox = (QCheckBox*)checkbox;
    settings->setValue(changingCheckbox->property("objectName").toString(),changingCheckbox->isChecked());
}

void PreferencesDialogue::toggleResourcesBackgroundCheckbox(bool checked)
{
    settings->setValue("resourcesKeepThreadRunning", checked);
}

void PreferencesDialogue::toggleCachedIsUsedCheckbox(bool checked)
{
    settings->setValue("cachedMemoryIsUsed", checked);
}

void PreferencesDialogue::toggleSmoothGraphsCheckbox(bool checked)
{
    settings->setValue("smoothGraphs", checked);
}
