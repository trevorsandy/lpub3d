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

void Gui::configureMpdCombo()
{
    gui->mpdCombo->clear();

    for (int i = 0; i < lpub->ldrawFile.subFileOrder().count(); i++) {
        const QString &subFile = lpub->ldrawFile.subFileOrder().at(i);
        gui->mpdCombo->addItem(subFile);
        if (lpub->ldrawFile.isUnofficialPart(subFile) == UNOFFICIAL_DATA)
            gui->mpdCombo->setItemData(i, QBrush(Preferences::darkTheme ? Qt::magenta : Qt::green), Qt::TextColorRole);
    }

    gui->mpdCombo->setToolTip(tr("Current Submodel: %1").arg(gui->mpdCombo->currentText()));

    if (gui->mpdCombo->count() < 6)
        return;

    gui->mComboDefaultText = gui->mpdCombo->currentText();

    gui->mpdCombo->setFocusPolicy(Qt::StrongFocus);
    gui->mpdCombo->setEditable(true);
    gui->mpdCombo->setInsertPolicy(QComboBox::NoInsert);

    QLineEdit *comboFilterEdit = gui->mpdCombo->lineEdit();

    gui->mComboPatternGroup = new QActionGroup(comboFilterEdit);
    gui->mComboFilterAction = comboFilterEdit->addAction(QIcon(":/resources/filter.png"), QLineEdit::TrailingPosition);
    gui->mComboFilterAction->setCheckable(true);
    gui->mComboFilterAction->setChecked(false);
    gui->connect(gui->mComboFilterAction, &QAction::triggered, gui, &Gui::comboFilterTriggered);

    QMenu *comboFilterMenu = new QMenu(comboFilterEdit);

    gui->mComboCaseSensitivityAction = comboFilterMenu->addAction(tr("Match Case"));
    gui->mComboCaseSensitivityAction->setCheckable(true);
    gui->connect(gui->mComboCaseSensitivityAction, &QAction::toggled, gui, &Gui::comboFilterChanged);

    comboFilterMenu->addSeparator();
    gui->mComboPatternGroup->setExclusive(true);
    QAction *patternAction = comboFilterMenu->addAction("Fixed String");
    patternAction->setData(QVariant(int(QRegExp::FixedString)));
    patternAction->setCheckable(true);
    patternAction->setChecked(true);
    gui->mComboPatternGroup->addAction(patternAction);
    patternAction = comboFilterMenu->addAction("Regular Expression");
    patternAction->setCheckable(true);
    patternAction->setData(QVariant(int(QRegExp::RegExp2)));
    gui->mComboPatternGroup->addAction(patternAction);
    patternAction = comboFilterMenu->addAction("Wildcard");
    patternAction->setCheckable(true);
    patternAction->setData(QVariant(int(QRegExp::Wildcard)));
    gui->mComboPatternGroup->addAction(patternAction);
    gui->connect(gui->mComboPatternGroup, &QActionGroup::triggered, gui, &Gui::comboFilterChanged);

    QToolButton *optionsButton = new QToolButton;
#ifndef QT_NO_CURSOR
    optionsButton->setCursor(Qt::ArrowCursor);
#endif
    optionsButton->setFocusPolicy(Qt::NoFocus);
    optionsButton->setStyleSheet("* { border: none; }");
    optionsButton->setIcon(QIcon(":/resources/gear_in.png"));
    optionsButton->setToolTip(tr("Filter Options..."));
    optionsButton->setMenu(comboFilterMenu);
    optionsButton->setPopupMode(QToolButton::InstantPopup);

    QWidgetAction *optionsAction = new QWidgetAction(this);
    optionsAction->setDefaultWidget(optionsButton);
    comboFilterEdit->addAction(optionsAction, QLineEdit::LeadingPosition);
    gui->connect(comboFilterEdit, &QLineEdit::textChanged, gui, &Gui::comboFilterTextChanged);

    gui->mComboFilterModel = new QSortFilterProxyModel(gui->mpdCombo);
    gui->mComboFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    gui->mComboFilterModel->setSourceModel(gui->mpdCombo->model());

    gui->mpdCombo->setCompleter(new QCompleter(gui->mComboFilterModel, gui->mpdCombo));
    gui->mpdCombo->completer()->setFilterMode(Qt::MatchContains);
    gui->mpdCombo->completer()->setCompletionMode(QCompleter::PopupCompletion);

    gui->connect(gui->mpdCombo,SIGNAL(activated(int)), gui, SLOT(mpdComboChanged(int)));
}

