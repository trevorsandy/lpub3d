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

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include <stdlib.h>
#include <LDLib/LDrawModelViewer.h>
#include <LDLib/LDPreferences.h>
#include <LDLib/LDInputHandler.h>
#include "ui_LDVPreferencesPanel.h"
#include <QStyleFactory>

class LDVWidget;

typedef enum
{
	LDVPollNone,
	LDVPollPrompt,
	LDVPollAuto,
	LDVPollBackground
} LDVPollMode;

class LDVPreferences : public QDialog, Ui::LDVPreferencesPanel
{
	Q_OBJECT
public:
    LDVPreferences(LDVWidget *modelWidget);
	~LDVPreferences(void);

	void setModelViewer(LDrawModelViewer *_modelViewer) {modelViewer = _modelViewer;}
	void abandonChanges(void);
	bool getAllowPrimitiveSubstitution(void);
	void getRGB(int color, int &r, int &g, int &b);
	void getBackgroundColor(int &r, int &g, int &b);
	bool getShowErrors(void);
	void setAniso(int);

	void setShowError(int errorNumber, bool value);
	bool getShowError(int errorNumber);

	bool getStatusBar(void) { return statusBar; }
	void setStatusBar(bool value);
	bool getToolBar(void) { return toolBar; }
	void setToolBar(bool value);
	void setButtonState(QCheckBox *button, bool state);
	void setButtonState(QRadioButton *button, bool state);
	void setWindowSize(int width, int height);
	int  getWindowWidth(void);
	int  getWindowHeight(void);
	void setDrawWireframe(bool);
	bool getDrawWireframe(void);
	void setUseWireframeFog(bool);
	bool getUseWireframeFog(void);
	void setRemoveHiddenLines(bool);
	bool getRemoveHiddenLines(void);
	void setTextureStud(bool);
	bool getTextureStud(void);
	void setEdgeOnly(bool);
	void setConditionalLine(bool);
	void setHighQuality(bool);
	void setAlwaysBlack(bool);
	bool getEdgeOnly(void) {return ldPrefs->getEdgesOnly();}
	bool getConditionalLine(void) {return ldPrefs->getDrawConditionalHighlights();}
	bool getHighQuality(void) {return ldPrefs->getUsePolygonOffset();}
	bool getAlwaysBlack(void) {return ldPrefs->getBlackHighlights();}
	void setRedBackFaces(bool);
	void setGreenFrontFaces(bool);
	void setBlueNeutralFaces(bool);
	bool getRedBackFaces(void) {return ldPrefs->getRedBackFaces();}
	bool getGreenFrontFaces(void) {return ldPrefs->getGreenFrontFaces();}
	bool getBlueNeutralFaces(void) {return ldPrefs->getBlueNeutralFaces();}
	void setShowsHighlightLines(bool);
	bool getShowsHighlightLines(void);
	void setUseLighting(bool);
	bool getUseLighting(void);
	bool getUseBFC(void);
	void setUseBFC(bool);
	bool getShowAxes(void);
	void setShowAxes(bool);
	void setAllowPrimitiveSubstitution(bool);
	void setUseSeams(bool);
	bool getUseSeams(void);

