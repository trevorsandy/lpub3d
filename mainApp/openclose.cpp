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

#include <QFileDialog>
#include <QDir>
#include <QTextEdit>
#include <QUndoStack>
#include <QSettings>

#include "lpub.h"
#include "lpub_preferences.h"
#include "editwindow.h"
#include "paths.h"
#include "threadworkers.h"

void Gui::open()
{  
  if (maybeSave()) {
    timer.start();
    QSettings Settings;
    QString modelDir;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"ProjectsPath"))) {
      modelDir = Settings.value(QString("%1/%2").arg(SETTINGS,"ProjectsPath")).toString();
    } else {
      modelDir = Preferences::ldrawPath + "/MODELS";
    }

    QString fileName = QFileDialog::getOpenFileName(
      this,
      tr("Open LDraw File"),
      modelDir,
      tr("LDraw Files (*.DAT;*.LDR;*.MPD;*.dat;*.ldr;*.mpd)"));

    QFileInfo info(fileName);

    if (!fileName.isEmpty()) {
      Settings.setValue(QString("%1/%2").arg(SETTINGS,"ProjectsPath"),info.path());
      openFile(fileName);
      displayPage();
      enableActions();
      emit messageSig(true, "File loaded. " + elapsedTime(timer.elapsed()));
      return;
    }
  }
  return;
}

void Gui::openRecentFile()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (action) {
    timer.start();
    QString fileName = action->data().toString();
    QFileInfo info(fileName);
    QDir::setCurrent(info.absolutePath());
    openFile(fileName);
    Paths::mkdirs();
    displayPage();
    enableActions();
    emit messageSig(true, "File loaded. " + elapsedTime(timer.elapsed()));
  }
}

void Gui::loadFile(const QString &file)
{
    QString fileName = file;
    QFileInfo info(fileName);
    if (info.exists()) {
        timer.start();
        QDir::setCurrent(info.absolutePath());
        openFile(fileName);
        Paths::mkdirs();
        displayPage();
        enableActions();
        emit messageSig(true, "File loaded. " + elapsedTime(timer.elapsed()));
    } else {
        QMessageBox::warning(NULL,QMessageBox::tr(VER_PRODUCTNAME_STR),
                             QMessageBox::tr("Unable to load file %1.")
                             .arg(fileName));
    }
}

void Gui::save()
{
#ifdef WATCHER
  if (isMpd()) {
    watcher.removePath(curFile);
  } else {
    QStringList list = ldrawFile.subFileOrder();
    QString foo;
    foreach (foo,list) {
      QString bar = QDir::currentPath() + "/" + foo;
      watcher.removePath(bar);
    }
  }
#endif
  if (curFile.isEmpty()) {
    saveAs();
  } else {
    saveFile(curFile);
  }

#ifdef WATCHER
  if (isMpd()) {
    watcher.addPath(curFile);
  } else {
    QStringList list = ldrawFile.subFileOrder();
    QString foo;
    foreach (foo,list) {
      QString bar = QDir::currentPath() + "/" + foo;
      watcher.addPath(bar);
    }
  }
#endif
}

void Gui::saveAs()
{
#ifdef WATCHER
  if (curFile != "") {
    if (isMpd()) {
      watcher.removePath(curFile);
    } else {
      QStringList list = ldrawFile.subFileOrder();
      QString foo; 
      foreach (foo,list) {
        QString bar = QDir::currentPath() + "/" + foo;
        watcher.removePath(bar);
      }
    }
  }
#endif
  QString fileName = QFileDialog::getSaveFileName(this,tr("Save As"),curFile,tr("LDraw (*.mpd *.ldr *.dat)"));
  if (fileName.isEmpty()) {
    return;
  }
  QFileInfo fileInfo(fileName);
  QString suffix = fileInfo.suffix();

  if (suffix == "mpd" ||
      suffix == "MPD" ||
      suffix == "ldr" ||
      suffix == "LDR" ||
      suffix == "dat" ||
      suffix == "DAT") {

    saveFile(fileName);
    closeFile();
    openFile(fileName);
    displayPage();
  } else {
    QMessageBox::warning(NULL,QMessageBox::tr(VER_PRODUCTNAME_STR),
                              QMessageBox::tr("Invalid LDraw suffix %1.  File not saved.")
                                .arg(suffix));

  }
#ifdef WATCHER
  if (curFile != "") {
    if (isMpd()) {
      watcher.addPath(curFile);
    } else {
      QStringList list = ldrawFile.subFileOrder();
      QString foo;
      foreach (foo,list) {
        QString bar = QDir::currentPath() + "/" + foo;
        watcher.addPath(bar);
      }
    }
  }
#endif
} 

bool Gui::maybeSave()
{
  if ( ! undoStack->isClean() ) {
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr(VER_PRODUCTNAME_STR),
            tr("The document has been modified.\n"
                "Do you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save) {
      save();
      return true;
    } else if (ret == QMessageBox::Cancel) {
      return false;
    }
  }
  return true;
}

bool Gui::saveFile(const QString &fileName)
{
  bool rc;
  rc = ldrawFile.saveFile(fileName);
  setCurrentFile(fileName);
  undoStack->setClean();
  if (rc) {
    statusBar()->showMessage(tr("File saved"), 2000);
  }
  return rc;
}