void Gui::comboFilterTextChanged(const QString& Text)
{
    if (gui->mComboFilterAction) {
        if (Text != gui->mComboDefaultText) {
            gui->mComboFilterAction->setIcon(QIcon(":/resources/resetaction.png"));
            gui->mComboFilterAction->setToolTip(tr("Reset"));
        } else {
            gui->mComboFilterAction->setIcon(QIcon(":/resources/filter.png"));
            gui->mComboFilterAction->setToolTip(tr(""));
        }
        QRegExp comboFilterRx(gui->mpdCombo->lineEdit()->text(),
                              gui->comboCaseSensitivity(),
                              gui->comboPatternSyntax());
        gui->mComboFilterModel->setFilterRegExp(comboFilterRx);
    }
}

void Gui::comboFilterTriggered()
{
    int index = gui->mpdCombo->currentIndex();
    gui->mpdCombo->setCurrentIndex(0);
    gui->mpdCombo->setToolTip(tr("Current Submodel: %1").arg(gui->mpdCombo->currentText()));
    gui->mComboFilterAction->setIcon(QIcon(":/resources/filter.png"));
    gui->mComboFilterAction->setToolTip(tr(""));
    if (index)
        mpdComboChanged(index);
}

Qt::CaseSensitivity Gui::comboCaseSensitivity() const
{
    return gui->mComboCaseSensitivityAction->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
}

void Gui::setComboCaseSensitivity(Qt::CaseSensitivity cs)
{
    gui->mComboCaseSensitivityAction->setChecked(cs == Qt::CaseSensitive);
}

static inline QRegExp::PatternSyntax patternSyntaxFromAction(const QAction *a)
{
    return static_cast<QRegExp::PatternSyntax>(a->data().toInt());
}

QRegExp::PatternSyntax Gui::comboPatternSyntax() const
{
    return patternSyntaxFromAction(gui->mComboPatternGroup->checkedAction());
}

void Gui::setComboPatternSyntax(QRegExp::PatternSyntax s)
{
    const QList<QAction*> actions = gui->mComboPatternGroup->actions();
    for (QAction *a : actions) {
        if (patternSyntaxFromAction(a) == s) {
            a->setChecked(true);
            break;
        }
    }
}

void Gui::open()
{  
  if (gui->maybeSave() && gui->saveBuildModification()) {
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

    fileLoadTimer.start();

    QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
      Settings.setValue(QString("%1/%2").arg(SETTINGS,"ProjectsPath"),fileInfo.path());
      if (!gui->openFile(fileName))
          return;
      Gui::displayPage();
      gui->enableActions();
      emit lpub->messageSig(LOG_STATUS, tr("Loaded LDraw file %1 (%2 pages, %3 parts). %4")
                                           .arg(fileInfo.fileName())
                                           .arg(Gui::maxPages)
                                           .arg(lpub->ldrawFile.getPartCount())
                                           .arg(Gui::elapsedTime(fileLoadTimer.elapsed())));
      return;
    }
  }
  return;
}

