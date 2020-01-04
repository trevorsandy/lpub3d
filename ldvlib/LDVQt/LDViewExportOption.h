/****************************************************************************
**
** Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __LDVIEWExportOption_H__
#define __LDVIEWExportOption_H__

#include <TCFoundation/TCStlIncludes.h>
#include <TCFoundation/TCTypedObjectArray.h>
#include <LDLib/LDrawModelViewer.h>
#include "ui_LDVExportOptionPanel.h"
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>

class LDVExportSetting : public LDExporterSetting
{
public:
    void resetLights();
};

class LDVWidget;
class LDVPreferences;
class LDrawModelViewer;
class LDExporter;
class LDExporterSetting;
class TCStringArray;

typedef std::map<LDExporterSetting *, QWidget *> SettingsMap;
typedef std::map<QPushButton *, QLineEdit *> ButtonMap;
typedef std::map<QLayout *, SettingsMap> GroupMap;

class LDViewExportOption : public QDialog, Ui::LDVExportOptionPanel
{
	Q_OBJECT
public:
    LDViewExportOption(LDVWidget *modelWidget);
	~LDViewExportOption();

    void applyExtraSearchDirs(void);
    void captureExtraSearchDirs(void);
	static TCStringArray* extraSearchDirs;

public slots:
    void enableApply(void);
    void doApply(void);
	void doOk(void);
	void doCancel(void);
	void doReset(void);
	void doResetGroup(void);
	void doBrowse(void);
    void show(void);

private slots:
    void applyLights(void);
    void applyLights(bool);
    void setLights(void);
    void setLights(int);
    void setLights(double);
    void addLight(void);
    void removeLight(void);
    void selectLight(int);

protected:
    void populateExportSettings(void);
	void resetSettings(SettingsMap &settings);

	LDrawModelViewer *m_modelViewer;
    LDExporter       *m_exporter;
    QWidget          *m_box;
    QVBoxLayout      *m_lay;
    bool              m_POVLightsLoaded;
    SettingsMap       m_settings;
    ButtonMap         m_button;
    GroupMap          m_groups;

    QComboBox        *m_liCombo;
    QLineEdit        *m_liNumEdit;
    QCheckBox        *m_liShadowsChk;
    QDoubleSpinBox   *m_liLatSpin;
    QDoubleSpinBox   *m_liLonSpin;
    QDoubleSpinBox   *m_liIntSpin;
    QSpinBox         *m_liASizeSpin;
    QSpinBox         *m_liALightsSpin;

    QLabel           *m_messageLabel;
    QStringList       m_pov_lightList;
    int               m_number;
};

#endif // __LDVIEWExportOption_H__
