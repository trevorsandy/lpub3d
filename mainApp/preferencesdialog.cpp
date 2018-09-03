/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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
#include "lc_application.h"
#include "updatecheck.h"

#include <LDVQt/LDVWidget.h>

#include "color.h"
#include "meta.h"
#include "lpub.h"
#include "version.h"

//QString PreferencesDialog::DEFS_URL = QString(VER_UPDATE_CHECK_JSON_URL).arg(qApp->applicationVersion());
QString PreferencesDialog::DEFS_URL = VER_UPDATE_CHECK_JSON_URL;

PreferencesDialog::PreferencesDialog(QWidget *_parent) :
    QDialog(_parent)
{
  ui.setupUi(this);

  QString ldrawPath = Preferences::ldrawPath;

  if (ldrawPath.isEmpty()) {
    ldrawPath = ".";
  }

  // hide 3rd party applicaton browse buttons
  ui.browseLDGLite->hide();
  ui.browseLDView->hide();
  ui.browsePOVRAY->hide();

  QPalette readOnlyPalette;
  readOnlyPalette.setColor(QPalette::Base,Qt::lightGray);

  // set 3rd party application dialogs to read-only
  ui.ldglitePath->setReadOnly(true);
  ui.ldglitePath->setPalette(readOnlyPalette);
  ui.ldviewPath->setReadOnly(true);
  ui.ldviewPath->setPalette(readOnlyPalette);
  ui.povrayPath->setReadOnly(true);
  ui.povrayPath->setPalette(readOnlyPalette);

  // set log path to read only
  ui.logPathEdit->setReadOnly(true);
  ui.logPathEdit->setPalette(readOnlyPalette);

  ui.ldrawPath->setText(                         ldrawPath);
  ui.pliName->setText(                           Preferences::pliFile);
  ui.altLDConfigPath->setText(                   Preferences::altLDConfigPath);
  ui.altLDConfigBox->setChecked(                 Preferences::altLDConfigPath != "");
  ui.pliBox->setChecked(                         Preferences::pliFile != "");
  ui.ldglitePath->setText(                       Preferences::ldgliteExe);
  ui.ldgliteBox->setChecked(                     Preferences::ldgliteExe != "");
  ui.povrayPath->setText(                        Preferences::povrayExe);
  ui.POVRayBox->setChecked(                      Preferences::povrayExe != "");
  ui.lgeoPath->setText(                          Preferences::lgeoPath);
  ui.lgeoBox->setChecked(                        Preferences::lgeoPath != "");
  ui.lgeoStlLibLbl->setText(                     Preferences::lgeoStlLib ? DURAT_LGEO_STL_LIB_INFO : "");
  ui.ldviewPath->setText(                        Preferences::ldviewExe);
  ui.ldviewBox->setChecked(                      Preferences::ldviewExe != "");
  ui.ldviewSingleCall_Chk->setChecked(           Preferences::enableLDViewSingleCall && Preferences::ldviewExe != "");
  ui.publishLogoBox->setChecked(                 Preferences::documentLogoFile != "");
  ui.publishLogoPath->setText(                   Preferences::documentLogoFile);
  ui.authorName_Edit->setText(                   Preferences::defaultAuthor);
  ui.displayAllAttributes_Chk->setChecked(       Preferences::displayAllAttributes);
  ui.generateCoverPages_Chk->setChecked(         Preferences::generateCoverPages);
  ui.publishTOC_Chk->setChecked(                 Preferences::printDocumentTOC);
  ui.doNotShowPageProcessDlgChk->setChecked(     Preferences::doNotShowPageProcessDlg);
  ui.publishURL_Edit->setText(                   Preferences::defaultURL);
  ui.publishEmail_Edit->setText(                 Preferences::defaultEmail);
  ui.publishDescriptionEdit->setText(            Preferences::publishDescription);
  ui.enableDownloader_Chk->setChecked(           Preferences::enableDownloader);
  ui.showUpdateNotifications_Chk->setChecked(    Preferences::showUpdateNotifications);
  ui.showAllNotificstions_Chk->setChecked(       Preferences::showAllNotifications);
  ui.checkUpdateFrequency_Combo->setCurrentIndex(Preferences::checkUpdateFrequency);
  ui.rendererTimeout->setValue(                  Preferences::rendererTimeout);
  ui.pageDisplayPauseSpin->setValue(             Preferences::pageDisplayPause);
  ui.povrayDisplay_Chk->setChecked(              Preferences::povrayDisplay);

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
  ui.fadeStepsColoursCombo->setCurrentIndex(int(ui.fadeStepsColoursCombo->findText(Preferences::fadeStepsColour)));
  QColor fadeColor = LDrawColor::color(Preferences::fadeStepsColour);
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
  if (Preferences::ldrawiniFound) {
      ui.lineEditIniFile->setText(QString("Using LDraw.ini File: %1").arg(Preferences::ldrawiniFile));
      ui.pushButtonReset->hide();
      ui.textEditSearchDirs->setReadOnly(true);
      ui.textEditSearchDirs->setPalette(readOnlyPalette);
      ui.textEditSearchDirs->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
      ui.textEditSearchDirs->setToolTip("Read only list of LDraw.ini search directories.");
  } else {
      ui.textEditSearchDirs->setToolTip("Editable list of search directories - add or edit search paths. Use a new line for each entry.");
      ui.textEditSearchDirs->setStatusTip("Added directories must be under the Unofficial directory.");
      ui.lineEditIniFile->setText(tr("%1")
                                  .arg(Preferences::ldSearchDirs.size() == 0 ?
                                         tr("Using default search. No search directories detected.") :
                                         tr("Using default %1 search.").arg(VER_PRODUCTNAME_STR)));
      ui.pushButtonReset->setEnabled(Preferences::ldSearchDirs.size() > 0);
  }

  if (Preferences::ldSearchDirs.size() > 0){
      foreach (QString searchDir, Preferences::ldSearchDirs)
        ui.textEditSearchDirs->append(searchDir);
  }

  connect(ui.textEditSearchDirs, SIGNAL(textChanged()),this, SLOT(pushButtonReset_SetState()));
  //end search dirs

  ui.preferredRenderer->setMaxCount(0);
  ui.preferredRenderer->setMaxCount(4);

  QFileInfo fileInfo(Preferences::povrayExe);
  int povRayIndex = ui.preferredRenderer->count();
  bool povRayExists = fileInfo.exists();
  povRayExists &= fileInfo.exists();
  if (povRayExists) {
      ui.preferredRenderer->addItem(RENDERER_POVRAY);
    }

  fileInfo.setFile(Preferences::ldgliteExe);
  int ldgliteIndex = ui.preferredRenderer->count();
  bool ldgliteExists = fileInfo.exists();
  if (ldgliteExists) {
    ui.preferredRenderer->addItem(RENDERER_LDGLITE);
  }

  fileInfo.setFile(Preferences::ldviewExe);
  int ldviewIndex = ui.preferredRenderer->count();
  bool ldviewExists = fileInfo.exists();
  if (ldviewExists) {
    ui.preferredRenderer->addItem(RENDERER_LDVIEW);
  }

  int nativeIndex = ui.preferredRenderer->count();
  ui.preferredRenderer->addItem(RENDERER_NATIVE);

  if (Preferences::preferredRenderer == RENDERER_LDVIEW && ldviewExists) {
    ui.preferredRenderer->setCurrentIndex(ldviewIndex);
    ui.preferredRenderer->setEnabled(true);
  } else if (Preferences::preferredRenderer == RENDERER_LDGLITE && ldgliteExists) {
    ui.preferredRenderer->setCurrentIndex(ldgliteIndex);
    ui.preferredRenderer->setEnabled(true);
  }  else if (Preferences::preferredRenderer == RENDERER_POVRAY && povRayExists) {
    ui.preferredRenderer->setCurrentIndex(povRayIndex);
    ui.preferredRenderer->setEnabled(true);
  } else if (Preferences::preferredRenderer == RENDERER_NATIVE) {
    ui.preferredRenderer->setCurrentIndex(nativeIndex);
    ui.preferredRenderer->setEnabled(true);
  } else {
    ui.preferredRenderer->setEnabled(false);
  }

  ui.tabWidget->setCurrentIndex(0);

//  if(!ldviewExists && !ldgliteExists && !povRayExists){
//      ui.tabWidget->setCurrentIndex(1);
//      ui.RenderMessage->setText("<font color='red'>Renderer not set!</font>");
//  } else {
//      ui.tabWidget->setCurrentIndex(0);
//  }

  bool centimeters = Preferences::preferCentimeters;
  ui.Centimeters->setChecked(centimeters);
  ui.Inches->setChecked(! centimeters);

  bool nativePovFileGen = Preferences::povFileGenerator  == RENDERER_NATIVE;
  bool renderPOVRay     = Preferences::preferredRenderer == RENDERER_POVRAY;
  ui.povNativeGenBox->setEnabled(renderPOVRay);
  ui.ldvPoVFileGenOptBtn->setEnabled(renderPOVRay);
  ui.ldvPoVFileGenPrefBtn->setEnabled(renderPOVRay);
  ui.povGenNativeRadio->setChecked(nativePovFileGen);
  ui.povGenLDViewRadio->setChecked(!nativePovFileGen);
  if (ui.povGenNativeRadio->isChecked())
      ui.ldvPOVSettingsBox->setTitle("Native POV file generation settings");
  else
  if (ui.povGenLDViewRadio->isChecked())
      ui.ldvPOVSettingsBox->setTitle("LDView POV file generation settings");

  ui.ldvPreferencesBtn->setEnabled(Preferences::preferredRenderer == RENDERER_LDVIEW);

  /* Themes */
  ui.themeCombo->addItem(THEME_DEFAULT);
  ui.themeCombo->addItem(THEME_DARK);
  ui.themeCombo->setCurrentText(Preferences::displayTheme);
  ui.themeAutoRestartBox->setChecked(Preferences::themeAutoRestart);

  /* [Experimental] LDView Image Matting */
  ui.imageMattingChk->setChecked(                Preferences::enableImageMatting);
  ui.imageMattingChk->setEnabled((Preferences::preferredRenderer == RENDERER_LDVIEW) && Preferences::enableFadeSteps);

  /* QSimpleUpdater start */
  m_updater = QSimpleUpdater::getInstance();

  connect (m_updater, SIGNAL (checkingFinished (QString)),
           this,        SLOT (updateChangelog  (QString)));

  QString version = qApp->applicationVersion();
  //QStringList updatableVersions = tr(VER_UPDATEABLE_VERSIONS_STR).split(",");
  QStringList updatableVersions = Preferences::availableVersions.split(",");
  ui.moduleVersion_Combo->addItems(updatableVersions);
  ui.moduleVersion_Combo->setCurrentIndex(int(ui.moduleVersion_Combo->findText(version)));

  ui.groupBoxChangeLog->setTitle(tr("Change Log for version %1").arg(version));
  QString readme = tr("%1/%2/%3").arg(Preferences::lpub3dPath).arg(Preferences::lpub3dDocsResourcePath).arg("README.txt");
  QFile file(readme);
  if (! file.open(QFile::ReadOnly | QFile::Text)){
      ui.changeLog_txbr->setText(tr("Failed to open %1\n%2").arg(readme,file.errorString()));
  } else {
      QTextStream in(&file);
      while (! in.atEnd()){
          ui.changeLog_txbr->append(in.readLine(0));
      }
  }
  /* QSimpleUpdater end */

  /* temperory turn off check for updates on mac/linux
  #ifndef Q_OS_WIN
    ui.groupBoxUpdates->setEnabled(false);
  #endif
  */

}

