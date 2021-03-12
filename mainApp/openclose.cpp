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

#include <QFileDialog>
#include <QDir>
#include <QComboBox>
#include <QLineEdit>
#include <QUndoStack>
#include <QSettings>

#include "lpub.h"
#include "lpub_preferences.h"
#include "editwindow.h"
#include "paths.h"
#include "threadworkers.h"
#include "messageboxresizable.h"
#include "separatorcombobox.h"
#include "metagui.h"
#include "lc_profile.h"
#include "lc_previewwidget.h"
#include "waitingspinnerwidget.h"

#include <LDVQt/LDVImageMatte.h>

enum FileOpts {
    OPT_NONE,
    OPT_OPEN_WITH,
    OPT_SAVE,
    OPT_SAVE_AS,
    OPT_SAVE_COPY,
    OPT_USE_CURRENT,
    OPT_USE_INCLUDE
};

void Gui::open()
{  
  if (maybeSave() && saveBuildModification()) {
    QSettings Settings;
    QString modelDir;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"ProjectsPath"))) {
      modelDir = Settings.value(QString("%1/%2").arg(SETTINGS,"ProjectsPath")).toString();
    } else {
      modelDir = Preferences::ldrawLibPath + "/models";
    }

    QString fileName = QFileDialog::getOpenFileName(
      this,
      tr("Open LDraw File"),
      modelDir,
      tr("LDraw Files (*.dat *.ldr *.mpd);;All Files (*.*)"));

    timer.start();

    QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
      Settings.setValue(QString("%1/%2").arg(SETTINGS,"ProjectsPath"),fileInfo.path());
      if (!openFile(fileName)) {
          emit messageSig(LOG_STATUS, QString("Load LDraw model file %1 aborted.").arg(fileName));
          return;
      }
      displayPage();
      enableActions();
      ldrawFile.showLoadMessages();
      emit messageSig(LOG_STATUS, gui->loadAborted() ?
                       QString("Load LDraw model file %1 aborted.").arg(fileName) :
                       QString("File loaded (%1 parts). %2")
                               .arg(ldrawFile.getPartCount())
                               .arg(elapsedTime(timer.elapsed())));
      return;
    }
  }
  return;
}

void Gui::openDropFile(QString &fileName){

  if (maybeSave() && saveBuildModification()) {
      timer.start();
      QFileInfo fileInfo(fileName);
      QString extension = fileInfo.suffix().toLower();
      bool ldr = false, mpd = false, dat= false;
      ldr = extension == "ldr";
      mpd = extension == "mpd";
      dat = extension == "dat";
      if (fileInfo.exists() && (ldr || mpd || dat)) {
          QSettings Settings;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,"ProjectsPath"),fileInfo.path());
          if (!openFile(fileName)) {
              emit messageSig(LOG_STATUS, QString("Load LDraw model file %1 aborted.").arg(fileName));
              return;
          }
          displayPage();
          enableActions();
          ldrawFile.showLoadMessages();
          emit messageSig(LOG_STATUS, gui->loadAborted() ?
                              QString("Load LDraw model file %1 aborted.").arg(fileName) :
                              QString("File loaded (%1 parts). %2")
                                      .arg(ldrawFile.getPartCount())
                                      .arg(elapsedTime(timer.elapsed())));
        } else {
          QString noExtension;
          if (extension.isEmpty())
              noExtension = QString("<br>No file exension specified. Set the file extension to .mpd,.ldr, or .dat.");
          emit messageSig(LOG_ERROR, QString("File not supported!<br>%1%2")
                          .arg(fileName).arg(noExtension));
        }
    }
}

void Gui::openFolderSelect(const QString &absoluteFilePath)
{
    auto openPath = [this](const QString& absolutePath)
    {
        bool ok = true;
        const QString path = QDir::fromNativeSeparators(absolutePath);
        // Hack to access samba shares with QDesktopServices::openUrl
        if (path.startsWith("//"))
            ok = QDesktopServices::openUrl(QDir::toNativeSeparators("file:" + path));
        else
            ok = QDesktopServices::openUrl(QUrl::fromLocalFile(path));

        if (!ok) {
            QErrorMessage *m = new QErrorMessage(this);
            m->showMessage(QString("%1\n%2").arg("Failed to open folder!").arg(path));
        }
    };

    const QString path = QDir::fromNativeSeparators(absoluteFilePath);
#ifdef Q_OS_WIN
    if (QFileInfo(path).exists()) {
        // Syntax is: explorer /select, "C:\Folder1\Folder2\file_to_select"
        // Dir separators MUST be win-style slashes

        // QProcess::startDetached() has an obscure bug. If the path has
        // no spaces and a comma(and maybe other special characters) it doesn't
        // get wrapped in quotes. So explorer.exe can't find the correct path and
        // displays the default one. If we wrap the path in quotes and pass it to
        // QProcess::startDetached() explorer.exe still shows the default path. In
        // this case QProcess::startDetached() probably puts its own quotes around ours.

        STARTUPINFO startupInfo;
        ::ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);

        PROCESS_INFORMATION processInfo;
        ::ZeroMemory(&processInfo, sizeof(processInfo));

        QString cmd = QString("explorer.exe /select,\"%1\"").arg(QDir::toNativeSeparators(absoluteFilePath));
        LPWSTR lpCmd = new WCHAR[cmd.size() + 1];
        cmd.toWCharArray(lpCmd);
        lpCmd[cmd.size()] = 0;

        bool ret = ::CreateProcessW(NULL, lpCmd, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
        delete [] lpCmd;

        if (ret) {
            ::CloseHandle(processInfo.hProcess);
            ::CloseHandle(processInfo.hThread);
        }
    }
    else {
        // If the item to select doesn't exist, try to open its parent
        openPath(path.left(path.lastIndexOf("/")));
    }
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    if (QFileInfo(path).exists()) {
        QProcess proc;
        QString output;
        proc.start("xdg-mime", QStringList() << "query" << "default" << "inode/directory");
        proc.waitForFinished();
        output = proc.readLine().simplified();
        if (output == "dolphin.desktop" || output == "org.kde.dolphin.desktop")
            proc.startDetached("dolphin", QStringList() << "--select" << QDir::toNativeSeparators(path));
        else if (output == "nautilus.desktop" || output == "org.gnome.Nautilus.desktop"
                 || output == "nautilus-folder-handler.desktop")
            proc.startDetached("nautilus", QStringList() << "--no-desktop" << QDir::toNativeSeparators(path));
        else if (output == "caja-folder-handler.desktop")
            proc.startDetached("caja", QStringList() << "--no-desktop" << QDir::toNativeSeparators(path));
        else if (output == "nemo.desktop")
            proc.startDetached("nemo", QStringList() << "--no-desktop" << QDir::toNativeSeparators(path));
        else if (output == "kfmclient_dir.desktop")
            proc.startDetached("konqueror", QStringList() << "--select" << QDir::toNativeSeparators(path));
        else
            openPath(path.left(path.lastIndexOf("/")));

        QProcess::ExitStatus status = proc.exitStatus();
        if (status != 0) {  // look for error
            QErrorMessage *m = new QErrorMessage(this);
            m->showMessage(QString("%1\n%2").arg("Failed to open working folder!").arg(path));
        }
    }
    else {
        // If the item to select doesn't exist, try to open its parent
        openPath(path.left(path.lastIndexOf("/")));
    }
#else // Q_OS_MAC
    openPath(path.left(path.lastIndexOf("/")));
#endif
}

