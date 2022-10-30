/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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

#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include "ui_preferences.h"
#include "preferencesdialog.h"
#include "lpub_preferences.h"
#include "application.h"
#include "updatecheck.h"

#include <LDVQt/LDVWidget.h>

#include "color.h"
#include "meta.h"
#include "lpub.h"
#include "version.h"
#include "lc_profile.h"
#include "lc_application.h"

//QString PreferencesDialog::DEFS_URL = QString(VER_UPDATE_CHECK_JSON_URL).arg(qApp->applicationVersion());
QString PreferencesDialog::DEFS_URL = VER_UPDATE_CHECK_JSON_URL;

PreferencesDialog::PreferencesDialog(QWidget* _parent, lcLibRenderOptions* Options) :
    QDialog(_parent), mOptions(Options)
{
  ui.setupUi(this);

  resetSceneColorsFlag = false;

  mLDrawLibPath = Preferences::ldrawLibPath;

  if (mLDrawLibPath.isEmpty()) {
    mLDrawLibPath = ".";
  }

  // hide 3rd party application browse buttons
  ui.browseLDGLite->hide();
  ui.browseLDView->hide();
  ui.browsePOVRAY->hide();

  QPalette readOnlyPalette = QApplication::palette();
  if (Preferences::displayTheme == THEME_DARK)
      readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));
  else
      readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]));
  readOnlyPalette.setColor(QPalette::Text,QColor(LPUB3D_DISABLED_TEXT_COLOUR));

  ldrawLibPathTitle            = QString("LDraw Library Path for %1").arg(Preferences::validLDrawPartsLibrary);
  QString fadeStepsColorTitle  = QString("Use %1 Global Fade Color").arg(Preferences::validLDrawPartsLibrary);
  QString ldrawSearchDirsTitle = QString("LDraw Content Search Directories for %1").arg(Preferences::validLDrawPartsLibrary);
  bool useLDViewSCall = (Preferences::enableLDViewSingleCall && Preferences::preferredRenderer == RENDERER_LDVIEW);

  // populate model file defaults from settings (versus Preferences)
  QSettings Settings;
  if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"Author"))) {
    ui.authorName_Edit->setText(                 Settings.value(QString("%1/%2").arg(DEFAULTS,"Author")).toString());
  }
  if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"URL"))) {
    ui.publishURL_Edit->setText(                 Settings.value(QString("%1/%2").arg(DEFAULTS,"URL")).toString());
  }
  if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"Email"))) {
    ui.publishEmail_Edit->setText(               Settings.value(QString("%1/%2").arg(DEFAULTS,"Email")).toString());
  }
  if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"))) {
    ui.publishLogoPath->setText(                 Settings.value(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile")).toString());
    QFileInfo fileInfo(ui.publishLogoPath->text());
    if (!fileInfo.exists()) {
      Settings.remove(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"));
      ui.publishLogoPath->clear();
    }
  }
  if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"PublishDescription"))) {
    ui.publishDescriptionEdit->setText(          Settings.value(QString("%1/%2").arg(DEFAULTS,"PublishDescription")).toString());
  }
  ui.publishLogoBox->setChecked(!ui.publishLogoPath->text().isEmpty());

  // set 3rd party application dialogues to read-only
  ui.ldglitePath->setReadOnly(true);
  ui.ldglitePath->setPalette(readOnlyPalette);
  ui.ldviewPath->setReadOnly(true);
  ui.ldviewPath->setPalette(readOnlyPalette);
  ui.povrayPath->setReadOnly(true);
  ui.povrayPath->setPalette(readOnlyPalette);

  // set log path to read only
  ui.logPathEdit->setReadOnly(true);
  ui.logPathEdit->setPalette(readOnlyPalette);

  // [WIP]
  ui.imageMatteBox->setEnabled(false);
  ui.imageMattingChk->setEnabled(false);

  showSaveOnRedrawFlag =                         Preferences::showSaveOnRedraw;
  showSaveOnUpdateFlag =                         Preferences::showSaveOnUpdate;

  ui.ldrawLibPathEdit->setText(                  mLDrawLibPath);
  ui.ldrawLibPathBox->setTitle(                  ldrawLibPathTitle);

  // preferred renderer
  ui.ldviewPath->setText(                        Preferences::ldviewExe);
  ui.ldviewBox->setVisible(                      Preferences::ldviewInstalled);
  ui.ldviewInstallBox->setVisible(              !Preferences::ldviewInstalled);

  ui.ldviewSingleCall_Chk->setChecked(           useLDViewSCall);
  ui.ldviewSnaphsotsList_Chk->setChecked(        Preferences::enableLDViewSnaphsotList && useLDViewSCall);
  ui.ldviewSnaphsotsList_Chk->setEnabled(        useLDViewSCall);

  ui.ldglitePath->setText(                       Preferences::ldgliteExe);
  ui.ldgliteBox->setVisible(                     Preferences::ldgliteInstalled);
  ui.ldgliteInstallBox->setVisible(             !Preferences::ldgliteInstalled);

  ui.povrayPath->setText(                        Preferences::povrayExe);
  ui.povrayBox->setVisible(                      Preferences::povRayInstalled);
  ui.povrayInstallBox->setVisible(              !Preferences::povRayInstalled);

  ui.povrayDisplay_Chk->setChecked(              Preferences::povrayDisplay);
  ui.povrayAutoCropBox->setChecked(              Preferences::povrayAutoCrop);
  ui.povrayRenderQualityCombo->setCurrentIndex(  Preferences::povrayRenderQuality);

  ui.lgeoBox->setEnabled(                        Preferences::usingDefaultLibrary);
  ui.lgeoPath->setText(                          Preferences::lgeoPath);
  ui.lgeoBox->setChecked(                        Preferences::lgeoPath != "");
  ui.lgeoStlLibLbl->setText(                     Preferences::lgeoStlLib ? DURAT_LGEO_STL_LIB_INFO : "");

  setRenderers();

  // end preferred renderer

  ui.preferencesTabWidget->setCurrentIndex(0);

  ui.fadeStepsUseColourBox->setTitle(            fadeStepsColorTitle);
  ui.pliControlEdit->setText(                    Preferences::pliControlFile);
  ui.altLDConfigPath->setText(                   Preferences::altLDConfigPath);
  ui.altLDConfigBox->setChecked(                 Preferences::altLDConfigPath != "");
  ui.pliControlBox->setChecked(                  Preferences::pliControlFile != "");

  ui.displayAllAttributes_Chk->setChecked(       Preferences::displayAllAttributes);
  ui.generateCoverPages_Chk->setChecked(         Preferences::generateCoverPages);
  ui.publishTOC_Chk->setChecked(                 Preferences::printDocumentTOC);
  ui.doNotShowPageProcessDlgChk->setChecked(     Preferences::doNotShowPageProcessDlg);
  ui.enableDownloader_Chk->setChecked(           Preferences::enableDownloader);
  ui.showDownloadRedirects_Chk->setChecked(      Preferences::showDownloadRedirects);
  ui.addLSynthSearchDirBox->setEnabled(          Preferences::archiveLSynthParts);
  ui.addLSynthSearchDirBox->setChecked(          Preferences::addLSynthSearchDir);
  ui.archiveLSynthPartsBox->setChecked(          Preferences::archiveLSynthParts);
  ui.showUpdateNotifications_Chk->setChecked(    Preferences::showUpdateNotifications);
  ui.showAllNotificstions_Chk->setChecked(       Preferences::showAllNotifications);
  ui.checkUpdateFrequency_Combo->setCurrentIndex(Preferences::checkUpdateFrequency);
  ui.rendererTimeout->setValue(                  Preferences::rendererTimeout);
  ui.pageDisplayPauseSpin->setValue(             Preferences::pageDisplayPause);

  ui.loadLastOpenedFileCheck->setChecked(        Preferences::loadLastOpenedFile);
  ui.extendedSubfileSearchCheck->setChecked(     Preferences::extendedSubfileSearch);
  ui.ldrawFilesLoadMsgsCombo->setCurrentIndex(   Preferences::ldrawFilesLoadMsgs);
  ui.projectionCombo->setCurrentIndex(           Preferences::perspectiveProjection ? 0 : 1);

  ui.saveOnRedrawChkBox->setChecked(             Preferences::saveOnRedraw);
  ui.saveOnUpdateChkBox->setChecked(             Preferences::saveOnUpdate);

  ui.loggingGrpBox->setChecked(                  Preferences::logging);
  ui.logPathEdit->setEnabled(                    Preferences::logging);
  ui.logPathEdit->setText(                       Preferences::logPath);

  ui.includeLogLevelBox->setChecked(             Preferences::includeLogLevel);
  ui.includeTimestampBox->setChecked(            Preferences::includeTimestamp);
  ui.includeLineNumberBox->setChecked(           Preferences::includeLineNumber);
  ui.includeFileNameBox->setChecked(             Preferences::includeFileName);
  ui.includeFunctionBox->setChecked(             Preferences::includeFunction);

  ui.logLevelGrpBox->setChecked(                 Preferences::logLevel);
  ui.logLevelsGrpBox->setChecked(                Preferences::logLevels);

  ui.fadeStepBox->setChecked(                    Preferences::enableFadeSteps);
  ui.fadeStepsUseColourBox->setEnabled(          Preferences::enableFadeSteps);
  ui.fadeStepsUseColourBox->setChecked(          Preferences::fadeStepsUseColour);
  ui.fadeStepsColoursCombo->setEnabled(          Preferences::enableFadeSteps && Preferences::fadeStepsUseColour);
  ui.fadeStepsOpacityBox->setEnabled(            Preferences::enableFadeSteps);
  ui.fadeStepsOpacitySlider->setEnabled(         Preferences::enableFadeSteps);
  ui.fadeStepsOpacitySlider->setValue(           Preferences::fadeStepsOpacity);

  ui.fadeStepsColoursCombo->addItems(LDrawColor::names());
  ui.fadeStepsColoursCombo->setCurrentIndex(int(ui.fadeStepsColoursCombo->findText(Preferences::validFadeStepsColour)));
  QColor fadeColor = LDrawColor::color(Preferences::validFadeStepsColour);
  if(fadeColor.isValid() ) {
      ui.fadeStepsColourLabel->setAutoFillBackground(true);
      QString styleSheet =
              QString("QLabel { background-color: rgb(%1, %2, %3); }")
              .arg(fadeColor.red())
              .arg(fadeColor.green())
              .arg(fadeColor.blue());
      ui.fadeStepsColourLabel->setStyleSheet(styleSheet);
  }

  ui.highlightStepBox->setChecked(               Preferences::enableHighlightStep);
  ui.highlightStepBtn->setEnabled(               Preferences::enableHighlightStep);
  ui.highlightStepLabel->setEnabled(             Preferences::enableHighlightStep);

  ui.highlightFirstStepBox->setChecked(          Preferences::highlightFirstStep);

  // Only enabled for LDGLite
  if (Preferences::preferredRenderer == RENDERER_LDGLITE)
    ui.highlightStepLineWidthSpin->setEnabled(   Preferences::enableHighlightStep);
  else
    ui.highlightStepLineWidthSpin->setEnabled(false);
  ui.highlightStepLineWidthSpin->setValue(       Preferences::highlightStepLineWidth);

  QColor highlightColour = QColor(               Preferences::highlightStepColour);
  if(highlightColour.isValid() ) {
    ui.highlightStepColorLabel->setAutoFillBackground(true);
    QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }")
            .arg(highlightColour.red())
            .arg(highlightColour.green())
            .arg(highlightColour.blue());
    ui.highlightStepColorLabel->setStyleSheet(styleSheet);
  }

  QStringList logLevels = tr(VER_LOGGING_LEVELS_STR).split(",");
  ui.logLevelCombo->addItems(logLevels);
  ui.logLevelCombo->setCurrentIndex(int(ui.logLevelCombo->findText(Preferences::loggingLevel)));

  ui.debugLevelBox->setChecked(                  Preferences::debugLevel);
  ui.traceLevelBox->setChecked(                  Preferences::traceLevel);
  ui.noticeLevelBox->setChecked(                 Preferences::noticeLevel);
  ui.infoLevelBox->setChecked(                   Preferences::infoLevel);
  ui.statusLevelBox->setChecked(                 Preferences::statusLevel);
  ui.errorLevelBox->setChecked(                  Preferences::errorLevel);
  ui.fatalLevelBox->setChecked(                  Preferences::fatalLevel);

  ui.includeAllLogAttribBox->setChecked(         Preferences::includeAllLogAttributes);
  ui.allLogLevelsBox->setChecked(                Preferences::allLogLevels);
  ui.logValiationLbl->hide();

  //search directories
  ui.lineEditIniFile->setPalette(readOnlyPalette);
  ui.lineEditIniFile->setReadOnly(true);
  ui.textEditSearchDirs->setWordWrapMode(QTextOption::WordWrap);
  ui.textEditSearchDirs->setLineWrapMode(QTextEdit::FixedColumnWidth);
  ui.textEditSearchDirs->setLineWrapColumnOrWidth(LINE_WRAP_WIDTH);
  ui.textEditSearchDirs->setReadOnly(true);
  ui.textEditSearchDirs->setToolTip("Read only list of LDraw.ini search directories.");
  if (Preferences::ldrawiniFound) {
      ui.lineEditIniFile->setText(QString("Using LDraw.ini File: %1").arg(Preferences::ldrawiniFile));
      ui.lineEditIniFile->setToolTip(tr("LDraw.ini file"));
      ui.textEditSearchDirs->setPalette(readOnlyPalette);
      ui.textEditSearchDirs->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  } else {
      ui.lineEditIniFile->setText(tr("%1")
                                  .arg(Preferences::ldSearchDirs.size() == 0 ?
                                         tr("Using default search. No search directories detected.") :
                                         tr("Using default %1 search.").arg(VER_PRODUCTNAME_STR)));
      ui.lineEditIniFile->setToolTip(tr("Default search"));
  }

  ui.groupBoxSearchDirs->setTitle(ldrawSearchDirsTitle);
  if (Preferences::ldSearchDirs.size() > 0){
      Q_FOREACH (QString searchDir, Preferences::ldSearchDirs)
        ui.textEditSearchDirs->append(searchDir);
  }
  //end search Dirs

  bool centimeters = Preferences::preferCentimeters;
  ui.Centimeters->setChecked(centimeters);
  ui.Inches->setChecked(! centimeters);

  bool applyCALocally = Preferences::applyCALocally;
  ui.applyCALocallyRadio->setChecked(applyCALocally);
  ui.applyCARendererRadio->setChecked(! applyCALocally);

  bool renderPOVRay = Preferences::preferredRenderer == RENDERER_POVRAY;
  ui.povNativeGenBox->setEnabled(renderPOVRay);
  ui.ldvPOVSettingsBox->setEnabled(renderPOVRay);
  ui.ldvPoVFileGenOptBtn->setEnabled(renderPOVRay);
  ui.ldvPoVFileGenPrefBtn->setEnabled(renderPOVRay);
  ui.povGenNativeRadio->setChecked(Preferences::useNativePovGenerator);
  ui.povGenLDViewRadio->setChecked(!Preferences::useNativePovGenerator);
  if (ui.povGenNativeRadio->isChecked()) {
      ui.ldvPOVSettingsBox->setTitle("Native POV file generation settings");
      ui.ldvPoVFileGenOptBtn->setToolTip("Open LDView POV generation dialogue");
      ui.ldvPoVFileGenPrefBtn->setToolTip("Open LDView preferences dialogue");
  } else {
      ui.ldvPoVFileGenOptBtn->setToolTip("Open LDView POV generation dialogue");
      ui.ldvPoVFileGenPrefBtn->setToolTip("Open LDView preferences dialogue");
      ui.ldvPOVSettingsBox->setTitle("LDView POV file generation settings");
  }

  connect(ui.ldvPoVFileGenOptBtn, SIGNAL(clicked()), this, SLOT(ldvPoVFileGenOptBtn_clicked()));
  connect(ui.ldvPoVFileGenPrefBtn, SIGNAL(clicked()), this, SLOT(ldvPoVFileGenPrefBtn_clicked()));

  ui.ldvPreferencesBtn->setEnabled(Preferences::preferredRenderer == RENDERER_LDVIEW);

  /* Themes */
  ui.themeCombo->addItem(THEME_DEFAULT);
  ui.themeCombo->addItem(THEME_DARK);
  ui.themeCombo->setCurrentText(Preferences::displayTheme);

  QPixmap colorPix(12, 12);
  sceneBackgroundColorStr = Preferences::sceneBackgroundColor;
  colorPix.fill(QColor(sceneBackgroundColorStr));
  ui.sceneBackgroundColorButton->setIcon(colorPix);
  connect(ui.sceneBackgroundColorButton, SIGNAL(clicked()), this, SLOT(sceneColorButtonClicked()));

  sceneGridColorStr = Preferences::sceneGridColor;
  colorPix.fill(QColor(sceneGridColorStr));
  ui.sceneGridColorButton->setIcon(colorPix);
  connect(ui.sceneGridColorButton, SIGNAL(clicked()), this, SLOT(sceneColorButtonClicked()));

  sceneRulerTickColorStr = Preferences::sceneRulerTickColor;
  colorPix.fill(QColor(sceneRulerTickColorStr));
  ui.sceneRulerTickColorButton->setIcon(colorPix);
  connect(ui.sceneRulerTickColorButton, SIGNAL(clicked()), this, SLOT(sceneColorButtonClicked()));

  sceneRulerTrackingColorStr = Preferences::sceneRulerTrackingColor;
  colorPix.fill(QColor(sceneRulerTrackingColorStr));
  ui.sceneRulerTrackingColorButton->setIcon(colorPix);
  connect(ui.sceneRulerTrackingColorButton, SIGNAL(clicked()), this, SLOT(sceneColorButtonClicked()));

  sceneGuideColorStr = Preferences::sceneGuideColor;
  colorPix.fill(QColor(sceneGuideColorStr));
  ui.sceneGuideColorButton->setIcon(colorPix);
  connect(ui.sceneGuideColorButton, SIGNAL(clicked()), this, SLOT(sceneColorButtonClicked()));

  /* [Experimental] LDView Image Matting */
  ui.imageMattingChk->setChecked(                Preferences::enableImageMatting);
  ui.imageMattingChk->setEnabled((Preferences::preferredRenderer == RENDERER_LDVIEW) && Preferences::enableFadeSteps);

  QString version = qApp->applicationVersion();
  QString revision = QString::fromLatin1(VER_REVISION_STR);
  QStringList updatableVersions = Preferences::availableVersions.split(",");
