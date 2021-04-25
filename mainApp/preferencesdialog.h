/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2021 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>

#include "ui_preferences.h"
#include "lc_qpreferencesdialog.h"
#include "qsimpleupdater.h"
#include "threadworkers.h"
#include "parmswindow.h"
#include "meta.h"

namespace Ui{
class PreferencesDialog;
}

class ThemeColorsDialog;
class PreferencesDialog : public QDialog
{
  Q_OBJECT
  
  public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();
	
    QString const ldrawLibPath();
    QString const altLDConfigPath();
    QString const lgeoPath();
    QString const pliControlFile();
    QString const povrayExe();
    QString const ldgliteExe();
    QString const ldviewExe();
    QString const moduleVersion();
    QString const defaultAuthor();
    QString const defaultURL();
    QString const defaultEmail();
    QString const documentLogoFile();
    QString const publishDescription();
    QString const logLevelCombo();
    QString const fadeStepsColour();
    QString const highlightStepColour();
    QString const sceneBackgroundColor();
    QString const sceneGridColor();
    QString const sceneRulerTickColor();
    QString const sceneRulerTrackingColor();
    QString const sceneGuideColor();
    QString const displayTheme();
    QMap<int, QString> const themeColours();
    bool          useNativePovGenerator();
    bool          displayAllAttributes();
    bool          generateCoverPages();
    bool          printDocumentTOC();
    bool          doNotShowPageProcessDlg();
    bool          loadLastOpenedFile();
    bool          extendedSubfileSearch();
    bool          centimeters();
    bool          applyCALocally();
    bool          enableFadeSteps();
    bool          fadeStepsUseColour();
    bool          enableHighlightStep();
    bool          enableImageMatting();
    bool          enableDocumentLogo();
    bool          enableLDViewSingleCall();
    bool          enableLDViewSnaphsotList();
    bool          showUpdateNotifications();
    bool          enableDownloader();
    bool          showDownloadRedirects();
    bool          showAllNotifications();
    bool          lgeoStlLib();
    bool          povrayDisplay();
    bool          povrayAutoCrop();
    bool          includeLogLevel();
    bool          includeTimestamp();
    bool          includeLineNumber();
    bool          includeFileName();
    bool          includeFunction();
    bool          includeAllLogAttrib();
    bool          showLineParseErrors();
    bool          showInsertErrors();
    bool          showBuildModErrors();
    bool          showIncludeFileErrors();
    bool          showAnnotationErrors();
    bool          showSaveOnRedraw();
    bool          showSaveOnUpdate();
    bool          addLSynthSearchDir();
    bool          archiveLSynthParts();
    bool          perspectiveProjection();
    bool          resetSceneColors();
    bool          saveOnRedraw();
    bool          saveOnUpdate();

    bool          loggingGrpBox();
    bool          logLevelGrpBox();
    bool          logLevelsGrpBox();

    bool          debugLevel();
    bool          traceLevel();
    bool          noticeLevel();
    bool          infoLevel();
    bool          statusLevel();
    bool          errorLevel();
    bool          fatalLevel();
    bool          allLogLevels();

    int           preferredRenderer();
    int           ldrawFilesLoadMsgs();
    int           checkUpdateFrequency();
    int           povrayRenderQuality();
    int           rendererTimeout();
    int           pageDisplayPause();
    int           fadeStepsOpacity();
    int           highlightStepLineWidth();
    bool          highlightFirstStep();

  public slots:
    void accept();
    void cancel();

  private slots:
    void on_browseLDraw_clicked();
    void on_browseAltLDConfig_clicked();
    void on_browseLGEO_clicked();
    void on_browsePli_clicked();
    void on_browsePublishLogo_clicked();
    void on_checkForUpdates_btn_clicked();
    void on_highlightStepBtn_clicked();
    void on_ldrawLibPathEdit_editingFinished();

