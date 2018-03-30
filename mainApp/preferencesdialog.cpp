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
#include "lc_application.h"
#include "updatecheck.h"

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

  QPalette readOnlyPalette;
  readOnlyPalette.setColor(QPalette::Base,Qt::lightGray);

  // hide 3rd party applicaton browse buttons
  ui.browseLDGLite->hide();
  ui.browseLDView->hide();
  ui.browsePOVRAY->hide();

  // set 3rd party application dialogs to read-only
  ui.ldglitePath->setReadOnly(true);
  ui.ldglitePath->setPalette(readOnlyPalette);
  ui.ldviewPath->setReadOnly(true);
  ui.ldviewPath->setPalette(readOnlyPalette);
  ui.povrayPath->setReadOnly(true);
  ui.povrayPath->setPalette(readOnlyPalette);

  ui.ldrawPath->setText(                            ldrawPath);
  ui.pliName->setText(                              Preferences::pliFile);
  ui.altLDConfigPath->setText(                      Preferences::altLDConfigPath);
  ui.altLDConfigBox->setChecked(                    Preferences::altLDConfigPath != "");
  ui.pliBox->setChecked(                            Preferences::pliFile != "");
  ui.ldglitePath->setText(                          Preferences::ldgliteExe);
  ui.ldgliteBox->setChecked(                        Preferences::ldgliteExe != "");
  ui.povrayPath->setText(                           Preferences::povrayExe);
  ui.POVRayBox->setChecked(                         Preferences::povrayExe != "");
  ui.lgeoPath->setText(                             Preferences::lgeoPath);
  ui.lgeoBox->setChecked(                           Preferences::lgeoPath != "");
  ui.lgeoStlLibLbl->setText(                        Preferences::lgeoStlLib ? DURAT_LGEO_STL_LIB_INFO : "");
  ui.ldviewPath->setText(                           Preferences::ldviewExe);
  ui.ldviewBox->setChecked(                         Preferences::ldviewExe != "");
  ui.ldviewSingleCall_Chk->setChecked(              Preferences::enableLDViewSingleCall && Preferences::ldviewExe != "");
  ui.fadeStepBox->setChecked(                       Preferences::enableFadeStep);
  ui.publishLogoBox->setChecked(                    Preferences::documentLogoFile != "");
  ui.publishLogoPath->setText(                      Preferences::documentLogoFile);
  ui.authorName_Edit->setText(                      Preferences::defaultAuthor);
  ui.displayAllAttributes_Chk->setChecked(          Preferences::displayAllAttributes);
  ui.generateCoverPages_Chk->setChecked(            Preferences::generateCoverPages);
  ui.publishTOC_Chk->setChecked(                    Preferences::printDocumentTOC);
  ui.publishURL_Edit->setText(                      Preferences::defaultURL);
  ui.publishEmail_Edit->setText(                    Preferences::defaultEmail);
  ui.publishDescriptionEdit->setText(               Preferences::publishDescription);
  ui.enableDownloader_Chk->setChecked(              Preferences::enableDownloader);
  ui.showUpdateNotifications_Chk->setChecked(       Preferences::showUpdateNotifications);
  ui.showAllNotificstions_Chk->setChecked(          Preferences::showAllNotifications);
  ui.checkUpdateFrequency_Combo->setCurrentIndex(   Preferences::checkUpdateFrequency);
  ui.rendererTimeout->setValue(                     Preferences::rendererTimeout);
  ui.povrayDisplay_Chk->setChecked(                 Preferences::povrayDisplay);

  ui.loggingGrpBox->setChecked(                     Preferences::logging);
  ui.logPathEdit->setText(                          Preferences::logPath);

  ui.includeLogLevelBox->setChecked(                Preferences::includeLogLevel);
  ui.includeTimestampBox->setChecked(               Preferences::includeTimestamp);
  ui.includeLineNumberBox->setChecked(              Preferences::includeLineNumber);
  ui.includeFileNameBox->setChecked(                Preferences::includeFileName);
  ui.includeFunctionBox->setChecked(                Preferences::includeFunction);

  ui.logLevelGrpBox->setChecked(                    Preferences::logLevel);
  ui.logLevelsGrpBox->setChecked(                   Preferences::logLevels);

  QStringList logLevels = tr(VER_LOGGING_LEVELS_STR).split(",");
  ui.logLevelCombo->addItems(logLevels);
  ui.logLevelCombo->setCurrentIndex(int(ui.logLevelCombo->findText(Preferences::loggingLevel)));

  ui.debugLevelBox->setChecked(                    Preferences::debugLevel);
  ui.traceLevelBox->setChecked(                    Preferences::traceLevel);
  ui.noticeLevelBox->setChecked(                   Preferences::noticeLevel);
  ui.infoLevelBox->setChecked(                     Preferences::infoLevel);
  ui.statusLevelBox->setChecked(                   Preferences::statusLevel);
  ui.errorLevelBox->setChecked(                    Preferences::errorLevel);
  ui.fatalLevelBox->setChecked(                    Preferences::fatalLevel);

  ui.includeAllLogAttribBox->setChecked(           Preferences::includeAllLogAttributes);
  ui.allLogLevelsBox->setChecked(                  Preferences::allLogLevels);
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
      ui.textEditSearchDirs->setToolTip("Editable list of search directories - add or edit search paths");
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
  ui.preferredRenderer->setMaxCount(3);

  QFileInfo fileInfo(Preferences::povrayExe);
  int povRayIndex = ui.preferredRenderer->count();
  bool povRayExists = fileInfo.exists();
  povRayExists &= fileInfo.exists();
  if (povRayExists) {
      ui.preferredRenderer->addItem("POVRay");
    }

  fileInfo.setFile(Preferences::ldgliteExe);
  int ldgliteIndex = ui.preferredRenderer->count();
  bool ldgliteExists = fileInfo.exists();
  if (ldgliteExists) {
    ui.preferredRenderer->addItem("LDGLite");
  }

  fileInfo.setFile(Preferences::ldviewExe);
  int ldviewIndex = ui.preferredRenderer->count();
  bool ldviewExists = fileInfo.exists();
  if (ldviewExists) {
    ui.preferredRenderer->addItem("LDView");
  }

  if (Preferences::preferredRenderer == "LDView" && ldviewExists) {
    ui.preferredRenderer->setCurrentIndex(ldviewIndex);
    ui.preferredRenderer->setEnabled(true);
  } else if (Preferences::preferredRenderer == "LDGLite" && ldgliteExists) {
    ui.preferredRenderer->setCurrentIndex(ldgliteIndex);
    ui.preferredRenderer->setEnabled(true);
  }  else if (Preferences::preferredRenderer == "POVRay" && povRayExists) {
      ui.preferredRenderer->setCurrentIndex(povRayIndex);
      ui.preferredRenderer->setEnabled(true);
  } else {
    ui.preferredRenderer->setEnabled(false);
  }

  if(!ldviewExists && !ldgliteExists && !povRayExists){
      ui.tabWidget->setCurrentIndex(1);
      ui.RenderMessage->setText("<font color='red'>You must set a renderer.</font>");
  } else {
      ui.tabWidget->setCurrentIndex(0);
  }

  QColor fadeColor = LDrawColor::color(Preferences::fadeStepColor);
  fadeStepMeta.fadeColor.setValue(LDrawColor::name(fadeColor.name()));
  ui.fadeStepColorLabel->setPalette(QPalette(fadeColor));
  ui.fadeStepColorLabel->setAutoFillBackground(true);
  ui.fadeStepColorsCombo->addItems(LDrawColor::names());
  ui.fadeStepColorsCombo->setCurrentIndex(int(ui.fadeStepColorsCombo->findText(Preferences::fadeStepColor)));
  connect(ui.fadeStepColorsCombo, SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(colorChange(QString const &)));

  bool centimeters = Preferences::preferCentimeters;
  ui.Centimeters->setChecked(centimeters);
  ui.Inches->setChecked(! centimeters);

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