void Gui::openDropFile(QString &fileName) {

  if (gui->maybeSave() && gui->saveBuildModification()) {
      fileLoadTimer.start();
      QFileInfo fileInfo(fileName);
      QString extension = fileInfo.suffix().toLower();
      bool ldr = false, mpd = false, dat= false;
      ldr = extension == "ldr";
      mpd = extension == "mpd";
      dat = extension == "dat";
      if (fileInfo.exists() && (ldr || mpd || dat)) {
          QSettings Settings;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,"ProjectsPath"),fileInfo.path());
          if (!gui->openFile(fileName))
              return;
          gui->displayPage();
          gui->enableActions();
          emit lpub->messageSig(LOG_STATUS, tr("Loaded LDraw file %1 (%2 pages, %3 parts). %4")
                                               .arg(fileInfo.fileName())
                                               .arg(Gui::maxPages)
                                               .arg(lpub->ldrawFile.getPartCount())
                                               .arg(Gui::elapsedTime(fileLoadTimer.elapsed())));
        } else {
          QString noExtension;
          if (extension.isEmpty())
              noExtension = tr("<br>No file exension specified. Set the file extension to .mpd,.ldr, or .dat.");
          emit gui->messageSig(LOG_ERROR, tr("File not supported!<br>%1%2")
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
    if (sender() == gui->getAct("openWorkingFolderAct.1")) {
        if (!Gui::getCurFile().isEmpty())
            gui->openFolderSelect(Gui::getCurFile());
    } else if (sender() == gui->getAct("openParameterFileFolderAct.1")) {
        gui->openFolderSelect(QDir::toNativeSeparators(Preferences::pliControlFile));
    }
}

void Gui::updateOpenWithActions()
{
    QSettings Settings;
    QString const openWithProgramListKey("OpenWithProgramList");
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,openWithProgramListKey))) {

      gui->programEntries = Settings.value(QString("%1/%2").arg(SETTINGS,openWithProgramListKey)).toStringList();

      gui->numPrograms = qMin(gui->programEntries.size(), Preferences::maxOpenWithPrograms);

      if (Preferences::debugLogging) {
          emit lpub->messageSig(LOG_DEBUG, tr("- Number of default openWith programs: %1").arg(gui->numPrograms));
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
      for (int i = 0; i < gui->numPrograms; ) {
        programData = gui->programEntries.at(i).split("|").last();
        programPath = programData;
        QStringList arguments;
        if (!programData.isEmpty())
          gui->setOpenWithProgramAndArgs(programPath,arguments);
        QFileInfo fileInfo(programPath);
        if (fileInfo.exists() && fileInfo.isFile()) {
          programName = gui->programEntries.at(i).split("|").first();
          QString text = programName;
          if (text.isEmpty())
              text = tr("&%1 %2").arg(i + 1).arg(fileInfo.fileName());
          gui->openWithActList[i]->setText(text);
          gui->openWithActList[i]->setData(programData); // includes arguments
          gui->openWithActList[i]->setIcon(getProgramIcon());
          gui->openWithActList[i]->setStatusTip(tr("Open current file with %2").arg(fileInfo.fileName()));
          gui->openWithActList[i]->setVisible(true);
          i++;
          if (Preferences::debugLogging) {
              lpub->messageSig(LOG_DEBUG, tr("- Add openWith program: %1. %2").arg(i).arg(programData));
          }
        } else {
          gui->programEntries.removeOne(gui->programEntries.at(i));
          --gui->numPrograms;
        }
      }

      // add system editor if exits
      if (!Preferences::systemEditor.isEmpty()) {
        QFileInfo fileInfo(Preferences::systemEditor);
        if (fileInfo.exists() && fileInfo.isFile()) {
          QString arguments;
          if (Preferences::usingNPP)
            arguments = QLatin1String(WINDOWS_NPP_LPUB3D_UDL_ARG);
          const int i = gui->numPrograms;
          programPath = fileInfo.absoluteFilePath();
          programName = fileInfo.completeBaseName();
          programName.replace(programName[0],programName[0].toUpper());
          programData = QString("'%1' %2").arg(programPath).arg(arguments);
          QString text = programName;
          if (text.isEmpty())
              text = tr("&%1 %2").arg(i + 1).arg(fileInfo.fileName());
          gui->openWithActList[i]->setText(text);
          gui->openWithActList[i]->setData(programData);
          gui->openWithActList[i]->setIcon(getProgramIcon());
          gui->openWithActList[i]->setStatusTip(tr("Open current file with %2").arg(fileInfo.fileName()));
          gui->openWithActList[i]->setVisible(true);
          gui->programEntries.append(QString("%1|%2").arg(programName).arg(programData));
          gui->numPrograms = gui->programEntries.size();
          if (Preferences::debugLogging) {
              lpub->messageSig(LOG_DEBUG, tr("- Add openWith system editor: %1").arg(programData));
          }
        }
      }

      if (Preferences::debugLogging) {
          lpub->messageSig(LOG_DEBUG, tr("- Number of loaded openWith programs: %1").arg(gui->numPrograms));
      }

      // hide empty program actions - redundant
      for (int j = gui->numPrograms; j < Preferences::maxOpenWithPrograms; j++)
        gui->openWithActList[j]->setVisible(false);

      // clear old menu actions
      if (gui->openWithMenu->actions().size())
          gui->openWithMenu->clear();

#ifdef Q_OS_WIN
      // add open with choice menu action first
      openWithMenu->addAction(getAct("openWithChoiceAct.1"));
      openWithMenu->addSeparator();
#endif

      // add menu actions from updated list
      for (int k = 0; k < gui->numPrograms; k++) {
        gui->openWithMenu->addAction(gui->openWithActList.at(k));
      }
    }
}

void Gui::openWithSetup()
{
    OpenWithProgramDialogGui openWithProgramDialogGui;
    openWithProgramDialogGui.setOpenWithProgram();
    updateOpenWithActions();
}

void Gui::setOpenWithProgramAndArgs(QString &program, QStringList &arguments)
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

void Gui::openWithChoice()
{
#ifdef Q_OS_WIN
    TCHAR sysdir[MAX_PATH]{};
    if (!GetSystemDirectory(sysdir, MAX_PATH)) return;
    std::wstring argwstr = L"shell32.dll,OpenAs_RunDLL " + QDir::toNativeSeparators(curFile).toStdWString();
    ShellExecute(::GetDesktopWindow(), 0, L"RUNDLL32.EXE", (LPCWSTR)argwstr.c_str(), sysdir, SW_SHOWNORMAL);
#endif
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
            gui->setOpenWithProgramAndArgs(program,arguments);
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
        gui->setOpenWithProgramAndArgs(program,arguments);
    }

    qint64 pid;
    QString workingDirectory = QDir::currentPath() + QDir::separator();
    QProcess::startDetached(program, arguments, workingDirectory, &pid);
    emit lpub->messageSig(LOG_INFO, tr("Launched %1 with pid=%2 %3%4")
                                        .arg(QFileInfo(filePath).fileName()).arg(pid)
                                        .arg(QFileInfo(program).fileName())
                                        .arg(arguments.size() ? " "+arguments.join(" ") : ""));

}

