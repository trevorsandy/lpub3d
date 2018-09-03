#ifndef __LDVIEWExportOption_H__
#define __LDVIEWExportOption_H__

#include <TCFoundation/TCStlIncludes.h>
#include <TCFoundation/TCTypedObjectArray.h>
#include <QDialog>
#include <QFileDialog>
#include <QLineEdit>
#include <QVBoxLayout>
#include "LDLib/LDrawModelViewer.h"

#include "ui_LDVExportOptionPanel.h"

class LDVPreferences;
class LDrawModelViewer;
class LDExporter;
class LDExporterSetting;
class TCStringArray;

typedef std::map<LDExporterSetting *, QWidget *> SettingsMap;
typedef std::map<QPushButton *, QLineEdit *> ButtonMap;
typedef std::map<QLayout *, SettingsMap> GroupMap;

namespace Ui{
class LDVExportOptionPanel;
}

class LDViewExportOption : public QDialog, Ui::LDVExportOptionPanel
{
	Q_OBJECT
public:
	LDViewExportOption(QWidget *parent,LDrawModelViewer *modelViewer, LDrawModelViewer::ExportType = LDrawModelViewer::ETPov);
	~LDViewExportOption();

	QString const getLights();
	void recordExtraSearchDirs(void);
	void populateExtraSearchDirs(void);
	static TCStringArray* extraSearchDirs;

public slots:
	int exec(void);
	void doOk(void);
	void doCancel(void);
	void doReset(void);
	void doResetGroup(void);
	void doBrowse(void);

private slots:
        void on_povAddLightBtn_clicked();
        void on_povRemoveLightBtn_clicked();
        void on_povLightsCombo_currentIndexChanged(int index);

protected:
//	void populateTypeBox(void);
	void populate(void);

	void doTypeBoxActivated(void);
	void resetSettings(SettingsMap &settings);

	LDrawModelViewer *m_modelViewer;
	LDExporter *m_exporter;
	QWidget *m_box;
	QVBoxLayout *m_lay;
	SettingsMap m_settings;
	ButtonMap m_button;
	GroupMap m_groups;
	int m_origType;

private:
	Ui::LDVExportOptionPanel ui;
	QWidget       *parent;
};

#endif // __LDVIEWExportOption_H__
