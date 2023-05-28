/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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

    QElapsedTimer timer;
    timer.start();

    QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
      Settings.setValue(QString("%1/%2").arg(SETTINGS,"ProjectsPath"),fileInfo.path());
      if (!openFile(fileName))
          return;
      displayPage();
      enableActions();
      emit lpub->messageSig(LOG_STATUS, tr("Loaded LDraw file %1 (%2 pages, %3 parts). %4")
                                           .arg(fileInfo.fileName())
                                           .arg(maxPages)
                                           .arg(lpub->ldrawFile.getPartCount())
                                           .arg(elapsedTime(timer.elapsed())));
      return;
    }
  }
  return;
}

void Gui::openDropFile(QString &fileName){

  if (maybeSave() && saveBuildModification()) {
      QElapsedTimer timer;
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
          if (!openFile(fileName))
              return;
          displayPage();
          enableActions();
          emit lpub->messageSig(LOG_STATUS, tr("Loaded LDraw file %1 (%2 pages, %3 parts). %4")
                                               .arg(fileInfo.fileName())
                                               .arg(maxPages)
                                               .arg(lpub->ldrawFile.getPartCount())
                                               .arg(elapsedTime(timer.elapsed())));
        } else {
          QString noExtension;
          if (extension.isEmpty())
              noExtension = tr("<br>No file exension specified. Set the file extension to .mpd,.ldr, or .dat.");
          emit messageSig(LOG_ERROR, tr("File not supported!<br>%1%2")
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
            m->showMessage(tr("Failed to open folder!\n%2").arg(path));
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
    if (sender() == getAct("openWorkingFolderAct.1")) {
        if (!getCurFile().isEmpty())
            openFolderSelect(getCurFile());
    } else if (sender() == getAct("openParameterFileFolderAct.1")) {
        openFolderSelect(QDir::toNativeSeparators(Preferences::pliControlFile));
    }
}

void Gui::updateOpenWithActions()
{
    QSettings Settings;
    QString const openWithProgramListKey("OpenWithProgramList");
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,openWithProgramListKey))) {

      programEntries = Settings.value(QString("%1/%2").arg(SETTINGS,openWithProgramListKey)).toStringList();

      numPrograms = qMin(programEntries.size(), Preferences::maxOpenWithPrograms);

      if (Preferences::debugLogging) {
          emit lpub->messageSig(LOG_DEBUG, tr("- Number of default openWith programs: %1").arg(numPrograms));
      }

      QString programData, programName, programPath;

      auto getProgramIcon = [&programPath] ()
      {
          const QString programName = QString("%1icon.png").arg(QFileInfo(programPath).baseName());
          const QString iconFile = QString("%1/%2").arg(QDir::tempPath()).arg(programName);
          if (!QFileInfo(iconFile).exists()) {
              QFileInfo programInfo(programPath);
              QFileSystemModel *fsModel = new QFileSystemModel;
              fsModel->setRootPath(programInfo.path());
              QIcon fileIcon = fsModel->fileIcon(fsModel->index(programInfo.filePath()));
              QPixmap iconPixmap = fileIcon.pixmap(16,16);
              if (!iconPixmap.save(iconFile))
                  emit lpub->messageSig(LOG_INFO,tr("- Could not save program file icon: %1").arg(iconFile));
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
        if (fileInfo.exists() && fileInfo.isFile()) {
          programName = programEntries.at(i).split("|").first();
          QString text = programName;
          if (text.isEmpty())
              text = tr("&%1 %2").arg(i + 1).arg(fileInfo.fileName());
          openWithActList[i]->setText(text);
          openWithActList[i]->setData(programData); // includes arguments
          openWithActList[i]->setIcon(getProgramIcon());
          openWithActList[i]->setStatusTip(tr("Open current file with %2").arg(fileInfo.fileName()));
          openWithActList[i]->setVisible(true);
          i++;
          if (Preferences::debugLogging) {
              lpub->messageSig(LOG_DEBUG, tr("- Add openWith program: %1. %2").arg(i).arg(programData));
          }
        } else {
          programEntries.removeOne(programEntries.at(i));
          --numPrograms;
        }
      }

      // add system editor if exits
      if (!Preferences::systemEditor.isEmpty()) {
        QFileInfo fileInfo(Preferences::systemEditor);
        if (fileInfo.exists() && fileInfo.isFile()) {
          QString arguments;
          if (Preferences::usingNPP)
            arguments = QLatin1String(WINDOWS_NPP_LPUB3D_UDL_ARG);
          const int i = numPrograms;
          programPath = fileInfo.absoluteFilePath();
          programName = fileInfo.completeBaseName();
          programName.replace(programName[0],programName[0].toUpper());
          programData = QString("'%1' %2").arg(programPath).arg(arguments);
          QString text = programName;
          if (text.isEmpty())
              text = tr("&%1 %2").arg(i + 1).arg(fileInfo.fileName());
          openWithActList[i]->setText(text);
          openWithActList[i]->setData(programData);
          openWithActList[i]->setIcon(getProgramIcon());
          openWithActList[i]->setStatusTip(tr("Open current file with %2").arg(fileInfo.fileName()));
          openWithActList[i]->setVisible(true);
          programEntries.append(QString("%1|%2").arg(programName).arg(programData));
          numPrograms = programEntries.size();
          if (Preferences::debugLogging) {
              lpub->messageSig(LOG_DEBUG, tr("- Add openWith system editor: %1").arg(programData));
          }
        }
      }

      if (Preferences::debugLogging) {
          lpub->messageSig(LOG_DEBUG, tr("- Number of loaded openWith programs: %1").arg(numPrograms));
      }

      // hide empty program actions - redundant
      for (int j = numPrograms; j < Preferences::maxOpenWithPrograms; j++)
        openWithActList[j]->setVisible(false);

      // clear old menu actions
      if (openWithMenu->actions().size())
          openWithMenu->clear();

      // add menu actions from updated list
      for (int k = 0; k < numPrograms; k++)
        openWithMenu->addAction(openWithActList.at(k));

      // enable menu accordingly
      openWithMenu->setEnabled(numPrograms > 0);
    }
}