void Gui::openWorkingFolder() {
    if (!getCurFile().isEmpty())
        openFolderSelect(getCurFile());
}

void Gui::updateOpenWithActions()
{
    QSettings Settings;
    QString const openWithProgramListKey("OpenWithProgramList");
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,openWithProgramListKey))) {

      programEntries = Settings.value(QString("%1/%2").arg(SETTINGS,openWithProgramListKey)).toStringList();

      numPrograms = qMin(programEntries.size(), Preferences::maxOpenWithPrograms);
      messageSig(LOG_DEBUG, QString("1. Number of Programs: %1").arg(numPrograms));

      QString programData, programName, programPath;

      auto getProgramIcon = [&programPath] ()
      {
          QStringList pathList   = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
          QString iconPath       = pathList.first();
          const QString iconFile = QString("%1/%2icon.png").arg(iconPath).arg(QFileInfo(programPath).baseName());
          if (!QFileInfo(iconFile).exists()) {
              QFileInfo programInfo(programPath);
              QFileSystemModel *fsModel = new QFileSystemModel;
              fsModel->setRootPath(programInfo.path());
              QIcon fileIcon = fsModel->fileIcon(fsModel->index(programInfo.filePath()));
              QPixmap iconPixmap = fileIcon.pixmap(16,16);
              if (!iconPixmap.save(iconFile))
                  emit gui->messageSig(LOG_INFO,QString("Could not save program file icon: %1").arg(iconFile));
              return fileIcon;
          }
          return QIcon(iconFile);
      };

      // filter programPaths that don't exist
      for (int i = 0; i < numPrograms; ) {
        programData = programEntries.at(i).split("|").last();
        programPath = programData;
        QStringList arguments;
        if (!programData.isEmpty())
            openWithProgramAndArgs(programPath,arguments);
        QFileInfo fileInfo(programPath);
        if (fileInfo.exists()) {
          programName = programEntries.at(i).split("|").first();
          QString text = programName;
          if (text.isEmpty())
              text = tr("&%1 %2").arg(i + 1).arg(fileInfo.fileName());
          openWithActList[i]->setText(text);
          openWithActList[i]->setData(programData); // includes arguments
          openWithActList[i]->setIcon(getProgramIcon());
          openWithActList[i]->setStatusTip(QString("Open current file with %2")
                                                   .arg(fileInfo.fileName()));
          openWithActList[i]->setVisible(true);
          i++;
        } else {
          programEntries.removeOne(programEntries.at(i));
          --numPrograms;
        }
      }
      messageSig(LOG_DEBUG, QString("2. Number of Programs: %1").arg(numPrograms));

      // hide empty program actions
      for (int j = numPrograms; j < Preferences::maxOpenWithPrograms; j++) {
        openWithActList[j]->setVisible(false);
      }
      openWithMenu->setEnabled(numPrograms > 0);
    }
}

void Gui::openWithSetup()
{
    OpenWithProgramDialogGui *openWithProgramDialogGui =
                              new OpenWithProgramDialogGui();
    openWithProgramDialogGui->setOpenWithProgram();
    updateOpenWithActions();
}

void Gui::openWithProgramAndArgs(QString &program, QStringList &arguments)
{
    QRegExp quoteRx("\"|'");
    QString valueAt0 = program.at(0);
    bool inside = valueAt0.contains(quoteRx);                             // true if the first character is " or '
    QStringList list = program.split(quoteRx, QString::SkipEmptyParts);   // Split by " or '
    if (list.size() == 1) {
        program = list.first();
    } else {
        QStringList values;
        Q_FOREACH (QString item, list) {
            if (inside) {                                                 // If 's' is inside quotes ...
                values.append(item);                                      // ... get the whole string
            } else {                                                      // If 's' is outside quotes ...
                values.append(item.split(" ", QString::SkipEmptyParts));  // ... get the split string
            }
            inside = !inside;
        }
        program = values.first();                                         //first value is application path
        values.removeFirst();                                             // remove application path from values
        arguments = values + arguments;                                   // prepend values to arguments
    }
}