void PreferencesDialog::colorChange(QString const &colorName)
{
  QColor fadeColor = LDrawColor::color(Preferences::fadeStepColor);
  QColor newFadeColor = LDrawColor::color(colorName);
  if (fadeColor != newFadeColor) {
    fadeStepMeta.fadeColor.setValue(LDrawColor::name(newFadeColor.name()));
    ui.fadeStepColorLabel->setPalette(QPalette(newFadeColor));
    ui.fadeStepColorLabel->setAutoFillBackground(true);
  }
}

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
      Preferences::enableFadeStep = ui.fadeStepBox->isChecked();
      partWorkerLDSearchDirs.resetSearchDirSettings();
      ui.textEditSearchDirs->clear();
      foreach (QString searchDir, Preferences::ldSearchDirs)
        ui.textEditSearchDirs->append(searchDir);

      box.setIcon (QMessageBox::Information);
      box.setStandardButtons (QMessageBox::Ok);
      box.setText( tr("Search directories have been reset with %1 entries.").arg(Preferences::ldSearchDirs.size()));
      emit gui->messageSig(true,box.text());
      box.exec();
    }
}

void PreferencesDialog::on_checkForUpdates_btn_clicked()
{
    checkForUpdates();
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

bool PreferencesDialog::povrayDisplay()
{
       return ui.povrayDisplay_Chk->isChecked();
}

QString const PreferencesDialog::fadeStepColor()
{
    if (ui.fadeStepColorsCombo->isEnabled()) {
      return ui.fadeStepColorsCombo->currentText();
    }
    return "";
}

QString const PreferencesDialog::documentLogoFile()
{
    if (ui.publishLogoBox->isChecked()){
        return ui.publishLogoPath->displayText();
    }
    return "";
}

bool PreferencesDialog::centimeters()
{
  return ui.Centimeters->isChecked();
}

bool  PreferencesDialog::enableFadeStep()
{
  return ui.fadeStepBox->isChecked();
}

bool PreferencesDialog::enableDocumentLogo()
{
  return ui.publishLogoBox->isChecked();
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
  //    DEFS_URL = QString(VER_UPDATE_CHECK_JSON_URL).arg(moduleVersion);
  if (m_updater->getModuleVersion(DEFS_URL) != moduleVersion)
    m_updater->setModuleVersion(DEFS_URL, moduleVersion);
  m_updater->setEnableDownloader(DEFS_URL, enableDownloader);
  m_updater->setShowAllNotifications(DEFS_URL, showAllNotifications);
  m_updater->setShowUpdateNotifications (DEFS_URL, showUpdateNotifications);

  /* Check for updates */
  m_updater->checkForUpdates (DEFS_URL);

}

void PreferencesDialog::accept(){
    bool missingParms = false;
    QFileInfo fileInfo;

    if (!ui.povrayPath->text().isEmpty() && (ui.povrayPath->text() != Preferences::povrayExe)){
        fileInfo.setFile(ui.povrayPath->text());
        bool povRayExists = fileInfo.exists();
        if (povRayExists) {
            Preferences::povrayExe = ui.povrayPath->text();
            ui.preferredRenderer->addItem("POVRay");
        } else {
            emit gui->messageSig(false,QString("POV-Ray path entered is not valid: %1").arg(ui.povrayPath->text()));
        }
    }
    if (!ui.ldglitePath->text().isEmpty() && (ui.ldglitePath->text() != Preferences::ldgliteExe)) {
        fileInfo.setFile(ui.ldglitePath->text());
        bool ldgliteExists = fileInfo.exists();
        if (ldgliteExists) {
            Preferences::ldgliteExe = ui.ldglitePath->text();
            ui.preferredRenderer->addItem("LDGLite");
        } else {
            emit gui->messageSig(false,QString("LDGLite path entered is not valid: %1").arg(ui.ldglitePath->text()));
        }
    }
    if (!ui.ldviewPath->text().isEmpty() && (ui.ldviewPath->text() != Preferences::ldviewExe)) {
        QString ldviewPath = ui.ldviewPath->text();
#ifndef Q_OS_LINUX
        fileInfo.setFile(ldviewPath);
#else
        // force use command line-only "ldview" (not "LDView") if not using Windows
        QFileInfo info(ldviewPath);
        fileInfo.setFile(QString("%1/%2").arg(info.absolutePath()).arg(info.fileName().toLower()));
#endif
        bool ldviewExists = fileInfo.exists();
        if (ldviewExists) {
            Preferences::ldviewExe = ldviewPath;
            ui.preferredRenderer->addItem("LDView");
        } else {
            emit gui->messageSig(false,QString("LDView path entered is not valid: %1").arg(ui.ldviewPath->text()));
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


void PreferencesDialog::on_ldviewBox_clicked(bool checked)
{
  if (! checked) {
      QMessageBox box;
      box.setIcon (QMessageBox::Information);
      box.setStandardButtons (QMessageBox::Ok);
      box.setWindowTitle(tr ("LDView Settings?"));
      box.setText (tr("LDView renderer settings are automatically set at application startup.\n"
                      "Changes will be reset at next application start? "));
      emit gui->messageSig(true,box.text());
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
        emit gui->messageSig(true,box.text());
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
      emit gui->messageSig(true,box.text());
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
    emit gui->messageSig(true,box.text());
    if (box.exec() == QMessageBox::Yes) {
      ui.altLDConfigPath->clear();
      ui.altLDConfigBox->setChecked(false);
    }
  }
}