void Gui::openWithSetup()
{
    OpenWithProgramDialogGui openWithProgramDialogGui;
    openWithProgramDialogGui.setOpenWithProgram();
    updateOpenWithActions();
}

void Gui::openWithProgramAndArgs(QString &program, QStringList &arguments)
{
    QRegExp quoteRx("\"|'");
    QString valueAt0 = program.at(0);
    bool inside = valueAt0.contains(quoteRx);                             // true if the first character is " or '
    QStringList list = program.split(quoteRx, SkipEmptyParts);            // Split by " or '
    if (list.size() == 1) {
        program = list.first();
    } else {
        QStringList values;
        for (QString &item : list) {
            if (inside) {                                                 // If 's' is inside quotes ...
                values.append(item);                                      // ... get the whole string
            } else {                                                      // If 's' is outside quotes ...
                values.append(item.split(" ", SkipEmptyParts));           // ... get the split string
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

    if (action)
        program = action->data().toString();

    if (program.isEmpty()) {
        program = Preferences::systemEditor;
        if (!program.isEmpty()) {
            openWithProgramAndArgs(program,arguments);
        }
#ifdef Q_OS_MACOS
        else {
            program = QString("open");
            arguments.prepend("-e");
        }
#else
        else {
            QDesktopServices::openUrl(QUrl("file:///"+filePath, QUrl::TolerantMode));
        }
#endif
    } else {
        openWithProgramAndArgs(program,arguments);
    }
    qint64 pid;
    QString workingDirectory = QDir::currentPath() + QDir::separator();
    QProcess::startDetached(program, arguments, workingDirectory, &pid);
    emit lpub->messageSig(LOG_INFO, tr("Launched %1 with pid=%2 %3%4...")
                                        .arg(QFileInfo(filePath).fileName()).arg(pid)
                                        .arg(QFileInfo(program).fileName())
                                        .arg(arguments.size() ? " "+arguments.join(" ") : ""));

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
    QElapsedTimer timer;
    timer.start();
    QString fileName = action->data().toString();
    QFileInfo fileInfo(fileName);
    if (!openFile(fileName))
        return;
    Paths::mkDirs();
    displayPage();
    enableActions();
    emit lpub->messageSig(LOG_STATUS, tr("Loaded LDraw file %1 (%2 pages, %3 parts). %4")
                                         .arg(fileInfo.fileName())
                                         .arg(maxPages)
                                         .arg(lpub->ldrawFile.getPartCount())
                                         .arg(elapsedTime(timer.elapsed())));
  }
}

void Gui::clearRecentFiles()
{
  QSettings Settings;
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,LPUB3D_RECENT_FILES_KEY))) {
    QStringList files = Settings.value(QString("%1/%2").arg(SETTINGS,LPUB3D_RECENT_FILES_KEY)).toStringList();
    files.clear();
    Settings.setValue(QString("%1/%2").arg(SETTINGS,LPUB3D_RECENT_FILES_KEY), files);
  }
  updateRecentFileActions();
}