#ifdef QT_DEBUG_MODE
  updatableVersions.prepend(version);
#endif
  ui.moduleVersion_Combo->addItems(updatableVersions);
  ui.moduleVersion_Combo->setCurrentIndex(int(ui.moduleVersion_Combo->findText(version)));
  QString versionInfo;
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
  versionInfo = tr("Change Log for version %1 revision %2 (%3)").arg(version, revision, QString::fromLatin1(VER_BUILD_TYPE_STR));
#else
  int revisionNumber = revision.toInt();
  versionInfo = tr("Change Log for version %1%2").arg(version, revisionNumber ? QString(" revision %1").arg(revision) : "");
#endif
  ui.groupBoxChangeLog->setTitle(versionInfo);
  ui.changeLog_txbr->setWordWrapMode(QTextOption::WordWrap);
  ui.changeLog_txbr->setLineWrapMode(QTextEdit::FixedColumnWidth);
  ui.changeLog_txbr->setLineWrapColumnOrWidth(LINE_WRAP_WIDTH);
  ui.changeLog_txbr->setOpenExternalLinks(true);

  /* QSimpleUpdater start */
  m_updaterCancelled = false;
  m_updater = QSimpleUpdater::getInstance();
  connect (m_updater, SIGNAL(checkingFinished (QString)),
           this,        SLOT(updateChangelog  (QString)));

  connect (m_updater, SIGNAL(cancel()),
           this,        SLOT(updaterCancelled()));
  //populate readme from the web
  m_updater->setChangelogOnly(DEFS_URL, true);
  m_updater->checkForUpdates (DEFS_URL);

  // show message options
  mShowLineParseErrors    = Preferences::lineParseErrors;
  mShowInsertErrors       = Preferences::showInsertErrors;
  mShowBuildModErrors     = Preferences::showBuildModErrors;
  mShowIncludeFileErrors  = Preferences::showIncludeFileErrors;
  mShowAnnotationErrors   = Preferences::showAnnotationErrors;

  // LcLib Preferences
  lcQPreferencesInit();

//#ifdef Q_OS_MACOS
//  resize(640, 835);
//#else
//  resize(510, 675);
//#endif
  setMinimumSize(500, 600);
  setSizeGripEnabled(true);
}

PreferencesDialog::~PreferencesDialog()
{}

void PreferencesDialog::setRenderers()
{
    bool ldgliteExists = false;
    bool povRayExists = false;
    bool ldviewExists = false;

    int povRayIndex = -1;
    int ldgliteIndex = -1;
    int ldviewIndex = -1;
    int nativeIndex = -1;

    QFileInfo fileInfo;

    ui.preferredRenderer->setMaxCount(0);
    ui.preferredRenderer->setMaxCount(4);

    if (Preferences::povRayInstalled) {
      fileInfo.setFile(Preferences::povrayExe);
      povRayIndex = ui.preferredRenderer->count();
      povRayExists = fileInfo.exists();
      povRayExists &= fileInfo.exists();
      if (povRayExists) {
          ui.preferredRenderer->addItem(rendererNames[RENDERER_POVRAY]);
        }
    } else {
      connect(ui.povrayInstall, SIGNAL(clicked()), this, SLOT(installRenderer()));
    }

    if (Preferences::ldgliteInstalled) {
      fileInfo.setFile(Preferences::ldgliteExe);
      ldgliteIndex = ui.preferredRenderer->count();
      ldgliteExists = fileInfo.exists();
      if (ldgliteExists) {
        ui.preferredRenderer->addItem(rendererNames[RENDERER_LDGLITE]);
      }
    } else {
      connect(ui.ldgliteInstall, SIGNAL(clicked()), this, SLOT(installRenderer()));
    }

    if (Preferences::ldviewInstalled) {
      fileInfo.setFile(Preferences::ldviewExe);
      ldviewIndex = ui.preferredRenderer->count();
      ldviewExists = fileInfo.exists();
      if (ldviewExists) {
        ui.preferredRenderer->addItem(rendererNames[RENDERER_LDVIEW]);
      }
    } else {
      connect(ui.ldviewInstall, SIGNAL(clicked()), this, SLOT(installRenderer()));
    }

    nativeIndex = ui.preferredRenderer->count();
    ui.preferredRenderer->addItem(rendererNames[RENDERER_NATIVE]);

    if (Preferences::preferredRenderer == RENDERER_LDVIEW && ldviewExists) {
      ui.preferredRenderer->setCurrentIndex(ldviewIndex);
      ui.preferredRenderer->setEnabled(true);
      ui.tabRenderers->setCurrentWidget(ui.LDViewTab);
    } else if (Preferences::preferredRenderer == RENDERER_LDGLITE && ldgliteExists) {
      ui.preferredRenderer->setCurrentIndex(ldgliteIndex);
      ui.preferredRenderer->setEnabled(true);
      ui.tabRenderers->setCurrentWidget(ui.LDGLiteTab);
    }  else if (Preferences::preferredRenderer == RENDERER_POVRAY && povRayExists) {
      ui.preferredRenderer->setCurrentIndex(povRayIndex);
      ui.preferredRenderer->setEnabled(true);
      ui.tabRenderers->setCurrentWidget(ui.POVRayTab);
    } else {
        ui.tabRenderers->setCurrentWidget(ui.NativeTab);
        if (Preferences::preferredRenderer == RENDERER_NATIVE) {
          ui.preferredRenderer->setCurrentIndex(nativeIndex);
          ui.preferredRenderer->setEnabled(true);
        } else {
          ui.preferredRenderer->setEnabled(false);
        }
    }
}

