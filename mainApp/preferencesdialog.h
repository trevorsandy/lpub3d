/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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
#include "threadworkers.h"
#include "parmswindow.h"
#include "meta.h"

// Set minimum size
class PreferencesSizeWidget : public QWidget
{
    Q_OBJECT
public:
    PreferencesSizeWidget(const QSize _sh, const QSize _msh, QWidget *_parent = 0) :
        QWidget(_parent), sh(_sh), msh(_msh) {}
    ~PreferencesSizeWidget() {}
    QSize sizeHint() const override { QSize s = QWidget::sizeHint(); return s.expandedTo(sh); }
    QSize minimumSizeHint() const override { QSize s = QWidget::minimumSizeHint(); return s.expandedTo(msh); }
private:
    QSize sh;
    QSize msh;
};

// LcLib Preferences
#include "lc_application.h"

struct lcLibRenderOptions
{
    lcPreferences Preferences;
    int AASamples;
    lcStudStyle StudStyle;
    QMap<QString, QKeySequence> KeyboardShortcuts;
    bool KeyboardShortcutsModified;
    bool KeyboardShortcutsDefault;
};

class CommandListColumnStretcher : public QObject
{
    Q_OBJECT

public:
    CommandListColumnStretcher(QTreeWidget* TreeWidget, int ColumnToStretch);

    bool eventFilter(QObject* Object, QEvent* Event) override;

private slots:
    void sectionResized(int LogicalIndex, int OldSize, int NewSize);

private:
    const int m_columnToStretch;
    bool m_interactiveResize;
    int m_stretchWidth;
};

namespace Ui{
class PreferencesDialog;
}

class BlenderPreferences;
class ThemeColorsDialog;
class PreferencesDialog : public QDialog
{
  Q_OBJECT
  
  public:
    explicit PreferencesDialog(QWidget *parent);
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
    bool          autoUpdateChangeLog();
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
    bool          inlineNativeContent();
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
    bool          showBuildModEditErrors();
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
    bool          warningLevel();
    bool          errorLevel();
    bool          fatalLevel();
    bool          allLogLevels();
    bool          suppressFPrint();

    int           preferredRenderer();
    int           ldrawFilesLoadMsgs();
    int           checkUpdateFrequency();
    int           povrayRenderQuality();
    int           rendererTimeout();
    int           pageDisplayPause();
    int           fadeStepsOpacity();
    int           highlightStepLineWidth();
    bool          highlightFirstStep();

    void          setPreferences();
    void          setOptions(lcLibRenderOptions* Options);

    bool          eventFilter(QObject* Object, QEvent* Event) override;

  public slots:
    void accept() override;
    void cancel();

    void on_shortcutAssign_clicked();
    void on_shortcutRemove_clicked();
    void on_shortcutsImport_clicked();
    void on_shortcutsExport_clicked();
    void on_shortcutsReset_clicked();
    void on_KeyboardFilterEdit_textEdited(const QString& Text);
    void commandChanged(QTreeWidgetItem *current);

  private slots:
    void on_browseLDraw_clicked();
    void on_browseAltLDConfig_clicked();
    void on_browseLGEO_clicked();
    void on_browsePli_clicked();
    void on_browsePublishLogo_clicked();
    void on_checkForUpdates_btn_clicked();
    void on_highlightStepBtn_clicked();
    void on_fadeStepsOpacitySlider_valueChanged(int);
    void on_ldrawLibPathEdit_editingFinished();

    void on_altLDConfigGrpBox_clicked(bool checked);
    void on_includeAllLogAttribBox_clicked(bool checked);
    void on_allLogLevelsBox_clicked(bool checked);
    void on_logLevelsGrpBox_clicked(bool checked);
    void on_logLevelGrpBox_clicked(bool checked);
    void on_archiveLSynthPartsBox_clicked(bool checked);

    void on_ldviewGrpBox_clicked(bool checked);
    void on_ldgliteGrpBox_clicked(bool checked);
    void on_povrayGrpBox_clicked(bool checked);

