/****************************************************************************
**
** Copyright (C) 2018 - 2023 Trevor SANDY. All rights reserved.
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
#include <QMap>

class LDVExportSetting : public LDExporterSetting
{
public:
	void resetLights();
};

class PovLight
{
public:
	PovLight(const QString & = "");
	bool parseString(const char* in, float *out);
	void setTypeInt();
	void setTypeText();
	void setItemText();
	QString getLightString();
	QString itemText;
	QString typeText;
	bool  isvalid;
	int   type;
	int   shadowless;
	float latitude;
	float longitude;
	float target[3];
	float color[3];
	float intensity;
	float fadeDistance;
	float fadePower;
	float radius;
	float falloff;
	float tightness;
	int   circle;
	int   width;
	int   height;
	int   rows;
	int   columns;
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
	void updateLight(void);
	void removeLight(void);
	void selectLight(int);

protected:
	PovLight getLight(void) const;
	void populateExportSettings(void);
	void resetSettings(SettingsMap &settings);

	LDrawModelViewer  *m_modelViewer;
	LDExporter        *m_exporter;
	QWidget           *m_box;
	QVBoxLayout       *m_lay;
	SettingsMap        m_settings;
	ButtonMap          m_button;
	GroupMap           m_groups;

	QLabel            *m_messageLabel;

	QPushButton       *m_addPovLightBtn;
	QPushButton       *m_removePovLightBtn;
	QPushButton       *m_updatePovLightBtn;

	QLineEdit         *m_PovLightNumEdit;
	QComboBox         *m_PovLightOptTypeCombo;
	QCheckBox         *m_PovLightOptShadowlessChk;
	QDoubleSpinBox    *m_PovLightOptLatitudeDSpin;
	QDoubleSpinBox    *m_PovLightOptLongitudeDSpin;
	QDoubleSpinBox    *m_PovLightOptTargetXDSpin;
	QDoubleSpinBox    *m_PovLightOptTargetYDSpin;
	QDoubleSpinBox    *m_PovLightOptTargetZDSpin;
	QDoubleSpinBox    *m_PovLightOptColorRDSpin;
	QDoubleSpinBox    *m_PovLightOptFadeDistanceDSpin;
	QDoubleSpinBox    *m_PovLightOptFadePowerDSpin;
	QDoubleSpinBox    *m_PovLightOptIntensityDSpin;
	QDoubleSpinBox    *m_PovLightOptSpotRadiusDSpin;
	QDoubleSpinBox    *m_PovLightOptSpotFalloffDSpin;
	QDoubleSpinBox    *m_PovLightOptSpotTightnessDSpin;
	QCheckBox         *m_PovLightOptAreaCircleChk;
	QSpinBox          *m_PovLightOptAreaWidthSpin;
	QSpinBox          *m_PovLightOptAreaHeightSpin;
	QSpinBox          *m_PovLightOptAreaRowsSpin;
	QSpinBox          *m_PovLightOptAreaColumnsSpin;
	QComboBox         *m_PovLightCombo;

	QStringList        m_PovLightList;
	QMap<int,PovLight> m_PovLightMap;
};

#endif // __LDVIEWExportOption_H__