PreferencesDialog::~PreferencesDialog()
{}

void PreferencesDialog::on_browseLDraw_clicked()
{
  Preferences::ldrawPreferences(true);
  ui.ldrawPath->setText(Preferences::ldrawPath);
}

void PreferencesDialog::on_browseAltLDConfig_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("LDraw (*.ldr);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif

    QString result = QFileDialog::getOpenFileName(this, tr("Select LDRaw LDConfig file"),
                                                  ui.altLDConfigPath->text().isEmpty() ? Preferences::ldrawPath : ui.altLDConfigPath->text(),
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
                                                       ui.lgeoPath->text().isEmpty() ? Preferences::ldrawPath : ui.lgeoPath->text(),
                                                       QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!result.isEmpty()) {
            ui.lgeoPath->setText(QDir::toNativeSeparators(result));
            ui.lgeoBox->setChecked(true);
        }
}

void PreferencesDialog::on_browsePli_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("LDraw (*.mpd;*.dat;*.ldr);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif

    QString result = QFileDialog::getOpenFileName(this, tr("Locate Parts List orientation/size file"),
                                                  ui.pliName->text().isEmpty() ? Preferences::lpubDataPath + "/extras" : ui.pliName->text(),
                                                  filter);

    if (!result.isEmpty()) {
      ui.pliName->setText(QDir::toNativeSeparators(result));
      ui.pliBox->setChecked(true);
    }
}