bool Gui::loadFile(const QString &file)
{
    return loadFile(file, false/*commandLine*/);
}

bool Gui::loadFile(const QString &file, bool console)
{
    if(Gui::resetCache) {
        emit lpub->messageSig(LOG_INFO,tr("Reset parts cache specified."));
        resetModelCache(QFileInfo(file).absoluteFilePath(), console);
    }

    QString fileName = file;
    QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
        QElapsedTimer timer;
        timer.start();
        if (!openFile(fileName)) {
            emit fileLoadedSig(false);
            return false;
        }
        // check if possible to load page number
        QSettings Settings;
        int inputPageNum = displayPageNum;
        if (Settings.contains(QString("%1/%2").arg(DEFAULTS,SAVE_DISPLAY_PAGE_NUM_KEY))) {
            inputPageNum = Settings.value(QString("%1/%2").arg(DEFAULTS,SAVE_DISPLAY_PAGE_NUM_KEY)).toInt();
            Settings.remove(QString("%1/%2").arg(DEFAULTS,SAVE_DISPLAY_PAGE_NUM_KEY));
        }
        Paths::mkDirs();
        cyclePageDisplay(inputPageNum);
        enableActions();
        emit lpub->messageSig(LOG_STATUS, tr("Loaded LDraw file %1 (%2 pages, %3 parts). %4")
                                             .arg(fileInfo.fileName())
                                             .arg(maxPages)
                                             .arg(lpub->ldrawFile.getPartCount())
                                             .arg(elapsedTime(timer.elapsed())));
        emit fileLoadedSig(true);
        return true;
    } else {
        emit messageSig(LOG_ERROR,tr("Unable to load file %1.").arg(fileName));
    }
    emit fileLoadedSig(false);
    return false;
}