void Gui::openWith(const QString &filePath)
{

    QAction *action = qobject_cast<QAction *>(sender());
    QStringList arguments = QStringList() << filePath;
    QString program;

    if (action) {
        program = action->data().toString();
        if (program.isEmpty()) {
#ifdef Q_OS_MACOS
            if (Preferences::systemEditor.isEmpty()) {
                program = QString("open");
                arguments.prepend("-e");
            } else {
                openWithProgramAndArgs(Preferences::systemEditor,arguments);
            }
#else
            openWithProgramAndArgs(Preferences::systemEditor,arguments);
#endif
        } else {
            openWithProgramAndArgs(program,arguments);
        }
        qint64 pid;
        QString workingDirectory = QDir::currentPath() + QDir::separator();
        QProcess::startDetached(program, {arguments}, workingDirectory, &pid);
        emit messageSig(LOG_INFO, QString("Launched %1 with %2...")
                        .arg(QFileInfo(filePath).fileName()).arg(QFileInfo(program).fileName()));
    }
}

void Gui::openWith()
{
    QString file = curFile;
    if (whichFile(OPT_OPEN_WITH) == OPT_USE_INCLUDE)
        file = curSubFile;
    openWith(file);
}

void Gui::openRecentFile()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (action) {
    timer.start();
    QString fileName = action->data().toString();
    QFileInfo fileInfo(fileName);
    QDir::setCurrent(fileInfo.absolutePath());
    if (!openFile(fileName)) {
        emit messageSig(LOG_STATUS, QString("Load LDraw model file %1 aborted.").arg(fileName));
        return;
    }
    currentStep = nullptr;
    Paths::mkDirs();
    displayPage();
    enableActions();
    ldrawFile.showLoadMessages();
    emit messageSig(LOG_STATUS, gui->loadAborted() ?
                        QString("Load LDraw model file %1 aborted.").arg(fileName) :
                        QString("File loaded (%1 parts). %2")
                                .arg(ldrawFile.getPartCount())
                                .arg(elapsedTime(timer.elapsed())));
  }
}



void Gui::clearRecentFiles()
{
  QSettings Settings;
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,"LPRecentFileList"))) {
    QStringList files = Settings.value(QString("%1/%2").arg(SETTINGS,"LPRecentFileList")).toStringList();
    files.clear();
     Settings.setValue(QString("%1/%2").arg(SETTINGS,"LPRecentFileList"), files);
   }
  updateRecentFileActions();
}

bool Gui::loadFile(const QString &file)
{
    currentStep = nullptr;

    QString fileName = file;
    QFileInfo info(fileName);
    if (info.exists()) {
        timer.start();
        QDir::setCurrent(info.absolutePath());
        if (!openFile(fileName)) {
            emit messageSig(LOG_STATUS, QString("Load LDraw model file %1 aborted.").arg(fileName));
            return false;
        }
        // check if possible to load page number
        QSettings Settings;
        if (Settings.contains(QString("%1/%2").arg(DEFAULTS,SAVE_DISPLAY_PAGE_NUM_KEY))) {
            displayPageNum = Settings.value(QString("%1/%2").arg(DEFAULTS,SAVE_DISPLAY_PAGE_NUM_KEY)).toInt();
            Settings.remove(QString("%1/%2").arg(DEFAULTS,SAVE_DISPLAY_PAGE_NUM_KEY));
        }
        Paths::mkDirs();
        displayPage();
        enableActions();
        ldrawFile.showLoadMessages();
        emit messageSig(LOG_INFO_STATUS, gui->loadAborted() ?
                            QString("Load LDraw model file %1 aborted.").arg(fileName) :
                            QString("File loaded (%1 parts). %2")
                                    .arg(ldrawFile.getPartCount())
                                    .arg(elapsedTime(timer.elapsed())));
        return true;
    } else {
        emit messageSig(LOG_ERROR,QString("Unable to load file %1.").arg(fileName));
    }
    return false;
}

void Gui::enableWatcher()
{
#ifdef WATCHER
    if (curFile != "") {
      if (isMpd()) {
        watcher.addPath(curFile);
      }
      QStringList filePaths = ldrawFile.getSubFilePaths();
      filePaths.removeDuplicates();
      if (filePaths.size()) {
        for (QString filePath : filePaths) {
          watcher.addPath(filePath);
        }
      }
    }
#endif
}

void Gui::disableWatcher()
{
#ifdef WATCHER
    if (curFile != "") {
      if (isMpd()) {
        watcher.removePath(curFile);
      }
      QStringList filePaths = ldrawFile.getSubFilePaths();
      filePaths.removeDuplicates();
      if (filePaths.size()) {
        for (QString filePath : filePaths) {
          watcher.removePath(filePath);
        }
      }
    }
#endif
}