void PreferencesDialog::on_browsePublishLogo_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("Logo (*.png);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif

    QString result = QFileDialog::getOpenFileName(this, tr("Select Document Logo"),
                                                  ui.publishLogoPath->text().isEmpty() ? Preferences::lpubDataPath + "/extras" : ui.publishLogoPath->text(),
                                                  filter);

    if (!result.isEmpty()) {
      result = QDir::toNativeSeparators(result);
      ui.publishLogoPath->setText(result);
      ui.publishLogoBox->setChecked(true);
    }
}

void PreferencesDialog::on_pushButtonReset_clicked()
{
  QMessageBox box;
  box.setIcon (QMessageBox::Question);
  box.setWindowTitle(tr ("Reset Search Directories?"));
  box.setDefaultButton   (QMessageBox::Yes);
  box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
  box.setText (tr("This action will reset your search directory settings to the LPub3D default.\n"
                  "Are you sure you want to continue? "));

  if (box.exec() == QMessageBox::Yes) {

      // get enable fade step setting
      Preferences::enableFadeSteps = ui.fadeStepBox->isChecked();
      // get enable highlight step setting
      Preferences::enableHighlightStep = ui.highlightStepBox->isChecked();
      gui->partWorkerLDSearchDirs.resetSearchDirSettings();
      ui.textEditSearchDirs->clear();
      foreach (QString searchDir, Preferences::ldSearchDirs)
        ui.textEditSearchDirs->append(searchDir);

      box.setIcon (QMessageBox::Information);
      box.setStandardButtons (QMessageBox::Ok);
      box.setText( tr("Search directories have been reset with %1 entries.").arg(Preferences::ldSearchDirs.size()));
      emit gui->messageSig(LOG_STATUS,box.text());
      box.exec();
    }
}