void Gui::openWith()
{
    QString file = Gui::curFile;
    if (whichFile(OPT_OPEN_WITH) == OPT_USE_INCLUDE)
        file = Gui::curSubFile;
    gui->openWith(file);
}

void Gui::loadLastOpenedFile() {
  Gui::m_lastDisplayedPage = Preferences::loadLastDisplayedPage;
  gui->updateRecentFileActions();
  int const fileIndex = 0;
  QAction *fileAction = recentFilesActs[fileIndex];
  if (fileAction) {
    QString const recentFile = fileAction->data().toString();
    if (QFileInfo(recentFile).isReadable()) {
      gui->loadFile(recentFile);
    }
  }
}

void Gui::openRecentFile()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (action) {
    fileLoadTimer.start();
    QString fileName = action->data().toString();
    QString const recentFile = action->data().toString();
    if (QFileInfo(recentFile).isReadable()) {
      gui->loadFile(recentFile);
    }
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
  gui->updateRecentFileActions();
}

bool Gui::loadFile(const QString &file)
{
    return gui->loadFile(file, false/*commandLine*/);
}

bool Gui::loadFile(const QString &file, bool console)
{
    if(Gui::resetCache) {
        emit lpub->messageSig(LOG_INFO,tr("Reset parts cache specified."));
        gui->resetModelCache(QFileInfo(file).absoluteFilePath(), console);
    }

    if (Preferences::modeGUI) {
        QSettings Settings;
        if (Settings.contains(QString("%1/%2").arg(RESTART, RESTART_APPLICATION_KEY))) {
            Gui::m_lastDisplayedPage = Preferences::loadLastDisplayedPage;
            Settings.remove(QString("%1/%2").arg(RESTART, RESTART_APPLICATION_KEY));
        }
    }

    bool fileLoaded = false;
    QString fileName = file;
    QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
        fileLoadTimer.start();
        if (!openFile(fileName)) {
            emit gui->fileLoadedSig(false);
            Gui::m_lastDisplayedPage = false;
            return false;
        }
        Paths::mkDirs();
        gui->cyclePageDisplay(Gui::displayPageNum);
        gui->enableActions();
        fileLoaded = true;
        emit lpub->messageSig(LOG_STATUS, tr("Loaded LDraw file %1 (%2 pages, %3 parts). %4")
                                             .arg(fileInfo.fileName())
                                             .arg(Gui::maxPages)
                                             .arg(lpub->ldrawFile.getPartCount())
                                             .arg(Gui::elapsedTime(fileLoadTimer.elapsed())));
    } else {
        emit gui->messageSig(LOG_ERROR,tr("Unable to load file %1.").arg(fileName));
    }

    emit gui->fileLoadedSig(fileLoaded);
    Gui::m_lastDisplayedPage = false;
    return fileLoaded;
}