    void on_highlightStepGrpBox_clicked(bool checked);
    void on_fadeStepGrpBox_clicked(bool checked);
    void on_fadeStepsUseColourGrpBox_clicked(bool checked);
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
    void on_autoUpdateChangeLogBox_clicked(bool checked);
    void on_updateChangeLogBtn_clicked();
    void on_themeColorsButton_clicked();

    void installRenderer();
    void setRenderers();
    void messageManagement();
    void sceneColorButtonClicked();

    void updateChangelog();
    void checkForUpdates();

    void ldvPoVFileGenOptBtn_clicked();
    void ldvPoVFileGenPrefBtn_clicked();

    // LcLib Preferences
    void on_antiAliasing_toggled();
    void on_edgeLines_toggled();
    void on_ConditionalLinesCheckBox_toggled();
    void on_LineWidthSlider_valueChanged();
    void on_MeshLODSlider_valueChanged();
    void on_studStyleCombo_currentIndexChanged(int index);
    void on_LPubFadeHighlight_toggled();
    void on_FadeSteps_toggled();
    void on_HighlightNewParts_toggled();
    void on_AutomateEdgeColor_toggled();
    void on_ViewpointsCombo_currentIndexChanged(int index);
    void on_ProjectionCombo_currentIndexChanged(int index);
    void on_cameraDefaultDistanceFactor_valueChanged(double value);
    void on_cameraDefaultPosition_valueChanged(double value);

    void AutomateEdgeColor();
    void ColorButtonClicked();
    void ResetFadeHighlightColor();
    void cameraPropertyReset();

    void lcQPreferencesInit();
    void lcQPreferencesAccept();

    void shortcutEditReset();
    void enableShortcutEditReset(const QString &displayText);

    void applyBlenderAddonPreferences();
    void resetBlenderAddonPreferences();
    void showBlenderAddonPaths();
    void enableBlenderAddonButton(bool);

    bool maybeSave();
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::PreferencesDialog ui;

    void updateCommandList(bool = false);
    void setShortcutModified(QTreeWidgetItem *treeItem, bool modified, bool invalid = false);
    bool SaveKeyboardShortcuts(const QString& FileName, int &Count);
    bool SaveKeyboardShortcuts(QTextStream& Stream, int &Count);
    bool LoadKeyboardShortcuts(const QString& FileName);
    bool LoadKeyboardShortcuts(QTextStream& Stream);
    bool isValidKeyboardShortcut(const QString &ObjectName, const QString &NewShortcut);
    bool isValidKeyboardShortcut(const QString &ObjectName, const QString &NewShortcut, QString &Result, bool Loading);

    void setBlenderAddonPreferences();

    QPushButton *blenderAddonApplyButton;
    QPushButton *blenderAddonResetButton;
    QPushButton *blenderAddonPathsButton;
    BlenderPreferences *blenderAddonPreferences;

    QDialog     *messageDialog;
    QToolButton *parseErrorTBtn;
    QLabel      *parseErrorLbl;
    QToolButton *insertErrorTBtn;
    QLabel      *insertErrorLbl;
    QToolButton *buildModErrorTBtn;
    QLabel      *buildModErrorLbl;
    QToolButton *buildModEditErrorTBtn;
    QLabel      *buildModEditErrorLbl;
    QToolButton *includeErrorTBtn;
    QLabel      *includeErrorLbl;
    QToolButton *annotationErrorTBtn;
    QLabel      *annotationErrorLbl;
    QToolButton *clearDetailErrorsTBtn;
    QLabel      *clearDetailErrorsLbl;
    QDialogButtonBox *messageButtonBox;

    QPalette readOnlyPalette;

    QMap<int, QString> editedThemeColors;

    int  nativeRendererIndex;
    int  previousRendererIndex;

    bool mShowLineParseErrors;
    bool mShowInsertErrors;
    bool mShowBuildModErrors;
    bool mShowBuildModEditErrors;
    bool mShowIncludeFileErrors;
    bool mShowAnnotationErrors;

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

    bool m_updateFinished;

    // LcLib Preferences
    lcLibRenderOptions* mOptions;
    bool mSetOptions;

    float mLineWidthRange[2];
    float mLineWidthGranularity;
    static constexpr float mMeshLODMultiplier = 25.0f;
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