void PreferencesDialog::installRenderer()
{
    ui.RenderMessage->setText("");

    int which = -1;
    if (sender() == ui.ldgliteInstall)
        which = RENDERER_LDGLITE;
    else if (sender() == ui.ldviewInstall)
        which = RENDERER_LDVIEW;
    else if (sender() == ui.povrayInstall)
        which = RENDERER_POVRAY;

    if (gui->installRenderer(which)) {
        setRenderers();
        switch (which)
        {
        case RENDERER_LDVIEW:
            ui.ldviewPath->setText(           Preferences::ldviewExe);
            ui.ldviewBox->setVisible(         Preferences::ldviewInstalled);
            ui.ldviewInstallBox->setVisible( !Preferences::ldviewInstalled);
            break;
        case RENDERER_LDGLITE:
            ui.ldglitePath->setText(          Preferences::ldgliteExe);
            ui.ldgliteBox->setVisible(        Preferences::ldgliteInstalled);
            ui.ldgliteInstallBox->setVisible(!Preferences::ldgliteInstalled);
            break;
        case RENDERER_POVRAY:
            ui.povrayPath->setText(           Preferences::povrayExe);
            ui.povrayBox->setVisible(         Preferences::povRayInstalled);
            ui.povrayInstallBox->setVisible( !Preferences::povRayInstalled);
            break;
        default:
            break;
        }
        ui.RenderMessage->setText(tr("%1 renderer installed successfully.").arg(rendererNames[which]));
    }
}

void PreferencesDialog::sceneColorButtonClicked()
{
    QObject *button = sender();
    QString title;
    QColor oldColor;
    QColorDialog::ColorDialogOptions dialogOptions;

    if (button == ui.sceneBackgroundColorButton)
    {
        oldColor = QColor(sceneBackgroundColorStr);
        title = tr("Select Scene Background Color");
        dialogOptions = nullptr;
    }
    else if (button == ui.sceneGridColorButton)
    {
        oldColor = QColor(sceneGridColorStr);
        title = tr("Select Scene Grid Color");
        dialogOptions = nullptr;
    }
    else if (button == ui.sceneRulerTickColorButton)
    {
        oldColor = QColor(sceneRulerTickColorStr);
        title = tr("Select Scene Ruler Tick Color");
        dialogOptions = nullptr;
    }
    else if (button == ui.sceneRulerTrackingColorButton)
    {
        oldColor = QColor(sceneRulerTrackingColorStr);
        title = tr("Select Ruler Tracking Color");
        dialogOptions = nullptr;
    }
    else if (button == ui.sceneGuideColorButton)
    {
        oldColor = QColor(sceneGuideColorStr);
        title = tr("Select Scene Guide Color");
        dialogOptions = nullptr;
    }
    else
        return;

    QColor newColor = QColorDialog::getColor(oldColor, this, title, dialogOptions);

    if (newColor == oldColor || !newColor.isValid())
        return;

    if (button == ui.sceneBackgroundColorButton)
    {
        sceneBackgroundColorStr = newColor.name();
    }
    else if (button == ui.sceneGridColorButton)
    {
        sceneGridColorStr = newColor.name();
    }
    else if (button == ui.sceneRulerTickColorButton)
    {
        sceneRulerTickColorStr = newColor.name();
    }
    else if (button == ui.sceneRulerTrackingColorButton)
    {
        sceneRulerTrackingColorStr = newColor.name();
    }
    else if (button == ui.sceneGuideColorButton)
    {
        sceneGuideColorStr = newColor.name();
    }

    QPixmap pix(12, 12);

    pix.fill(newColor);
    ((QToolButton*)button)->setIcon(pix);
}

void PreferencesDialog::on_ldrawLibPathEdit_editingFinished()
{
    QString newPath = QDir::toNativeSeparators(ui.ldrawLibPathEdit->text().trimmed().toLower());
    QString oldPath = QDir::toNativeSeparators(mLDrawLibPath.toLower());
    if (newPath != oldPath) {
        QMessageBox box;
        if (!Preferences::checkLDrawLibrary(ui.ldrawLibPathEdit->text())) {
            box.setIcon (QMessageBox::Warning);
            box.setWindowTitle(tr ("Suspicious LDraw Directory!"));
            box.setDefaultButton   (QMessageBox::Ok);
            box.setStandardButtons (QMessageBox::Ok);
            box.setText (tr("The selected path [%1] does not "
                            "appear to be a valid LDraw Library.")
                            .arg(ui.ldrawLibPathEdit->text()));
            box.exec();
        } else {
            if (Preferences::validLDrawLibrary != Preferences::validLDrawLibraryChange) {
                ui.lgeoBox->setEnabled(Preferences::validLDrawLibraryChange == LEGO_LIBRARY);
                QString ldrawTitle = QString("LDraw Library Path for %1® Parts")
                                             .arg(Preferences::validLDrawLibraryChange);
                ui.ldrawLibPathBox->setTitle(ldrawTitle);
                ui.ldrawLibPathBox->setStyleSheet("QGroupBox::title { color : red; }");
            }
        }
    } else {
        ui.ldrawLibPathBox->setTitle(ldrawLibPathTitle);
        ui.ldrawLibPathBox->setStyleSheet("");
    }
}

void PreferencesDialog::on_browseLDraw_clicked()
{
    Preferences::ldrawPreferences(true);
    ui.ldrawLibPathEdit->setText(Preferences::ldrawLibPath);
    if (Preferences::validLDrawLibrary != Preferences::validLDrawLibraryChange) {
        ui.lgeoBox->setEnabled(Preferences::validLDrawLibraryChange == LEGO_LIBRARY);
        QString ldrawTitle = QString("LDraw Library Path for %1® Parts")
                                     .arg(Preferences::validLDrawLibraryChange);
        ui.ldrawLibPathBox->setTitle(ldrawTitle);
        ui.ldrawLibPathBox->setStyleSheet("QGroupBox::title { color : red; }");
    } else {
        ui.ldrawLibPathBox->setTitle(ldrawLibPathTitle);
        ui.ldrawLibPathBox->setStyleSheet("");
    }
}

void PreferencesDialog::on_browseAltLDConfig_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("LDraw Files (*.ldr);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif

    QString result = QFileDialog::getOpenFileName(this, tr("Select LDraw LDConfig file"),
                                                  ui.altLDConfigPath->text().isEmpty() ? Preferences::ldrawLibPath : ui.altLDConfigPath->text(),
                                                  filter);

    if (!result.isEmpty()) {
        result = QDir::toNativeSeparators(result);
        ui.altLDConfigPath->setText(result);
        ui.altLDConfigBox->setChecked(true);
    }
}

void PreferencesDialog::on_browseLGEO_clicked()
{

    QString result = QFileDialog::getExistingDirectory(this, tr("Locate LGEO Directory"),
                                                       ui.lgeoPath->text().isEmpty() ? Preferences::ldrawLibPath : ui.lgeoPath->text(),
                                                       QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!result.isEmpty()) {
            ui.lgeoPath->setText(QDir::toNativeSeparators(result));
            ui.lgeoBox->setChecked(true);
        }
}

void PreferencesDialog::on_browsePli_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("LDraw Files (*.mpd;*.dat;*.ldr);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif
    QString filePath = QFileDialog::getOpenFileName(
                this, tr("Locate Parts List orientation/size file"),
                ui.pliControlEdit->text().isEmpty() ? Preferences::lpubDataPath + "/extras" : ui.pliControlEdit->text(),
                filter);
    if (!filePath.isEmpty()) {
      QString cwd = QDir::currentPath();
      if (filePath.startsWith(cwd))
        filePath = filePath.replace(cwd,".");
      ui.pliControlEdit->setText(QDir::toNativeSeparators(filePath));
      ui.pliControlBox->setChecked(true);
    }
}

void PreferencesDialog::on_browsePublishLogo_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("Image Files (*.png);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif
    QString filePath = QFileDialog::getOpenFileName(
                this, tr("Select Default Document Logo"),
                ui.publishLogoPath->text().isEmpty() ? Preferences::lpubDataPath + "/extras" : ui.publishLogoPath->text(),
                filter);
    if (!filePath.isEmpty()) {
      QString cwd = QDir::currentPath();
      if (filePath.startsWith(cwd))
        filePath = filePath.replace(cwd,".");
      filePath = QDir::toNativeSeparators(filePath);
      ui.publishLogoPath->setText(filePath);
      ui.publishLogoBox->setChecked(true);
    }
}

void PreferencesDialog::on_checkForUpdates_btn_clicked()
{
    checkForUpdates();
}

void PreferencesDialog::on_ldviewSingleCall_Chk_clicked(bool checked)
{
    ui.ldviewSnaphsotsList_Chk->setEnabled(checked);
}

void PreferencesDialog::on_includeAllLogAttribBox_clicked(bool checked)
{
  ui.includeLogLevelBox->setChecked(checked);
  ui.includeTimestampBox->setChecked(checked);
  ui.includeLineNumberBox->setChecked(checked);
  ui.includeFileNameBox->setChecked(checked);
  ui.includeFunctionBox->setChecked(checked);
}

void PreferencesDialog::on_allLogLevelsBox_clicked(bool checked)
{
  ui.debugLevelBox->setChecked(checked);
  ui.traceLevelBox->setChecked(checked);
  ui.noticeLevelBox->setChecked(checked);
  ui.infoLevelBox->setChecked(checked);
  ui.statusLevelBox->setChecked(checked);
  ui.errorLevelBox->setChecked(checked);
  ui.fatalLevelBox->setChecked(checked);
}

void PreferencesDialog::on_logLevelsGrpBox_clicked(bool checked)
{
  ui.logValiationLbl->hide();
  ui.statusLevelBox->setChecked(checked);
  ui.logLevelGrpBox->setChecked(!checked);
}
void PreferencesDialog::on_logLevelGrpBox_clicked(bool checked)
{
  ui.logValiationLbl->hide();
  ui.logLevelsGrpBox->setChecked(!checked);
}

void PreferencesDialog::on_ldviewBox_clicked(bool checked)
{
  if (! checked) {
      QMessageBox box;
      box.setIcon (QMessageBox::Information);
      box.setStandardButtons (QMessageBox::Ok);
      box.setWindowTitle(tr ("LDView Settings?"));
      box.setText (tr("The 'is installed' LDView setting is automatically set at application startup.\n"
                      "Changes will be reset at next application start? "));
      emit gui->messageSig(LOG_STATUS,box.text());
      box.exec();
    }
}

void PreferencesDialog::on_ldgliteBox_clicked(bool checked)
{
    if (! checked) {
        QMessageBox box;
        box.setIcon (QMessageBox::Information);
        box.setStandardButtons (QMessageBox::Ok);
        box.setWindowTitle(tr ("LDGLite Settings?"));
        box.setText (tr("The 'is installed' LDGLite setting is automatically set at application startup.\n"
                        "Changes will be reset at next application start? "));
        emit gui->messageSig(LOG_STATUS,box.text());
        box.exec();
    }
}

void PreferencesDialog::on_povrayBox_clicked(bool checked)
{
  if (! checked) {
      QMessageBox box;
      box.setIcon (QMessageBox::Information);
      box.setStandardButtons (QMessageBox::Ok);
      box.setWindowTitle(tr ("Raytracer (POV-Ray) Settings?"));
      box.setText (tr("The 'is installed' Raytracer (POV-Ray) setting is automatically set at application startup.\n"
                      "Changes will be reset at next application start? "));
      emit gui->messageSig(LOG_STATUS,box.text());
      box.exec();
  }
}