void Gui::enableWatcher()
{
#ifdef WATCHER
    if (Gui::curFile != "") {
      watcher.addPath(Gui::curFile);
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
    if (Gui::curFile != "") {
      watcher.removePath(Gui::curFile);
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
    bool includeFile    = lpub->ldrawFile.isIncludeFile(Gui::curSubFile);
    bool dirtyUndoStack = ! undoStack->isClean();
    bool curFileExists    = ! Gui::curFile.isEmpty();
    bool showDialog     = false;

    bool includeChecked = option == OPT_SAVE;
    bool currentChecked = !includeChecked;

    switch (option) {
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
        QString elidedText = currentMetrics.elidedText(QFileInfo(Gui::curFile).fileName(),
                                                       Qt::ElideRight, currentButton->width());
        currentButton->setText(tr("Current file: %1").arg(elidedText));
        currentButton->setChecked(currentChecked);
        saveWhichFrmLayout->addRow(currentButton);

        // include file
        QRadioButton * includeButton = new QRadioButton("", dialog);
        includeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QFontMetrics includeMetrics(includeButton->font());
        elidedText = includeMetrics.elidedText(QFileInfo(Gui::curSubFile).fileName(),
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
  gui->disableWatcher();

  QString file = Gui::curFile;
  if (gui->whichFile(OPT_SAVE) == OPT_USE_INCLUDE)
      file = Gui::curSubFile;

  if (file.isEmpty()) {
    gui->saveAs();
  } else {
    gui->saveFile(file);
  }

 gui->enableWatcher();
}

void Gui::saveAs()
{
  gui->disableWatcher();

  QString file = Gui::curFile;
  if (gui->whichFile(OPT_SAVE_AS) == OPT_USE_INCLUDE)
      file = Gui::curSubFile;

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
    gui->saveFile(fileName);
    gui->closeFileOperations();
    int loadMsgType = Preferences::ldrawFilesLoadMsgs;
    if (loadMsgType)
        Preferences::ldrawFilesLoadMsgs = static_cast<int>(NEVER_SHOW);
    gui->openFile(fileName);
    if (loadMsgType)
        Preferences::ldrawFilesLoadMsgs = loadMsgType;
    Gui::displayPage();
  } else {
    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),
                              QMessageBox::tr("Unsupported LDraw file extension %1 specified.  File not saved.")
                                .arg(extension));

  }
  gui->enableWatcher();
} 

void Gui::saveCopy()
{
    QString file = Gui::curFile;
    if (gui->whichFile(OPT_SAVE_COPY) == OPT_USE_INCLUDE)
        file = Gui::curSubFile;

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
    gui->saveFile(fileName);
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

  if ( ! gui->undoStack->isClean() && proceed) {
    if (Preferences::modeGUI && prompt) {

      QMessageBoxResizable box;
      box.setWindowIcon(QIcon());
      box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
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

      if (saveSender) {
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
        gui->saveBuildModification();
        gui->save();
      } else
      if (ExecReturn == QMessageBox::Cancel) {
        return false;
      }
    } else {
      gui->save();
      emit lpub->messageSig(LOG_INFO,tr("Open document has been saved!"));
    }
  }
  return true;
}

bool Gui::saveFile(const QString &fileName)
{
  bool rc;
  rc = lpub->ldrawFile.saveFile(fileName);
  gui->setCurrentFile(fileName);
  undoStack->setClean();
  if (rc) {
    int lines = lpub->ldrawFile.savedLines();
    const QString message = tr("File %1 saved (%2 lines)")
                                .arg(QFileInfo(fileName).fileName()).arg(lines);
    emit lpub->messageSig(LOG_INFO, message);
    gui->statusBar()->showMessage(message, 2000);
  }
  return rc;
}

// This call performs LPub3D file close operations - does not clear Gui::curFile
// use closeModelFile() to definitively close the current file in LPub3D
void Gui::closeFileOperations()
{
  Gui::pa = Gui::sa = 0;
  Gui::buildModJumpForward = false;
  Gui::pageDirection = PAGE_NEXT;
  Gui::pageProcessParent = PROC_NONE;
  Gui::pageProcessRunning = PROC_NONE;
  lpub->ldrawFile.empty();
  gui->editWindow->clearWindow();
  gui->mpdCombo->clear();
  gui->mpdCombo->setEnabled(false);
  gui->mpdCombo->setEditable(false);
  gui->setGoToPageCombo->clear();
  gui->setGoToPageCombo->setEnabled(false);
  gui->setPageLineEdit->clear();
  gui->setPageLineEdit->setEnabled(false);
  gui->openWithMenu->setEnabled(false);
  Gui::topOfPages.clear();
  Gui::pageSizes.clear();
  gui->undoStack->clear();
  Gui::pageDirection = PAGE_NEXT;
  Gui::buildModJumpForward = false;
  Gui::setAbortProcess(false);
  Preferences::resetFadeSteps();
  Preferences::resetHighlightStep();
  Preferences::resetPreferredRenderer();
  Preferences::unsetBuildModifications();
  LDrawColor::removeUserDefinedColors();
  //Visual Editor
  if (Preferences::modeGUI) {
      enableVisualBuildModification();
      enable3DActions(false);
      emit gui->clearViewerWindowSig();
      emit gui->updateAllViewsSig();
  }
  lpub->SetStudStyle(nullptr, true/*reload*/);
  lpub->SetAutomateEdgeColor(nullptr);
  Preferences::preferredRendererPreferences();
  Preferences::fadestepPreferences();
  Preferences::highlightstepPreferences();
  if (!Preferences::enableFadeSteps && !Preferences::enableHighlightStep) {
      LDrawColourParts::clearGeneratedColorParts();
      gui->partWorkerLDSearchDirs.removeCustomDirs();
  }
  Gui::submodelIconsLoaded = false;
  gui->SetSubmodelIconsLoaded(Gui::submodelIconsLoaded);
  if (!Gui::curFile.isEmpty())
      emit lpub->messageSig(LOG_DEBUG, tr("File closed - %1.").arg(Gui::curFile));
  gui->getAct("loadStatusAct.1")->setEnabled(false);
  gui->ReloadVisualEditor();
}

// This call definitively closes and clears from Gui::curFile, the current model file
void Gui::closeModelFile()
{
  if (gui->maybeSave()) {
    gui->disableWatcher();
    QString const topModel = lpub->ldrawFile.topLevelFile();
    Gui::curFile.clear();       // clear file from Gui::curFile here...
    // Editor
    emit gui->clearEditorWindowSig();
    // Gui
    Gui::clearPage(true);
    gui->disableActions();
    gui->disableEditActions();
    gui->closeFileOperations();           // perform LPub3D file close operations here...
    gui->editModeWindow->close();
    gui->getAct("editModelFileAct.1")->setText(tr("Edit current model file"));
    gui->getAct("editModelFileAct.1")->setStatusTip(tr("Edit LDraw file with detached LDraw Editor"));
    if (!topModel.isEmpty())
        emit lpub->messageSig(LOG_INFO, tr("Model unloaded. File closed - %1.").arg(topModel));
    QString windowTitle = QString::fromLatin1(VER_FILEDESCRIPTION_STR);
    int REV = QString::fromLatin1(VER_REVISION_STR).toInt();
    QString versionInfo;
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
    versionInfo = QString("%1 v%2%3 (%4)")
                          .arg(QString::fromLatin1(VER_PRODUCTNAME_STR), QString::fromLatin1(VER_PRODUCTVERSION_STR), REV ? QString(" r%1").arg(VER_REVISION_STR) : QString(), QString::fromLatin1(VER_BUILD_TYPE_STR));
#else

    versionInfo = QString("%1 v%2%3")
                          .arg(QString::fromLatin1(VER_PRODUCTNAME_STR), QString::fromLatin1(VER_PRODUCTVERSION_STR), REV ? QString(" r%1").arg(VER_REVISION_STR) : QString());
#endif

    gui->setWindowTitle(QString("%1[*] - %2").arg(windowTitle).arg(versionInfo));
  }
}

/***************************************************************************
 * File opening closing stuff
 **************************************************************************/

bool Gui::openFile(const QString &fileName)
{
  if (gui->maybeSave() && gui->saveBuildModification()) {
    if (Preferences::modeGUI)
      gui->waitingSpinner->start();
  } else {
    return false;
  }

  gui->disableWatcher();

  Gui::setCountWaitForFinished(false);
  Gui::suspendFileDisplay = true;
  Gui::parsedMessages.clear();
  Preferences::unsetBuildModifications();
  Preferences::setInitFadeSteps();
  Preferences::setInitHighlightStep();
  Preferences::setInitPreferredRenderer();
  Gui::clearPage(true);
  gui->closeFileOperations();
  if (lcGetPreferences().mViewPieceIcons)
      gui->mPliIconsPath.clear();
  QFileInfo fileInfo(fileName);
  emit lpub->messageSig(LOG_INFO_STATUS, tr("Loading file '%1'...").arg(fileInfo.fileName()));
  gui->setPageLineEdit->setText(tr("Loading..."));
  gui->setGoToPageCombo->addItem(tr("Loading..."));
  gui->mpdCombo->addItem(tr("Loading..."));
  QDir::setCurrent(fileInfo.absolutePath());
  if (lpub->ldrawFile.loadFile(fileInfo.absoluteFilePath()) != 0) {
      emit lpub->messageSig(LOG_INFO_STATUS, lpub->ldrawFile._loadAborted ?
                                tr("Load LDraw file '%1' aborted.").arg(fileInfo.absoluteFilePath()) :
                                tr("Load LDraw file '%1' failed.").arg(fileInfo.absoluteFilePath()));
      gui->closeModelFile();
      if (gui->waitingSpinner->isSpinning())
          gui->waitingSpinner->stop();
      return false;
  }
  Gui::displayPageNum = 1 + Gui::pa;
  Gui::prevDisplayPageNum = Gui::displayPageNum;
  Paths::mkDirs();
  gui->getAct("loadStatusAct.1")->setEnabled(true);
  gui->getAct("editModelFileAct.1")->setText(tr("Edit %1").arg(fileInfo.fileName()));
  gui->getAct("editModelFileAct.1")->setStatusTip(tr("Edit LDraw file %1 with detached LDraw Editor").arg(fileInfo.fileName()));
  int unarchivedParts = lpub->ldrawFile.getSupportPartsNotInArchive();
  if (unarchivedParts) {
      QMessageBoxResizable box;
      box.setWindowIcon(QIcon());
      box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
      box.setTextFormat (Qt::RichText);
      box.setWindowTitle(tr ("Update LDraw Unofficial Archive Library"));
      box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      QString const missingParts = unarchivedParts == ExcludedParts::EP_HELPER_AND_LSYNTH
              ? tr("Helper and LSynth")
              : unarchivedParts == ExcludedParts::EP_HELPER
                ? tr("Helper")
                : tr("LSynth");
      QString const title = "<b>" + tr ("%1 parts in loaded model are not in archive library.").arg(missingParts) + "</b><br>" +
                                    tr ("Would you like to archive your LDraw unofficial parts now?");
      box.setText (title);
      QString searchDirs;
      QFontMetrics fontMetrics = box.fontMetrics();
      for (const QString &ldDir: Preferences::ldSearchDirs)
          searchDirs.append(QString(" - %1<br>").arg(fontMetrics.elidedText(ldDir, Qt::ElideMiddle, box.width())));
      QString const text = tr("LDraw <b>%1</b> parts were detected in the loaded model file <i>%2</i>.<br><br>"
                              "Parts not in the archive library will not be rendered by the "
                              "%3 Visual Editor or Native renderer.<br><br>"
                              "%3 will archive parts from your search directory paths.<br>"
                              "%4").arg(missingParts).arg(fileInfo.fileName()).arg(VER_PRODUCTNAME_STR).arg(searchDirs);
      box.setInformativeText (text);
      box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
      box.setDefaultButton   (QMessageBox::Yes);

      if (box.exec() == QMessageBox::Yes) {
          gui->loadLDSearchDirParts(false/*Process*/, true/*OnDemand*/, false/*Update*/);
      }
  }

  enableLPubFadeOrHighlight(false/*fadeEnabled*/, false/*highlightEnabled*/, true/*waitForFinish*/);

  QString previewLoadPath = QDir::toNativeSeparators(QString("%1/%2").arg(QDir::currentPath()).arg(Paths::tmpDir));
  lcSetProfileString(LC_PROFILE_PREVIEW_LOAD_PATH, previewLoadPath);
  emit lpub->messageSig(LOG_INFO, tr("Loading user interface items..."));
  gui->attitudeAdjustment();
  gui->configureMpdCombo();
  gui->setCurrentFile(fileInfo.absoluteFilePath());
  gui->connect(gui->setGoToPageCombo,SIGNAL(activated(int)), gui, SLOT(setGoToPage(int)));
  gui->undoStack->setClean();
  gui->openWithMenu->setEnabled(gui->numPrograms);
  for (int i = 0; i < gui->numPrograms; i++) {
    QFileInfo programFileInfo(gui->programEntries.at(i).split("|").last());
    gui->openWithActList[i]->setStatusTip(tr("Open %1 with %2")
                                        .arg(fileInfo.fileName())
                                        .arg(programFileInfo.fileName()));
  }

  gui->insertFinalModelStep();  //insert final fully coloured model if fadeSteps turned on

  gui->generateCoverPages();    //auto-generate cover page

  gui->enableWatcher();

  defaultResolutionType(Preferences::preferCentimeters);

  emit lpub->messageSig(LOG_INFO, tr("Open file '%1' completed.").arg(fileInfo.absoluteFilePath()));
  return true;
}

int Gui::setupFadeOrHighlight(bool setupFadeSteps, bool setupHighlightStep)
{
  if (!setupFadeSteps && !setupHighlightStep)
    return 0;

  if (!Gui::m_fadeStepsSetup || !Gui::m_highlightStepSetup) {
    QString const message = setupFadeSteps && setupHighlightStep
        ? tr("Setup and load fade and highlight color parts...")
        : setupFadeSteps
            ? tr("Setup and load fade color parts...")
            : tr("Setup and load highlight color parts...");
    emit lpub->messageSig(LOG_INFO_STATUS, message);
  }

  if (!Gui::m_fadeStepsSetup && !Gui::m_highlightStepSetup) {
    gui->ldrawColorPartsLoad();
    gui->writeGeneratedColorPartsToTemp();
    gui->partWorkerLDSearchDirs.addCustomDirs();
  }

  if (setupFadeSteps && !Gui::m_fadeStepsSetup) {
    if (Preferences::enableImageMatting)
      LDVImageMatte::clearMatteCSIImages();
    gui->partWorkerLDSearchDirs.setDoFadeStep(true);
    gui->processFadeColourParts(true/*overwrite*/, setupFadeSteps);
    Gui::m_fadeStepsSetup = true;
  }

  if (setupHighlightStep && !Gui::m_highlightStepSetup) {
    gui->partWorkerLDSearchDirs.setDoHighlightStep(true);
    gui->processHighlightColourParts(false/*overwrite*/, setupHighlightStep);
    Gui::m_highlightStepSetup = true;
  }
  return 0;
}

void Gui::enableLPubFadeOrHighlight(bool enableFadeSteps, bool enableHighlightStep ,bool waitForFinish)
{
  if (Gui::m_fadeStepsSetup && Gui::m_highlightStepSetup)
    return;

  if (enableFadeSteps)
    emit lpub->messageSig(LOG_INFO_STATUS,tr("LPub Fade Steps is ENABLED."));
  if (enableHighlightStep)
    emit lpub->messageSig(LOG_INFO_STATUS,tr("LPub Highlight Step is ENABLED."));

  QFuture<int> future = QtConcurrent::run([&]()->int{
      bool enableFade = (Gui::m_fadeStepsSetup || enableFadeSteps) ? true : lpub->setFadeStepsFromCommand();
      bool enableHighlight = (Gui::m_highlightStepSetup || enableHighlightStep) ? true : lpub->setHighlightStepFromCommand();
      return gui->setupFadeOrHighlight(enableFade, enableHighlight);
  });
  if (waitForFinish)
      future.waitForFinished();
  else
      gui->futureWatcher.setFuture(future);
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
      gui->recentFilesActs[i]->setText(text);
      gui->recentFilesActs[i]->setData(files[i]);
      gui->recentFilesActs[i]->setStatusTip(fileInfo.absoluteFilePath());
      gui->recentFilesActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MAX_RECENT_FILES; j++) {
      gui->recentFilesActs[j]->setVisible(false);
    }
    gui->recentFilesSeparatorAct->setVisible(numRecentFiles > 0);
  }
}