void Gui::enableWatcher()
{
#ifdef WATCHER
    if (curFile != "") {
      watcher.addPath(curFile);
      QStringList filePaths = lpub->ldrawFile.getSubFilePaths();
      filePaths.removeDuplicates();
      if (filePaths.size()) {
        for (QString &filePath : filePaths) {
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
      watcher.removePath(curFile);
      QStringList filePaths = lpub->ldrawFile.getSubFilePaths();
      filePaths.removeDuplicates();
      if (filePaths.size()) {
        for (QString &filePath : filePaths) {
          watcher.removePath(filePath);
        }
      }
    }
#endif
}

int Gui::whichFile(int option) {
    bool includeFile    = lpub->ldrawFile.isIncludeFile(curSubFile);
    bool dirtyUndoStack = ! undoStack->isClean();
    bool curFileExists    = ! curFile.isEmpty();
    bool showDialog     = false;

    bool includeChecked = option == OPT_SAVE;
    bool currentChecked = !includeChecked;

    switch (option){
    case OPT_SAVE:
        showDialog = curFileExists && includeFile && dirtyUndoStack;
        break;
    case OPT_SAVE_AS:
    case OPT_SAVE_COPY:
    case OPT_OPEN_WITH:
        showDialog = curFileExists && includeFile;
        break;
    default:
        break;
    }

    if (showDialog && Preferences::modeGUI) {

        QDialog *dialog = new QDialog();
        dialog->setWindowTitle(tr("File to Save"));
        QFormLayout *form = new QFormLayout(dialog);

        QGroupBox *saveWhichGrpBox = new QGroupBox(tr("Select which file to save"));
        form->addWidget(saveWhichGrpBox);
        QFormLayout *saveWhichFrmLayout = new QFormLayout(saveWhichGrpBox);

        // current file
        QRadioButton * currentButton = new QRadioButton("", dialog);
        currentButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QFontMetrics currentMetrics(currentButton->font());
        QString elidedText = currentMetrics.elidedText(QFileInfo(curFile).fileName(),
                                                       Qt::ElideRight, currentButton->width());
        currentButton->setText(tr("Current file: %1").arg(elidedText));
        currentButton->setChecked(currentChecked);
        saveWhichFrmLayout->addRow(currentButton);

        // include file
        QRadioButton * includeButton = new QRadioButton("", dialog);
        includeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QFontMetrics includeMetrics(includeButton->font());
        elidedText = includeMetrics.elidedText(QFileInfo(curSubFile).fileName(),
                                               Qt::ElideRight, includeButton->width());
        includeButton->setText(tr("Include file: %1").arg(elidedText));
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
    int loadMsgType = Preferences::ldrawFilesLoadMsgs;
    if (loadMsgType)
        Preferences::ldrawFilesLoadMsgs = static_cast<int>(NEVER_SHOW);
    openFile(fileName);
    if (loadMsgType)
        Preferences::ldrawFilesLoadMsgs = loadMsgType;
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
  SaveOnSenderType saveSender = SaveOnSenderType(sender);
  if (saveSender ==  SaveOnRedraw) {
      senderLabel = QLatin1String("redraw");
      proceed = Preferences::showSaveOnRedraw;
  } else
  if (saveSender ==  SaveOnUpdate) {
     senderLabel = QLatin1String("update");
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
        saveBuildModification();
        save();
      } else
      if (ExecReturn == QMessageBox::Cancel) {
        return false;
      }
    } else {
      save();
      emit lpub->messageSig(LOG_INFO,tr("Open document has been saved!"));
    }
  }
  return true;
}

bool Gui::saveFile(const QString &fileName)
{
  bool rc;
  rc = lpub->ldrawFile.saveFile(fileName);
  setCurrentFile(fileName);
  undoStack->setClean();
  if (rc) {
    int lines = lpub->ldrawFile.savedLines();
    const QString message = tr("File %1 saved (%2 lines)")
                                .arg(QFileInfo(fileName).fileName()).arg(lines);
    emit lpub->messageSig(LOG_INFO, message);
    statusBar()->showMessage(message, 2000);
  }
  return rc;
}

// This call performs LPub3D file close operations - does not clear curFile
// use closeModelFile() to definitively close the current file in LPub3D
void Gui::closeFile()
{
  pa = sa = 0;
  buildModJumpForward = false;
  pageDirection = PAGE_NEXT;
  pageProcessRunning = PROC_NONE;
  lpub->ldrawFile.empty();
  editWindow->clearWindow();
  mpdCombo->clear();
  mpdCombo->setEnabled(false);
  setGoToPageCombo->clear();
  setGoToPageCombo->setEnabled(false);
  setPageLineEdit->clear();
  setPageLineEdit->setEnabled(false);
  topOfPages.clear();
  pageSizes.clear();
  undoStack->clear();
  pageDirection = PAGE_NEXT;
  buildModJumpForward = false;
  setAbortProcess(false);
  Preferences::resetFadeSteps();
  Preferences::resetHighlightStep();
  Preferences::resetPreferredRenderer();
  Preferences::unsetBuildModifications();
  LDrawColor::removeUserDefinedColors();
  //Visual Editor
  if (Preferences::modeGUI) {
      enableVisualBuildModification();
      enable3DActions(false);
      emit clearViewerWindowSig();
      emit updateAllViewsSig();
  }
  lpub->SetStudStyle(nullptr, true/*reload*/);
  lpub->SetAutomateEdgeColor(nullptr);
  Preferences::preferredRendererPreferences();
  Preferences::fadestepPreferences();
  Preferences::highlightstepPreferences();
  if (!Preferences::enableFadeSteps && !Preferences::enableHighlightStep) {
      LDrawColourParts::clearGeneratedColorParts();
      partWorkerLDSearchDirs.removeCustomDirs();
  }
  submodelIconsLoaded = false;
  SetSubmodelIconsLoaded(submodelIconsLoaded);
  if (!curFile.isEmpty())
      emit lpub->messageSig(LOG_DEBUG, tr("File closed - %1.").arg(curFile));
  getAct("loadStatusAct.1")->setEnabled(false);
  ReloadVisualEditor();
}

// This call definitively closes and clears from curFile, the current model file
void Gui::closeModelFile()
{
  if (maybeSave()) {
    disableWatcher();
    QString topModel = lpub->ldrawFile.topLevelFile();
    curFile.clear();       // clear file from curFile here...
    // Editor
    emit clearEditorWindowSig();
    // Gui
    clearPage(KpageView,KpageScene,true);
    disableActions();
    disableEditActions();
    closeFile();           // perform LPub3D file close operations here...
    editModeWindow->close();
    getAct("editModelFileAct.1")->setText(tr("Edit current model file"));
    getAct("editModelFileAct.1")->setStatusTip(tr("Edit LDraw file with detached LDraw Editor"));
    if (!topModel.isEmpty())
        emit lpub->messageSig(LOG_INFO, tr("Model unloaded. File closed - %1.").arg(topModel));
    QString windowTitle = QString::fromLatin1(VER_FILEDESCRIPTION_STR);
    QString versionInfo;
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
    versionInfo = QString("%1 v%2 r%3 (%4)")
                          .arg(QString::fromLatin1(VER_PRODUCTNAME_STR), QString::fromLatin1(VER_PRODUCTVERSION_STR), QString::fromLatin1(VER_REVISION_STR), QString::fromLatin1(VER_BUILD_TYPE_STR));
#else
    int revisionNumber = QString::fromLatin1(VER_REVISION_STR).toInt();
    versionInfo = QString("%1 v%2%3")
                          .arg(QString::fromLatin1(VER_PRODUCTNAME_STR), QString::fromLatin1(VER_PRODUCTVERSION_STR), revisionNumber ? QString(" r%1").arg(VER_REVISION_STR) : "");
#endif

    setWindowTitle(QString("%1[*] - %2").arg(windowTitle).arg(versionInfo));
  }
}

/***************************************************************************
 * File opening closing stuff
 **************************************************************************/

bool Gui::openFile(const QString &fileName)
{

  if (maybeSave() && saveBuildModification()) {
    if (Preferences::modeGUI)
      waitingSpinner->start();
  } else {
    return false;
  }

  disableWatcher();

  setCountWaitForFinished(false);
  suspendFileDisplay = true;
  parsedMessages.clear();
  Preferences::unsetBuildModifications();
  Preferences::setInitFadeSteps();
  Preferences::setInitHighlightStep();
  Preferences::setInitPreferredRenderer();
  clearPage(KpageView,KpageScene,true);
  closeFile();
  if (lcGetPreferences().mViewPieceIcons)
      mPliIconsPath.clear();
  QFileInfo fileInfo(fileName);
  emit lpub->messageSig(LOG_INFO_STATUS, tr("Loading file '%1'...").arg(fileInfo.fileName()));
  setPageLineEdit->setText(tr("Loading..."));
  setGoToPageCombo->addItem(tr("Loading..."));
  mpdCombo->addItem(tr("Loading..."));
  QDir::setCurrent(fileInfo.absolutePath());
  if (lpub->ldrawFile.loadFile(fileInfo.absoluteFilePath()) != 0) {
      emit lpub->messageSig(LOG_INFO_STATUS, lpub->ldrawFile._loadAborted ?
                                tr("Load LDraw file '%1' aborted.").arg(fileInfo.absoluteFilePath()) :
                                tr("Load LDraw file '%1' failed.").arg(fileInfo.absoluteFilePath()));
      closeModelFile();
      if (waitingSpinner->isSpinning())
          waitingSpinner->stop();
      return false;
  }
  displayPageNum = 1 + pa;
  prevDisplayPageNum = displayPageNum;
  Paths::mkDirs();
  getAct("loadStatusAct.1")->setEnabled(true);
  getAct("editModelFileAct.1")->setText(tr("Edit %1").arg(fileInfo.fileName()));
  getAct("editModelFileAct.1")->setStatusTip(tr("Edit LDraw file %1 with detached LDraw Editor").arg(fileInfo.fileName()));
  if (lpub->ldrawFile.getHelperPartsNotInArchive()) {
      QPixmap _icon = QPixmap(":/icons/lpub96.png");
      QMessageBoxResizable box;
      box.setWindowIcon(QIcon());
      box.setIconPixmap (_icon);
      box.setTextFormat (Qt::RichText);
      box.setWindowTitle(tr ("Update LDraw Unofficial Archive Library"));
      box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      QString title = "<b>" + tr ("LDraw unofficial parts in loaded model are not in archive library.") + "</b><br>" +
                              tr ("Would you like to archive your LDraw unofficial parts now?");
      box.setText (title);
      QString searchDirs;
      QFontMetrics fontMetrics = box.fontMetrics();
      for (const QString &ldDir: Preferences::ldSearchDirs)
          searchDirs.append(QString(" - %1<br>").arg(fontMetrics.elidedText(ldDir, Qt::ElideMiddle, box.width())));
      QString text = tr("LDraw <b>helper</b> parts were detected in the loaded model file <i>%1</i>.<br><br>"
                        "Parts not in the archive library will not be rendered by the "
                        "%2 Visual Editor or Native renderer.<br><br>"
                        "%2 will archive parts from your search directory paths.<br>"
                        "%3").arg(fileInfo.fileName()).arg(VER_PRODUCTNAME_STR).arg(searchDirs);
      box.setInformativeText (text);
      box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
      box.setDefaultButton   (QMessageBox::Yes);

      if (box.exec() == QMessageBox::Yes) {
          loadLDSearchDirParts(false/*Process*/, true/*OnDemand*/, false/*Update*/);
      }
  }

  enableLPubFadeOrHighlight(false/*fadeEnabled*/, false/*highlightEnabled*/, true/*waitForFinish*/);

  QString previewLoadPath = QDir::toNativeSeparators(QString("%1/%2").arg(QDir::currentPath()).arg(Paths::tmpDir));
  lcSetProfileString(LC_PROFILE_PREVIEW_LOAD_PATH, previewLoadPath);
  emit lpub->messageSig(LOG_INFO, tr("Loading user interface items..."));
  attitudeAdjustment();
  mpdCombo->clear();
  mpdCombo->addItems(lpub->ldrawFile.subFileOrder());
  mpdCombo->setToolTip(tr("Current Submodel: %1").arg(mpdCombo->currentText()));
  connect(mpdCombo,SIGNAL(activated(int)), this,    SLOT(mpdComboChanged(int)));
  connect(setGoToPageCombo,SIGNAL(activated(int)), this, SLOT(setGoToPage(int)));
  setCurrentFile(fileInfo.absoluteFilePath());
  undoStack->setClean();
  for (int i = 0; i < numPrograms; i++) {
    QFileInfo programFileInfo(programEntries.at(i).split("|").last());
    openWithActList[i]->setStatusTip(tr("Open %1 with %2")
                                        .arg(fileInfo.fileName())
                                        .arg(programFileInfo.fileName()));
  }

  insertFinalModelStep();  //insert final fully coloured model if fadeSteps turned on

  generateCoverPages();    //auto-generate cover page

  enableWatcher();

  defaultResolutionType(Preferences::preferCentimeters);

  emit lpub->messageSig(LOG_INFO, tr("Open file '%1' completed.").arg(fileInfo.absoluteFilePath()));
  return true;
}

int Gui::setupFadeOrHighlight(bool setupFadeSteps, bool setupHighlightStep)
{
  if (!setupFadeSteps && !setupHighlightStep)
    return 0;

  if (!m_fadeStepsSetup || !m_highlightStepSetup) {
    QString const message = setupFadeSteps && setupHighlightStep
        ? tr("Setup and load fade and highlight color parts...")
        : setupFadeSteps
            ? tr("Setup and load fade color parts...")
            : tr("Setup and load highlight color parts...");
    emit lpub->messageSig(LOG_INFO_STATUS, message);
  }

  if (!m_fadeStepsSetup && !m_highlightStepSetup) {
    ldrawColorPartsLoad();
    writeGeneratedColorPartsToTemp();
    partWorkerLDSearchDirs.addCustomDirs();
  }

  if (setupFadeSteps && !m_fadeStepsSetup) {
    if (Preferences::enableImageMatting)
      LDVImageMatte::clearMatteCSIImages();
    partWorkerLDSearchDirs.setDoFadeStep(true);
    processFadeColourParts(true/*overwrite*/, setupFadeSteps);
    m_fadeStepsSetup = true;
  }

  if (setupHighlightStep && !m_highlightStepSetup) {
    partWorkerLDSearchDirs.setDoHighlightStep(true);
    processHighlightColourParts(false/*overwrite*/, setupHighlightStep);
    m_highlightStepSetup = true;
  }
  return 0;
}

void Gui::enableLPubFadeOrHighlight(bool enableFadeSteps, bool enableHighlightStep ,bool waitForFinish)
{
  if (m_fadeStepsSetup && m_highlightStepSetup)
    return;

  if (enableFadeSteps)
    emit lpub->messageSig(LOG_INFO_STATUS,tr("LPub Fade Steps is ENABLED."));
  if (enableHighlightStep)
    emit lpub->messageSig(LOG_INFO_STATUS,tr("LPub Highlight Step is ENABLED."));

  QFuture<int> future = QtConcurrent::run([&]()->int{
      bool enableFade = (m_fadeStepsSetup || enableFadeSteps) ? true : lpub->setFadeStepsFromCommand();
      bool enableHighlight = (m_highlightStepSetup || enableHighlightStep) ? true : lpub->setHighlightStepFromCommand();

      return gui->setupFadeOrHighlight(enableFade, enableHighlight);
  });
  waitForFinish ? future.waitForFinished() : gui->futureWatcher.setFuture(future);
}

void Gui::updateRecentFileActions()
{
  QSettings Settings;
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,LPUB3D_RECENT_FILES_KEY))) {
    QStringList files = Settings.value(QString("%1/%2").arg(SETTINGS,LPUB3D_RECENT_FILES_KEY)).toStringList();

    int numRecentFiles = qMin(files.size(), int(MAX_RECENT_FILES));

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
    Settings.setValue(QString("%1/%2").arg(SETTINGS,LPUB3D_RECENT_FILES_KEY), files);

    for (int i = 0; i < numRecentFiles; i++) {
      QFileInfo fileInfo(files[i]);
      QString text = tr("&%1 %2").arg(i + 1).arg(fileInfo.fileName());
      recentFilesActs[i]->setText(text);
      recentFilesActs[i]->setData(files[i]);
      recentFilesActs[i]->setStatusTip(fileInfo.absoluteFilePath());
      recentFilesActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MAX_RECENT_FILES; j++) {
      recentFilesActs[j]->setVisible(false);
    }
    recentFilesSeparatorAct->setVisible(numRecentFiles > 0);
  }
}