int Gui::whichFile(int option) {
    bool includeFile    = ldrawFile.isIncludeFile(curSubFile);
    bool dirtyUndoStack = ! undoStack->isClean();
    bool currentFile    = ! curFile.isEmpty();
    bool showDialog     = false;

    bool includeChecked = option == OPT_SAVE;
    bool currentChecked = !includeChecked;

    switch (option){
    case OPT_SAVE:
        showDialog = currentFile && includeFile && dirtyUndoStack;
        break;
    case OPT_SAVE_AS:
    case OPT_SAVE_COPY:
    case OPT_OPEN_WITH:
        showDialog = currentFile && includeFile;
        break;
    default:
        break;
    }

    if (showDialog && Preferences::modeGUI) {

        QDialog *dialog = new QDialog();
        dialog->setWindowTitle("File to Save");
        QFormLayout *form = new QFormLayout(dialog);

        QGroupBox *saveWhichGrpBox = new QGroupBox("Select which file to save");
        form->addWidget(saveWhichGrpBox);
        QFormLayout *saveWhichFrmLayout = new QFormLayout(saveWhichGrpBox);

        // current file
        QRadioButton * currentButton = new QRadioButton("", dialog);
        currentButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QFontMetrics currentMetrics(currentButton->font());
        QString elidedText = currentMetrics.elidedText(QFileInfo(curFile).fileName(),
                                                       Qt::ElideRight, currentButton->width());
        currentButton->setText(QString("Current file: %1").arg(elidedText));
        currentButton->setChecked(currentChecked);
        saveWhichFrmLayout->addRow(currentButton);

        // include file
        QRadioButton * includeButton = new QRadioButton("", dialog);
        includeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QFontMetrics includeMetrics(includeButton->font());
        elidedText = includeMetrics.elidedText(QFileInfo(curSubFile).fileName(),
                                               Qt::ElideRight, includeButton->width());
        includeButton->setText(QString("Include file: %1").arg(elidedText));
        includeButton->setChecked(includeChecked);
        saveWhichFrmLayout->addRow(includeButton);

        // button box
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                   Qt::Horizontal, dialog);
        form->addRow(&buttonBox);
        QObject::connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
        QObject::connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
        dialog->setMinimumWidth(250);

        if (dialog->exec() == QDialog::Accepted) {
            if (currentButton->isChecked())
                return OPT_USE_CURRENT;
            else
                return OPT_USE_INCLUDE;
        }
    }
    return OPT_NONE;
}

void Gui::save()
{
  disableWatcher();

  QString file = curFile;
  if (whichFile(OPT_SAVE) == OPT_USE_INCLUDE)
      file = curSubFile;

  if (file.isEmpty()) {
    saveAs();
  } else {
    saveFile(file);
  }

 enableWatcher();
}

void Gui::saveAs()
{
  disableWatcher();

  QString file = curFile;
  if (whichFile(OPT_SAVE_AS) == OPT_USE_INCLUDE)
      file = curSubFile;

  QString fileName = QFileDialog::getSaveFileName(this,tr("Save As"),file,tr("LDraw Files (*.mpd *.ldr *.dat);;All Files (*.*)"));
  if (fileName.isEmpty()) {
    return;
  }
  QFileInfo fileInfo(fileName);
  QString extension = fileInfo.suffix().toLower();

  if (extension == "mpd" ||
      extension == "ldr" ||
      extension == "dat" ||
      extension.isEmpty()) {
    saveFile(fileName);
    closeFile();
    openFile(fileName);
    displayPage();
  } else {
    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),
                              QMessageBox::tr("Unsupported LDraw file extension %1 specified.  File not saved.")
                                .arg(extension));

  }
  enableWatcher();
} 

void Gui::saveCopy()
{
    QString file = curFile;
    if (whichFile(OPT_SAVE_COPY) == OPT_USE_INCLUDE)
        file = curSubFile;

  QString fileName = QFileDialog::getSaveFileName(this,tr("Save As"),file,tr("LDraw Files (*.mpd *.ldr *.dat);;All Files (*.*)"));
  if (fileName.isEmpty()) {
    return;
  }

  QFileInfo fileInfo(fileName);
  QString extension = fileInfo.suffix().toLower();

  if (extension == "mpd" ||
      extension == "ldr" ||
      extension == "dat" ||
      extension.isEmpty()) {
    saveFile(fileName);
  } else {
    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),
                              QMessageBox::tr("Unsupported LDraw file extension %1 specified.  File not saved.")
                                .arg(extension));
  }
}