    void on_altLDConfigBox_clicked(bool checked);
    void on_includeAllLogAttribBox_clicked(bool checked);
    void on_allLogLevelsBox_clicked(bool checked);
    void on_logLevelsGrpBox_clicked(bool checked);
    void on_logLevelGrpBox_clicked(bool checked);
    void on_archiveLSynthPartsBox_clicked(bool checked);

    void on_ldviewBox_clicked(bool checked);
    void on_ldgliteBox_clicked(bool checked);
    void on_povrayBox_clicked(bool checked);

    void on_highlightStepBox_clicked(bool checked);
    void on_fadeStepBox_clicked(bool checked);
    void on_fadeStepsUseColourBox_clicked(bool checked);
    void on_fadeStepsColoursCombo_currentIndexChanged(const QString &colorName);

    void on_preferredRenderer_currentIndexChanged(const QString &arg1);
    void on_projectionCombo_currentIndexChanged(const QString &arg1);
    void on_loggingGrpBox_clicked(bool checked);

    void on_ldvPreferencesBtn_clicked();
    void on_povGenNativeRadio_clicked(bool checked);
    void on_povGenLDViewRadio_clicked(bool checked);
    void on_ldviewSingleCall_Chk_clicked(bool checked);
    void on_applyCALocallyRadio_clicked(bool checked);

    void on_resetSceneColorsButton_clicked(bool checked);
    void on_saveOnRedrawChkBox_clicked(bool checked);
    void on_saveOnUpdateChkBox_clicked(bool checked);

    void on_optionsButton_clicked(bool checked);

    void ldvPoVFileGenOptBtn_clicked();
    void ldvPoVFileGenPrefBtn_clicked();

    void installRenderer();
    void setRenderers();
    void messageManagement();
    void sceneColorButtonClicked();
    void updateChangelog (QString url);
    void checkForUpdates();
    void updaterCancelled();

    void on_themeColorsButton_clicked();

private:
    Ui::PreferencesDialog ui;

    QWidget     *parent;

    QDialog     *messageDialog;
    QToolButton *parseErrorTBtn;
    QLabel      *parseErrorLbl;
    QToolButton *insertErrorTBtn;
    QLabel      *insertErrorLbl;
    QToolButton *buildModErrorTBtn;
    QLabel      *buildModErrorLbl;
    QToolButton *includeErrorTBtn;
    QLabel      *includeErrorLbl;
    QToolButton *annotationErrorTBtn;
    QLabel      *annotationErrorLbl;
    QToolButton *clearDetailErrorsTBtn;
    QLabel      *clearDetailErrorsLbl;
    QDialogButtonBox *messageButtonBox;

    QMap<int, QString> editedThemeColors;

    bool mShowLineParseErrors;
    bool mShowInsertErrors;
    bool mShowBuildModErrors;
    bool mShowIncludeFileErrors;
    bool mShowAnnotationErrors;

    QSimpleUpdater  *m_updater;
    bool             m_updaterCancelled;
    static QString   DEFS_URL;
    QString ldrawLibPathTitle;
    QString mLDrawLibPath;
    QString sceneBackgroundColorStr;
    QString sceneGridColorStr;
    QString sceneRulerTickColorStr;
    QString sceneRulerTrackingColorStr;
    QString sceneGuideColorStr;
    bool resetSceneColorsFlag;
    bool showSaveOnRedrawFlag;
    bool showSaveOnUpdateFlag;
};

/***********************************************************************
 *
 * Theme Colors
 *
 **********************************************************************/

class ThemeColorsDialog : public QObject
{
    Q_OBJECT
public:
    ThemeColorsDialog(){}
    ~ThemeColorsDialog(){}

    QMap<int, QString> getEditedThemeColors();

public slots:
    void setThemeColor();
    void toggleDefaultsTab();
    void resetThemeColors();
    void resetThemeColor();

protected:
    QColor getColor(const int index,
                    const QString &name);

private:
    QDialog    *dialog;
    QTabWidget *tabs;
    QList<QToolButton *> colorButtonList;
    QList<QToolButton *> resetButtonList;
    QString mThemeColors[THEME_NUM_COLORS];
};

#endif