void PreferencesDialog::on_altLDConfigBox_clicked(bool checked)
{
  if (! checked && ! ui.altLDConfigPath->text().isEmpty()) {
    QMessageBox box;
    box.setIcon (QMessageBox::Warning);
    box.setStandardButtons (QMessageBox::Yes|QMessageBox::Cancel);
    box.setWindowTitle(tr ("Alternate LDConfig File"));
    box.setText (tr("This action will remove %1 from your settings.\n"
                    "Are you sure you want to continue? ")
                    .arg(ui.altLDConfigPath->text()));
    emit gui->messageSig(LOG_STATUS,box.text());
    if (box.exec() == QMessageBox::Yes) {
      ui.altLDConfigPath->clear();
      ui.altLDConfigBox->setChecked(false);
    }
  }
}

void PreferencesDialog::on_archiveLSynthPartsBox_clicked(bool checked)
{
    ui.addLSynthSearchDirBox->setEnabled(checked);
}

void PreferencesDialog::on_fadeStepsColoursCombo_currentIndexChanged(const QString &colorName)
{
  QColor newFadeColor = LDrawColor::color(colorName);
  if(newFadeColor.isValid() ) {
      ui.fadeStepsColourLabel->setAutoFillBackground(true);
      QString styleSheet =
          QString("QLabel { background-color: rgb(%1, %2, %3); }")
          .arg(newFadeColor.red())
          .arg(newFadeColor.green())
          .arg(newFadeColor.blue());
      ui.fadeStepsColourLabel->setStyleSheet(styleSheet);
    }
}

void PreferencesDialog::on_highlightStepBtn_clicked()
{
  QColor highlightColour = QColorDialog::getColor(ui.highlightStepColorLabel->palette().background().color(), this );
  if(highlightColour.isValid()) {
    ui.highlightStepColorLabel->setAutoFillBackground(true);
    QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }")
        .arg(highlightColour.red())
        .arg(highlightColour.green())
        .arg(highlightColour.blue());
    ui.highlightStepColorLabel->setStyleSheet(styleSheet);
  }
}

void PreferencesDialog::on_fadeStepBox_clicked(bool checked)
{
  ui.fadeStepsUseColourBox->setEnabled(checked);
  ui.fadeStepsColoursCombo->setEnabled(checked);
  ui.fadeStepsOpacityBox->setEnabled(checked);
  ui.fadeStepsOpacitySlider->setEnabled(checked);

  /* [Experimental] LDView Image Matting */
  ui.imageMattingChk->setEnabled((Preferences::preferredRenderer == RENDERER_LDVIEW) && checked);
}

void PreferencesDialog::on_fadeStepsUseColourBox_clicked(bool checked)
{
  ui.fadeStepsColoursCombo->setEnabled(checked);
}

void PreferencesDialog::on_highlightStepBox_clicked(bool checked)
{
  ui.highlightStepBtn->setEnabled(checked);
  ui.highlightStepLabel->setEnabled(checked);
  // Only enabled for LDGLite
  if (ui.preferredRenderer->currentText() == rendererNames[RENDERER_LDGLITE])
    ui.highlightStepLineWidthSpin->setEnabled(checked);
  else
    ui.highlightStepLineWidthSpin->setEnabled(false);
}

void PreferencesDialog::on_preferredRenderer_currentIndexChanged(const QString &currentText)
{
      bool ldviewEnabled  = (currentText == rendererNames[RENDERER_LDVIEW]);
      bool povrayEnabled  = (currentText == rendererNames[RENDERER_POVRAY]);
      bool ldgliteEnabled = (currentText == rendererNames[RENDERER_LDGLITE]);
      bool nativeEnabled  = (currentText == rendererNames[RENDERER_NATIVE]);
      ui.povNativeGenBox->setEnabled(povrayEnabled);
      ui.ldvPOVSettingsBox->setEnabled(povrayEnabled);
      ui.ldvPreferencesBtn->setEnabled(ldviewEnabled);
      ui.ldvPoVFileGenOptBtn->setEnabled(povrayEnabled);
      ui.ldvPoVFileGenPrefBtn->setEnabled(povrayEnabled);
      if (ui.povGenNativeRadio->isChecked())
          ui.ldvPOVSettingsBox->setTitle("Native POV file generation settings");
      else
      if (ui.povGenLDViewRadio->isChecked())
          ui.ldvPOVSettingsBox->setTitle("LDView POV file generation settings");

      if (ldviewEnabled)
          ui.tabRenderers->setCurrentWidget(ui.LDViewTab);
      else if (povrayEnabled)
          ui.tabRenderers->setCurrentWidget(ui.POVRayTab);
      else if (ldgliteEnabled)
          ui.tabRenderers->setCurrentWidget(ui.LDGLiteTab);
      else if (nativeEnabled)
          ui.tabRenderers->setCurrentWidget(ui.NativeTab);

      bool applyCARenderer = ldviewEnabled && ui.projectionCombo->currentText() == "Perspective";
      ui.applyCALocallyRadio->setChecked(! applyCARenderer);
      ui.applyCARendererRadio->setChecked(applyCARenderer);

      /* [Experimental] LDView Image Matting */
      ui.imageMattingChk->setEnabled(ldviewEnabled && Preferences::enableFadeSteps);
}

void PreferencesDialog::on_projectionCombo_currentIndexChanged(const QString &currentText)
{
    bool applyCARenderer = ui.preferredRenderer->currentText() == rendererNames[RENDERER_LDVIEW] &&
                           currentText == "Perspective";
    ui.applyCALocallyRadio->setChecked(! applyCARenderer);
    ui.applyCARendererRadio->setChecked(applyCARenderer);
}

void PreferencesDialog::on_applyCALocallyRadio_clicked(bool checked)
{
    bool applyCARenderer = ui.preferredRenderer->currentText() == rendererNames[RENDERER_LDVIEW] &&
                           ui.projectionCombo->currentText() == "Perspective";
    if (checked && applyCARenderer) {
      QMessageBox box;
      box.setIcon (QMessageBox::Warning);
      box.setStandardButtons (QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
      box.setWindowTitle(tr ("Perspective Projection"));
      box.setText (tr("The preferred renderer is set to %1 and projection is Perspective.<br>"
                      "This configureaiton requires camera angles to be set by %1.<br>"
                      "Are you sure you want to change the camera angles settings ?")
                      .arg(rendererNames[RENDERER_LDVIEW]));
      box.setDefaultButton(QMessageBox::No);
      if (box.exec() == QMessageBox::No) {
          ui.applyCALocallyRadio->setChecked(!checked);
          ui.applyCARendererRadio->setChecked(checked);
      } else {
          ui.applyCALocallyRadio->setChecked(checked);
          ui.applyCARendererRadio->setChecked(!checked);
      }
    }
}

void PreferencesDialog::on_ldvPreferencesBtn_clicked()
{
    ldvWidget = new LDVWidget(this);
    ldvWidget->showLDVPreferences();
}

void PreferencesDialog::on_povGenNativeRadio_clicked(bool checked)
{
    if (checked) {
        ui.ldvPOVSettingsBox->setTitle("Native POV file generation settings");
        ui.ldvPoVFileGenOptBtn->setToolTip("Open Native POV generation dialogue");
        ui.ldvPoVFileGenPrefBtn->setToolTip("Open Native preferences dialogue");
    }
}

void PreferencesDialog::on_povGenLDViewRadio_clicked(bool checked)
{
    if (checked) {
        ui.ldvPOVSettingsBox->setTitle("LDView POV file generation settings");
        ui.ldvPoVFileGenOptBtn->setToolTip("Open LDView POV generation dialogue");
        ui.ldvPoVFileGenPrefBtn->setToolTip("Open LDView preferences dialogue");
    }
}

void PreferencesDialog::on_saveOnRedrawChkBox_clicked(bool checked)
{
  Q_UNUSED(checked)
  showSaveOnUpdateFlag = true;
}

void PreferencesDialog::on_saveOnUpdateChkBox_clicked(bool checked)
{
  Q_UNUSED(checked)
  showSaveOnUpdateFlag = true;
}

void PreferencesDialog::on_resetSceneColorsButton_clicked(bool checked)
{
    resetSceneColorsFlag = checked;

    if (Preferences::displayTheme == THEME_DARK) {
        sceneBackgroundColorStr    = Preferences::themeColors[THEME_DARK_SCENE_BACKGROUND_COLOR];
        sceneGridColorStr          = Preferences::themeColors[THEME_DARK_GRID_PEN];
        sceneRulerTickColorStr     = Preferences::themeColors[THEME_DARK_RULER_TICK_PEN];
        sceneRulerTrackingColorStr = Preferences::themeColors[THEME_DARK_RULER_TRACK_PEN];
        sceneGuideColorStr         = Preferences::themeColors[THEME_DARK_GUIDE_PEN];
    } else {
        sceneBackgroundColorStr    = Preferences::themeColors[THEME_DEFAULT_SCENE_BACKGROUND_COLOR];
        sceneGridColorStr          = Preferences::themeColors[THEME_DEFAULT_GRID_PEN];
        sceneRulerTickColorStr     = Preferences::themeColors[THEME_DEFAULT_RULER_TICK_PEN];
        sceneRulerTrackingColorStr = Preferences::themeColors[THEME_DEFAULT_RULER_TRACK_PEN];
        sceneGuideColorStr         = Preferences::themeColors[THEME_DEFAULT_GUIDE_PEN];
    }

    QPixmap pix(12, 12);

    pix.fill(QColor(sceneBackgroundColorStr));
    ui.sceneBackgroundColorButton->setIcon(pix);

    pix.fill(QColor(sceneGridColorStr));
    ui.sceneGridColorButton->setIcon(pix);

    pix.fill(QColor(sceneRulerTickColorStr));
    ui.sceneRulerTickColorButton->setIcon(pix);

    pix.fill(QColor(sceneRulerTrackingColorStr));
    ui.sceneRulerTrackingColorButton->setIcon(pix);

    pix.fill(QColor(sceneGuideColorStr));
    ui.sceneGuideColorButton->setIcon(pix);
}

void PreferencesDialog::on_themeColorsButton_clicked()
{
    ThemeColorsDialog *themeColorsDialog = new ThemeColorsDialog();
    editedThemeColors = themeColorsDialog->getEditedThemeColors();
}

void PreferencesDialog::ldvPoVFileGenOptBtn_clicked()
{
    if (ui.povGenNativeRadio->isChecked())
        ldvWidget = new LDVWidget(this,NativePOVIni,true);
    else
        ldvWidget = new LDVWidget(this,LDViewPOVIni,true);

    ldvWidget->showLDVExportOptions();
}

void PreferencesDialog::ldvPoVFileGenPrefBtn_clicked()
{
    if (ui.povGenNativeRadio->isChecked())
        ldvWidget = new LDVWidget(this,NativePOVIni,true);
    else
        ldvWidget = new LDVWidget(this,LDViewPOVIni,true);

    ldvWidget->showLDVPreferences();
}

void PreferencesDialog::on_loggingGrpBox_clicked(bool checked)
{
      ui.logPathEdit->setEnabled(checked);
}

void PreferencesDialog::on_optionsButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    // options dialogue
    messageDialog = new QDialog();
    messageDialog->setWindowTitle("Messages");
    QFormLayout *form = new QFormLayout(messageDialog);

    // options - parse errors
    QGroupBox *parseErrorGrpBox = new QGroupBox("Message Categories");
    form->addWidget(parseErrorGrpBox);
    QGridLayout *parseErrorLayout = new QGridLayout(parseErrorGrpBox);

    QCheckBox* parseErrorChkBox = new QCheckBox("Show model line parse errors", messageDialog);
    parseErrorChkBox->setChecked(Preferences::lineParseErrors);
    parseErrorLayout->addWidget(parseErrorChkBox,0,0,1,2);
    parseErrorTBtn = new QToolButton(messageDialog);
    parseErrorLayout->addWidget(parseErrorTBtn,1,0);
    parseErrorLbl = new QLabel("", messageDialog);
    parseErrorLayout->addWidget(parseErrorLbl,1,1);
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    parseErrorLayout->addWidget(separator,2,0,1,2);
    QObject::connect(parseErrorTBtn, SIGNAL(clicked()), this, SLOT(messageManagement()));

    // options - insert errors
    QCheckBox * insertErrorChkBox = new QCheckBox("Show model insert errors", messageDialog);
    insertErrorChkBox ->setChecked(Preferences::showInsertErrors);
    parseErrorLayout->addWidget(insertErrorChkBox,3,0,1,2);
    insertErrorTBtn = new QToolButton(messageDialog);
    parseErrorLayout->addWidget(insertErrorTBtn,4,0);
    insertErrorLbl = new QLabel("", messageDialog);
    parseErrorLayout->addWidget(insertErrorLbl,4,1);
    separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    parseErrorLayout->addWidget(separator,5,0,1,2);
    QObject::connect(insertErrorTBtn, SIGNAL(clicked()), this, SLOT(messageManagement()));

    // options - build modification errors
    QCheckBox * buildModErrorChkBox = new QCheckBox("Show build modification errors", messageDialog);
    buildModErrorChkBox->setChecked(Preferences::showBuildModErrors);
    parseErrorLayout->addWidget(buildModErrorChkBox,6,0,1,2);
    buildModErrorTBtn = new QToolButton(messageDialog);
    parseErrorLayout->addWidget(buildModErrorTBtn,7,0);
    buildModErrorLbl = new QLabel("", messageDialog);
    parseErrorLayout->addWidget(buildModErrorLbl,7,1);
    separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    parseErrorLayout->addWidget(separator,8,0,1,2);
    QObject::connect(buildModErrorTBtn, SIGNAL(clicked()), this, SLOT(messageManagement()));

    // options - include file errors
    QCheckBox * includeFileErrorChkBox = new QCheckBox("Show include file errors", messageDialog);
    includeFileErrorChkBox->setChecked(Preferences::showIncludeFileErrors);
    parseErrorLayout->addWidget(includeFileErrorChkBox,9,0,1,2);
    includeErrorTBtn = new QToolButton(messageDialog);
    parseErrorLayout->addWidget(includeErrorTBtn,10,0);
    includeErrorLbl = new QLabel("", messageDialog);
    parseErrorLayout->addWidget(includeErrorLbl,10,1);
    separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    parseErrorLayout->addWidget(separator,11,0,1,2);
    QObject::connect(includeErrorTBtn, SIGNAL(clicked()), this, SLOT(messageManagement()));

    // options - annotation errors
    QCheckBox * annotationErrorChkBox = new QCheckBox("Show annotation errors", messageDialog);
    annotationErrorChkBox->setChecked(Preferences::showAnnotationErrors);
    parseErrorLayout->addWidget(annotationErrorChkBox,12,0,1,2);
    annotationErrorTBtn = new QToolButton(messageDialog);
    parseErrorLayout->addWidget(annotationErrorTBtn,13,0);
    annotationErrorLbl = new QLabel("", messageDialog);
    parseErrorLayout->addWidget(annotationErrorLbl,13,1);
    QObject::connect(annotationErrorTBtn, SIGNAL(clicked()), this, SLOT(messageManagement()));
    separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    parseErrorLayout->addWidget(separator,14,0,1,2);

    // options - clear all detail messages
    clearDetailErrorsTBtn = new QToolButton(messageDialog);
    parseErrorLayout->addWidget(clearDetailErrorsTBtn,15,0);
    clearDetailErrorsLbl = new QLabel("", messageDialog);
    parseErrorLayout->addWidget(clearDetailErrorsLbl,15,1);
    QObject::connect(clearDetailErrorsTBtn, SIGNAL(clicked()), this, SLOT(messageManagement()));

    // options - button box
    messageButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                             Qt::Horizontal, messageDialog);
    form->addRow(messageButtonBox);
    QObject::connect(messageButtonBox, SIGNAL(accepted()), messageDialog, SLOT(accept()));
    QObject::connect(messageButtonBox, SIGNAL(rejected()), messageDialog, SLOT(reject()));
    messageDialog->setMinimumSize(250,200);

    // load message counts
    messageManagement();

    if (messageDialog->exec() == QDialog::Accepted) {
        mShowLineParseErrors   = parseErrorChkBox->isChecked();
        mShowInsertErrors      = insertErrorChkBox ->isChecked();
        mShowBuildModErrors    = buildModErrorChkBox->isChecked();
        mShowIncludeFileErrors = includeFileErrorChkBox->isChecked();
        mShowAnnotationErrors  = annotationErrorChkBox->isChecked();
    }
}