bool Gui::maybeSave(bool prompt, int sender /*SaveOnNone=0*/)
{
  QString senderLabel;
  bool proceed = true;
  SaveOnSender saveSender = SaveOnSender(sender);
  if (saveSender ==  SaveOnRedraw) {
      senderLabel = "redraw";
      proceed = Preferences::showSaveOnRedraw;
  } else
  if (saveSender ==  SaveOnUpdate) {
     senderLabel = "update";
     proceed = Preferences::showSaveOnUpdate;
  }

  if ( ! undoStack->isClean() && proceed) {
    if (Preferences::modeGUI && prompt) {
      // Get the application icon as a pixmap
      QPixmap _icon = QPixmap(":/icons/lpub96.png");
      if (_icon.isNull())
          _icon = QPixmap (":/icons/update.png");

      QMessageBoxResizable box;
      box.setWindowIcon(QIcon());
      box.setIconPixmap (_icon);
      box.setTextFormat (Qt::RichText);
      box.setWindowTitle(tr ("%1 Document").arg(VER_PRODUCTNAME_STR));
      box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      QString title = "<b>" + tr ("Document changes detected&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;") + "</b>";
      QString text = tr("The document has been modified.<br>"
                        "Do you want to save your changes?");
      box.setText (title);
      box.setInformativeText (text);
      box.setStandardButtons (QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
      box.setDefaultButton   (QMessageBox::Save);

      if (saveSender){
          QCheckBox *cb = new QCheckBox(tr("Do not show save changes on %1 message again.").arg(senderLabel));
          box.setCheckBox(cb);
          QObject::connect(cb, &QCheckBox::stateChanged, [&saveSender](int state) {
              bool checked = true;
              if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
                  checked = false;
              }
              if (saveSender == SaveOnRedraw) {
                  Preferences::setShowSaveOnRedrawPreference(checked);
              } else {
                  Preferences::setShowSaveOnUpdatePreference(checked);
              }
          });
      }

      int ExecReturn = box.exec();
      if (ExecReturn == QMessageBox::Save) {
        save();
      } else
      if (ExecReturn == QMessageBox::Cancel) {
        return false;
      }
    } else {
      save();
      emit messageSig(LOG_INFO,tr("Open document has been saved!"));
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
    statusBar()->showMessage(tr("File %1 saved").arg(QFileInfo(fileName).fileName()), 2000);
  }
  return rc;
}

// This call performs LPub3D file close operations - does not clear curFile
// use closeModelFile() to definitively close the current file in LPub3D
void Gui::closeFile()
{
  pa = sa = 0;
  ldrawFile.empty();
  editWindow->clearWindow();
  mpdCombo->clear();
  ClearPreviewWidget();
  mpdCombo->setEnabled(false);
  setGoToPageCombo->clear();
  setGoToPageCombo->setEnabled(false);
  setPageLineEdit->clear();
  setPageLineEdit->setEnabled(false);
  topOfPages.clear();
  pageSizes.clear();
  undoStack->clear();
  pageDirection = PAGE_NEXT;
  emit clearViewerWindowSig();
  emit updateAllViewsSig();
  Preferences::preferredRendererPreferences();
  Preferences::fadestepPreferences();
  Preferences::highlightstepPreferences();
  if (Preferences::enableFadeSteps || Preferences::enableHighlightStep)
      ldrawColourParts.clearGeneratedColorParts();
  submodelIconsLoaded = false;
  SetSubmodelIconsLoaded(submodelIconsLoaded);
  if (!curFile.isEmpty())
      emit messageSig(LOG_DEBUG, QString("File closed - %1.").arg(curFile));
}

// This call definitively closes and clears from curFile, the current model file
void Gui::closeModelFile(){
  if (maybeSave() && saveBuildModification()) {
    disableWatcher();
    QString topModel = ldrawFile.topLevelFile();
    curFile.clear();       // clear file from curFile here...
    //3D Viewer
    if (Preferences::modeGUI) {
        enableBuildModMenuAndActions();
        enable3DActions(false);
        emit clearViewerWindowSig();
        emit updateAllViewsSig();
    }
    // Editor
    emit clearEditorWindowSig();
    // Gui
    clearPage(KpageView,KpageScene,true);
    disableActions();
    disableActions2();
    closeFile();           // perform LPub3D file close operations here...
    editModeWindow->close();
    editModelFileAct->setText(tr("Edit current model file"));
    editModelFileAct->setStatusTip(tr("Edit loaded LDraw model file with detached LDraw Editor"));
    emit messageSig(LOG_INFO, QString("Model unloaded. File closed - %1.").arg(topModel));

    QString windowName = VER_FILEDESCRIPTION_STR;
    QString windowVersion;
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
    windowVersion = QString("v%1 r%2 (%3)")
            .arg(VER_PRODUCTVERSION_STR)
            .arg(VER_REVISION_STR)
            .arg(VER_BUILD_TYPE_STR);
#else
    windowVersion = QString("v%1%2")
            .arg(VER_PRODUCTVERSION_STR)
            .arg(QString(VER_REVISION_STR).toInt() ?
                     QString(" r%1").arg(VER_REVISION_STR) :
                     QString());
#endif

    setWindowTitle(tr("%1[*] - %2").arg(windowName).arg(windowVersion));
  }
}

/***************************************************************************
 * File opening closing stuff
 **************************************************************************/

bool Gui::openFile(QString &fileName)
{

  if (Preferences::modeGUI)
    waitingSpinner->start();

  disableWatcher();

  pageDirection = PAGE_NEXT;
  mloadingFile = true;
  parsedMessages.clear();
  clearPage(KpageView,KpageScene,true);
  closeFile();
  if (GetViewPieceIcons())
      mPliIconsPath.clear();
  emit messageSig(LOG_INFO_STATUS, QString("Loading LDraw model file '%1'...").arg(fileName));
  setPageLineEdit->setText(QString("Loading..."));
  setGoToPageCombo->addItem(QString("Loading..."));
  mpdCombo->addItem(QString("Loading..."));
  if (ldrawFile.loadFile(fileName) != 0) {
      closeModelFile();
      return false;
  }
  displayPageNum = 1 + pa;
  QFileInfo info(fileName);
  QDir::setCurrent(info.absolutePath());
  Paths::mkDirs();
  editModelFileAct->setText(tr("Edit %1").arg(info.fileName()));
  editModelFileAct->setStatusTip(tr("Edit loaded LDraw model file %1 with detached LDraw Editor").arg(info.fileName()));
  setupFadeSteps = setFadeStepsFromCommand();
  setupHighlightStep = setHighlightStepFromCommand();
  if (setupFadeSteps || setupHighlightStep) {
    ldrawColorPartsLoad();
    writeGeneratedColorPartsToTemp();
    // archive fade/highlight colour parts
    partWorkerLDSearchDirs.addCustomDirs();
    if (setupFadeSteps) {
      if (Preferences::enableImageMatting)
        LDVImageMatte::clearMatteCSIImages();
      emit messageSig(LOG_INFO_STATUS, "Loading fade color parts...");
      partWorkerLDSearchDirs.setDoFadeStep(true);
      processFadeColourParts(false/*overwrite*/, !Preferences::enableFadeSteps/*setup*/);
    }
    if (setupHighlightStep) {
      emit messageSig(LOG_INFO_STATUS, "Loading highlight color parts...");
      partWorkerLDSearchDirs.setDoHighlightStep(true);
      processHighlightColourParts(false/*overwrite*/, !Preferences::enableHighlightStep/*setup*/);
    }
  }
  QString previewLoadPath = QDir::toNativeSeparators(QString("%1/%2").arg(QDir::currentPath()).arg(Paths::tmpDir));
  lcSetProfileString(LC_PROFILE_PREVIEW_LOAD_PATH, previewLoadPath);
  emit messageSig(LOG_INFO, "Loading user interface items...");
  attitudeAdjustment();
  mpdCombo->clear();
  mpdCombo->addItems(ldrawFile.subFileOrder());
  mpdCombo->setToolTip(tr("Current Submodel: %1").arg(mpdCombo->currentText()));
  connect(mpdCombo,SIGNAL(activated(int)), this,    SLOT(mpdComboChanged(int)));
  connect(setGoToPageCombo,SIGNAL(activated(int)), this, SLOT(setGoToPage(int)));
  setCurrentFile(fileName);
  undoStack->setClean();
  curFile = fileName;
  for (int i = 0; i < numPrograms; i++) {
    QFileInfo fileInfo(programEntries.at(i).split("|").last());
    openWithActList[i]->setStatusTip(QString("Open %1 with %2")
                                  .arg(QFileInfo(curFile).fileName())
                                  .arg(fileInfo.fileName()));
  }

  insertFinalModelStep();    //insert final fully coloured model if fadeStep turned on

  generateCoverPages();  //auto-generate cover page

  enableWatcher();

  defaultResolutionType(Preferences::preferCentimeters);

  emit messageSig(LOG_INFO, QString("Open file '%1' completed.").arg(fileName));
  return true;
}

void Gui::updateRecentFileActions()
{
  QSettings Settings;
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,"LPRecentFileList"))) {
    QStringList files = Settings.value(QString("%1/%2").arg(SETTINGS,"LPRecentFileList")).toStringList();

    int numRecentFiles = qMin(files.size(), int(MaxRecentFiles));

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
      recentFilesActs[i]->setStatusTip(fileInfo.absoluteFilePath());
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
  QString windowName;
  if (fileName.size() == 0) {
    windowName = VER_FILEDESCRIPTION_STR;
  } else {
    QFileInfo fileInfo(fileName);
    windowName = fileInfo.fileName();
  }
  QString windowVersion;
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
  windowVersion = QString("%1 v%2 r%3 (%4)")
                          .arg(VER_PRODUCTNAME_STR)
                          .arg(VER_PRODUCTVERSION_STR)
                          .arg(VER_REVISION_STR)
                          .arg(VER_BUILD_TYPE_STR);
#else
  windowVersion = QString("%1 v%2%3")
                          .arg(VER_PRODUCTNAME_STR)
                          .arg(VER_PRODUCTVERSION_STR)
                          .arg(QString(VER_REVISION_STR).toInt() ?
                                   QString(" r%1").arg(VER_REVISION_STR) :
                                   QString());
#endif

  setWindowTitle(tr("%1[*] - %2").arg(windowName).arg(windowVersion));

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

void Gui::loadLastOpenedFile(){
    updateRecentFileActions();
    if (recentFilesActs[0]) {
        loadFile(recentFilesActs[0]->data().toString());
    }
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

  QMessageBoxResizable box;
  box.setWindowIcon(QIcon());
  box.setIconPixmap (_icon);
  box.setTextFormat (Qt::RichText);
  box.setWindowTitle(tr ("%1 File Change").arg(VER_PRODUCTNAME_STR));
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  QString title = "<b>" + tr ("External change detected") + "</b>";
  QString text = tr("\"%1\" contents were changed by an external source. Reload?").arg(path);
  box.setText (title);
  box.setInformativeText (text);
  box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
  box.setDefaultButton   (QMessageBox::Yes);

  if (box.exec() == QMessageBox::Yes) {
    changeAccepted = true;
    int goToPage = displayPageNum;
    QString fileName = path;
    if (ldrawFile.isIncludeFile(QFileInfo(path).fileName()))
      fileName = curFile;
    if (!openFile(fileName)) {
      emit messageSig(LOG_STATUS, QString("Load LDraw model file %1 aborted.").arg(fileName));
      return;
    }
    displayPageNum = goToPage;
    displayPage();
  }
}

void Gui::writeGeneratedColorPartsToTemp() {
  emit messageSig(LOG_INFO_STATUS, "Writing generated color parts to tmp folder...");
  LDrawFile::_currentLevels.clear();
  for (int i = 0; i < ldrawFile._subFileOrder.size(); i++) {
    QString fileName = ldrawFile._subFileOrder[i];
    if (ldrawColourParts.isLDrawColourPart(fileName)) {
      ldrawFile.normalizeHeader(fileName);
      QStringList content = ldrawFile.contents(fileName);
      emit messageSig(LOG_INFO, "Writing generated part to temp directory: " + fileName + "...");
      writeToTmp(fileName,content);
    }
  }
  LDrawFile::_currentLevels.clear();
}

bool Gui::setFadeStepsFromCommand()
{
  QString result;
  Where topLevelModel(gui->topLevelFile(),0);
  QRegExp fadeRx = QRegExp("FADE_STEP ENABLED\\s*(GLOBAL)?\\s*TRUE");
  bool setupFadeSteps = Preferences::enableFadeSteps;
  if (!setupFadeSteps)
    setupFadeSteps = stepContains(topLevelModel,fadeRx,result,1);

  if (!setupFadeSteps) {
    fadeRx.setPattern("FADE_STEP SETUP\\s*(GLOBAL)?\\s*TRUE");
    setupFadeSteps = stepContains(topLevelModel,fadeRx,result,1);
  }

  emit messageSig(LOG_INFO_STATUS,QString("Fade Previous Steps %1.")
                                          .arg(Preferences::enableFadeSteps ? "is ON" : setupFadeSteps ? "Setup is Enabled" : "is OFF"));

  fadeRx.setPattern("FADE_STEP OPACITY\\s*(?:GLOBAL)?\\s*(\\d+)");
  stepContains(topLevelModel,fadeRx,result,1);
  if (!result.isEmpty()) {
    bool ok = result.toInt(&ok);
    int fadeStepsOpacityCompare = Preferences::fadeStepsOpacity;
    Preferences::fadeStepsOpacity = ok ? result.toInt() : FADE_OPACITY_DEFAULT;
    bool fadeStepsOpacityChanged = Preferences::fadeStepsOpacity != fadeStepsOpacityCompare;
    if (fadeStepsOpacityChanged)
      emit messageSig(LOG_INFO,QString("Fade Step Transparency changed from %1 to %2 percent")
                                       .arg(fadeStepsOpacityCompare)
                                       .arg(Preferences::fadeStepsOpacity));
  }

  fadeRx.setPattern("FADE_STEP COLOR\\s*(?:GLOBAL)?\\s*\"(\\w+)\"");
  stepContains(topLevelModel,fadeRx,result,1);
  if (!result.isEmpty()) {
    QColor ParsedColor = LDrawColor::color(result);
    bool fadeStepsUseColorCompare = Preferences::fadeStepsUseColour;
    Preferences::fadeStepsUseColour = ParsedColor.isValid();
    if (Preferences::fadeStepsUseColour != fadeStepsUseColorCompare)
      emit messageSig(LOG_INFO,QString("Use Fade Color is %1")
                                       .arg(Preferences::fadeStepsUseColour ? "ON" : "OFF"));
    QString fadeStepsColourCompare = Preferences::validFadeStepsColour;
    Preferences::validFadeStepsColour = ParsedColor.isValid() ? result : Preferences::validFadeStepsColour;
    if (QString(Preferences::validFadeStepsColour).toLower() != fadeStepsColourCompare.toLower())
      emit messageSig(LOG_INFO,QString("Fade Step Color preference changed from %1 to %2")
                                       .arg(fadeStepsColourCompare.replace("_"," "))
                                       .arg(QString(Preferences::validFadeStepsColour).replace("_"," ")));
  }

  return setupFadeSteps;
}

bool Gui::setHighlightStepFromCommand()
{
  QString result;
  Where topLevelModel(gui->topLevelFile(),0);
  QRegExp highlightRx = QRegExp("HIGHLIGHT_STEP ENABLED\\s*(GLOBAL)?\\s*TRUE");
  bool setupHighlightStep = Preferences::enableHighlightStep;
  if (!setupHighlightStep)
    setupHighlightStep = stepContains(topLevelModel,highlightRx,result,1);

  if (!Preferences::enableHighlightStep) {
    highlightRx.setPattern("HIGHLIGHT_STEP SETUP\\s*(GLOBAL)?\\s*TRUE");
    setupHighlightStep = stepContains(topLevelModel,highlightRx,result,1);
  }

  messageSig(LOG_INFO,QString("Highlight Current Step %1.")
                              .arg(Preferences::enableHighlightStep ? "is ON" : setupHighlightStep ? "Setup is Enabled" : "is OFF"));

  highlightRx.setPattern("HIGHLIGHT_STEP COLOR\\s*(?:GLOBAL)?\\s*\"(0x|#)([\\da-fA-F]+)\"");
  if (stepContains(topLevelModel,highlightRx,result)) {
    result = QString("%1%2").arg(highlightRx.cap(1),highlightRx.cap(2));
    QColor ParsedColor = QColor(result);
    QString highlightStepColourCompare = Preferences::highlightStepColour;
    Preferences::highlightStepColour = ParsedColor.isValid() ? result : Preferences::validFadeStepsColour;
    bool highlightStepColorChanged = QString(Preferences::highlightStepColour).toLower() != highlightStepColourCompare.toLower();
    if (highlightStepColorChanged)
      messageSig(LOG_INFO,QString("Highlight Step Color preference changed from %1 to %2")
                                  .arg(highlightStepColourCompare)
                                  .arg(Preferences::highlightStepColour));
  }

  return setupHighlightStep;
}

bool Gui::setPreferredRendererFromCommand(const QString &preferredRenderer)
{
  if (preferredRenderer.isEmpty())
      return false;

  bool useLDVSingleCall = false;
  bool useLDVSnapShotList = false;
  bool useNativeRenderer = false;
  bool useNativeGenerator = true;
  bool rendererChanged = false;
  bool renderFlagChanged = false;

  QString message;
  QString renderer;
  QString command = preferredRenderer.toLower();
  if (command == "native") {
    renderer = RENDERER_NATIVE;
    useNativeRenderer = true;
  } else if (command == "ldview") {
    renderer = RENDERER_LDVIEW;
  } else if ((useLDVSingleCall = command == "ldview-sc")) {
    renderer = RENDERER_LDVIEW;
  } else if ((useLDVSnapShotList = command == "ldview-scsl")) {
    renderer = RENDERER_LDVIEW;
  } else if (command == "ldglite") {
    renderer = RENDERER_LDGLITE;
  } else if (command == "povray") {
    renderer = RENDERER_POVRAY;
  } else if (command == "povray-ldv") {
    renderer = RENDERER_POVRAY;
    useNativeGenerator = false;
  } else {
    emit messageSig(LOG_ERROR,QString("Invalid renderer console command option specified: '%1'.").arg(renderer));
    return rendererChanged;
  }

  rendererChanged = Preferences::preferredRenderer != renderer;

  if (renderer == RENDERER_LDVIEW) {
    if (Preferences::enableLDViewSingleCall != useLDVSingleCall) {
      message = QString("Renderer preference use LDView Single Call changed from %1 to %2.")
                        .arg(Preferences::enableLDViewSingleCall ? "Yes" : "No")
                        .arg(useLDVSingleCall ? "Yes" : "No");
      emit messageSig(LOG_INFO,message);
      Preferences::enableLDViewSingleCall = useLDVSingleCall;
      renderFlagChanged = true;
    }
    if (Preferences::enableLDViewSnaphsotList != useLDVSnapShotList) {
      message = QString("Renderer preference use LDView Snapshot List changed from %1 to %2.")
                        .arg(Preferences::enableLDViewSnaphsotList ? "Yes" : "No")
                        .arg(useLDVSingleCall ? "Yes" : "No");
      emit messageSig(LOG_INFO,message);
      Preferences::enableLDViewSnaphsotList = useLDVSnapShotList;
      if (useLDVSnapShotList)
          Preferences::enableLDViewSingleCall = true;
      if (!renderFlagChanged)
        renderFlagChanged = true;
    }
  } else if (renderer == RENDERER_POVRAY) {
    if (Preferences::useNativePovGenerator != useNativeGenerator) {
      message = QString("Renderer preference POV file generator changed from %1 to %2.")
                        .arg(Preferences::useNativePovGenerator ? RENDERER_NATIVE : RENDERER_LDVIEW)
                        .arg(useNativeGenerator ? RENDERER_NATIVE : RENDERER_LDVIEW);
      emit messageSig(LOG_INFO,message);
      Preferences::useNativePovGenerator = useNativeGenerator;
      if (!renderFlagChanged)
        renderFlagChanged = true;
    }
  }

  if (rendererChanged || renderFlagChanged) {
    message = QString("Renderer preference changed from %1 to %2%3.")
                      .arg(Preferences::preferredRenderer)
                      .arg(renderer)
                      .arg(renderer == RENDERER_POVRAY ? QString(" (POV file generator is %1)")
                                                                 .arg(Preferences::useNativePovGenerator ? RENDERER_NATIVE : RENDERER_LDVIEW) :
                           renderer == RENDERER_LDVIEW ? useLDVSingleCall ?
                                                         useLDVSnapShotList ? QString(" (Single Call using Export File List)") :
                                                                              QString(" (Single Call)") :
                                                                              QString() : QString());
    emit messageSig(LOG_INFO,message);
  }

  if (rendererChanged) {
    Preferences::preferredRenderer   = renderer;
    Preferences::usingNativeRenderer = useNativeRenderer;
    Render::setRenderer(Preferences::preferredRenderer);
    Preferences::preferredRendererPreferences(true/*global*/);
    Preferences::updatePOVRayConfigFiles();
  }

  return rendererChanged;
}

//void Gui::dropEvent(QDropEvent* event)
//{
//  const QMimeData* mimeData = event->mimeData();

//  if (mimeData->hasUrls()) {

//      QList<QUrl> urlList = mimeData->urls();

//      // load only the first file in the list;
//      QString fileName = urlList.at(0).toLocalFile();

//      if (urlList.size() > 1) {
//          QMessageBox::warning(nullptr,
//                               QMessageBox::tr(VER_PRODUCTNAME_STR),
//                               QMessageBox::tr("%1 files selected.\nOnly file %2 will be opened.")
//                               .arg(urlList.size())
//                               .arg(fileName));
//        }

//      openDropFile(fileName);
//      event->acceptProposedAction();
//    }
//}

//void Gui::dragEnterEvent(QDragEnterEvent* event)
//{
//  if (event->mimeData()->hasUrls()) {
//      event->acceptProposedAction();
//    }
//}

//void Gui::dragMoveEvent(QDragMoveEvent* event)
//{
//  if (event->mimeData()->hasUrls()) {
//      event->acceptProposedAction();
//    }
//}

//void Gui::dragLeaveEvent(QDragLeaveEvent* event)
//{
//  event->accept();
//}

QString Gui::elapsedTime(const qint64 &duration) {

  qint64 elapsed = duration;
  int milliseconds = int(elapsed % 1000);
  elapsed /= 1000;
  int seconds = int(elapsed % 60);
  elapsed /= 60;
  int minutes = int(elapsed % 60);
  elapsed /= 60;
  int hours = int(elapsed % 24);

  return QString("Elapsed time: %1%2%3")
                 .arg(hours >   0 ?
                                QString("%1 %2 ")
                                        .arg(hours)
                                        .arg(hours > 1 ? "hours" : "hour")
                                : QString())
                 .arg(minutes > 0 ?
                                QString("%1 %2 ")
                                        .arg(minutes)
                                        .arg(minutes > 1 ? "minutes" : "minute")
                                : QString())
                 .arg(QString("%1.%2 %3")
                              .arg(seconds)
                              .arg(milliseconds,3,10,QLatin1Char('0'))
                              .arg(seconds > 1 ? "seconds" : "second"));
}
