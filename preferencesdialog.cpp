/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include "ui_preferences.h"
#include "preferencesdialog.h"
#include "lpub_preferences.h"

PreferencesDialog::PreferencesDialog(QWidget     *_parent)
{
  ui.setupUi(this);
  
  QString ldrawPath = Preferences::ldrawPath;
  
  if (ldrawPath == "") {
    ldrawPath = ".";
  }
  
  ui.ldrawPath->setText(    ldrawPath);
  ui.pliName->setText(      Preferences::pliFile);
  ui.pliBox->setChecked(    Preferences::pliFile != "");
  ui.ldglitePath->setText(  Preferences::ldgliteExe);
	ui.ldgliteBox->setChecked(Preferences::ldgliteExe != "");
	ui.l3pPath->setText(  Preferences::l3pExe);
	ui.povrayPath->setText(Preferences::povrayExe);
	ui.l3pBox->setChecked(Preferences::l3pExe != "" && Preferences::povrayExe != "");
	ui.lgeoPath->setText(Preferences::lgeoPath);
	ui.lgeoBox->setChecked(Preferences::lgeoPath != "");
  ui.ldviewPath->setText(   Preferences::ldviewExe);
  ui.ldviewBox->setChecked( Preferences::ldviewExe != "");
  
  ui.preferredRenderer->setMaxCount(0);
	ui.preferredRenderer->setMaxCount(3);
	
	QFileInfo fileInfo(Preferences::l3pExe);
	int l3pIndex = ui.preferredRenderer->count();
	bool l3pExists = fileInfo.exists();
	fileInfo.setFile(Preferences::povrayExe);
	l3pExists &= fileInfo.exists();
	if (l3pExists) {
		ui.preferredRenderer->addItem("L3P");
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
  
  parent = _parent;
  if (Preferences::preferredRenderer == "LDView" && ldviewExists) {
    ui.preferredRenderer->setCurrentIndex(ldviewIndex);
    ui.preferredRenderer->setEnabled(true);
  } else if (Preferences::preferredRenderer == "LDGLite" && ldgliteExists) {
    ui.preferredRenderer->setCurrentIndex(ldgliteIndex);
    ui.preferredRenderer->setEnabled(true);
  }  else if (Preferences::preferredRenderer == "L3P" && l3pExists) {
	  ui.preferredRenderer->setCurrentIndex(l3pIndex);
	  ui.preferredRenderer->setEnabled(true);
  } else {
    ui.preferredRenderer->setEnabled(false);
  }
  bool centimeters = Preferences::preferCentimeters;
  ui.Centimeters->setChecked( centimeters );
  ui.Inches->setChecked( ! centimeters );
}

void PreferencesDialog::on_browseLDraw_clicked()
{
  Preferences::ldrawPreferences(true);
}

void PreferencesDialog::on_browseLGEO_clicked()
{
	QFileDialog dialog(parent);
	dialog.setFileMode(QFileDialog::Directory);
	dialog.setWindowTitle(tr("Locate LGEO Directory"));
	dialog.setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	
	if (dialog.exec()) {
		QStringList selectedFiles = dialog.selectedFiles();
		
		if (selectedFiles.size() == 1) {
			ui.lgeoPath->setText(selectedFiles[0]);
			QFileInfo  fileInfo(selectedFiles[0]);
			ui.lgeoBox->setChecked(fileInfo.exists());
		}
	}
}

void PreferencesDialog::on_browsePli_clicked()
{
  QFileDialog dialog(parent);
  
  dialog.setWindowTitle(tr("Locate Parts List orientation/size file"));
  dialog.setFileMode(QFileDialog::ExistingFile);

  dialog.setFilter("LDraw (*.mpd,*.dat,*.ldr)");

  if (dialog.exec()) {
    QStringList selectedFiles = dialog.selectedFiles();
    
    if (selectedFiles.size() == 1) {
      ui.pliName->setText(selectedFiles[0]);
      QFileInfo  fileInfo(ui.pliName->displayText());
      ui.pliBox->setChecked(fileInfo.exists());
    }
  }
}

void PreferencesDialog::on_browseLDView_clicked()
{
  QFileDialog dialog(parent);
  
  dialog.setWindowTitle(tr("Locate LDView program"));
  dialog.setFileMode(QFileDialog::ExistingFile);

#ifdef __APPLE__
  //dialog.setFilter("Program (*.app,*.App)");
#else
  dialog.setFilter("Program (*.exe)");
#endif
  if (dialog.exec()) {
    QStringList selectedFiles = dialog.selectedFiles();
    
    if (selectedFiles.size() == 1) {
      ui.ldviewPath->setText(selectedFiles[0]);
      QFileInfo  fileInfo(selectedFiles[0]);
      if (fileInfo.exists()) {
        int ldviewIndex = ui.preferredRenderer->findText("LDView");
        if (ldviewIndex < 0) {
          ui.preferredRenderer->addItem("LDView");
        }
        ui.preferredRenderer->setEnabled(true);
      } 
      ui.ldviewBox->setChecked(fileInfo.exists());
    }
  }
}

void PreferencesDialog::on_browseLDGLite_clicked()
{
  QFileDialog dialog(parent);
  
  dialog.setWindowTitle(tr("Locate LDGLite program"));
  dialog.setFileMode(QFileDialog::ExistingFile);

#ifdef __APPLE__
  //dialog.setFilter("Program (*.app,*.App)");
#else
  dialog.setFilter("Program (*.exe)");
#endif
  if (dialog.exec()) {
    QStringList selectedFiles = dialog.selectedFiles();
    
    if (selectedFiles.size() == 1) {
      ui.ldglitePath->setText(selectedFiles[0]);
      QFileInfo  fileInfo(selectedFiles[0]);
      if (fileInfo.exists()) {
        int ldgliteIndex = ui.preferredRenderer->findText("LDGLite");
        if (ldgliteIndex < 0) {
          ui.preferredRenderer->addItem("LDGLite");
        }
        ui.preferredRenderer->setEnabled(true);
      } 
      ui.ldgliteBox->setChecked(fileInfo.exists());
    }
  }
}

void PreferencesDialog::on_browseL3P_clicked()
{
	QFileDialog dialog(parent);
	
	dialog.setWindowTitle(tr("Locate L3P program"));
	dialog.setFileMode(QFileDialog::ExistingFile);
	
#ifdef __APPLE__
	//dialog.setFilter("Program (*.app,*.App)");
#else
	dialog.setFilter("Program (*.exe)");
#endif
	if (dialog.exec()) {
		QStringList selectedFiles = dialog.selectedFiles();
		
		if (selectedFiles.size() == 1) {
			ui.l3pPath->setText(selectedFiles[0]);
			QFileInfo  fileInfo(selectedFiles[0]);
			QFileInfo povrayInfo(ui.povrayPath->text());
			if (fileInfo.exists() && povrayInfo.exists()) {
				int l3pIndex = ui.preferredRenderer->findText("L3P");
				if (l3pIndex < 0) {
					ui.preferredRenderer->addItem("L3P");
				}
				ui.preferredRenderer->setEnabled(true);
			}
			ui.l3pBox->setChecked(fileInfo.exists() && povrayInfo.exists());
		}
	}
}

void PreferencesDialog::on_browsePOVRAY_clicked()
{
	QFileDialog dialog(parent);
	
	dialog.setWindowTitle(tr("Locate POV-RAY program"));
	dialog.setFileMode(QFileDialog::ExistingFile);
	
#ifdef __APPLE__
	//dialog.setFilter("Program (*.app,*.App)");
#else
	dialog.setFilter("Program (*.exe)");
#endif
	if (dialog.exec()) {
		QStringList selectedFiles = dialog.selectedFiles();
		
		if (selectedFiles.size() == 1) {
			ui.povrayPath->setText(selectedFiles[0]);
			QFileInfo  fileInfo(selectedFiles[0]);
			QFileInfo l3pInfo(ui.l3pPath->text());
			if (fileInfo.exists() && l3pInfo.exists()) {
				int l3pIndex = ui.preferredRenderer->findText("L3P");
				if (l3pIndex < 0) {
					ui.preferredRenderer->addItem("L3P");
				}
				ui.preferredRenderer->setEnabled(true);
			}
			ui.l3pBox->setChecked(fileInfo.exists() && l3pInfo.exists());
		}
	}
}

QString const PreferencesDialog::ldrawPath()
{
  return ui.ldrawPath->displayText();
}

QString const PreferencesDialog::lgeoPath()
{
	if (ui.l3pBox->isChecked() && ui.lgeoBox->isChecked()){
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
	if (ui.l3pBox->isChecked()) {
		return ui.povrayPath->displayText();
	}
	return "";
}
QString const PreferencesDialog::l3pExe()
{
	if (ui.l3pBox->isChecked()) {
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

bool const PreferencesDialog::centimeters()
{
  return ui.Centimeters->isChecked();
}