void PreferencesDialog::messageManagement()
{

    auto countErrors = [this] ()
    {
        int lineParseErrorCount   = 0;
        int insertErrorCount      = 0;
        int buildModErrorCount    = 0;
        int includeFileErrorCount = 0;
        int annotationErrorCount  = 0;

        if (Preferences::messagesNotShown.size()) {
            bool ok;
            Q_FOREACH (QString message,Preferences::messagesNotShown) {
                int key = QString(message.at(0)).toInt(&ok);
                if (ok) {
                    Preferences::MsgKey msgKey = Preferences::MsgKey(key);
                    switch(msgKey)
                    {
                    case Preferences::ParseErrors:
                        lineParseErrorCount++; break;
                    case Preferences::InsertErrors:
                        insertErrorCount++; break;
                    case Preferences::BuildModErrors:
                        buildModErrorCount++; break;
                    case Preferences::IncludeFileErrors:
                        includeFileErrorCount++; break;
                    case Preferences::AnnotationErrors:
                        annotationErrorCount++; break;
                    default:
                        break;
                    }
                }
            }
        }

        parseErrorLbl->setText(QString("Clear %1 model line parse errors").arg(lineParseErrorCount));
        parseErrorTBtn->setEnabled(lineParseErrorCount);
        buildModErrorLbl->setText(QString("Clear %1 build modification errors").arg(buildModErrorCount));
        buildModErrorTBtn->setEnabled(buildModErrorCount);
        insertErrorLbl->setText(QString("Clear %1 model insert errors").arg(insertErrorCount));
        insertErrorTBtn->setEnabled(insertErrorCount);
        includeErrorLbl->setText(QString("Clear %1 include file errors") .arg(includeFileErrorCount));
        includeErrorTBtn->setEnabled(includeFileErrorCount);
        annotationErrorLbl->setText(QString("Clear %1 annotation errors").arg(annotationErrorCount));
        annotationErrorTBtn->setEnabled(annotationErrorCount);
    };

    auto clearErrors = [](Preferences::MsgKey msgKey)
    {
        int counter = 0;
        if (msgKey == Preferences::NumKeys) {
            counter = Preferences::messagesNotShown.size();
            Preferences::messagesNotShown.clear();
            return counter;
        }
        Preferences::messagesNotShown.erase(std::remove_if(
            Preferences::messagesNotShown.begin(), Preferences::messagesNotShown.end(),
            [&counter, &msgKey](const QString &message) {
                bool ok;
                int keyInt = QString(message.at(0)).toInt(&ok);
                if (ok) {
                    counter++;
                    return msgKey == Preferences::MsgKey(keyInt);
                }
                return false;
            }), Preferences::messagesNotShown.end());
        return counter;
    };

    int cleared = 0;
    QString clearDetailText;
    const QString style = QString("QLabel { color : %1; }")
            .arg(Preferences::displayTheme == THEME_DARK ? "#ff9999" : "#ff0000");
    if (sender() == parseErrorTBtn) {
        cleared = clearErrors(Preferences::ParseErrors);
        parseErrorLbl->setStyleSheet(style);
        parseErrorLbl->setText(QString("Cleared %1 model line parse errors").arg(cleared));
    } else if (sender() == insertErrorTBtn) {
        cleared = clearErrors(Preferences::InsertErrors);
        insertErrorLbl->setStyleSheet(style);
        insertErrorLbl->setText(QString("Cleared %1 model insert errors").arg(cleared));
    } else if (sender() == buildModErrorTBtn) {
        cleared = clearErrors(Preferences::BuildModErrors);
        buildModErrorLbl->setStyleSheet(style);
        buildModErrorLbl->setText(QString("Cleared %1 build modification errorss").arg(cleared));
    } else if (sender() == includeErrorTBtn) {
        cleared = clearErrors(Preferences::IncludeFileErrors);
        includeErrorLbl->setStyleSheet(style);
        includeErrorLbl->setText(QString("Cleared %1 include file error").arg(cleared));
    } else if (sender() == annotationErrorTBtn) {
        cleared = clearErrors(Preferences::AnnotationErrors);
        annotationErrorLbl->setStyleSheet(style);
        annotationErrorLbl->setText(QString("Cleared %1 annotation errors").arg(cleared));
    } else if (sender() == clearDetailErrorsTBtn) {
        cleared = clearErrors(Preferences::NumKeys);
        clearDetailErrorsLbl->setStyleSheet(style);
        clearDetailText = QString("Cleared");
        countErrors();
    } else {
        countErrors();
        clearDetailText = QString("Clear");
    }
    clearDetailErrorsLbl->setText(QString("%1 all %2 errors")
                                  .arg(clearDetailText)
                                  .arg(sender() == clearDetailErrorsTBtn ? cleared :
                                                                           Preferences::messagesNotShown.size()));
    clearDetailErrorsTBtn->setEnabled(Preferences::messagesNotShown.size());
    messageButtonBox->button(QDialogButtonBox::Cancel)->setEnabled(!cleared);
}

QString const PreferencesDialog::ldrawLibPath()
{
  return ui.ldrawLibPathEdit->displayText();
}

QString const PreferencesDialog::altLDConfigPath()
{
  if (ui.altLDConfigBox->isChecked()) {
    return ui.altLDConfigPath->displayText();
  }
  return "";
}

QString const PreferencesDialog::lgeoPath()
{
    if (ui.povrayBox->isChecked() && ui.lgeoBox->isChecked()){
        return ui.lgeoPath->displayText();
    }
    return "";
}

QString const PreferencesDialog::pliControlFile()
{
  if (ui.pliControlBox->isChecked()) {
    return ui.pliControlEdit->displayText();
  }
  return "";
}

QString const PreferencesDialog::ldviewExe()
{
  if (ui.ldviewBox->isChecked()) {
    return ui.ldviewPath->displayText();
  }
  return "";
}

QString const PreferencesDialog::ldgliteExe()
{
  if (ui.ldgliteBox->isChecked()) {
    return ui.ldglitePath->displayText();
  }
    return "";
}

QString const PreferencesDialog::povrayExe()
{
    if (ui.povrayBox->isChecked()) {
        return ui.povrayPath->displayText();
    }
    return "";
}

int PreferencesDialog::preferredRenderer()
{
  if (ui.preferredRenderer->isEnabled()) {
    return rendererMap[ui.preferredRenderer->currentText()];
  }
  return RENDERER_INVALID;
}

bool PreferencesDialog::useNativePovGenerator()
{
    return ui.povGenNativeRadio->isChecked();
}

bool PreferencesDialog::perspectiveProjection()
{
   return ui.projectionCombo->currentIndex() == 0 ? true : false;
}

bool PreferencesDialog::saveOnRedraw()
{
       return ui.saveOnRedrawChkBox->isChecked();
}

bool PreferencesDialog::saveOnUpdate()
{
       return ui.saveOnUpdateChkBox->isChecked();
}

bool PreferencesDialog::povrayDisplay()
{
    return ui.povrayDisplay_Chk->isChecked();
}

int PreferencesDialog::povrayRenderQuality()
{
    return ui.povrayRenderQualityCombo->currentIndex();
}

bool PreferencesDialog::povrayAutoCrop()
{
    return ui.povrayAutoCropBox->isChecked();
}

bool PreferencesDialog::loadLastOpenedFile()
{
    return ui.loadLastOpenedFileCheck->isChecked();
}