void Gui::setCurrentFile(const QString &fileName)
{
  curFile = fileName;
  QString windowTitle;
  if (fileName.size() == 0) {
    windowTitle = QString::fromLatin1(VER_FILEDESCRIPTION_STR);
  } else {
    QFileInfo fileInfo(fileName);
    windowTitle = fileInfo.fileName();
  }
  QString versionInfo;
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
  versionInfo = QString("%1 v%2 r%3 (%4)")
                        .arg(QString::fromLatin1(VER_PRODUCTNAME_STR), QString::fromLatin1(VER_PRODUCTVERSION_STR), QString::fromLatin1(VER_REVISION_STR), QString::fromLatin1(VER_BUILD_TYPE_STR));
#else
  int revisionNumber = QString::fromLatin1(VER_REVISION_STR).toInt();
  versionInfo = QString("%1 v%2%3")
                        .arg(QString::fromLatin1(VER_PRODUCTNAME_STR), QString::fromLatin1(VER_PRODUCTVERSION_STR), revisionNumber ? QString(" r%1").arg(VER_REVISION_STR) : "");
#endif

  setWindowTitle(tr("%1[*] - %2").arg(windowTitle).arg(versionInfo));

  if (fileName.size() > 0) {
    QSettings Settings;
    QStringList files = Settings.value(QString("%1/%2").arg(SETTINGS,LPUB3D_RECENT_FILES_KEY)).toStringList();
    files.removeAll("");
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MAX_RECENT_FILES) {
      files.removeLast();
    }
    Settings.setValue(QString("%1/%2").arg(SETTINGS,LPUB3D_RECENT_FILES_KEY), files);
  }
  updateRecentFileActions();
}

