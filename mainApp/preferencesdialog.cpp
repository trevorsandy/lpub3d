/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

  ui.ldrawPath->setText(                            ldrawPath);
  ui.pliName->setText(                              Preferences::pliFile);
  ui.pliBox->setChecked(                            Preferences::pliFile != "");
  ui.ldglitePath->setText(                          Preferences::ldgliteExe);
  ui.ldgliteBox->setChecked(                        Preferences::ldgliteExe != "");
  ui.l3pPath->setText(                              Preferences::l3pExe);
  ui.povrayPath->setText(                           Preferences::povrayExe);
  ui.POVRayBox->setChecked(                         Preferences::l3pExe != "" && Preferences::povrayExe != "");
  ui.lgeoPath->setText(                             Preferences::lgeoPath);
  ui.lgeoBox->setChecked(                           Preferences::lgeoPath != "");
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
  QPalette palette;
  palette.setColor(QPalette::Base,Qt::lightGray);
  ui.lineEditIniFile->setPalette(palette);
  ui.lineEditIniFile->setReadOnly(true);
  if (Preferences::ldrawiniFound) {
      ui.lineEditIniFile->setText(QString("Using LDraw.ini File: %1").arg(Preferences::ldrawiniFile));
      ui.pushButtonReset->hide();   
      ui.textEditSearchDirs->setReadOnly(true);
      ui.textEditSearchDirs->setPalette(palette);
      ui.textEditSearchDirs->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
      ui.textEditSearchDirs->setToolTip("Read only list of LDraw.ini search directories.");
    } else {
      ui.textEditSearchDirs->setToolTip("Editable list of search directories.");
      ui.textEditSearchDirs->setStatusTip("Added directories must be under the Unofficial directory.");
      ui.lineEditIniFile->setText(tr("%1")
                                  .arg(Preferences::ldSearchDirs.size() == 0 ?
                                         tr("Using default search. No search directories detected.") :
                                         tr("Using default %1 search.").arg(VER_PRODUCTNAME_STR)));
      ui.pushButtonReset->setEnabled(Preferences::ldSearchDirs.size() > 0);
    }

  if (Preferences::ldSearchDirs.size() > 0){
      foreach (QString iniFilePath, Preferences::ldSearchDirs)
        ui.textEditSearchDirs->append(iniFilePath);
    }

  connect(ui.textEditSearchDirs, SIGNAL(textChanged()),this, SLOT(pushButtonReset_SetState()));
  //end search dirs

  ui.preferredRenderer->setMaxCount(0);
  ui.preferredRenderer->setMaxCount(3);
	
	QFileInfo fileInfo(Preferences::l3pExe);
    int povRayIndex = ui.preferredRenderer->count();
    bool povRayExists = fileInfo.exists();
	fileInfo.setFile(Preferences::povrayExe);
    povRayExists &= fileInfo.exists();
    if (povRayExists) {
        ui.preferredRenderer->addItem("POV-Ray");
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
  }  else if (Preferences::preferredRenderer == "POV-Ray" && povRayExists) {
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

  ui.fadeStepColorLabel->setPalette(QPalette(LDrawColor::color(Preferences::fadeStepColor)));
  ui.fadeStepColorLabel->setAutoFillBackground(true);
  ui.fadeStepColorsCombo->addItems(LDrawColor::names());
  ui.fadeStepColorsCombo->setCurrentIndex(int(ui.fadeStepColorsCombo->findText(Preferences::fadeStepColor)));
  fadeStepMeta.fadeColor.setValue(LDrawColor::name(Preferences::fadeStepColor));
  connect(ui.fadeStepColorsCombo,SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(colorChange(QString const &)));

  bool centimeters = Preferences::preferCentimeters;
  ui.Centimeters->setChecked(centimeters);
  ui.Inches->setChecked(! centimeters);

  /* QSimpleUpdater start */
  m_updater = QSimpleUpdater::getInstance();

  connect (m_updater, SIGNAL (checkingFinished (QString)),
           this,        SLOT (updateChangelog  (QString)));

  QString version = qApp->applicationVersion();
  QStringList updatableVersions = tr(VER_UPDATEABLE_VERSIONS_STR).split(",");
  ui.moduleVersion_Combo->addItems(updatableVersions);
  ui.moduleVersion_Combo->setCurrentIndex(int(ui.moduleVersion_Combo->findText(version)));

  ui.groupBoxChangeLog->setTitle(tr("Change Log for version %1").arg(version));
  QString readme = tr("%1/%2/%3").arg(Preferences::lpub3dPath).arg(Preferences::lpub3dMacResourcePath).arg("README.txt");
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

#ifndef Q_OS_WIN
  ui.groupBoxUpdates->setEnabled(false);
#endif

}

PreferencesDialog::~PreferencesDialog()
{}

void PreferencesDialog::colorChange(QString const &colorName)
{
  QColor qcolor = LDrawColor::color(Preferences::fadeStepColor);
  QColor newColor = LDrawColor::color(colorName);
  if (qcolor != newColor) {
    fadeStepMeta.fadeColor.setValue(LDrawColor::name(newColor.name()));
    ui.fadeStepColorLabel->setPalette(QPalette(newColor));
    ui.fadeStepColorLabel->setAutoFillBackground(true);
  }
}