bool PreferencesDialog::extendedSubfileSearch()
{
    return ui.extendedSubfileSearchCheck->isChecked();
}

int PreferencesDialog::ldrawFilesLoadMsgs()
{
    return ui.ldrawFilesLoadMsgsCombo->currentIndex();
}

QString const PreferencesDialog::fadeStepsColour()
{
    return ui.fadeStepsColoursCombo->currentText();
}

bool PreferencesDialog::showSaveOnRedraw()
{
 return showSaveOnRedrawFlag;
}

bool PreferencesDialog::showSaveOnUpdate()
{
 return showSaveOnUpdateFlag;
}

QString const PreferencesDialog::highlightStepColour()
{
    return ui.highlightStepColorLabel->palette().background().color().name();
}

QString const PreferencesDialog::documentLogoFile()
{
    if (ui.publishLogoBox->isChecked()){
        return ui.publishLogoPath->displayText();
    }
    return QString();
}

int PreferencesDialog::fadeStepsOpacity()
{
  return ui.fadeStepsOpacitySlider->value();
}

int PreferencesDialog::highlightStepLineWidth()
{
  return ui.highlightStepLineWidthSpin->value();
}

bool PreferencesDialog::highlightFirstStep()
{
       return ui.highlightFirstStepBox->isChecked();
}

bool PreferencesDialog::enableImageMatting()
{
       return ui.imageMattingChk->isChecked();
}

bool PreferencesDialog::centimeters()
{
  return ui.Centimeters->isChecked();
}

bool PreferencesDialog::applyCALocally()
{
  return ui.applyCALocallyRadio->isChecked();
}

bool  PreferencesDialog::enableFadeSteps()
{
  return ui.fadeStepBox->isChecked();
}

bool  PreferencesDialog::enableHighlightStep()
{
  return ui.highlightStepBox->isChecked();
}

bool PreferencesDialog::enableDocumentLogo()
{
  return ui.publishLogoBox->isChecked();
}

bool PreferencesDialog::fadeStepsUseColour()
{
    return ui.fadeStepsUseColourBox->isChecked();
}

bool PreferencesDialog::enableLDViewSingleCall()
{
  return ui.ldviewSingleCall_Chk->isChecked();
}

bool PreferencesDialog::enableLDViewSnaphsotList()
{
       return ui.ldviewSnaphsotsList_Chk->isChecked();
}

bool  PreferencesDialog::displayAllAttributes()
{
  return ui.displayAllAttributes_Chk->isChecked();
}

bool  PreferencesDialog::generateCoverPages()
{
  return ui.generateCoverPages_Chk->isChecked();
}

bool  PreferencesDialog::printDocumentTOC()
{
  return ui.publishTOC_Chk->isChecked();
}

bool  PreferencesDialog::doNotShowPageProcessDlg()
{
  return ui.doNotShowPageProcessDlgChk->isChecked();
}

QString const PreferencesDialog::displayTheme()
{
  return ui.themeCombo->currentText();
}

QMap<int, QString> const PreferencesDialog::themeColours()
{
    return editedThemeColors;
}

QString const PreferencesDialog::sceneBackgroundColor()
{
  return sceneBackgroundColorStr;
}

QString const PreferencesDialog::sceneGridColor()
{
  return sceneGridColorStr;
}

QString const PreferencesDialog::sceneRulerTickColor()
{
  return sceneRulerTickColorStr;
}

QString const PreferencesDialog::sceneRulerTrackingColor()
{
  return sceneRulerTrackingColorStr;
}

QString const PreferencesDialog::sceneGuideColor()
{
  return sceneGuideColorStr;
}

bool PreferencesDialog::resetSceneColors()
{
  return resetSceneColorsFlag;
}

QString const PreferencesDialog::defaultURL()
{   
  return ui.publishURL_Edit->displayText();
}

QString const PreferencesDialog::defaultEmail()
{
  return ui.publishEmail_Edit->displayText();
}

QString const PreferencesDialog::defaultAuthor()
{
  return ui.authorName_Edit->displayText();
}

QString const PreferencesDialog::publishDescription()
{
  return ui.publishDescriptionEdit->toPlainText();
}

bool PreferencesDialog::addLSynthSearchDir()
{
  return ui.addLSynthSearchDirBox->isChecked();
}

bool PreferencesDialog::archiveLSynthParts()
{
  return ui.archiveLSynthPartsBox->isChecked();
}

bool PreferencesDialog::showUpdateNotifications()
{
  return ui.showUpdateNotifications_Chk->isChecked();
}

bool PreferencesDialog::enableDownloader()
{
  return ui.enableDownloader_Chk->isChecked();
}

bool PreferencesDialog::showDownloadRedirects()
{
  return ui.showDownloadRedirects_Chk->isChecked();
}

bool PreferencesDialog::showAllNotifications()
{
  return ui.showAllNotificstions_Chk->isChecked();
}

int PreferencesDialog::checkUpdateFrequency()
{
  return ui.checkUpdateFrequency_Combo->currentIndex();
}

int PreferencesDialog::rendererTimeout()
{
  return ui.rendererTimeout->value();
}

int PreferencesDialog::pageDisplayPause()
{
  return ui.pageDisplayPauseSpin->value();
}

bool PreferencesDialog::showLineParseErrors()
{
  return mShowLineParseErrors;
}

bool PreferencesDialog::showInsertErrors()
{
  return mShowInsertErrors ;
}

bool PreferencesDialog::showBuildModErrors()
{
  return mShowBuildModErrors;
}

bool PreferencesDialog::showIncludeFileErrors()
{
  return mShowIncludeFileErrors;
}

bool PreferencesDialog::inlineNativeContent()
{
  QSettings Settings;
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"InlineNativeContent")))
    return Preferences::inlineNativeContent;
  return Settings.value(QString("%1/%2").arg(SETTINGS,"InlineNativeContent")).toBool();

  // return ui.inlineNativeContentBox->isChecked(); // future implementation
}

bool PreferencesDialog::showAnnotationErrors()
{
  return mShowAnnotationErrors;
}

bool PreferencesDialog::includeLogLevel()
{
  return ui.includeLogLevelBox->isChecked();
}
bool PreferencesDialog::includeTimestamp()
{
  return ui.includeTimestampBox->isChecked();
}
bool PreferencesDialog::includeLineNumber()
{
  return ui.includeLineNumberBox->isChecked();
}
bool PreferencesDialog::includeFileName()
{
  return ui.includeFileNameBox->isChecked();
}
bool PreferencesDialog::includeFunction()
{
  return ui.includeFunctionBox->isChecked();
}
bool PreferencesDialog::includeAllLogAttrib()
{
  return ui.includeAllLogAttribBox->isChecked();
}
bool PreferencesDialog::loggingGrpBox()
{
  return ui.loggingGrpBox->isChecked(); //off-level
}
bool PreferencesDialog::logLevelGrpBox()
{
  return ui.logLevelGrpBox->isChecked();
}
const QString PreferencesDialog::logLevelCombo()
{
  return ui.logLevelCombo->currentText();
}
bool PreferencesDialog::logLevelsGrpBox()
{
  return ui.logLevelsGrpBox->isChecked();
}
bool PreferencesDialog::debugLevel()
{
  return ui.debugLevelBox->isChecked();
}
bool PreferencesDialog::traceLevel()
{
  return ui.traceLevelBox->isChecked();
}
bool PreferencesDialog::noticeLevel()
{
  return ui.noticeLevelBox->isChecked();
}
bool PreferencesDialog::infoLevel()
{
  return ui.infoLevelBox->isChecked();
}
bool PreferencesDialog::statusLevel()
{
  return ui.statusLevelBox->isChecked();
}
bool PreferencesDialog::errorLevel()
{
  return ui.errorLevelBox->isChecked();
}
bool PreferencesDialog::fatalLevel()
{
  return ui.fatalLevelBox->isChecked();
}
bool PreferencesDialog::allLogLevels()
{
  return ui.allLogLevelsBox->isChecked();
}

void PreferencesDialog::updateChangelog (QString url) {
    bool processing = true;
    auto processRequest = [this, &url, &processing] ()
    {
        if (m_updater->getUpdateAvailable(url) || m_updater->getChangelogOnly(url)) {
            QString versionInfo = ui.groupBoxChangeLog->title();
            if (!m_updaterCancelled) {
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
#ifdef QT_DEBUG_MODE
                versionInfo = tr("Change Log for version %1 revision %2 (%3)")
                                 .arg(qApp->applicationVersion(), QString::fromLatin1(VER_REVISION_STR), QString::fromLatin1(VER_BUILD_TYPE_STR));
#else
                versionInfo = tr("Change Log for version %1 revision %2 (%3)")
                                 .arg(m_updater->getLatestVersion(url), m_updater->getLatestRevision(DEFS_URL), QString::fromLatin1(VER_BUILD_TYPE_STR));
#endif
#else
                int revisionNumber = m_updater->getLatestRevision(DEFS_URL).toInt();
                versionInfo = tr("Change Log for version %1%2")
                                 .arg(m_updater->getLatestVersion(url), revisionNumber ? QString(" revision %1").arg(m_updater->getLatestRevision(DEFS_URL)) : "");
#endif
                if (m_updater->compareVersionStr(url, m_updater->getLatestVersion(url), PLAINTEXT_CHANGE_LOG_CUTOFF_VERSION))
                    ui.changeLog_txbr->setHtml(m_updater->getChangelog (url));
                else
                    ui.changeLog_txbr->setText(m_updater->getChangelog (url));
            } else {
                ui.changeLog_txbr->setHtml("");
            }
            ui.groupBoxChangeLog->setTitle(versionInfo);
        }
        processing = false;
    };

    if (url == DEFS_URL) {
        processRequest();
        while (processing)
            QApplication::processEvents();
    }
}

QString const PreferencesDialog::moduleVersion()
{
   return ui.moduleVersion_Combo->currentText();
}

void PreferencesDialog::updaterCancelled()
{
  m_updaterCancelled = true;
}

void PreferencesDialog::checkForUpdates () {
    bool processing = true;
    auto processRequest = [this, &processing] ()
    {
        const QString htmlNotes = ui.changeLog_txbr->toHtml();

        ui.changeLog_txbr->setHtml("<p><span style=\"color: #0000ff;\">Updating change log, please wait...</span></p>");

        /* Get settings from the UI */
        QString moduleVersion = ui.moduleVersion_Combo->currentText();
        QString moduleRevision = QString::fromLatin1(VER_REVISION_STR);
        bool showRedirects = ui.showDownloadRedirects_Chk->isChecked();
        bool enableDownloader = ui.enableDownloader_Chk->isChecked();
        bool showAllNotifications = ui.showAllNotificstions_Chk->isChecked();
        bool showUpdateNotifications = ui.showUpdateNotifications_Chk->isChecked();

        /* Apply the settings */
        if (m_updater->getModuleVersion(DEFS_URL) != moduleVersion)
            m_updater->setModuleVersion(DEFS_URL, moduleVersion);
        if (m_updater->getModuleRevision(DEFS_URL) != moduleRevision)
            m_updater->setModuleRevision(DEFS_URL, moduleRevision);
        m_updater->setShowRedirects(DEFS_URL, showRedirects);
        m_updater->setDownloaderEnabled(DEFS_URL, enableDownloader);
        m_updater->setNotifyOnFinish(DEFS_URL, showAllNotifications);
        m_updater->setNotifyOnUpdate (DEFS_URL, showUpdateNotifications);

        /* Check for updates */
        m_updater->checkForUpdates (DEFS_URL);

        if (!m_updaterCancelled) {
            QSettings Settings;
            Settings.setValue(QString("%1/%2").arg(UPDATES,"LastCheck"), QDateTime::currentDateTimeUtc());
        }

        if (ui.changeLog_txbr->document()->isEmpty())
                   ui.changeLog_txbr->setHtml(htmlNotes);

        processing = false;
    };

    processRequest();
    while (processing)
        QApplication::processEvents();
}