void Gui::loadLastOpenedFile() {
  updateRecentFileActions();
  int const fileIndex = 0;
  QAction *fileAction = recentFilesActs[fileIndex];
  if (fileAction) {
    QString const recentFile = fileAction->data().toString();
    if (QFileInfo(recentFile).isReadable()) {
      loadFile(recentFile);
    }
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
    QString absoluteFilePath = path;
    QString fileName = QFileInfo(path).fileName();
    if (lpub->ldrawFile.isIncludeFile(fileName) || static_cast<bool>(lpub->ldrawFile.isUnofficialPart(fileName)))
      absoluteFilePath = curFile;
    if (!openFile(absoluteFilePath))
      return;
    displayPageNum = goToPage;
    displayPage();
  }
}

void Gui::writeGeneratedColorPartsToTemp() {
  emit lpub->messageSig(LOG_INFO_STATUS, tr("Writing generated color parts to tmp folder..."));
  int count = 0;
  for (int i = 0; i < lpub->ldrawFile._subFileOrder.size(); i++) {
    QString fileName = lpub->ldrawFile._subFileOrder[i].toLower();
    if (LDrawColourParts::isLDrawColourPart(fileName)) {
      count++;
      lpub->ldrawFile.normalizeHeader(fileName);
      QStringList content = lpub->ldrawFile.contents(fileName);
      emit lpub->messageSig(LOG_INFO, tr("Writing generated part %1 to temp directory: %2...").arg(count).arg(fileName));
      writeToTmp(fileName,content);
    }
  }
  if (!count)
      emit lpub->messageSig(LOG_INFO, tr("No generated parts written."));
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

  return tr("Elapsed time: %1%2%3")
                 .arg(hours   >   0 ?
                                QString("%1 %2 ")
                                        .arg(hours)
                                        .arg(hours   > 1 ? tr("hours")   : tr("hour")) :
                                QString())
                 .arg(minutes > 0 ?
                                QString("%1 %2 ")
                                        .arg(minutes)
                                        .arg(minutes > 1 ? tr("minutes") : tr("minute")) :
                                QString())
                 .arg(QString("%1.%2 %3")
                              .arg(seconds)
                              .arg(milliseconds,3,10,QLatin1Char('0'))
                              .arg(seconds > 1 ? tr("seconds") : tr("second")));
}
