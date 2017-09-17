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
#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QCheckBox>
#include <QString>
#include <QRadioButton>

namespace Ui {
class PreferencesDialogue;
}

class PreferencesDialogue : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialogue(QWidget *parent, QSettings *settings);
    ~PreferencesDialogue();

public slots:
    void toggleDivideCpuCheckbox(bool checked);
    void updateProcessesIntervalSpinner(double value);
    void updateStandardsRadioButton();
    void toggleStackedCpuCheckbox(bool checked);
    void updateResourcesIntervalSpinner(double value);
    void toggleProcessCheckbox(QWidget* checkbox);
    void toggleResourcesBackgroundCheckbox(bool checked);
    void toggleCachedIsUsedCheckbox(bool checked);
    void toggleSmoothGraphsCheckbox(bool checked);

private:
    Ui::PreferencesDialogue *ui;
    QSettings *settings;
    QRadioButton *IECStandard, *JEDECStandard, *SIStandard;
    void checkStandardsRadioButtonBasedOnSettingValue();
};

#endif // PREFERENCESDIALOG_H