void PreferencesDialog::accept(){
    bool missingParms = false;
    QFileInfo fileInfo;

    if (resetSceneColorsFlag)
        emit gui->messageSig(LOG_INFO,QString("Scene Colors have been reset."));

    if (!ui.povrayPath->text().isEmpty() && (ui.povrayPath->text() != Preferences::povrayExe)){
        fileInfo.setFile(ui.povrayPath->text());
        bool povRayExists = fileInfo.exists();
        if (povRayExists) {
            Preferences::povrayExe = ui.povrayPath->text();
            ui.preferredRenderer->addItem(rendererNames[RENDERER_POVRAY]);
        } else {
            emit gui->messageSig(LOG_ERROR,QString("POV-Ray path entered is not valid: %1").arg(ui.povrayPath->text()));
        }
    }
    if (!ui.ldglitePath->text().isEmpty() && (ui.ldglitePath->text() != Preferences::ldgliteExe)) {
        fileInfo.setFile(ui.ldglitePath->text());
        bool ldgliteExists = fileInfo.exists();
        if (ldgliteExists) {
            Preferences::ldgliteExe = ui.ldglitePath->text();
            ui.preferredRenderer->addItem(rendererNames[RENDERER_LDGLITE]);
        } else {
            emit gui->messageSig(LOG_ERROR,QString("LDGLite path entered is not valid: %1").arg(ui.ldglitePath->text()));
        }
    }
    if (!ui.ldviewPath->text().isEmpty() && (ui.ldviewPath->text() != Preferences::ldviewExe)) {
        QString ldviewPath = ui.ldviewPath->text();
#ifndef Q_OS_LINUX
        fileInfo.setFile(ldviewPath);
#else
        // force use command line-only "ldview" (not RENDERER_LDVIEW) if not using Windows
        QFileInfo info(ldviewPath);
        fileInfo.setFile(QString("%1/%2").arg(info.absolutePath()).arg(info.fileName().toLower()));
#endif
        bool ldviewExists = fileInfo.exists();
        if (ldviewExists) {
            Preferences::ldviewExe = ldviewPath;
            ui.preferredRenderer->addItem(rendererNames[RENDERER_LDVIEW]);
        } else {
            emit gui->messageSig(LOG_ERROR,QString("LDView path entered is not valid: %1").arg(ui.ldviewPath->text()));
        }
    }

    // LcLib Preferences
    lcQPreferencesAccept();

    if(ui.preferredRenderer->count() == 0 || ui.ldrawLibPathEdit->text().isEmpty()){
        missingParms = true;
        if (ui.preferredRenderer->count() == 0){
            ui.ldglitePath->setPlaceholderText("At lease one renderer must be defined");
            ui.ldviewPath->setPlaceholderText("At lease one renderer must be defined");
            ui.povrayPath->setPlaceholderText("At lease one renderer must be defined");
            ui.ldrawLibPathEdit->setPlaceholderText("LDraw path must be defined");
        }
      }
    if (ui.includesGrpBox->isChecked() &&
        ! ui.includeLogLevelBox->isChecked() &&
        ! ui.includeTimestampBox->isChecked() &&
        ! ui.includeLineNumberBox->isChecked() &&
        ! ui.includeFileNameBox->isChecked() &&
        ! ui.includeFunctionBox->isChecked()){
        missingParms = true;
        ui.logValiationLbl->show();
        ui.logValiationLbl->setText("At lease one attribute must be included.");
        ui.logValiationLbl->setStyleSheet("QLabel { background-color : red; color : white; }");
      }
    if (ui.logLevelsGrpBox->isChecked() &&
        ! ui.fatalLevelBox->isChecked() &&
        ! ui.errorLevelBox->isChecked() &&
        ! ui.statusLevelBox->isChecked() &&
        ! ui.infoLevelBox->isChecked() &&
        ! ui.noticeLevelBox->isChecked() &&
        ! ui.traceLevelBox->isChecked() &&
        ! ui.debugLevelBox->isChecked()){
        missingParms = true;
        ui.logValiationLbl->show();
        ui.logValiationLbl->setText("At lease one logging level must be selected.");
        ui.logValiationLbl->setStyleSheet("QLabel { background-color : red; color : white; }");
      }
    if (missingParms){
        if (QMessageBox::Yes == QMessageBox::question(this, "Close Dialogue?",
                              "Required settings are missing.\n Are you sure you want to exit?",
                              QMessageBox::Yes|QMessageBox::No)){
            QDialog::reject(); //keep open
        }
    } else {
        QDialog::accept();
    }
}

void PreferencesDialog::cancel(){
  QDialog::reject();
}

/***********************************************************************
 *
 * Theme Colors
 *
 **********************************************************************/

QMap<int, QString> ThemeColorsDialog::getEditedThemeColors()
{
    dialog = new QDialog(nullptr);
    dialog->setWindowTitle(tr("Edit Theme Colors"));

    // Default Theme Tab
    tabs = new QTabWidget(dialog);
    tabs->setMovable(false);
    QVBoxLayout *layout = new QVBoxLayout();

    dialog->setLayout(layout);
    layout->addWidget(tabs);

    QWidget *container = new QWidget(tabs);
    QGridLayout *gridLayout = new QGridLayout(container);
    gridLayout->setSizeConstraint(QLayout::SetFixedSize);
    container->setLayout(gridLayout);

    QScrollArea *scrollArea = new QScrollArea(tabs);
    scrollArea->setWidget(container);

    auto setControls = [this, &gridLayout, &container] (const int begin, const int end)
    {
        for (int i = begin; i < end; i++) {
            mThemeColors[i] = Preferences::themeColors[i];
            // description label
            QLabel *label = new QLabel(Preferences::defaultThemeColors[i].label, container);
            gridLayout->addWidget(label,i,0);

            // color button
            QToolButton *colorButton = new QToolButton(container);
            QPixmap pix(12, 12);
            pix.fill(getColor(i, mThemeColors[i]));
            colorButton->setIcon(pix);
            colorButton->setProperty("index", i);
            colorButton->setProperty("color", mThemeColors[i]);
            colorButton->setToolTip("Edit Color");
            gridLayout->addWidget(colorButton,i,1);
            connect(colorButton, SIGNAL(clicked()), this, SLOT(setThemeColor()));
            colorButtonList << colorButton;

            // reset button
            QToolButton *resetButton = new QToolButton(container);
            resetButton->setText(tr("..."));
            resetButton->setProperty("index", i);
            QString toolTipText = QString("Reset");
            if (mThemeColors[i] == Preferences::defaultThemeColors[i].color) {
                resetButton->setEnabled(false);
            } else {
                // prepare color icon
                QImage image(16, 16, QImage::Format_RGB888);
                image.fill(getColor(i, Preferences::defaultThemeColors[i].color));
                QPainter painter(&image);
                painter.setPen(Qt::darkGray);
                painter.drawRect(0, 0, image.width() - 1, image.height() - 1);
                painter.end();
                QByteArray ba;
                QBuffer buffer(&ba);
                buffer.open(QIODevice::WriteOnly);
                image.save(&buffer, "PNG");
                buffer.close();
                const char*  format = "<table><tr><td>Reset To %1</td><td style=\"vertical-align:middle\"><img src=\"data:image/png;base64,%2\"/></td></tr></table>";
                toolTipText = QString(format).arg(Preferences::defaultThemeColors[i].color.toUpper(), QString(buffer.data().toBase64()));
            }
            resetButton->setToolTip(toolTipText);
            gridLayout->addWidget(resetButton,i,2);
            connect(resetButton, SIGNAL(clicked()), this, SLOT(resetThemeColor()));
            resetButtonList << resetButton;
        }
    };

    setControls(THEME_DEFAULT_SCENE_BACKGROUND_COLOR, THEME_DARK_SCENE_BACKGROUND_COLOR);

    scrollArea->setMinimumWidth(
                container->sizeHint().width() +
                2 * scrollArea->frameWidth() +
                scrollArea->verticalScrollBar()->sizeHint().width());

    tabs->addTab(scrollArea,"Default Theme");

    // Dark Theme Tab
    container = new QWidget(tabs);
    gridLayout = new QGridLayout(container);
    gridLayout->setSizeConstraint(QLayout::SetFixedSize);
    container->setLayout(gridLayout);

    scrollArea = new QScrollArea(tabs);
    scrollArea->setWidget(container);

    setControls(THEME_DARK_SCENE_BACKGROUND_COLOR, THEME_NUM_COLORS);

    scrollArea->setMinimumWidth(
                container->sizeHint().width() +
                2 * scrollArea->frameWidth() +
                scrollArea->verticalScrollBar()->sizeHint().width());

    tabs->addTab(scrollArea,"Dark Theme");

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, dialog);
    dialog->layout()->addWidget(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

    QPushButton *button = new QPushButton(tr("Reset"));
    button->setToolTip(tr("Reset theme colors to defaults"));
    buttonBox.addButton(button,QDialogButtonBox::ActionRole);
    QObject::connect(button,SIGNAL(clicked()), this, SLOT(resetThemeColors()));

    button = new QPushButton(tr("Defaults"));
    button->setToolTip(tr("Toggle 'Defaults' tab"));
    buttonBox.addButton(button,QDialogButtonBox::ActionRole);
    QObject::connect(button,SIGNAL(clicked()), this, SLOT(toggleDefaultsTab()));

    bool themeDark = Preferences::displayTheme == THEME_DARK ;

    QMap<int, QString> editedColors;

    dialog->setMinimumSize(200,400);

    tabs->setCurrentIndex(themeDark ? 1 : 0);

    if (dialog->exec() == QDialog::Accepted) {
        for (int i = 0; i < THEME_NUM_COLORS; i++) {
            if (mThemeColors[i] != Preferences::themeColors[i]) {
                editedColors.insert(i, mThemeColors[i].toUpper());
                LC_PROFILE_KEY key = LC_PROFILE_KEY(-1);
                switch (i)
                {
                case THEME_DARK_VIEWER_BACKGROUND_COLOR:
                case THEME_DEFAULT_VIEWER_BACKGROUND_COLOR:
                    key = LC_PROFILE_BACKGROUND_COLOR;
                    break;
                case THEME_DARK_VIEWER_GRADIENT_COLOR_TOP:
                case THEME_DEFAULT_VIEWER_GRADIENT_COLOR_TOP:
                    key = LC_PROFILE_GRADIENT_COLOR_TOP;
                    break;
                case THEME_DARK_VIEWER_GRADIENT_COLOR_BOTTOM:
                case THEME_DEFAULT_VIEWER_GRADIENT_COLOR_BOTTOM:
                    key = LC_PROFILE_GRADIENT_COLOR_BOTTOM;
                    break;
                case THEME_DARK_AXES_COLOR:
                case THEME_DEFAULT_AXES_COLOR:
                    key = LC_PROFILE_AXES_COLOR;
                    break;
                case THEME_DARK_OVERLAY_COLOR:
                case THEME_DEFAULT_OVERLAY_COLOR:
                    key = LC_PROFILE_OVERLAY_COLOR;
                    break;
                case THEME_DARK_MARQUEE_BORDER_COLOR:
                case THEME_DEFAULT_MARQUEE_BORDER_COLOR:
                    key = LC_PROFILE_MARQUEE_BORDER_COLOR;
                    break;
                case THEME_DARK_MARQUEE_FILL_COLOR:
                case THEME_DEFAULT_MARQUEE_FILL_COLOR:
                    key = LC_PROFILE_MARQUEE_FILL_COLOR;
                    break;
                case THEME_DARK_INACTIVE_VIEW_COLOR:
                case THEME_DEFAULT_INACTIVE_VIEW_COLOR:
                    key = LC_PROFILE_INACTIVE_VIEW_COLOR;
                    break;
                case THEME_DARK_ACTIVE_VIEW_COLOR:
                case THEME_DEFAULT_ACTIVE_VIEW_COLOR:
                    key = LC_PROFILE_ACTIVE_VIEW_COLOR;
                    break;
                case THEME_DARK_GRID_STUD_COLOR:
                case THEME_DEFAULT_GRID_STUD_COLOR:
                    key = LC_PROFILE_GRID_STUD_COLOR;
                    break;
                case THEME_DARK_GRID_LINE_COLOR:
                case THEME_DEFAULT_GRID_LINE_COLOR:
                    key = LC_PROFILE_GRID_LINE_COLOR;
                    break;
                case THEME_DARK_TEXT_COLOR:
                case THEME_DEFAULT_TEXT_COLOR:
                    key = LC_PROFILE_TEXT_COLOR;
                    break;
                case THEME_DARK_VIEW_SPHERE_COLOR:
                case THEME_DEFAULT_VIEW_SPHERE_COLOR:
                    key = LC_PROFILE_VIEW_SPHERE_COLOR;
                    break;
                case THEME_DARK_VIEW_SPHERE_TEXT_COLOR:
                case THEME_DEFAULT_VIEW_SPHERE_TEXT_COLOR:
                    key = LC_PROFILE_VIEW_SPHERE_TEXT_COLOR;
                    break;
                case THEME_DARK_VIEW_SPHERE_HLIGHT_COLOR:
                case THEME_DEFAULT_VIEW_SPHERE_HLIGHT_COLOR:
                    key = LC_PROFILE_VIEW_SPHERE_HIGHLIGHT_COLOR;
                    break;
                case THEME_DARK_OBJECT_SELECTED_COLOR:
                case THEME_DEFAULT_OBJECT_SELECTED_COLOR:
                    key = LC_PROFILE_OBJECT_SELECTED_COLOR;
                    break;
                case THEME_DARK_OBJECT_FOCUSED_COLOR:
                case THEME_DEFAULT_OBJECT_FOCUSED_COLOR:
                    key = LC_PROFILE_OBJECT_FOCUSED_COLOR;
                    break;
                case THEME_DARK_CAMERA_COLOR:
                case THEME_DEFAULT_CAMERA_COLOR:
                    key = LC_PROFILE_CAMERA_COLOR;
                    break;
                case THEME_DARK_LIGHT_COLOR:
                case THEME_DEFAULT_LIGHT_COLOR:
                    key = LC_PROFILE_LIGHT_COLOR;
                    break;
                case THEME_DARK_CONTROL_POINT_COLOR:
                case THEME_DEFAULT_CONTROL_POINT_COLOR:
                    key = LC_PROFILE_CONTROL_POINT_COLOR;
                    break;
                case THEME_DARK_CONTROL_POINT_FOCUSED_COLOR:
                case THEME_DEFAULT_CONTROL_POINT_FOCUSED_COLOR:
                    key = LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR;
                    break;
                case THEME_DARK_BM_OBJECT_SELECTED_COLOR:
                case THEME_DEFAULT_BM_OBJECT_SELECTED_COLOR:
                    key = LC_PROFILE_BM_OBJECT_SELECTED_COLOR;
                    break;
                default:
                    break;
                }

                if (key > LC_PROFILE_KEY(-1)) {
                    int alpha = 255;
                    switch (key)
                    {
                    case LC_PROFILE_GRID_STUD_COLOR:
                        alpha = 192;
                        break;
                    case LC_PROFILE_MARQUEE_FILL_COLOR:
                        alpha = 64;
                        break;
                    case LC_PROFILE_CONTROL_POINT_COLOR:
                    case LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR:
                        alpha = 128;
                    default:
                        break;
                    }

                    QColor tc     = QColor(mThemeColors[i]);
                    quint32 color = qint32(LC_RGBA(tc.red(), tc.green(), tc.blue(), alpha));
                    switch (key)
                    {
                    case LC_PROFILE_OBJECT_SELECTED_COLOR:
                    case LC_PROFILE_OBJECT_FOCUSED_COLOR:
                    case LC_PROFILE_CAMERA_COLOR:
                    case LC_PROFILE_LIGHT_COLOR:
                    case LC_PROFILE_CONTROL_POINT_COLOR:
                    case LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR:
                    case LC_PROFILE_BM_OBJECT_SELECTED_COLOR:
                        lcSetProfileUInt(key, color);
                        break;
                    default:
                        lcSetProfileInt(key, color);
                    }

#ifdef QT_DEBUG_MODE
                    emit gui->messageSig(LOG_DEBUG, QString("Display theme color set LC_PROFILE_KEY(%1): %2").arg(key).arg(color));
#endif
                }
            }
        }
    }

    return editedColors;
}