void Gui::setCurrentFile(const QString &fileName)
{
  Gui::curFile = fileName;
  QString windowTitle;
  if (fileName.size() == 0) {
    windowTitle = QString::fromLatin1(VER_FILEDESCRIPTION_STR);
  } else {
    QFileInfo fileInfo(fileName);
    windowTitle = fileInfo.fileName();
  }

  QString versionInfo;
  int REV = QString::fromLatin1(VER_REVISION_STR).toInt();
#if defined LP3D_CONTINUOUS_BUILD || defined LP3D_DEVOPS_BUILD || defined LP3D_NEXT_BUILD
  versionInfo = QString("%1 v%2%3 (%4)")
                        .arg(QString::fromLatin1(VER_PRODUCTNAME_STR), QString::fromLatin1(VER_PRODUCTVERSION_STR), REV ? QString(" r%1").arg(VER_REVISION_STR) : QString(), QString::fromLatin1(VER_BUILD_TYPE_STR));
#else
  versionInfo = QString("%1 v%2%3")
                        .arg(QString::fromLatin1(VER_PRODUCTNAME_STR), QString::fromLatin1(VER_PRODUCTVERSION_STR), REV ? QString(" r%1").arg(VER_REVISION_STR) : QString());
#endif

  gui->setWindowTitle(tr("%1[*] - %2").arg(windowTitle).arg(versionInfo));

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
  gui->updateRecentFileActions();
}

