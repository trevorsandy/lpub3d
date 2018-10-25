/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
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

	QString const getLights();
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
    void resetLights(void);
    void show(void);

private slots:
        void on_povAddLightBtn_clicked();
        void on_povRemoveLightBtn_clicked();
        void on_povLightsCombo_currentIndexChanged(int index);

protected:
    void populateExportSettings(void);
    void setLights(void);
    void applyLights(void);
	void resetSettings(SettingsMap &settings);

	LDrawModelViewer *m_modelViewer;
	LDExporter *m_exporter;
	QWidget *m_box;
	QVBoxLayout *m_lay;
	SettingsMap m_settings;
	ButtonMap m_button;
	GroupMap m_groups;
    bool m_nativePOVIni;
};

#endif // __LDVIEWExportOption_H__
