/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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

#include "color.h"
#include "meta.h"
#include "lpub.h"


PreferencesDialog::PreferencesDialog(QWidget *_parent) :
    QDialog(_parent)
{
  ui.setupUi(this);
  
  QString ldrawPath = Preferences::ldrawPath;
  
  if (ldrawPath.isEmpty()) {
    ldrawPath = ".";
  }

  QString leocadLibFile = Preferences::leocadLibFile;
  if (leocadLibFile.isEmpty()) {
      leocadLibFile = ".";
  }
  
  ui.ldrawPath->setText(                    ldrawPath);
  ui.leocadLibFile->setText(                leocadLibFile);
  ui.pliName->setText(                      Preferences::pliFile);
  ui.pliBox->setChecked(                    Preferences::pliFile != "");
  ui.ldglitePath->setText(                  Preferences::ldgliteExe);
  ui.ldgliteBox->setChecked(                Preferences::ldgliteExe != "");
  ui.l3pPath->setText(                      Preferences::l3pExe);
  ui.povrayPath->setText(                   Preferences::povrayExe);
  ui.POVRayBox->setChecked(                 Preferences::l3pExe != "" && Preferences::povrayExe != "");
  ui.lgeoPath->setText(                     Preferences::lgeoPath);
  ui.lgeoBox->setChecked(                   Preferences::lgeoPath != "");
  ui.ldviewPath->setText(                   Preferences::ldviewExe);
  ui.ldviewBox->setChecked(                 Preferences::ldviewExe != "");
  ui.fadeStepBox->setChecked(               Preferences::enableFadeStep);
  ui.publishLogoBox->setChecked(            Preferences::documentLogoFile != "");
  ui.publishLogoPath->setText(              Preferences::documentLogoFile);
  ui.authorName_Edit->setText(              Preferences::defaultAuthor);
  ui.displayAllAttributes_Chk->setChecked(  Preferences::displayAllAttributes);
  ui.generageCoverPages_Chk->setChecked(    Preferences::generageCoverPages);
  ui.publishTOC_Chk->setChecked(            Preferences::printDocumentTOC);
  ui.publishURL_Edit->setText(              Preferences::defaultURL);
  ui.publishEmail_Edit->setText(            Preferences::defaultEmail);
  ui.publishDescriptionEdit->setText(       Preferences::publishDescription);
  ui.chkBoxSilent->setChecked(              Preferences::silentUpdate);
  ui.comboCheckForUpdates->setCurrentIndex( Preferences::checkForUpdates);

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
  connect(ui.fadeStepBox,SIGNAL(clicked(bool)),
          this,SLOT(processFadeColourParts(bool)));

  bool centimeters = Preferences::preferCentimeters;
  ui.Centimeters->setChecked(centimeters);
  ui.Inches->setChecked(! centimeters);

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

void PreferencesDialog::on_browseLeoCADLibrary_clicked()
{
  Preferences::leocadLibPreferences(true);
  ui.leocadLibFile->setText(Preferences::leocadLibFile);
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
                                                  ui.ldglitePath->text().isEmpty() ? Preferences::lpubPath : ui.ldglitePath->text(),
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
                                                  ui.l3pPath->text().isEmpty() ? Preferences::lpubPath : ui.l3pPath->text(),
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

void PreferencesDialog::processFadeColourParts(bool clicked)
{
    if (clicked && !gui->getCurFile().isEmpty())
        colourParts.processFadeColorParts();
}

QString const PreferencesDialog::ldrawPath()
{
  return ui.ldrawPath->displayText();
}

QString const PreferencesDialog::leocadLibFile()
{
  return ui.leocadLibFile->displayText();
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

bool  PreferencesDialog::displayAllAttributes()
{
  return ui.displayAllAttributes_Chk->isChecked();
}

bool  PreferencesDialog::generageCoverPages()
{
  return ui.generageCoverPages_Chk->isChecked();
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

bool PreferencesDialog::silentUpdate()
{
  return ui.chkBoxSilent->isChecked();
}

int PreferencesDialog::checkForUpdates()
{
  return ui.comboCheckForUpdates->currentIndex();
}

void PreferencesDialog::accept(){
    if(ui.preferredRenderer->count() == 0 || ui.ldrawPath->text().isEmpty() || ui.leocadLibFile->text().isEmpty()){
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::yellow);
        if (ui.ldrawPath->text().isEmpty())
            ui.ldrawPath->setPlaceholderText("LDraw directry must be defined");
        if (ui.leocadLibFile->text().isEmpty())
            ui.leocadLibFile->setPlaceholderText("Archive library must be configured");
        if (ui.preferredRenderer->count() == 0){
            ui.ldglitePath->setPlaceholderText("At lease one renderer must be defined");
            ui.ldviewPath->setPlaceholderText("At lease one renderer must be defined");
            ui.povrayPath->setPlaceholderText("At lease one renderer must be defined");
            ui.l3pPath->setPlaceholderText("Reqired if POV-Ray defined");
        }
        if (QMessageBox::Yes == QMessageBox::question(this, "Close Dialog?",
                              "Required settings are missing, Are you sure you want to exit?",
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