void ThemeColorsDialog::resetThemeColors()
{
    if (QMessageBox::Yes == QMessageBox::question(dialog, tr("Reset Theme Colors ?"),
                          "Reset all Theme colors to defaults.<br>Are you sure ?",
                          QMessageBox::Yes|QMessageBox::No)) {
        for (int i = 0; i < THEME_NUM_COLORS; i++) {
            if (mThemeColors[i] == Preferences::defaultThemeColors[i].color)
                continue;
            else
                mThemeColors[i] = Preferences::defaultThemeColors[i].color;
            QPixmap pix(12, 12);
            pix.fill(getColor(i, mThemeColors[i]));
            colorButtonList[i]->setIcon(pix);
        }
    }
}

void ThemeColorsDialog::resetThemeColor()
{
    QObject* button = sender();
    int index = button->property("index").toInt();
    if (mThemeColors[index] == Preferences::themeColors[index])
       return;

    mThemeColors[index] = Preferences::themeColors[index];

    QPixmap pix(12, 12);
    pix.fill(getColor(index, (mThemeColors[index])));
    colorButtonList[index]->setIcon(pix);

    if (mThemeColors[index] == Preferences::defaultThemeColors[index].color) {
        resetButtonList[index]->setToolTip(tr("Reset"));
        resetButtonList[index]->setEnabled(false);
    } else {
        QImage image(16, 16, QImage::Format_RGB888);
        image.fill(getColor(index, Preferences::defaultThemeColors[index].color));
        QPainter painter(&image);
        painter.setPen(Qt::darkGray);
        painter.drawRect(0, 0, image.width() - 1, image.height() - 1);
        painter.end();
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        buffer.close();
        const char* format = "<table><tr><td>Reset To %1 </td><td style=\"vertical-align:middle\"><img src=\"data:image/png;base64,%2\"/></td></tr></table>";
        QString text = QString(format).arg(Preferences::defaultThemeColors[index].color.toUpper(), QString(buffer.data().toBase64()));
        resetButtonList[index]->setToolTip(text);
        resetButtonList[index]->setEnabled(true);
    }
}

void ThemeColorsDialog::setThemeColor()
{
    QObject* button = sender();
    QString title = tr("Select Theme Color");

    int index = button->property("index").toInt();

    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
    QColor oldColor  = getColor(index, button->property("color").toString());
    QColor newColor  = QColorDialog::getColor(oldColor, dialog, title, dialogOptions);

    if (newColor.isValid())
        newColor = getColor(index, newColor.name());
    else
        return;

    if (newColor == oldColor)
        return;

    mThemeColors[index] = newColor.name().toUpper();

    QPixmap pix(12, 12);
    pix.fill(newColor);
    ((QToolButton*)button)->setIcon(pix);

    QImage image(16, 16, QImage::Format_RGB888);
    image.fill(oldColor);
    QPainter painter(&image);
    painter.setPen(Qt::darkGray);
    painter.drawRect(0, 0, image.width() - 1, image.height() - 1);
    painter.end();
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    buffer.close();
    const char* format = "<table><tr><td>Reset To %1</td><td style=\"vertical-align:middle\"><img src=\"data:image/png;base64,%2\"/></td></tr></table>";
    QString text = QString(format).arg(mThemeColors[index].toUpper(), QString(buffer.data().toBase64()));
    resetButtonList[index]->setToolTip(text);
    resetButtonList[index]->setEnabled(true);
}

QColor ThemeColorsDialog::getColor(const int index, const QString &name)
{
    // treat obaque colours
    QColor color(name);
    switch (index)
    {
    case THEME_DARK_GRID_STUD_COLOR:
    case THEME_DEFAULT_GRID_STUD_COLOR:
        color.setAlpha(192);
        break;
    case THEME_DEFAULT_LINE_SELECT:
        color.setAlpha(30);
        break;
    case THEME_DARK_LINE_HIGHLIGHT_EDITOR_SELECT:
    case THEME_DEFAULT_LINE_HIGHLIGHT_EDITOR_SELECT:
    case THEME_DARK_LINE_HIGHLIGHT_VIEWER_SELECT:
    case THEME_DEFAULT_LINE_HIGHLIGHT_VIEWER_SELECT:
        color = color.lighter(180);
        break;
    case THEME_DARK_MARQUEE_FILL_COLOR:
    case THEME_DEFAULT_MARQUEE_FILL_COLOR:
        color.setAlpha(64);
        break;
    case THEME_DARK_CONTROL_POINT_COLOR:
    case THEME_DEFAULT_CONTROL_POINT_COLOR:
    case THEME_DARK_CONTROL_POINT_FOCUSED_COLOR:
    case THEME_DEFAULT_CONTROL_POINT_FOCUSED_COLOR:
        color.setAlpha(128);
    default:
        break;
    }
    if (Preferences::displayTheme == THEME_DARK && index == THEME_DARK_LINE_ERROR) {
        color = color.lighter(180);
    }
    return color;
}

void ThemeColorsDialog::toggleDefaultsTab()
{
    if (tabs->count() < 3) {

        QWidget *container = new QWidget(tabs);
        QGridLayout *gridLayout = new QGridLayout(container);
        gridLayout->setSizeConstraint(QLayout::SetFixedSize);
        container->setLayout(gridLayout);

        QScrollArea *scrollArea = new QScrollArea(tabs);
        scrollArea->setWidget(container);

        QPalette readOnlyPalette = QApplication::palette();
        if (Preferences::displayTheme == THEME_DARK)
            readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));
        else
            readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]));
        readOnlyPalette.setColor(QPalette::Text,QColor(LPUB3D_DISABLED_TEXT_COLOUR));
        for (int i = THEME_DEFAULT_SCENE_BACKGROUND_COLOR; i < THEME_NUM_COLORS; i++) {
            QString labelText = Preferences::defaultThemeColors[i].label;
            if (i < THEME_DARK_SCENE_BACKGROUND_COLOR)
                labelText.prepend("Default ");
            else
                labelText.prepend("Dark ");
            // description label
            QLabel *label = new QLabel(labelText, container);
            gridLayout->addWidget(label,i,0);

            // default color label
            label = new QLabel(container);
            label->setFixedWidth(16);
            label->setFrameStyle(QFrame::Sunken|QFrame::Panel);
            QColor color = getColor(i, Preferences::defaultThemeColors[i].color);
            QString styleSheet =
                    QString("QLabel { background-color: %1; }")
                    .arg(!color.isValid() ? "transparent" :
                         QString("rgba(%1, %2, %3, %4)")
                         .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()));
            label->setAutoFillBackground(true);
            label->setStyleSheet(styleSheet);
            gridLayout->addWidget(label,i,1);

            // default color hex (readOnly
            QLineEdit *readOnlyColor = new QLineEdit(container);
            readOnlyColor->setPalette(readOnlyPalette);
            readOnlyColor->setReadOnly(true);
            readOnlyColor->setText(Preferences::defaultThemeColors[i].color.toUpper());
            gridLayout->addWidget(readOnlyColor,i,2);
        }

        scrollArea->setMinimumWidth(
                    container->sizeHint().width() +
                    2 * scrollArea->frameWidth() +
                    scrollArea->verticalScrollBar()->sizeHint().width());

        tabs->addTab(scrollArea,"Defaults");
        tabs->setCurrentIndex(2);
    } else {
        tabs->removeTab(2);
        tabs->setCurrentIndex(Preferences::displayTheme == THEME_DARK ? 1 : 0);
    }
    dialog->adjustSize();
}