void Gui::closeFile()
{
  ldrawFile.empty();
  undoStack->clear();
  editWindow->textEdit()->document()->clear();
  editWindow->textEdit()->document()->setModified(false);
  mpdCombo->setMaxCount(0);
  mpdCombo->setMaxCount(1000);
}

/***************************************************************************
 * File opening closing stuff
 **************************************************************************/

void Gui::openFile(QString &fileName)
{

#ifdef WATCHER
  if (curFile != "") {
    if (isMpd()) {
      watcher.removePath(curFile);
    } else { 
      QStringList list = ldrawFile.subFileOrder();
      QString foo;
      foreach (foo,list) {
        QString bar = QDir::currentPath() + "/" + foo;
        watcher.removePath(bar);
      }
    }
  }
#endif

  clearPage(KpageView,KpageScene);
  closeFile();
  displayPageNum = 1;
  QFileInfo info(fileName);
  QDir::setCurrent(info.absolutePath());
  Paths::mkdirs();
  emit messageSig(true, "Loading LDraw model file...");
  ldrawFile.loadFile(fileName);
  emit messageSig(true, "Loading fade colour parts...");
  processFadeColourParts();
  emit messageSig(true, "Loading user interface items...");
  attitudeAdjustment();
  mpdCombo->setMaxCount(0);
  mpdCombo->setMaxCount(1000);
  mpdCombo->addItems(ldrawFile.subFileOrder());
  setCurrentFile(fileName);
  emit messageSig(true, "Loading editor display...");
  displayFile(&ldrawFile,ldrawFile.topLevelFile());
  undoStack->setClean();
  curFile = fileName;
  insertFinalModel();    //insert final fully coloured model if fadeStep turned on
  generateCoverPages();  //autogenerate cover page

#ifdef WATCHER
  if (isMpd()) {
    watcher.addPath(curFile);
  } else {
    QStringList list = ldrawFile.subFileOrder();
    QString foo;
    foreach (foo,list) {
      QString bar = QDir::currentPath() + "/" + foo;
      watcher.addPath(bar);
    }
  }
#endif
  defaultResolutionType(Preferences::preferCentimeters);
  emit messageSig(true,"File opened.");
}

void Gui::updateRecentFileActions()
{
  QSettings Settings;
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,"LPRecentFileList"))) {
    QStringList files = Settings.value(QString("%1/%2").arg(SETTINGS,"LPRecentFileList")).toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    // filter filest that don't exist

    for (int i = 0; i < numRecentFiles; ) {
      QFileInfo fileInfo(files[i]);
      if (fileInfo.exists()) {
        i++;
      } else {
        files.removeOne(files[i]);
        --numRecentFiles;
      }
    }
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"LPRecentFileList"), files);

    for (int i = 0; i < numRecentFiles; i++) {
      QFileInfo fileInfo(files[i]);
      QString text = tr("&%1 %2").arg(i + 1).arg(fileInfo.fileName());
      recentFilesActs[i]->setText(text);
      recentFilesActs[i]->setData(files[i]);
      recentFilesActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; j++) {
      recentFilesActs[j]->setVisible(false);
    }
    separatorAct->setVisible(numRecentFiles > 0);
  }
}

void Gui::setCurrentFile(const QString &fileName)
{
  QString shownName;
  if (fileName.size() == 0) {
    shownName = VER_FILEDESCRIPTION_STR;
  } else {
    QFileInfo fileInfo(fileName);
    
    shownName = fileInfo.fileName();
  }
  
  setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr(VER_PRODUCTNAME_STR)));

  if (fileName.size() > 0) {
    QSettings Settings;
    QStringList files = Settings.value(QString("%1/%2").arg(SETTINGS,"LPRecentFileList")).toStringList();
    files.removeAll("");
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles) {
      files.removeLast();
    }
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"LPRecentFileList"), files);
  }
  updateRecentFileActions();
}

void Gui::fileChanged(const QString &path)
{
  if (! changeAccepted)
    return;

  changeAccepted = false;

  // Get the application icon as a pixmap
  QPixmap _icon = QPixmap(":/icons/lpub96.png");
  if (_icon.isNull())
      _icon = QPixmap (":/icons/update.png");

  QMessageBox box;
  box.setWindowIcon(QIcon());
  box.setIconPixmap (_icon);
  box.setTextFormat (Qt::RichText);
  box.setWindowTitle(tr ("%1 File Change").arg(VER_PRODUCTNAME_STR));
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  QString title = "<b>" + tr ("External change detected") + "</b>";
  QString text = tr("\"%1\" contents were changed by an external source.  Reload?").arg(path);
  box.setText (title);
  box.setInformativeText (text);
  box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
  box.setDefaultButton   (QMessageBox::Yes);

  if (box.exec() == QMessageBox::Yes) {
    changeAccepted = true;
    QString fileName = path;
    openFile(fileName);
    drawPage(KpageView,KpageScene,false);
  }
}

QString Gui::elapsedTime(const qint64 &time){

    int secs = time / 1000;
    int mins = (secs / 60) % 60;
    secs = secs % 60;
    int msecs = time % 1000;

    return QString("Elapsed time is %1:%2:%3")
    .arg(mins, 2, 10, QLatin1Char('0'))
    .arg(secs,  2, 10, QLatin1Char('0'))
    .arg(msecs,  3, 10, QLatin1Char('0'));

}