void Gui::reloadFromDisk()
{
  if (!QFileInfo(Gui::curFile).isReadable())
    return;
  int goToPage = Gui::displayPageNum;
  if (!gui->openFile(Gui::curFile))
    return;
  Gui::displayPageNum = goToPage;
  Gui::displayPage();
}

void Gui::fileChanged(const QString &path)
{
  if (! changeAccepted)
    return;

  changeAccepted = false;

  QMessageBoxResizable box;
  box.setWindowIcon(QIcon());
  box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
  box.setTextFormat (Qt::RichText);
  box.setWindowTitle(tr ("%1 File Change").arg(VER_PRODUCTNAME_STR));
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  QString title = "<b>" + tr ("External change detected") + "</b>";
  QString text = tr("Current file contents were changed by an external source<br>\"%1\".<br>Reload ?").arg(path);
  box.setText (title);
  box.setInformativeText (text);
  box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
  box.setDefaultButton   (QMessageBox::Yes);

  Preferences::messageBoxAdjustWidth(qobject_cast<QMessageBox*>(&box), title, text);

  if (box.exec() == QMessageBox::Yes) {
    changeAccepted = true;
    int goToPage = Gui::displayPageNum;
    QString absoluteFilePath = path;
    QString fileName = QFileInfo(path).fileName();
    if (lpub->ldrawFile.isIncludeFile(fileName) || static_cast<bool>(lpub->ldrawFile.isUnofficialPart(fileName)))
      absoluteFilePath = Gui::curFile;
    if (!gui->openFile(absoluteFilePath))
      return;
    Gui::displayPageNum = goToPage;
    Gui::displayPage();
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
      QStringList const content = lpub->ldrawFile.contents(fileName);
      emit lpub->messageSig(LOG_INFO, tr("Writing generated part %1 to temp directory: %2...").arg(count).arg(fileName));
      gui->writeToTmp(fileName,content);
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

QString Gui::elapsedTime(const qint64 &duration, bool pretty)
{
  qint64 elapsed = duration;
  int milliseconds = int(elapsed % 1000);
  elapsed /= 1000;
  int seconds = int(elapsed % 60);
  elapsed /= 60;
  int minutes = int(elapsed % 60);
  elapsed /= 60;
  int hours = int(elapsed % 24);

  return tr("%1%2%3%4")
            .arg(pretty        ?
                           tr("Elapsed time: ") : QString())
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