void PreferencesDialog::on_checkForUpdates_btn_clicked()
{
    checkForUpdates();
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
      box.setText (tr("LDView renderer settings are automatically set at application startup.\n"
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
        box.setText (tr("LDGLite renderer settings are automatically set at application startup.\n"
                        "Changes will be reset at next application start? "));
        emit gui->messageSig(LOG_STATUS,box.text());
        box.exec();
    }
}

void PreferencesDialog::on_POVRayBox_clicked(bool checked)
{
  if (! checked) {
      QMessageBox box;
      box.setIcon (QMessageBox::Information);
      box.setStandardButtons (QMessageBox::Ok);
      box.setWindowTitle(tr ("Raytracer (POV-Ray) Settings?"));
      box.setText (tr("Raytracer (POV-Ray) renderer settings are automatically set at application startup.\n"
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
  if( highlightColour.isValid()) {
      if(highlightColour.isValid() ) {
          ui.highlightStepColorLabel->setAutoFillBackground(true);
          QString styleSheet =
              QString("QLabel { background-color: rgb(%1, %2, %3); }")
              .arg(highlightColour.red())
              .arg(highlightColour.green())
              .arg(highlightColour.blue());
          ui.highlightStepColorLabel->setStyleSheet(styleSheet);
        }
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
  if (ui.preferredRenderer->currentText() == RENDERER_LDGLITE)
    ui.highlightStepLineWidthSpin->setEnabled(checked);
  else
    ui.highlightStepLineWidthSpin->setEnabled(false);
}

void PreferencesDialog::on_preferredRenderer_currentIndexChanged(const QString &currentText)
{
      bool ldviewEnabled = (currentText == RENDERER_LDVIEW);
      bool povrayEnabled = (currentText == RENDERER_POVRAY);
      ui.povNativeGenBox->setEnabled(povrayEnabled);
      ui.ldvPreferencesBtn->setEnabled(ldviewEnabled);
      ui.ldvPoVFileGenOptBtn->setEnabled(povrayEnabled);
      ui.ldvPoVFileGenPrefBtn->setEnabled(povrayEnabled);
      if (ui.povGenNativeRadio->isChecked())
          ui.ldvPOVSettingsBox->setTitle("Native POV file generation settings");
      else
      if (ui.povGenLDViewRadio->isChecked())
          ui.ldvPOVSettingsBox->setTitle("LDView POV file generation settings");

      /* [Experimental] LDView Image Matting */
      ui.imageMattingChk->setEnabled(ldviewEnabled && Preferences::enableFadeSteps);
}

void PreferencesDialog::on_povGenNativeRadio_clicked(bool checked)
{
    if (checked)
        ui.ldvPOVSettingsBox->setTitle("Native POV file generation settings");
}

void PreferencesDialog::on_povGenLDViewRadio_clicked(bool checked)
{
    if (checked)
        ui.ldvPOVSettingsBox->setTitle("LDView POV file generation settings");
}

void PreferencesDialog::on_ldvPreferencesBtn_clicked()
{
    ldvWidget = new LDVWidget(this);
    ldvWidget->setIniFlag(LDViewIni);
    ldvWidget->showLDVPreferences();
}

void PreferencesDialog::on_ldvPoVFileGenOptBtn_clicked()
{
     ldvWidget = new LDVWidget(this);
     if (ui.povGenNativeRadio->isChecked())
         ldvWidget->setIniFlag(NativePOVIni);
     else if (ui.povGenLDViewRadio->isChecked())
         ldvWidget->setIniFlag(LDViewPOVIni);
     ldvWidget->showLDVExportOptions();
}

void PreferencesDialog::on_ldvPoVFileGenPrefBtn_clicked()
{
     ldvWidget = new LDVWidget(this);
     if (ui.povGenNativeRadio->isChecked())
         ldvWidget->setIniFlag(NativePOVIni);
     else if (ui.povGenLDViewRadio->isChecked())
         ldvWidget->setIniFlag(LDViewPOVIni);
     ldvWidget->showLDVPreferences();
}

void PreferencesDialog::on_loggingGrpBox_clicked(bool checked)
{
      ui.logPathEdit->setEnabled(checked);
}

void PreferencesDialog::pushButtonReset_SetState()
{
  ui.pushButtonReset->setEnabled(true);
}

QString const PreferencesDialog::ldrawPath()
{
  return ui.ldrawPath->displayText();
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
    if (ui.POVRayBox->isChecked() && ui.lgeoBox->isChecked()){
        return ui.lgeoPath->displayText();
    }
    return "";
}

QString const PreferencesDialog::pliFile()
{
  if (ui.pliBox->isChecked()) {
    return ui.pliName->displayText();
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
    if (ui.POVRayBox->isChecked()) {
        return ui.povrayPath->displayText();
    }
    return "";
}

QString const PreferencesDialog::preferredRenderer()
{
  if (ui.preferredRenderer->isEnabled()) {
    return ui.preferredRenderer->currentText();
  }
  return "";
}

QString const PreferencesDialog::povFileGenerator()
{
    if (ui.povGenLDViewRadio->isChecked())
      return RENDERER_LDVIEW;
    else
      return RENDERER_NATIVE;
}

bool PreferencesDialog::povrayDisplay()
{
    return ui.povrayDisplay_Chk->isChecked();
}

QString const PreferencesDialog::fadeStepsColour()
{
    return ui.fadeStepsColoursCombo->currentText();
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
    return "";
}

int PreferencesDialog::fadeStepsOpacity()
{
  return ui.fadeStepsOpacitySlider->value();
}

int PreferencesDialog::highlightStepLineWidth()
{
  return ui.highlightStepLineWidthSpin->value();
}

bool PreferencesDialog::enableImageMatting()
{
       return ui.imageMattingChk->isChecked();
}

bool PreferencesDialog::centimeters()
{
  return ui.Centimeters->isChecked();
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

bool PreferencesDialog::themeAutoRestart()
{
  return ui.themeAutoRestartBox->isChecked();
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

bool PreferencesDialog::showUpdateNotifications()
{
  return ui.showUpdateNotifications_Chk->isChecked();
}

bool PreferencesDialog::enableDownloader()
{
  return ui.enableDownloader_Chk->isChecked();
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
  return ui.loggingGrpBox->isChecked(); //offlevel
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

QStringList const PreferencesDialog::searchDirSettings()
{
    QString textEditContents = ui.textEditSearchDirs->toPlainText();
    QStringList newContent;
    if (! textEditContents.isEmpty()) {
        newContent = textEditContents.split("\n");
    }
    return newContent;
}

void PreferencesDialog::updateChangelog (QString url) {
    if (url == DEFS_URL) {
        if (m_updater->getUpdateAvailable(url)) {
            ui.groupBoxChangeLog->setTitle(tr("Change Log for version %1").arg(m_updater->getLatestVersion(url)));
            ui.changeLog_txbr->setText (m_updater->getChangelog (url));
        }
    }
}

QString const PreferencesDialog::moduleVersion()
{
   return ui.moduleVersion_Combo->currentText();
}

void PreferencesDialog::checkForUpdates () {
  /* Get settings from the UI */
  QString moduleVersion = ui.moduleVersion_Combo->currentText();
  bool enableDownloader = ui.enableDownloader_Chk->isChecked();
  bool showAllNotifications = ui.showAllNotificstions_Chk->isChecked();
  bool showUpdateNotifications = ui.showUpdateNotifications_Chk->isChecked();

  /* Apply the settings */
  if (m_updater->getModuleVersion(DEFS_URL) != moduleVersion)
    m_updater->setModuleVersion(DEFS_URL, moduleVersion);
  m_updater->setDownloaderEnabled(DEFS_URL, enableDownloader);
  m_updater->setNotifyOnFinish(DEFS_URL, showAllNotifications);
  m_updater->setNotifyOnUpdate (DEFS_URL, showUpdateNotifications);

  /* Check for updates */
  m_updater->checkForUpdates (DEFS_URL);

  QSettings Settings;
  Settings.setValue("Updates/LastCheck", QDateTime::currentDateTimeUtc());
}

void PreferencesDialog::accept(){
    bool missingParms = false;
    QFileInfo fileInfo;

    if (!ui.povrayPath->text().isEmpty() && (ui.povrayPath->text() != Preferences::povrayExe)){
        fileInfo.setFile(ui.povrayPath->text());
        bool povRayExists = fileInfo.exists();
        if (povRayExists) {
            Preferences::povrayExe = ui.povrayPath->text();
            ui.preferredRenderer->addItem(RENDERER_POVRAY);
        } else {
            emit gui->messageSig(LOG_ERROR,QString("POV-Ray path entered is not valid: %1").arg(ui.povrayPath->text()));
        }
    }
    if (!ui.ldglitePath->text().isEmpty() && (ui.ldglitePath->text() != Preferences::ldgliteExe)) {
        fileInfo.setFile(ui.ldglitePath->text());
        bool ldgliteExists = fileInfo.exists();
        if (ldgliteExists) {
            Preferences::ldgliteExe = ui.ldglitePath->text();
            ui.preferredRenderer->addItem(RENDERER_LDGLITE);
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
            ui.preferredRenderer->addItem(RENDERER_LDVIEW);
        } else {
            emit gui->messageSig(LOG_ERROR,QString("LDView path entered is not valid: %1").arg(ui.ldviewPath->text()));
        }
    }
    if(ui.preferredRenderer->count() == 0 || ui.ldrawPath->text().isEmpty()){
        missingParms = true;
        if (ui.preferredRenderer->count() == 0){
            ui.ldglitePath->setPlaceholderText("At lease one renderer must be defined");
            ui.ldviewPath->setPlaceholderText("At lease one renderer must be defined");
            ui.povrayPath->setPlaceholderText("At lease one renderer must be defined");
            ui.ldrawPath->setPlaceholderText("LDRaw path must be defined");
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
        ui.logValiationLbl->setStyleSheet("QLabel { background-color : red; color : blue; }");
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
        ui.logValiationLbl->setStyleSheet("QLabel { background-color : red; color : blue; }");
      }
    if (missingParms){
        if (QMessageBox::Yes == QMessageBox::question(this, "Close Dialog?",
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