void PreferencesDialog::on_browseLDraw_clicked()
{
  Preferences::ldrawPreferences(true);
  ui.ldrawPath->setText(Preferences::ldrawPath);
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

void PreferencesDialog::on_browseLDView_clicked()
{     
#ifdef Q_OS_WIN
    QString filter(tr("Executable Files (*.exe);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif

    QString result = QFileDialog::getOpenFileName(this, tr("Locate LDView Executable"),
                                                  ui.ldviewPath->text(),
                                                  filter);

    if (!result.isEmpty()) {
        QFileInfo resultInfo(result);
#ifdef Q_OS_LINUX
        // force use command line-only "ldview" (not "LDView") if not using Windows
        result = QString("%1/%2").arg(resultInfo.absolutePath()).arg(resultInfo.fileName().toLower());
#elif defined Q_OS_MAC
        // access command line inside the LDView.app wrapper
        result = QString("%1/%2").arg(resultInfo.absoluteFilePath()).arg("Contents/MacOS/LDView");
#endif

        result = QDir::toNativeSeparators(result);
        ui.ldviewPath->setText(result);
        int ldviewIndex = ui.preferredRenderer->findText("LDView");
        if (ldviewIndex < 0) {
            ui.preferredRenderer->addItem("LDView");
        }
        ui.preferredRenderer->setEnabled(true);
        ui.ldviewBox->setChecked(true);
        ui.RenderMessage->setText("");
    }
}

void PreferencesDialog::on_browseLDGLite_clicked()
{           
#ifdef Q_OS_WIN
    QString filter(tr("Executable Files (*.exe);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif

    QString result = QFileDialog::getOpenFileName(this, tr("Locate LDGLite Executable"),
                                                  ui.ldglitePath->text().isEmpty() ? Preferences::lpub3dPath : ui.ldglitePath->text(),
                                                  filter);

    if (!result.isEmpty()) {
        result = QDir::toNativeSeparators(result);
        ui.ldglitePath->setText(result);
        int ldgliteIndex = ui.preferredRenderer->findText("LDGLite");
        if (ldgliteIndex < 0) {
            ui.preferredRenderer->addItem("LDGLite");
        }
        ui.preferredRenderer->setEnabled(true);
        ui.ldgliteBox->setChecked(true);
        ui.RenderMessage->setText("");
    }
}

void PreferencesDialog::on_browseL3P_clicked()
{                  
#ifdef Q_OS_WIN
    QString filter(tr("Executable Files (*.exe);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif

    QString result = QFileDialog::getOpenFileName(this, tr("Locate L3P Executable"),
                                                  ui.l3pPath->text().isEmpty() ? Preferences::lpub3dPath : ui.l3pPath->text(),
                                                  filter);

    if (!result.isEmpty()) {
        result = QDir::toNativeSeparators(result);
        ui.l3pPath->setText(result);
        QFileInfo povRayInfo(ui.povrayPath->text());
        if (povRayInfo.exists()) {
            int povRayIndex = ui.preferredRenderer->findText("POV-Ray");
            if (povRayIndex < 0) {
                ui.preferredRenderer->addItem("POV-Ray");
            }
            ui.preferredRenderer->setEnabled(true);
        }
        ui.POVRayBox->setChecked(povRayInfo.exists());
        ui.RenderMessage->setText("");
    }
}

void PreferencesDialog::on_browsePOVRAY_clicked()
{
#ifdef Q_OS_WIN
    QString filter(tr("Executable Files (*.exe);;All Files (*.*)"));
#else
    QString filter(tr("All Files (*.*)"));
#endif

    QString result = QFileDialog::getOpenFileName(this, tr("Locate POV-Ray Executable"),
                                                  ui.povrayPath->text(),
                                                  filter);

    if (!result.isEmpty()) {
        result = QDir::toNativeSeparators(result);
        ui.povrayPath->setText(result);
        QFileInfo l3pInfo(ui.l3pPath->text());
        if (l3pInfo.exists()) {
            int povRayIndex = ui.preferredRenderer->findText("POV-Ray");
            if (povRayIndex < 0) {
                ui.preferredRenderer->addItem("POV-Ray");
            }
            ui.preferredRenderer->setEnabled(true);
        }
        ui.POVRayBox->setChecked(l3pInfo.exists());
        ui.RenderMessage->setText("");
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
      foreach (QString iniFilePath, Preferences::ldSearchDirs)
        ui.textEditSearchDirs->append(iniFilePath);

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
QString const PreferencesDialog::l3pExe()
{
    if (ui.POVRayBox->isChecked()) {
		return ui.l3pPath->displayText();
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

    bool l3pExists = false;
    if (!ui.l3pPath->text().isEmpty() && (ui.l3pPath->text() != Preferences::l3pExe)) {
        fileInfo.setFile(ui.l3pPath->text());
        l3pExists = fileInfo.exists();
        if (!l3pExists)
            emit gui->messageSig(false,QString("L3P path entered is not valid: %1").arg(ui.l3pPath->text()));
    }
    bool povRayExists = false;
    if (!ui.povrayPath->text().isEmpty() && (ui.povrayPath->text() != Preferences::povrayExe)){
        fileInfo.setFile(ui.povrayPath->text());
        povRayExists &= fileInfo.exists();
        if (!povRayExists)
            emit gui->messageSig(false,QString("POV-Ray path entered is not valid: %1").arg(ui.povrayPath->text()));
    }
    if (l3pExists && povRayExists) {
        Preferences::l3pExe    = ui.l3pPath->text();
        Preferences::povrayExe = ui.povrayPath->text();
        ui.preferredRenderer->addItem("POV-Ray");
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
            ui.l3pPath->setPlaceholderText("Reqired if POV-Ray defined");
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