    static char *getLastOpenPath(char *pathKey = nullptr);
    static void setLastOpenPath(const char *path, char *pathKey = nullptr);
	static char *getLDrawDir(void);
	static void setLDrawDir(const char *path);
	static long getMaxRecentFiles(void);
	static char *getRecentFile(int index);
	static void setRecentFile(int index, char *filename);
	static LDVPollMode getPollMode(void);
	static void setPollMode(LDVPollMode value);
	static LDInputHandler::ViewMode getViewMode(void);
	static void setViewMode(LDInputHandler::ViewMode value);
    static void setCheckPartTracker(bool value);
    static bool getCheckPartTracker(void);
	static bool getLatLongMode(void);
	static void setLatLongMode(bool);
	static bool getKeepRightSideUp(void);
	static void setKeepRightSideUp(bool);
	static bool getPovAspectRatio(void);
	void performHotKey(int);
	void setupPrefSetsList(void);
	void userDefaultChangedAlertCallback(TCAlert *alert);
	void checkLightVector(void);
	void browseForDir(QString prompt, QLineEdit *textField, QString &dir);
	QString getSaveDir(LDPreferences::SaveOp saveOp,const std::string &filename) { return QString(ldPrefs->getDefaultSaveDir(saveOp, filename).c_str()); }

#ifdef WIN32
    int getFSAAFactor(void);
    bool getUseNvMultisampleFilter(void);
    void setupAntialiasing(void);
#endif

public slots:
	void doApply(void);
	void doOk(void);
	void enableApply(void);
	void enableProxy(void);
	void disableProxy(void);
	void doCancel(void);
	void doResetGeneral(void);
	void doResetGeometry(void);
	void doResetEffects(void);
	void doResetPrimitives(void);
	void doResetUpdates(void);
	void doResetTimesUpdates(void);
	void doWireframeCutaway(bool value);
	void doLighting(bool value);
	void doProxyServer(bool value);
	void doUpdateMissingparts(bool value);
	void doStereo(bool value);
	void doWireframe(bool value);
	void doSortTransparency(bool value);
	void doStippleTransparency(bool value);
	void doBFC(bool value);
	void doEdgeLines(bool value);
	void doConditionalShow(bool value);
	void doPrimitiveSubstitution(bool value);
	void doTextureStuds(bool value);
	void doNewPreferenceSet(void);
	void doDelPreferenceSet(void);
	void doHotkeyPreferenceSet(void);
	bool doPrefSetSelected() {return doPrefSetSelected(false);}
	void doPrefSetSelected(QListWidgetItem *, QListWidgetItem *) {doPrefSetSelected(false);}
	bool doPrefSetSelected(bool);
	void doPrefSetsApply(void);
	void show(void);
	void doBackgroundColor();
	void doDefaultColor();
	void doAnisotropic();
	void doAnisotropicSlider(int);
	void doDrawLightDats();
	void doSaveDefaultViewAngle();
#ifdef WIN32	
    void fsaaModeBoxChanged(const QString&);
#endif // WIN32	
	void snapshotSaveDirBoxChanged();
	void partsListsSaveDirBoxChanged();
	void exportsListsSaveDirBoxChanged();
	void snapshotSaveDirBrowse();
	void partsListsSaveDirBrowse();
	void exportsSaveDirBrowse();
    void enableStudLogoCombo();

protected:
	void doGeneralApply(void);
	void doGeometryApply(void);
	void doEffectsApply(void);
	void doPrimitivesApply(void);
	void doUpdatesApply(void);

	void loadSettings(void);
	void loadOtherSettings(void);

	void loadDefaultOtherSettings(void);

	void reflectSettings(void);
	void reflectGeneralSettings(void);
	void reflectGeometrySettings(void);
	void reflectWireframeSettings(void);
	void reflectBFCSettings(void);
	void reflectEffectsSettings(void);
	void reflectPrimitivesSettings(void);
	void reflectUpdatesSettings(void);

    void setDoubleRangeValue(QDoubleSpinBox *rangeConrol, float value);
    void setRangeValue(QSpinBox *rangeConrol, int value);
	void setRangeValue(QSlider *rangeConrol, int value);

	void enableWireframeCutaway(void);
	void enableLighting(void);
	void enableStereo(void);
	void enableWireframe(void);
	void enableBFC(void);
	void enableEdgeLines(void);
	void enableConditionalShow(void);
	void enablePrimitiveSubstitution(void);
	void enableTextureStuds(void);
	void enableTexmaps(void);
	void enableProxyServer(void);

	void disableWireframeCutaway(void);
	void disableLighting(void);
	void disableStereo(void);
	void disableWireframe(void);
	void disableBFC(void);
	void disableEdgeLines(void);
	void disableConditionalShow(void);
	void disablePrimitiveSubstitution(void);
	void disableTextureStuds(void);
	void disableTexmaps(void);
	void disableProxyServer(void);
	void updateTexmapsEnabled(void);
	void setupDefaultRotationMatrix(void);
	void uncheckLightDirections(void);
	LDPreferences::LightDirection getSelectedLightDirection(void);
	void selectLightDirection(LDPreferences::LightDirection);
	void updateSaveDir(QLineEdit *textField, QPushButton *button,
                       LDPreferences::DefaultDirMode dirMode,
                       QString &filename);
	void setupSaveDir(QComboBox *comboBox, QLineEdit *textField,
                      QPushButton *button, LDPreferences::DefaultDirMode dirMode,
                      QString &filename);
	void setupSaveDirs(void);
	const char *getPrefSet(int);
	const char *getSelectedPrefSet(void);
    void selectPrefSet(const char *prefSet = nullptr, bool force = false);
	char *getHotKey(int);
	int getHotKey(const char*);
	int getCurrentHotKey(void);
	void saveCurrentHotKey(void);

	char *getErrorKey(int errorNumber);
	static const QString &getRecentFileKey(int index);

	LDrawModelViewer *modelViewer;
    LDPreferences    *ldPrefs;

	QColor backgroundColor;
	QColor defaultColor;

	bool checkAbandon;
	int hotKeyIndex;

	// Other Settings
	bool statusBar;
	bool toolBar;
	int windowWidth;
	int windowHeight;
	QString snapshotDir, partsListDir, exportDir;
    //QIntValidator *proxyPortValidator;
};

#endif // __PREFERENCES_H__
