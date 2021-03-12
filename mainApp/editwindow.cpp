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

/****************************************************************************
 *
 * The editwindow is used to display the LDraw file to the user.  The Gui
 * portion of the program (see lpub.h) decides what files and line numbers
 * are displayed.  The edit window has as little responsibility as is
 * possible.  It does work the syntax highlighter implemented in
 * highlighter.(h,cpp)
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QtWidgets>

#include <QComboBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QtConcurrent>

#include "lc_global.h"
#include "editwindow.h"
#include "lpubalert.h"
#include "highlighter.h"
#include "highlightersimple.h"
#include "ldrawfiles.h"
#include "messageboxresizable.h"
#include "waitingspinnerwidget.h"
#include "threadworkers.h"

#include "version.h"
#include "paths.h"
#include "lpub_preferences.h"

#include "lc_mainwindow.h"
#include "pli.h"
#include "color.h"
#include "ldrawpartdialog.h"
#include "ldrawcolordialog.h"

#include "lc_viewwidget.h"
#include "lc_previewwidget.h"
#include "pieceinf.h"
#include "lc_colors.h"

EditWindow *editWindow;

EditWindow::EditWindow(QMainWindow *parent, bool _modelFileEdit_) :
  QMainWindow(parent),isIncludeFile(false),_modelFileEdit(_modelFileEdit_),_pageIndx(0)
{
    editWindow  = this;

    _textEdit   = new QTextEditor(_modelFileEdit, this);

    loadModelWorker = new LoadModelWorker();

    verticalScrollBar = _textEdit->verticalScrollBar();

    setTextEditHighlighter();

    setSelectionHighlighter();

    completer = new QCompleter(this);
    completer->setModel(modelFromFile(":/resources/autocomplete.lst"));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);

    _textEdit->setLineWrapMode(QTextEditor::NoWrap);
    _textEdit->setUndoRedoEnabled(true);
    _textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    _textEdit->popUp = nullptr;
    _textEdit->setCompleter(completer);

    showLineType    = LINE_HIGHLIGHT;
    isReadOnly      = false;

    createActions();
    updateOpenWithActions();
    createToolBars();
    highlightCurrentLine();

    if (Preferences::editorBufferedPaging) {
        _textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        connect(verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(verticalScrollValueChanged(int)));
    }

    connect(&futureWatcher, &QFutureWatcher<int>::finished, this, &EditWindow::contentLoaded);
    connect(_textEdit, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
    connect(_textEdit, SIGNAL(cursorPositionChanged()),  this, SLOT(highlightCurrentLine()));
    connect(_textEdit, SIGNAL(updateSelectedParts()),   this, SLOT(updateSelectedParts()));
    connect(_textEdit, SIGNAL(triggerPreviewLine()),  this,  SLOT(triggerPreviewLine()));

    setCentralWidget(_textEdit);

    if (modelFileEdit()) {
        _saveSubfileIndex = 0;
        QObject::connect(&fileWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(modelFileChanged(const QString&)));
        editWindow->statusBar()->show();
        readSettings();
    }

    setMinimumSize(200, 200);
}

QAbstractItemModel *EditWindow::modelFromFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
            words << line.trimmed();
    }

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    return new QStringListModel(words, completer);
}

void EditWindow::setSelectionHighlighter()
{
    QColor highlightColor;
    if (Preferences::displayTheme == THEME_DARK) {
        highlightColor = QColor(Preferences::themeColors[THEME_DARK_LINE_SELECT]);
        highlightColor.setAlpha(50);
    } else {
        highlightColor = QColor(Preferences::themeColors[THEME_DEFAULT_LINE_SELECT]);
        highlightColor.setAlpha(30);
    }

    auto palette = _textEdit->palette();
    palette.setBrush(QPalette::Highlight, highlightColor);
    palette.setBrush(QPalette::HighlightedText, QBrush(Qt::NoBrush));

    _textEdit->setPalette(palette);
}

void EditWindow::setTextEditHighlighter()
{
    if (Preferences::editorDecoration == SIMPLE)
      highlighterSimple = new HighlighterSimple(_textEdit->document());
    else
      highlighter = new Highlighter(_textEdit->document());

    setSelectionHighlighter();

    highlightCurrentLine();
}

void EditWindow::previewLine()
{
    lcPreferences& Preferences = lcGetPreferences();
    if (!Preferences.mPreviewEnabled)
        return;

    if (isIncludeFile || !sender() || sender() != previewLineAct)
        return;

    QStringList partKeys = previewLineAct->data().toString().split("|");
    int colorCode        = partKeys.at(0).toInt();
    QString partType     = partKeys.at(1);

    if (Preferences.mPreviewPosition != lcPreviewPosition::Floating && !modelFileEdit()) {
        gMainWindow->PreviewPiece(partType, colorCode, false);
        return;
    } else {
        lcPreview *Preview = new lcPreview();
        lcViewWidget *ViewWidget = new lcViewWidget(nullptr/*parent*/, Preview/*owner*/);

        if (Preview && ViewWidget) {
            QPoint position;
            switch (Preferences.mPreviewLocation)
            {
            case lcPreviewLocation::TopRight:
                position = mapToGlobal(rect().topRight());
                break;
            case lcPreviewLocation::TopLeft:
                position = mapToGlobal(rect().topLeft());
                break;
            case lcPreviewLocation::BottomRight:
                position = mapToGlobal(rect().bottomRight());
                break;
            default:
                position = mapToGlobal(rect().bottomLeft());
                break;
            }

            ViewWidget->setAttribute(Qt::WA_DeleteOnClose, true);

            if (Preview->SetCurrentPiece(partType, colorCode))  {
                ViewWidget->SetPreviewPosition(rect(), position);
                return;
            }
        }
    }

    emit lpubAlert->messageSig(LOG_ERROR, QString("Part preview for %1 failed.").arg(partType));
}

#ifdef QT_DEBUG_MODE
void EditWindow::previewViewerFile()
{
    lcPreferences& Preferences = lcGetPreferences();
    if (Preferences.mPreviewEnabled && !isIncludeFile) {
        QString partKey = QString("%1|%2").arg("16").arg(QFileInfo(fileName).fileName());
        previewLineAct->setData(partKey);
        previewLineAct->setEnabled(true);
        emit previewLineAct->triggered();
    }
}
#endif

void EditWindow::updateOpenWithActions()
{
    numOpenWithPrograms = 0;
    QSettings Settings;
    QString const openWithProgramListKey("OpenWithProgramList");
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,openWithProgramListKey))) {

        QStringList programEntries = Settings.value(QString("%1/%2").arg(SETTINGS,openWithProgramListKey)).toStringList();

        numOpenWithPrograms = qMin(programEntries.size(), Preferences::maxOpenWithPrograms);

        emit lpubAlert->messageSig(LOG_DEBUG, QString("1. Number of Programs: %1").arg(numOpenWithPrograms));

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
                    emit lpubAlert->messageSig(LOG_INFO,QString("Could not save program file icon: %1").arg(iconFile));
                return fileIcon;
            }
            return QIcon(iconFile);
        };

        // filter programPaths that don't exist
        for (int i = 0; i < numOpenWithPrograms; ) {
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
                --numOpenWithPrograms;
            }
        }
        emit lpubAlert->messageSig(LOG_DEBUG, QString("2. Number of Programs: %1").arg(numOpenWithPrograms));

        // hide empty program actions
        for (int j = numOpenWithPrograms; j < Preferences::maxOpenWithPrograms; j++) {
            openWithActList[j]->setVisible(false);
        }
    }
}

void EditWindow::openWithProgramAndArgs(QString &program, QStringList &arguments)
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

void EditWindow::openWith()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QStringList arguments = QStringList() << fileName;
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
        emit lpubAlert->messageSig(LOG_INFO, QString("Launched %1 with %2...")
                                  .arg(QFileInfo(fileName).fileName()).arg(QFileInfo(program).fileName()));
    }
}

void EditWindow::createOpenWithActions()
{
    for (int i = 0; i < Preferences::maxOpenWithPrograms; i++) {
        QAction *openWithAct = new QAction(this);
        openWithAct->setVisible(false);
        connect(openWithAct, SIGNAL(triggered()), this, SLOT(openWith()));
        if (i < openWithActList.size()) {
            openWithActList.replace(i,openWithAct);
        } else {
            openWithActList.append(openWithAct);
        }
    }
}

void EditWindow::createActions()
{
    createOpenWithActions();

    editModelFileAct = new QAction(QIcon(":/resources/editldraw.png"),tr("Edit current model file"), this);
    editModelFileAct->setStatusTip(tr("Edit loaded LDraw file in detached LDraw Editor"));
    connect(editModelFileAct, SIGNAL(triggered()), this, SIGNAL(editModelFileSig()));

    previewLineAct = new QAction(QIcon(":/resources/previewpart.png"),tr("Preview highlighted line..."), this);
    previewLineAct->setStatusTip(tr("Display the part on the highlighted line in a popup 3D viewer"));
    connect(previewLineAct, SIGNAL(triggered()), this, SLOT(previewLine()));

#ifdef QT_DEBUG_MODE
    previewViewerFileAct = new QAction(QIcon(":/resources/previewpart.png"),tr("Preview 3DViewer file..."), this);
    previewViewerFileAct->setStatusTip(tr("Display 3DViewer file in a popup 3D viewer"));
    connect(previewViewerFileAct, SIGNAL(triggered()), this, SLOT(previewViewerFile()));
#endif

    cutAct = new QAction(QIcon(":/resources/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcut(tr("Ctrl+X"));
    cutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard - Ctrl+X"));
    connect(cutAct, SIGNAL(triggered()), _textEdit, SLOT(cut()));

    copyAct = new QAction(QIcon(":/resources/copy.png"), tr("&Copy"), this);
    copyAct->setShortcut(tr("Ctrl+C"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard - Ctrl+C"));
    connect(copyAct, SIGNAL(triggered()), _textEdit, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/resources/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcut(tr("Ctrl+V"));
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection - Ctrl+V"));
    connect(pasteAct, SIGNAL(triggered()), _textEdit, SLOT(paste()));

    findAct = new QAction(QIcon(":/resources/find.png"), tr("&Find"), this);
    findAct->setShortcut(tr("Ctrl+F"));
    findAct->setStatusTip(tr("Find object - Ctrl+F"));
    connect(findAct, SIGNAL(triggered()), _textEdit, SLOT(findDialog()));

    redrawAct = new QAction(QIcon(":/resources/redraw.png"), tr("&Redraw"), this);
    redrawAct->setShortcut(tr("Ctrl+R"));
    redrawAct->setStatusTip(tr("Redraw page, reset model caches - Ctrl+R"));
    connect(redrawAct, SIGNAL(triggered()), this, SLOT(redraw()));

    updateAct = new QAction(QIcon(":/resources/update.png"), tr("&Update"), this);
    updateAct->setShortcut(tr("Ctrl+U"));
    updateAct->setStatusTip(tr("Update page - Ctrl+U"));
    connect(updateAct, SIGNAL(triggered(bool)), this, SLOT(update(bool)));

    delAct = new QAction(QIcon(":/resources/delete.png"), tr("&Delete"), this);
    delAct->setShortcut(tr("DEL"));
    delAct->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    delAct->setStatusTip(tr("Delete the selection - DEL"));
    connect(delAct, SIGNAL(triggered()), _textEdit, SLOT(cut()));

    selAllAct = new QAction(QIcon(":/resources/selectall.png"), tr("&Select All"), this);
    selAllAct->setShortcut(tr("Ctrl+A"));
    selAllAct->setStatusTip(tr("Select all page content - Ctrl+A"));
    connect(selAllAct, SIGNAL(triggered()), _textEdit, SLOT(selectAll()));

    showAllCharsAct = new QAction(QIcon(":/resources/showallcharacters.png"), tr("Show All Characters"), this);
    showAllCharsAct->setShortcut(tr("Ctrl+J"));
    showAllCharsAct->setStatusTip(tr("Show all characters - Ctrl+J"));
    showAllCharsAct->setCheckable(true);
    connect(showAllCharsAct, SIGNAL(triggered()), this, SLOT(showAllCharacters()));

    toggleCmmentAct = new QAction(QIcon(":/resources/togglecomment.png"), tr("Toggle Line Comment"), this);
    toggleCmmentAct->setShortcut(tr("Ctrl+D"));
    toggleCmmentAct->setStatusTip(tr("Add or remove comment from selected line - Ctrl+D"));
    connect(toggleCmmentAct, SIGNAL(triggered()), _textEdit, SLOT(toggleComment()));

    topAct = new QAction(QIcon(":/resources/topofdocument.png"), tr("Top of Document"), this);
    topAct->setShortcut(tr("Ctrl+T"));
    topAct->setStatusTip(tr("Go to the top of document - Ctrl+T"));
    connect(topAct, SIGNAL(triggered()), this, SLOT(topOfDocument()));

    bottomAct = new QAction(QIcon(":/resources/bottomofdocument.png"), tr("Bottom of Document"), this);
    bottomAct->setShortcut(tr("Ctrl+B"));
    bottomAct->setStatusTip(tr("Go to the bottom of document - Ctrl+B"));
    connect(bottomAct, SIGNAL(triggered()), this, SLOT(bottomOfDocument()));

    connect(_textEdit, SIGNAL(copyAvailable(bool)),
            cutAct,    SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(copyAvailable(bool)),
            copyAct,   SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(copyAvailable(bool)),
             delAct,   SLOT(setEnabled(bool)));

    // edit model file
    exitAct = new QAction(QIcon(":/resources/exit.png"),tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Close this window - Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    saveAct = new QAction(QIcon(":/resources/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk - Ctrl+S"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));

    saveCopyAct = new QAction(QIcon(":/resources/savecopy.png"),tr("Save a Copy As..."), this);
    saveCopyAct->setShortcut(tr("Ctrl+Shift+C"));
    saveCopyAct->setStatusTip(tr("Save a copy of the document under a new name - Ctrl+Shift+C"));
    saveCopyAct->setEnabled(false);
    connect(saveCopyAct, SIGNAL(triggered()), this, SLOT(saveFileCopy()));

    undoAct = new QAction(QIcon(":/resources/editundo.png"), tr("Undo"), this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last change - Ctrl+Z"));
    connect(undoAct, SIGNAL(triggered()), _textEdit, SLOT(undo()));
    redoAct = new QAction(QIcon(":/resources/editredo.png"), tr("Redo"), this);
#ifdef __APPLE__
    redoAct->setShortcut(tr("Ctrl+Shift+Z"));
    redoAct->setStatusTip(tr("Redo last change - Ctrl+Shift+Z"));
#else
    redoAct->setShortcut(tr("Ctrl+Y"));
    redoAct->setStatusTip(tr("Redo last change - Ctrl+Y"));
#endif
    connect(redoAct, SIGNAL(triggered()), _textEdit, SLOT(redo()));

    preferencesAct = new QAction(QIcon(":/resources/preferences.png"),tr("Preferences"), this);
    preferencesAct->setStatusTip(tr("Set your preferences for LDraw Editor"));
    connect(preferencesAct, SIGNAL(triggered()), this, SLOT(preferences()));

    openFolderAct = new QAction(QIcon(":/resources/openworkingfolder.png"),tr("Open Working Folder"), this);
    openFolderAct->setShortcut(tr("Alt+Shift+2"));
    openFolderAct->setStatusTip(tr("Open file working folder - Alt+Shift+2"));
    connect(openFolderAct, SIGNAL(triggered()), this, SLOT(openFolder()));

    copyFullPathToClipboardAct = new QAction(QIcon(":/resources/copytoclipboard.png"),tr("Full Path to Clipboard"), this);
    copyFullPathToClipboardAct->setShortcut(tr("Alt+Shift+3"));
    copyFullPathToClipboardAct->setStatusTip(tr("Copy full file path to clipboard - Alt+Shift+3"));
    connect(copyFullPathToClipboardAct, SIGNAL(triggered()), this, SLOT(updateClipboard()));

    copyFileNameToClipboardAct = new QAction(QIcon(":/resources/copytoclipboard.png"),tr("File Name to Clipboard"), this);
    copyFileNameToClipboardAct->setShortcut(tr("Alt+Shift+0"));
    copyFileNameToClipboardAct->setStatusTip(tr("Copy file name to clipboard - Alt+Shift+0"));
    connect(copyFileNameToClipboardAct, SIGNAL(triggered()), this, SLOT(updateClipboard()));

    openWithToolbarAct = new QAction(QIcon(":/resources/openwith.png"), tr("Open With..."), this);
    openWithToolbarAct->setStatusTip(tr("Open model file with selected application"));

    editColorAct = new QAction(QIcon(":/resources/editcolor.png"),tr("Change color..."), this);
    editColorAct->setStatusTip(tr("Edit the part color"));
    connect(editColorAct, SIGNAL(triggered()), this, SLOT(editLineItem()));

    editPartAct = new QAction(QIcon(":/resources/editpart.png"),tr("Change part..."), this);
    editPartAct->setStatusTip(tr("Edit this part"));
    connect(editPartAct, SIGNAL(triggered()), this, SLOT(editLineItem()));
/*
    substitutePartAct = new QAction(QIcon(":/resources/editplisubstituteparts.png"),tr("Substitute part..."), this);
    substitutePartAct->setStatusTip(tr("Substitute this part"));
    connect(substitutePartAct, SIGNAL(triggered()), this, SLOT(editLineItem()));

    removeSubstitutePartAct = new QAction(QIcon(":/resources/removesubstitutepart.png"),tr("Remove Substitute..."), this);
    removeSubstitutePartAct->setStatusTip(tr("Replace this substitute part with the original part."));
    connect(substitutePartAct, SIGNAL(triggered()), this, SLOT(editLineItem()));
*/
    connect(_textEdit, SIGNAL(undoAvailable(bool)),
             undoAct,  SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(redoAvailable(bool)),
             redoAct,  SLOT(setEnabled(bool)));
    connect(saveAct,   SIGNAL(triggered(bool)),
             this,     SLOT(  updateDisabled(bool)));
    connect(_textEdit, SIGNAL(textChanged()),
             this,     SLOT(enableSave()));

    disableActions();
}

void EditWindow::disableActions()
{
    editModelFileAct->setEnabled(false);
    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    delAct->setEnabled(false);
    updateAct->setEnabled(false);

    redrawAct->setEnabled(false);
    selAllAct->setEnabled(false);
    findAct->setEnabled(false);
    topAct->setEnabled(false);
    toggleCmmentAct->setEnabled(false);
    bottomAct->setEnabled(false);
    showAllCharsAct->setEnabled(false);

    openFolderAct->setEnabled(false);
    copyFullPathToClipboardAct->setEnabled(false);
    copyFileNameToClipboardAct->setEnabled(false);

    undoAct->setEnabled(false);
    redoAct->setEnabled(false);
    saveAct->setEnabled(false);
    saveCopyAct->setEnabled(false);
    openWithToolbarAct->setEnabled(false);
}

void EditWindow::enableActions()
{
    editModelFileAct->setEnabled(true);

    redrawAct->setEnabled(true);
    selAllAct->setEnabled(true);
    findAct->setEnabled(true);
    topAct->setEnabled(true);
    saveCopyAct->setEnabled(true);
    toggleCmmentAct->setEnabled(true);
    bottomAct->setEnabled(true);
    showAllCharsAct->setEnabled(true);
    openFolderAct->setEnabled(true);
}

void EditWindow::clearEditorWindow()
{
    fileName.clear();
    disableActions();
}

void EditWindow::createToolBars()
{
    editToolBar = addToolBar(tr("Editor Edit Toolbar"));
    editToolBar->setObjectName("EditorEditToolbar");
    if (modelFileEdit()) {
        mpdCombo = new QComboBox(this);
        mpdCombo->setMinimumContentsLength(25);
        mpdCombo->setInsertPolicy(QComboBox::InsertAtBottom);
        mpdCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
        mpdCombo->setToolTip(tr("Go to submodel"));
        mpdCombo->setStatusTip("Use dropdown to go to submodel");
        connect(mpdCombo,SIGNAL(activated(int)),
                this,    SLOT(mpdComboChanged(int)));

        editToolBar->addAction(preferencesAct);
        editToolBar->addSeparator();
        editToolBar->addAction(exitAct);
        editToolBar->addAction(saveAct);
        editToolBar->addAction(saveCopyAct);
        editToolBar->addSeparator();
        editToolBar->addAction(undoAct);
        editToolBar->addAction(redoAct);

        mpdComboSeparatorAct = editToolBar->addSeparator();
        mpdComboAct = editToolBar->addWidget(mpdCombo);

#ifndef QT_NO_CLIPBOARD
        editToolBar->addSeparator();
        editToolBar->addAction(copyFileNameToClipboardAct);
        editToolBar->addAction(copyFullPathToClipboardAct);
#endif
        editToolBar->addSeparator();
        editToolBar->addAction(openFolderAct);
        QMenu *openWithMenu = new QMenu(tr("Open With Menu"));
        openWithMenu->setEnabled(false);
        openWithToolbarAct->setMenu(openWithMenu);
        if (numOpenWithPrograms) {
            openWithMenu->setEnabled(true);
            for (int i = 0; i < numOpenWithPrograms; i++)
                openWithMenu->addAction(openWithActList.at(i));
        }
        editToolBar->addAction(openWithToolbarAct);
        editToolBar->addSeparator();
    } else {
        editToolBar->addAction(preferencesAct);
    }
    editToolBar->addAction(topAct);
    editToolBar->addAction(bottomAct);
    editToolBar->addAction(toggleCmmentAct);
//    editToolBar->addAction(showAllCharsAct);
    editToolBar->addAction(selAllAct);
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(findAct);
    editToolBar->addAction(delAct);
    editToolBar->addAction(updateAct);
    editToolBar->addAction(redrawAct);

    toolsToolBar = addToolBar(tr("Editor Tools Toolbar"));
    toolsToolBar->setObjectName("EditorToolsToolbar");
    toolsToolBar->setEnabled(false);

    toolsToolBar->addAction(editColorAct);
    toolsToolBar->addAction(editPartAct);
/*    toolsToolBar->addAction(substitutePartAct); */
    if (modelFileEdit())
        toolsToolBar->addAction(previewLineAct);
#ifdef QT_DEBUG_MODE
    toolsToolBar->addSeparator();
    toolsToolBar->addAction(previewViewerFileAct);
#endif
}

void EditWindow::setReadOnly(bool enabled)
{
    QString title = windowTitle();

    disconnect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
               this,                  SLOT(  contentsChange(int,int,int)));

    isReadOnly = enabled;

    _textEdit->setReadOnly(isReadOnly);

    if (isReadOnly) {
        title = tr("%1 (Read-Only)").arg(title);
        updateAct->setVisible(      !isReadOnly);
        redrawAct->setVisible(      !isReadOnly);
        saveAct->setVisible(        !isReadOnly);
        undoAct->setVisible(        !isReadOnly);
        redoAct->setVisible(        !isReadOnly);
        toggleCmmentAct->setVisible(!isReadOnly);
        cutAct->setVisible(         !isReadOnly);
        pasteAct->setVisible(       !isReadOnly);
        delAct->setVisible(         !isReadOnly);
    }

    setWindowTitle(title);
}

bool EditWindow::setValidPartLine()
{
    QString partType, titleType = "part";
    int validCode, colorCode = LDRAW_MATERIAL_COLOUR;
    QTextCursor cursor = _textEdit->textCursor();
    cursor.select(QTextCursor::LineUnderCursor);
    QString selection = cursor.selection().toPlainText();
    QStringList list;
    bool colorOk = false, isSubstitute = false;

    toolsToolBar->setEnabled(false);
    editColorAct->setText(tr("Edit color"));
    editPartAct->setText(tr("Edit part"));
/*    substitutePartAct->setText(tr("Substitute part")); */
    copyFullPathToClipboardAct->setEnabled(false);
    copyFileNameToClipboardAct->setEnabled(false);

    if (selection.startsWith("1 ")) {
        list = selection.split(" ", QString::SkipEmptyParts);

        validCode = list[1].toInt(&colorOk);

        // 0 1           2 3 4 5 6 7 8 9 10 11 12 13 14
        // 1 <colorCode> 0 0 0 0 0 1 1 0 0  0  1  0  <part type>
        for (int i = 14; i < list.size(); i++)
            partType += (list[i]+" ");

    } else if (selection.contains(" PLI BEGIN SUB ")) {
        // 0 1     2   3     4   5           6
        // 0 !LPUB PLI BEGIN SUB <part type> <colorCode>
        list = selection.split(" ", QString::SkipEmptyParts);

        partType = list[5];

        validCode = list[6].toInt(&colorOk);

        selection.append("|sub");

        isSubstitute = true;

    } else {
        return false;
    }

    if (partType.isEmpty())
        return false;

    toolsToolBar->setEnabled(true);

    if (colorOk)
        colorCode = validCode;

    partType = partType.trimmed();

    emit lpubAlert->messageSig(LOG_DEBUG,
                               QString("Editor PartType: %1, ColorCode: %2, Line: %3")
                               .arg(partType).arg(colorCode).arg(selection));

    QString partKey = QString("%1|%2").arg(colorCode).arg(partType);

    QString elidedPartType = partType.size() > 20 ? QString(partType.left(17) + "..." + partType.right(3)) : partType;

    lcPreferences& Preferences = lcGetPreferences();
    if (modelFileEdit()) {
        if (Preferences.mPreviewEnabled && !isIncludeFile) {
            previewLineAct->setText(tr("Preview %1 %2...").arg(titleType).arg(elidedPartType));
            previewLineAct->setData(partKey);
            previewLineAct->setEnabled(true);
        }
    }

    if (_subFileList.contains(partType.toLower())) {
        if (modelFileEdit()) {
            titleType = "subfile";
            if (Preferences.mPreviewEnabled) {
                previewLineAct->setText(tr("Preview %1 %2...").arg(titleType).arg(elidedPartType));
                previewLineAct->setStatusTip(tr("Display the %1 on the highlighted line in a popup 3D viewer").arg(titleType));
            }
        }

        copyFullPathToClipboardAct->setEnabled(true);
        copyFullPathToClipboardAct->setData(partType);
    }

    copyFileNameToClipboardAct->setEnabled(true);
    copyFileNameToClipboardAct->setData(partType);

    if (colorCode != LDRAW_MATERIAL_COLOUR) {
        editColorAct->setText(tr("Edit line color %1 (%2)...").arg(gColorList[lcGetColorIndex(colorCode)].Name).arg(colorCode));
        editColorAct->setData(QString("%1|%2").arg(colorCode).arg(selection));
        editColorAct->setEnabled(true);
    }

    editPartAct->setText(tr("Edit line  %1 %2...").arg(titleType).arg(elidedPartType));
    editPartAct->setData(QString("%1|%2").arg(partType).arg(colorCode));
    editPartAct->setEnabled(true);

/*
    actionText = tr("Substitute  %1...").arg(elidedPartType);
    if (isSubstitute) {
        substitutePartAct->setText(tr("Change %1").arg(actionText));
        removeSubstitutePartAct->setText(tr("Remove %1").arg(actionText));
        removeSubstitutePartAct->setData(partKey);
        removeSubstitutePartAct->setEnabled(true);
        QMenu *removeMenu = new QMenu(tr("Remove %1").arg(actionText));
        removeMenu->setIcon(QIcon(":/resources/removesubstitutepart.png"));
        removeMenu->addAction(removeSubstitutePartAct);
        substitutePartAct->setMenu(removeMenu);
        partKey.append("|sub");
    } else {
        substitutePartAct->setText(actionText);
    }
    substitutePartAct->setData(partKey);
    substitutePartAct->setEnabled(true);
*/

    if (numOpenWithPrograms)
        openWithToolbarAct->setEnabled(true);

    return true;
}

void EditWindow::showContextMenu(const QPoint &pt)
{
    QMenu *menu = _textEdit->createStandardContextMenu();

    if (!fileName.isEmpty()) {
        if (_subFileListPending) {
            emit getSubFileListSig();
            while (_subFileListPending)
                QApplication::processEvents();
        }

        menu->addSeparator();
        menu->addAction(openFolderAct);
#ifndef QT_NO_CLIPBOARD
        menu->addAction(copyFileNameToClipboardAct);
        menu->addAction(copyFullPathToClipboardAct);
#endif
        if (!modelFileEdit()) {
            editModelFileAct->setText(tr("Edit %1").arg(QFileInfo(fileName).fileName()));
            editModelFileAct->setStatusTip(tr("Edit %1 in detached LDraw Editor").arg(QFileInfo(fileName).fileName()));
            menu->addAction(editModelFileAct);
        }

        if (setValidPartLine()) {
            menu->addSeparator();
            QMenu *toolsMenu = new QMenu(tr("Tools..."));
            toolsMenu->setIcon(QIcon(":/resources/tools.png"));
            menu->addMenu(toolsMenu);
            toolsMenu->addAction(editColorAct);
            toolsMenu->addAction(editPartAct);
/*            toolsMenu->addAction(substitutePartAct); */

            if (modelFileEdit())
                toolsMenu->addAction(previewLineAct);

            QMenu *openWithMenu = new QMenu(tr("Open With..."));
            openWithMenu->setIcon(QIcon(":/resources/openwith.png"));
            menu->addMenu(openWithMenu);
            if (numOpenWithPrograms) {
                for (int i = 0; i < numOpenWithPrograms; i++)
                    openWithMenu->addAction(openWithActList.at(i));
            }
        }
        menu->addSeparator();
    }

    menu->addAction(topAct);
    menu->addAction(bottomAct);
    menu->addAction(toggleCmmentAct);
    menu->addAction(findAct);
    menu->addAction(updateAct);
    menu->addAction(redrawAct);
    menu->exec(_textEdit->mapToGlobal(pt));
    delete menu;
}

void EditWindow::editLineItem()
{
    QString findText;
    QString replaceText;

    QStringList elements, items;
    if (sender() == editColorAct) {
        elements = editColorAct->data().toString().split("|");
        int colorCode = elements.first().toInt();
        int newColorIndex = -1;
        items = elements.at(1).split(" ", QString::SkipEmptyParts);
        QColor qcolor(gColorList[lcGetColorIndex(colorCode)].CValue);
        QColor newColor = LDrawColorDialog::getLDrawColor(colorCode, newColorIndex, this);
        if (newColor.isValid() && qcolor != newColor) {
            if (elements.size() == 3)  // selection is a substitute meta command
                items[6] = QString::number(lcGetColorCode(newColorIndex));
            else
                items[1] = QString::number(lcGetColorCode(newColorIndex));
        }
        replaceText = items.join(" ");
    } else if (sender() == editPartAct) {
        elements = editPartAct->data().toString().split("|");
        findText = elements.first();
        PieceInfo *partInfo = LDrawPartDialog::getLDrawPart(QString("%1;%2").arg(findText).arg(elements.last()));
        if (partInfo)
            replaceText = partInfo->mFileName;
        else
            replaceText = findText;
    }
/*
    else if (sender() == substitutePartAct) {
        QStringList items = substitutePartAct->data().toString().split("|");
        if (items.size() > 1) {

        }
    } else if (sender() == removeSubstitutePartAct) {
        ;
    }
*/

    int current = 0;
    int selectedLines = 0;

    QTextCursor cursor = _textEdit->textCursor();
    if(cursor.selection().isEmpty())
        cursor.select(QTextCursor::LineUnderCursor);

    QString str = cursor.selection().toPlainText();
    selectedLines = str.count("\n")+1;

    cursor.beginEditBlock();

    while (true)
    {
        if (current == selectedLines) {
           break;
        }

        bool result = false;

        cursor.select(QTextCursor::LineUnderCursor);
        QString selection = cursor.selectedText();
        if (!selection.isEmpty())
        {
            if (sender() == editPartAct) {
                QTextDocument::FindFlags flags;
                result = _textEdit->find(findText, flags);
            } else {
                _textEdit->setTextCursor(cursor);
                result = true;
            }
        } else {
            break;
        }

        if (result) {
            _textEdit->textCursor().insertText(replaceText);
            if (++current < selectedLines) {
                cursor.movePosition(QTextCursor::Down);
                _textEdit->setTextCursor(cursor);
            }
        }
    }

    cursor.endEditBlock();
}

void EditWindow::triggerPreviewLine()
{
    if (!isIncludeFile) {
        if (_subFileListPending) {
            emit getSubFileListSig();
            while (_subFileListPending)
                QApplication::processEvents();
        }
        if (setValidPartLine())
            emit previewLineAct->triggered();
    }
}

void EditWindow::setSubmodel(int index)
{
    mpdCombo->setCurrentIndex(index);
}

void EditWindow::mpdComboChanged(int index)
{
    Q_UNUSED(index)
    QString newSubFile = mpdCombo->currentText();
    if (_curSubFile != newSubFile) {
        _curSubFile = newSubFile;
        QString findText = QString("0 FILE %1").arg(_curSubFile);
        _textEdit->moveCursor(QTextCursor::Start);
        if (!_textEdit->find(findText))
            statusBar()->showMessage(tr("Did not find submodel '%1'").arg(findText));
    }
}

void EditWindow::contentsChange(
  int position,
  int charsRemoved,
  int charsAdded)
{
  QString addedChars;

  if (charsAdded) {
    addedChars = _textEdit->toPlainText();
    if (addedChars.size() == 0) {
      return;
    }

    addedChars = addedChars.mid(position,charsAdded);
  }

  contentsChange(fileName, position, charsRemoved, addedChars);

  if (!Preferences::saveOnUpdate) {
     updateDisabled(false);
  }
}

void EditWindow::openFolderSelect(const QString& absoluteFilePath)
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

void EditWindow::openFolder() {
    if (!fileName.isEmpty()) {
        QString filePath = QDir::fromNativeSeparators(fileName);
        if (!filePath.count("/"))
            filePath = QDir::currentPath() + QDir::separator() + Paths::tmpDir + QDir::separator() + fileName;
        openFolderSelect(filePath);
    }
}

#ifndef QT_NO_CLIPBOARD
void EditWindow::updateClipboard()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        bool fullPath = false;
        QString data;
        if (action == copyFullPathToClipboardAct) {
            fullPath = true;
            QString fullFilePath = QDir::currentPath() + "/" + Paths::tmpDir + "/" + action->data().toString();
            data =  QDir::toNativeSeparators(fullFilePath );
        } else if (action == copyFileNameToClipboardAct) {
            data = action->data().toString();
        }

        if (data.isEmpty()) {
            emit lpubAlert->messageSig(LOG_ERROR, QString("Copy to clipboard - Sender: %1, No data detected")
                                               .arg(sender()->metaObject()->className()));
            return;
        }

        QGuiApplication::clipboard()->setText(data, QClipboard::Clipboard);

        QString efn =QFileInfo(data).fileName();
        // Text elided to 20 chars
        QString _fileName = QString("File '%1' %2")
                           .arg(efn.size() > 20 ?
                                efn.left(17) + "..." +
                                efn.right(3) : efn)
                           .arg(fullPath ? "full path" : "name");

        emit lpubAlert->messageSig(LOG_INFO_STATUS, QString("%1 copied to clipboard.").arg(_fileName));
    }
}
#endif

void EditWindow::modelFileChanged(const QString &_fileName)
{
  _saveSubfileIndex = mpdCombo->currentIndex();
  fileWatcher.removePath(_fileName);
  emit refreshModelFileSig();
}

bool EditWindow::maybeSave()
{
  if (isReadOnly)
      return false;

  if (!QFileInfo(fileName).exists())
      return false;

  bool rc = true;

  if (_textEdit->document()->isModified()) {
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

    int ExecReturn = box.exec();
    if (ExecReturn == QMessageBox::Save) {
      rc = saveFile();
    } else
    if (ExecReturn == QMessageBox::Cancel) {
      rc = false;
    }
  }
  return rc;
}

bool EditWindow::saveFile()
{
    bool rc = false;
    bool disableWatcher = true;

    // check for dirty editor
    if (_textEdit->document()->isModified())
    {
        if (modelFileEdit() && !fileName.isEmpty())
            fileWatcher.removePath(fileName);

        QAction *action = qobject_cast<QAction *>(sender());
        if (action == saveAct)
            disableWatcher = false;
        else
            emit disableWatcherSig();

        QFile file(fileName);
        if (! file.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(nullptr,
                                 tr("Model File Editor"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return rc;
        }

        if (showAllCharsAct->isChecked()) {
            _textEdit->blockSignals(true);
            _textEdit->showAllCharacters(false);
        }

        QTextDocumentWriter writer(fileName, "plaintext");
        writer.setCodec(_textEdit->getIsUTF8() ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
        rc = writer.write(_textEdit->document());

        if (rc){
            saveAct->setEnabled(false);
            _textEdit->document()->setModified(false);
            statusBar()->showMessage(tr("File %1 saved").arg(fileName), 2000);
        }

        if (disableWatcher)
            emit enableWatcherSig();

        if (showAllCharsAct->isChecked()) {
            _textEdit->showAllCharacters(true);
            _textEdit->blockSignals(false);
        }

        if (modelFileEdit() && !fileName.isEmpty())
            fileWatcher.addPath(fileName);
    }

  return rc;
}

bool EditWindow::saveFileCopy()
{
  QFileInfo fileInfo(fileName);
  QString extension = fileInfo.suffix().toLower();
  QString fileCopyName = QFileDialog::getSaveFileName(this,
                                                      tr("Save Copy As"),
                                                      fileInfo.baseName() + "_copy." + extension,
                                                      tr("LDraw Files (*.mpd *.ldr *.dat);;All Files (*.*)"));
  if (fileCopyName.isEmpty()) {
    return false;
  }

  fileInfo.setFile(fileCopyName);
  extension = fileInfo.suffix().toLower();

  bool rc = false;

  if (extension == "mpd" ||
      extension == "ldr" ||
      extension == "dat" ||
      extension.isEmpty()) {

      QFile file(fileCopyName);
      if (! file.open(QFile::WriteOnly | QFile::Text)) {
          QMessageBox::warning(nullptr,
                               tr("Model File Editor"),
                               tr("Cannot write file %1:\n%2.")
                               .arg(fileCopyName)
                               .arg(file.errorString()));
          return false;
      }

      if (showAllCharsAct->isChecked()) {
          _textEdit->blockSignals(true);
          _textEdit->showAllCharacters(false);
      }

      QTextDocumentWriter writer(fileCopyName, "plaintext");
      writer.setCodec(_textEdit->getIsUTF8() ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
      bool rc = writer.write(_textEdit->document());

      if (rc)
          statusBar()->showMessage(tr("File %1 saved").arg(fileCopyName));

      if (showAllCharsAct->isChecked()) {
          _textEdit->showAllCharacters(true);
          _textEdit->blockSignals(false);
      }

  } else {
    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),
                              QMessageBox::tr("Unsupported LDraw file extension %1 specified.  File not saved.")
                                .arg(extension));
  }

  return rc;
}

void EditWindow::updateSelectedParts() {

    if (isIncludeFile)
        return;

    toolsToolBar->setEnabled(setValidPartLine());

    if (modelFileEdit() || !gMainWindow || !gMainWindow->isVisible())
        return;

    int lineNumber = 0;
    int currentLine = 0;
    int selectedLines = 0;
    bool clearSelection = false;
    bool selectionStep = Preferences::editorLoadSelectionStep;
    bool highlightLines = Preferences::editorHighlightLines;
    TypeLine typeLine = { -1/*fileOrderIndex*/, 0/*lineNumber*/ };

    QVector<TypeLine> lineTypeIndexes;
    QVector<int> toggleLines;

    QTextCursor cursor = _textEdit->textCursor();

    if(!cursor.hasSelection())
        cursor.select(QTextCursor::LineUnderCursor);

    QStringList content = cursor.selection().toPlainText().split("\n");

    selectedLines = content.size();

    if (!selectedLines)
        return;

    QTextCursor::MoveOperation nextLine = cursor.anchor() < cursor.position() ? QTextCursor::Up : QTextCursor::Down;

    auto getSelectedLineNumber = [&cursor] () {

        int lineNumber = 0;

        cursor.movePosition(QTextCursor::StartOfLine);

        while(cursor.positionInBlock()>0) {
            cursor.movePosition(QTextCursor::Up);
            lineNumber++;
        }

        QTextBlock block = cursor.block().previous();

        while(block.isValid()) {
            lineNumber += block.lineCount();
            block = block.previous();
        }

        return lineNumber;
    };

    while (currentLine < selectedLines)
    {
        if (selectionStep)
        {
            lineNumber = getSelectedLineNumber();
            typeLine = { fileOrderIndex, lineNumber };
            if (!stepLines.isInScope(lineNumber))
            {
                clearEditorHighlightLines();
                emit setStepForLineSig(typeLine);
            }
        }

        if (content.at(currentLine).startsWith("1") ||
            content.at(currentLine).contains(" PLI BEGIN SUB "))
        {
            if (!selectionStep)
            {
                lineNumber = getSelectedLineNumber();
                typeLine = { fileOrderIndex, lineNumber };
            }

            lineTypeIndexes.append(typeLine);

            if (highlightLines) {
                if (stepLines.isInScope(lineNumber))
                {
                    toggleLines.append(lineNumber);
                    clearSelection = savedSelection.contains(lineNumber);
                    highlightSelectedLines(toggleLines, clearSelection, true/*editorSelection*/);
                    if (clearSelection)
                        savedSelection.removeAll(lineNumber);
                    else
                        savedSelection.append(lineNumber);
                    toggleLines.clear();
                }
            }
        }

        cursor.movePosition(nextLine);
        currentLine++;
    }

    if (!highlightLines)
        clearEditorHighlightLines();

    if (lineTypeIndexes.size())
       emit SelectedPartLinesSig(lineTypeIndexes);
}

void EditWindow::clearEditorHighlightLines()
{
    if (savedSelection.size()) {
        highlightSelectedLines(savedSelection, true/*clear*/, true/*editor*/);
        savedSelection.clear();
    }
}

void EditWindow::highlightSelectedLines(QVector<int> &lines, bool clear)
{
    highlightSelectedLines(lines, clear, false/*editorSelection*/);
}

void EditWindow::highlightSelectedLines(QVector<int> &lines, bool clear, bool editorSelection)
{
    auto highlightLines = [this, &editorSelection] (QVector<int> &linesToFormat, bool clear)
    {
        QTextCursor highlightCursor(_textEdit->document());

        QTextCursor textCursor(_textEdit->document());

        if (!_textEdit->isReadOnly()) {

            QColor lineColor = QColor(Qt::transparent);

            bool applyFormat = linesToFormat.size() || (editorSelection && savedSelection.size());
            if (applyFormat) {
                if (clear) {
                    if (Preferences::displayTheme == THEME_DARK) {
                        lineColor = QColor(Preferences::themeColors[THEME_DARK_PALETTE_BASE]);
                    } else {
                        lineColor = QColor(Preferences::themeColors[THEME_DEFAULT_VIEWER_BACKGROUND_COLOR]);
                    }
                } else {
                    if (Preferences::displayTheme == THEME_DARK) {
                        lineColor = QColor(editorSelection ?
                                               Preferences::themeColors[THEME_DARK_LINE_HIGHLIGHT_EDITOR_SELECT] :
                                               Preferences::themeColors[THEME_DARK_LINE_HIGHLIGHT_VIEWER_SELECT]);
                        lineColor.setAlpha(100); // make 60% transparent
                    } else {
                        lineColor = QColor(editorSelection ?
                                               Preferences::themeColors[THEME_DEFAULT_LINE_HIGHLIGHT_EDITOR_SELECT] :
                                               Preferences::themeColors[THEME_DEFAULT_LINE_HIGHLIGHT_VIEWER_SELECT]);
                    }
                    lineColor = lineColor.lighter(180);
                }
            }

            QTextCharFormat plainFormat(highlightCursor.charFormat());
            QTextCharFormat colorFormat = plainFormat;
            colorFormat.setBackground(lineColor);

            if (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
                if (applyFormat) {
                    for (int i = 0; i < linesToFormat.size(); ++i) {
                        QTextBlock block = _textEdit->document()->findBlockByLineNumber(linesToFormat.at(i));
                        int blockPos     = block.position();
                        highlightCursor.setPosition(blockPos);

                        if (!highlightCursor.isNull()) {
                            highlightCursor.select(QTextCursor::LineUnderCursor);
                            highlightCursor.mergeCharFormat(colorFormat);
                        }
                    }
                } else {
                    highlightCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
                    highlightCursor.mergeCharFormat(colorFormat);
                }
            }
        }
    };

    disconnect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
               this,                  SLOT(  contentsChange(int,int,int)));

    // apply or clear savedSelection lines if any
    if (savedSelection.size()) {
        QVector<int> validSelection;
        bool clearSelection = !editorSelection;
        if (clearSelection) {
            validSelection = savedSelection;
            savedSelection.clear();
        } else {
            for (int line : savedSelection)
                if (!lines.contains(line))
                    validSelection.append(line);
            if (validSelection.size())
                savedSelection = validSelection;
        }
        highlightLines(validSelection, clearSelection);
    }

    // apply highlighting, toggle on from editor or selection from viewer
    highlightLines(lines, clear);

    connect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
            this,                  SLOT(  contentsChange(int,int,int)));
}

void EditWindow::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (isIncludeFile) {
        QTextCursor cursor = _textEdit->textCursor();
        cursor.select(QTextCursor::LineUnderCursor);
        QString selection = cursor.selection().toPlainText();
        if (selection.startsWith("1"))
            showLineType = LINE_ERROR;
        else
            showLineType = LINE_HIGHLIGHT;
    }

    if (!_textEdit->isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor;
        if (Preferences::displayTheme == THEME_DARK) {
            if (showLineType == LINE_ERROR)
                lineColor = QColor(Preferences::themeColors[THEME_DARK_LINE_ERROR]).lighter(180);
            else
                lineColor = QColor(Preferences::themeColors[THEME_DARK_LINE_HIGHLIGHT]);
        } else {
            if (showLineType == LINE_ERROR)
                lineColor = QColor(Preferences::themeColors[THEME_DEFAULT_LINE_ERROR]);
            else
                lineColor = QColor(Preferences::themeColors[THEME_DEFAULT_LINE_HIGHLIGHT]);
        }

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = _textEdit->textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

     _textEdit->setExtraSelections(extraSelections);
}

void EditWindow::topOfDocument(){
    _textEdit->moveCursor(QTextCursor::Start);
}

void EditWindow::bottomOfDocument(){
    _textEdit->moveCursor(QTextCursor::End);
}

void EditWindow::showAllCharacters(){
    _textEdit->blockSignals(true);
    _textEdit->showAllCharacters(showAllCharsAct->isChecked());
    _textEdit->blockSignals(false);
    _textEdit->document()->setModified(false);
}

void EditWindow::pageUpDown(
  QTextCursor::MoveOperation op,
  QTextCursor::MoveMode      moveMode)
{
  disconnect(verticalScrollBar, SIGNAL(valueChanged(int)),
             this,              SLOT(verticalScrollValueChanged(int)));

  QTextCursor cursor = _textEdit->textCursor();
  bool moved = false;
  qreal lastY = _textEdit->cursorRect(cursor).top();
  qreal distance = 0;
  qreal delta;
  // move using movePosition to keep the cursor's x
  do {
    qreal y = _textEdit->cursorRect(cursor).top();
    delta = qAbs(y - lastY);
    distance += delta;
    lastY = y;
    moved = cursor.movePosition(op, moveMode);
  } while (moved && distance < _textEdit->viewport()->height());

  if (moved) {
    if (op == QTextCursor::Up) {
      cursor.movePosition(QTextCursor::Down, moveMode);
      verticalScrollBar->triggerAction(QAbstractSlider::SliderPageStepAdd);
    } else {
      cursor.movePosition(QTextCursor::Up, moveMode);
      verticalScrollBar->triggerAction(QAbstractSlider::SliderPageStepSub);
    }
  }

  connect(verticalScrollBar, SIGNAL(valueChanged(int)),
          this,              SLOT(verticalScrollValueChanged(int)));
}

void EditWindow::showLine(int lineNumber, int lineType)
{
  showLineType = lineType;
  showLineNumber = lineNumber;

  if (Preferences::editorBufferedPaging &&
      showLineNumber > Preferences::editorLinesPerPage &&
      showLineNumber > _pageIndx) {

      int linesNeeded = showLineNumber - _pageIndx;
      int pages = linesNeeded / Preferences::editorLinesPerPage;

      if ((linesNeeded % Preferences::editorLinesPerPage))
          pages++;

      if (pages) {
          waitingSpinnerStart();

          emit lpubAlert->messageSig(LOG_INFO_STATUS,QString("Show Line %1 - Loading buffered page %2 lines...")
                                     .arg(lineNumber).arg(linesNeeded));

          for (int i = 0; i < pages && !_contentLoading; i++) {
              QApplication::processEvents();
              loadPagedContent();
          }

          emit lpubAlert->messageSig(LOG_STATUS,QString());
#ifdef QT_DEBUG_MODE
          emit lpubAlert->messageSig(LOG_DEBUG,QString("ShowLine add %1 %2 to line %3 from line %4.")
                                     .arg(pages).arg(pages == 1 ? "page" : "pages").arg(lineNumber).arg(_pageIndx + 1));
#endif
          waitingSpinnerStop();
      }
  }

  _textEdit->moveCursor(QTextCursor::Start,QTextCursor::MoveAnchor);
  for (int i = 0; i < showLineNumber; i++)
    _textEdit->moveCursor(QTextCursor::Down/*QTextCursor::EndOfLine*/,QTextCursor::MoveAnchor/*QTextCursor::KeepAnchor*/);
  _textEdit->ensureCursorVisible();

  pageUpDown(QTextCursor::Up, QTextCursor::KeepAnchor);
}

void EditWindow::updateDisabled(bool state){
    if (sender() == saveAct)
    {
        updateAct->setDisabled(true);
    } else
    if (sender() == updateAct &&
       !Preferences::saveOnUpdate)
    {
        updateAct->setDisabled(state);
        if (!modelFileEdit())
            emit updateDisabledSig(state);
    } else {
        updateAct->setDisabled(state);
    }
}

void EditWindow::setSubFiles(const QStringList& subFiles){
    _subFileList = subFiles;
    _subFileListPending = false;
}

void EditWindow::setPagedContent(const QStringList & content)
{
#ifdef QT_DEBUG_MODE
    emit lpubAlert->messageSig(LOG_DEBUG,QString("Set paged content line count: %1, content size %2")
                               .arg(lineCount)
                               .arg(content.size()));
#endif
    _pageContent = content;
    if (_pageContent.size())
        loadPagedContent();
}

void EditWindow::setPlainText(const QString &content)
{
#ifdef QT_DEBUG_MODE
    emit lpubAlert->messageSig(LOG_DEBUG,QString("Set plain text line count: %1, content size %2")
                               .arg(lineCount)
                               .arg(content.count(QRegExp("\\r\\n?|\\n")) + 1));
#endif
    _textEdit->setPlainText(content);
}

void EditWindow::setLineScope(const StepLines& lineScope)
{
    stepLines = lineScope;
}

void EditWindow::setLineCount(int count)
{
    lineCount = count;
}

void EditWindow::displayFile(
  LDrawFile       *ldrawFile,
  const QString   &fileName)
{
   const StepLines lineScope;
   displayFile(ldrawFile, fileName, lineScope);
}

void EditWindow::displayFile(
  LDrawFile       *ldrawFile,
  const QString   &_fileName,
  const StepLines &lineScope)
{
  reloaded        = _fileName == fileName;
  fileName        = _fileName;
  lineCount       = 0;
  _pageIndx       = 0;
  _contentLoaded  = false;
  _waitingSpinner = nullptr;
  displayTimer.start();
  QString content;

  disconnect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
             this,                  SLOT(  contentsChange(int,int,int)));

  if (ldrawFile) {
    _subFileListPending = true;
    fileOrderIndex = ldrawFile->getSubmodelIndex(_fileName);
    isIncludeFile  = ldrawFile->isIncludeFile(_fileName);
    stepLines      = lineScope;
    clearEditorHighlightLines();
  }

#ifdef QT_DEBUG_MODE
    emit lpubAlert->messageSig(LOG_DEBUG,QString("1. Editor Load Starting..."));
#endif

  if (fileName == "") {
    _textEdit->document()->clear();

  } else if (modelFileEdit()) {
    if (!ldrawFile && !QFileInfo(_fileName).exists()) {
      _textEdit->document()->setModified(false);
      connect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
              this,                  SLOT(  contentsChange(int,int,int)));
      return;
    }

    waitingSpinnerStart();

    fileWatcher.removePath(fileName);

    disconnect(_textEdit, SIGNAL(textChanged()),
               this,      SLOT(enableSave()));

    _textEdit->document()->clear();

    QString discFileName = QDir::fromNativeSeparators(fileName);
    bool useDiscFile = discFileName.count("/");

    bool isUTF8 = ldrawFile ? LDrawFile::_currFileIsUTF8 : true;
    _textEdit->setIsUTF8(isUTF8);

    QFuture<int> loadModelFuture = QtConcurrent::run(loadModelWorker->loadModel, ldrawFile, fileName, _modelFileEdit, isUTF8, useDiscFile);
    futureWatcher.setFuture(loadModelFuture);

  } // Detached Editor
  else
  {

    waitingSpinnerStart();

    lineCount = ldrawFile->size(fileName);

    if (Preferences::editorBufferedPaging && lineCount > Preferences::editorLinesPerPage) {
#ifdef QT_DEBUG_MODE
      emit lpubAlert->messageSig(LOG_DEBUG,QString("3. Editor Load Paged Text Started..."));
#endif
      _pageContent = ldrawFile->contents(fileName);

      loadPagedContent();

    } else {
#ifdef QT_DEBUG_MODE
      emit lpubAlert->messageSig(LOG_DEBUG,QString("3. Editor Load Plain Text Started..."));
#endif
      // loadContentBlocks(ldrawFile->contents(fileName),true/ *initial load* /);

      _textEdit->setPlainText(ldrawFile->contents(fileName).join("\n"));
    }

    waitingSpinnerStop();

    _textEdit->document()->setModified(false);

    connect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
            this,                  SLOT(  contentsChange(int,int,int)));

    enableActions();

#ifdef QT_DEBUG_MODE
    emit lpubAlert->messageSig(LOG_DEBUG,QString("6. %1 Document loaded with %2 lines")
                                                 .arg(QFileInfo(fileName).baseName())
                                                 .arg(_textEdit->document()->lineCount()));
#endif
  }  // Docked Editor
}

void EditWindow::contentLoaded()
{
    waitingSpinnerStop();

    const QString message = tr("%1 File %2: %3, %4 lines - %5")
            .arg(isIncludeFile ? "Include" : "Model")
            .arg(reloaded ? "Updated" : "Loaded")
            .arg(QFileInfo(fileName).fileName())
            .arg(lineCount)
            .arg(lpubAlert->elapsedTime(displayTimer.elapsed()));

    if (futureWatcher.future().result()) {
        _textEdit->document()->clear();
        _textEdit->document()->setModified(false);
        connect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
                this,                  SLOT(  contentsChange(int,int,int)));
        emit lpubAlert->messageSig(LOG_ERROR, QString("Editor not loaded for %1").arg(fileName));
        return;
    }

    if (Preferences::modeGUI) {
        if (modelFileEdit()) {
            bool enableMpdCombo = !isIncludeFile && _subFileList.size();
            mpdCombo->setEnabled(enableMpdCombo);
            if (enableMpdCombo) {
                bool mpdComboActionFound = false;
                for (QAction *action : editToolBar->actions()) {
                    if ((mpdComboActionFound = action == mpdComboSeparatorAct))
                        break;
                }
                mpdCombo->setMaxCount(0);
                mpdCombo->setMaxCount(1000);
                mpdCombo->addItems(_subFileList);
                if(_saveSubfileIndex) {
                    mpdCombo->setCurrentIndex(_saveSubfileIndex);
                    _saveSubfileIndex = 0;
                }
                if (!mpdComboActionFound) {
                    int index = editToolBar->actions().indexOf(redoAct) + 1;
                    editToolBar->insertAction(editToolBar->actions().at(index), mpdComboAct);
                    editToolBar->insertAction(mpdComboAct, mpdComboSeparatorAct);
                }
            } else {
                editToolBar->removeAction(mpdComboSeparatorAct);
                editToolBar->removeAction(mpdComboAct);
            }

            connect(_textEdit,  SIGNAL(textChanged()),
                    this,       SLOT(enableSave()));

            exitAct->setEnabled(true);

            statusBar()->showMessage(message);

            if (modelFileEdit()) {
                if(QFileInfo(fileName).exists())
                    fileWatcher.addPath(fileName);
            }
        }

        _contentLoaded = true;

        _textEdit->document()->setModified(false);

        connect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
                this,                  SLOT(  contentsChange(int,int,int)));

        enableActions();
    }

#ifdef QT_DEBUG_MODE
    previewViewerFileAct->setEnabled(true);
    emit lpubAlert->messageSig(LOG_DEBUG,"5. "+message);
#endif
}

void EditWindow::waitingSpinnerStart()
{
    if (!Preferences::modeGUI)
        return;

    if (_waitingSpinner) {
        if (_waitingSpinner->isSpinning())
            _waitingSpinner->stop();
    } else {
        QColor spinnerColor(
               Preferences::displayTheme == THEME_DARK ?
               Preferences::themeColors[THEME_DARK_PALETTE_TEXT] : LPUB3D_DEFAULT_COLOUR);
        _waitingSpinner = new WaitingSpinnerWidget(this);
        _waitingSpinner->setColor(QColor(spinnerColor));
        _waitingSpinner->setRoundness(70.0);
        _waitingSpinner->setMinimumTrailOpacity(15.0);
        _waitingSpinner->setTrailFadePercentage(70.0);
        _waitingSpinner->setNumberOfLines(12);
        _waitingSpinner->setLineLength(10);
        _waitingSpinner->setLineWidth(5);
        _waitingSpinner->setInnerRadius(10);
        _waitingSpinner->setRevolutionsPerSecond(1);
        _waitingSpinner->setTextColor(_waitingSpinner->color());
        _waitingSpinner->setText(tr("Loading..."));
    }
    _waitingSpinner->start();

#ifdef QT_DEBUG_MODE
    emit lpubAlert->messageSig(LOG_DEBUG,QString("2. Waiting Spinner Started"));
#endif

    QApplication::processEvents();
}

void EditWindow::waitingSpinnerStop()
{
  if (!Preferences::modeGUI)
      return;

  if (_waitingSpinner && _waitingSpinner->isSpinning()) {
    _waitingSpinner->stop();

#ifdef QT_DEBUG_MODE
    emit lpubAlert->messageSig(LOG_DEBUG,QString("4. Waiting Spinner Stopped"));
#endif
  }
}

void EditWindow::redraw()
{
  if (modelFileEdit() && Preferences::saveOnRedraw)
      saveFile();
  redrawSig();
}

void EditWindow::update(bool state)
{
  if (modelFileEdit() && Preferences::saveOnUpdate)
      saveFile();
  updateDisabled(state);
  updateSig();
}

void EditWindow::clearWindow() {
  _textEdit->document()->clear();
  _textEdit->document()->setModified(false);
  fileName.clear();
}

void EditWindow::enableSave()
{
  if (_textEdit->document()->isModified())
  {
    saveAct->setEnabled(true);
  }
}

void EditWindow::readSettings()
{
    QSettings Settings;
    Settings.beginGroup(EDITWINDOW);
    restoreGeometry(Settings.value("Geometry").toByteArray());
    restoreState(Settings.value("State").toByteArray());
    QSize size = Settings.value("Size", QDesktopWidget().availableGeometry(this).size()*0.5).toSize();
    resize(size);
    Settings.endGroup();
}

void EditWindow::writeSettings()
{
    QSettings Settings;
    Settings.beginGroup(EDITWINDOW);
    Settings.setValue("Geometry", saveGeometry());
    Settings.setValue("State", saveState());
    Settings.setValue("Size", size());
    Settings.endGroup();
}

void EditWindow::closeEvent(QCloseEvent *_event)
{
    if (!modelFileEdit())
        return;

    if (!QFileInfo(fileName).exists())
        return;

    fileWatcher.removePath(fileName);

    writeSettings();

    mpdCombo->setMaxCount(0);
    mpdCombo->setMaxCount(1000);

    if (maybeSave() || isReadOnly) {
        _event->accept();
    } else {
        _event->ignore();
    }
}

void EditWindow::preferences()
{
    const QString windowTitle       = QString("Editor Preferences");
    int editorDecoration            = Preferences::editorDecoration;
    int editorLinesPerPage          = Preferences::editorLinesPerPage;
    bool editorBufferedPaging       = Preferences::editorBufferedPaging;
    bool editorHighlightLines       = Preferences::editorHighlightLines;
    bool editorLoadSelectionStep    = Preferences::editorLoadSelectionStep;
    // modelFileEdit() only
    bool editorPreviewOnDoubleClick = Preferences::editorPreviewOnDoubleClick;

    auto showMessage = [&windowTitle] (const QString change) {
        QPixmap _icon = QPixmap(":/icons/lpub96.png");
        if (_icon.isNull())
            _icon = QPixmap (":/icons/update.png");

        QMessageBoxResizable box;
        box.setWindowIcon(QIcon());
        box.setIconPixmap (_icon);
        box.setTextFormat (Qt::RichText);
        box.setWindowTitle(windowTitle);
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        QString title = "<b>" + change + "</b>";
        QString text  = QString("%1 will take effect the next time LPub3D is started.").arg(change);
        box.setText (title);
        box.setInformativeText (text);
        box.setStandardButtons (QMessageBox::Ok);

        box.exec();
    };

    // options dialogue
    QDialog *dialog = new QDialog();
    dialog->setWindowTitle(windowTitle);
    QFormLayout *form = new QFormLayout(dialog);

    // options - editor decoration
    QGroupBox *editorDecorationGrpBox = new QGroupBox(tr("Editor Text Decoration"));
    form->addWidget(editorDecorationGrpBox);
    QFormLayout *editorDecorationSubform = new QFormLayout(editorDecorationGrpBox);

    QLabel    *editorDecorationLabel = new QLabel(tr("Text Decoration:"), dialog);
    QComboBox * editorDecorationCombo = new QComboBox(dialog);
    editorDecorationCombo->addItem(tr("Simple"));
    editorDecorationCombo->addItem(tr("Standard"));
    editorDecorationCombo->setCurrentIndex(editorDecoration);
    editorDecorationCombo->setToolTip(tr("Set text decoration. Fancy decoration will slow-down loading very large models"));
    editorDecorationSubform->addRow(editorDecorationLabel, editorDecorationCombo);

    // options - buffered paging
    QGroupBox *editorBufferedPagingGrpBox = new QGroupBox(tr("Buffered paging"));
    editorBufferedPagingGrpBox->setCheckable(true);
    editorBufferedPagingGrpBox->setChecked(editorBufferedPaging);
    editorBufferedPagingGrpBox->setToolTip(tr("Set buffered paging. Improve the loading times for very large models"));
    form->addWidget(editorBufferedPagingGrpBox);
    QFormLayout *editorBufferedPagingSubform = new QFormLayout(editorBufferedPagingGrpBox);

    QLabel   *editorLinesPerPageLabel = new QLabel(tr("Lines Per Buffered Page:"), dialog);
    QSpinBox *editorLinesPerPageSpin  = new QSpinBox(dialog);
    editorLinesPerPageSpin->setRange(EDITOR_MIN_LINES_DEFAULT,EDITOR_MAX_LINES_DEFAULT);
    editorLinesPerPageSpin->setValue(editorLinesPerPage);
    editorLinesPerPageSpin->setToolTip(tr("Set lines per page between %1 and %2 to optimize scrolling.").arg(EDITOR_MIN_LINES_DEFAULT).arg(EDITOR_MAX_LINES_DEFAULT));
    editorBufferedPagingSubform->addRow(editorLinesPerPageLabel, editorLinesPerPageSpin);

    // options - selected lines
    QCheckBox *editorHighlightLinesBox = nullptr;
    QCheckBox *editorLoadSelectionStepBox = nullptr;
    QCheckBox *editorPreviewOnDoubleClickBox = nullptr;

        QGroupBox *editorSelectedItemsGrpBox = new QGroupBox(tr("Selected Items"));
        form->addWidget(editorSelectedItemsGrpBox);
        QFormLayout *editorSelectedItemsSubform = new QFormLayout(editorSelectedItemsGrpBox);
    if (!modelFileEdit()) {
        editorHighlightLinesBox = new QCheckBox(tr("Highlight Selected Lines"), dialog);
        editorHighlightLinesBox->setToolTip(tr("Highlight selected line(s) when clicked in Editor"));
        editorHighlightLinesBox->setChecked(editorHighlightLines);
        editorSelectedItemsSubform->addRow(editorHighlightLinesBox);

        editorLoadSelectionStepBox = new QCheckBox(tr("Load Selection Step in 3DViewer"), dialog);
        editorLoadSelectionStepBox->setToolTip(tr("Load the first step (on multi-line select) of selected lines in the 3DViewer"));
        editorLoadSelectionStepBox->setChecked(editorLoadSelectionStep);
        editorSelectedItemsSubform->addRow(editorLoadSelectionStepBox);
    } // modelFileEdit()
    else  {
        editorPreviewOnDoubleClickBox = new QCheckBox(tr("Floating Preview On Double Click"), dialog);
        editorPreviewOnDoubleClickBox->setToolTip(tr("Launch floating preview window on valid part double click"));
        editorPreviewOnDoubleClickBox->setChecked(editorHighlightLines);
        editorSelectedItemsSubform->addRow(editorPreviewOnDoubleClickBox);
    } // ! modelFileEdit()


    // options - button box
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, dialog);
    form->addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
    dialog->setMinimumSize(220,100);

    if (dialog->exec() == QDialog::Accepted) {
        QSettings Settings;
        Preferences::editorDecoration     = editorDecorationCombo->currentIndex();
        if (editorDecoration != Preferences::editorDecoration) {
            showMessage("LDraw editor text decoration change");
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorDecoration"),Preferences::editorDecoration);
            emit lpubAlert->messageSig(LOG_INFO,QString("LDraw editor text decoration changed to %1").arg(Preferences::editorDecoration == SIMPLE ? "Simple" : "Standard"));
        }
        Preferences::editorBufferedPaging = editorBufferedPagingGrpBox->isChecked();
        if (editorBufferedPaging != Preferences::editorBufferedPaging) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorBufferedPaging"),Preferences::editorBufferedPaging);
            emit lpubAlert->messageSig(LOG_INFO,QString("Editor buffered paging is %1").arg(Preferences::editorBufferedPaging ? "On" : "Off"));
        }
        Preferences::editorLinesPerPage   = editorLinesPerPageSpin->text().toInt();
        if (editorLinesPerPage != Preferences::editorLinesPerPage) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorLinesPerPage"),Preferences::editorLinesPerPage);
            emit lpubAlert->messageSig(LOG_INFO,QString("Buffered lines par page changed from %1 to %2").arg(editorLinesPerPage).arg(Preferences::editorLinesPerPage));
        }

        if (! modelFileEdit()) {
            Preferences::editorHighlightLines   = editorHighlightLinesBox->isChecked();
            if (editorHighlightLines != Preferences::editorHighlightLines) {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorHighlightLines"),Preferences::editorHighlightLines);
                emit lpubAlert->messageSig(LOG_INFO,QString("Highlight selected lines changed from %1 to %2").arg(editorHighlightLines).arg(Preferences::editorLinesPerPage));
            }

            Preferences::editorLoadSelectionStep   = editorLoadSelectionStepBox->isChecked();
            if (editorLoadSelectionStep != Preferences::editorLoadSelectionStep) {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorLoadSelectionStep"),Preferences::editorLoadSelectionStep);
                emit lpubAlert->messageSig(LOG_INFO,QString("Load selection step in 3DViewer changed from %1 to %2").arg(editorLoadSelectionStep).arg(Preferences::editorLoadSelectionStep));
            }
        } // ! modelFileEdit()
        else {
            Preferences::editorPreviewOnDoubleClick = editorPreviewOnDoubleClickBox->isChecked();
            if (editorPreviewOnDoubleClick != Preferences::editorPreviewOnDoubleClick) {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorPreviewOnDoubleClick"),Preferences::editorPreviewOnDoubleClick);
                emit lpubAlert->messageSig(LOG_INFO,QString("Launch floating preview part double click changed from %1 to %2").arg(editorPreviewOnDoubleClick).arg(Preferences::editorPreviewOnDoubleClick));
            }
        } // modelFileEdit()
    }
}

/*
 *
 * Buffered Paging section
 *
 */

void  EditWindow::verticalScrollValueChanged(int value)
{
    if (_contentLoaded || _contentLoading)
        return;

    if (value > (verticalScrollBar->maximum() * 0.90 /*trigger load at 90% page scroll*/)) {
        emit lpubAlert->messageSig(LOG_INFO_STATUS,QString("Loading buffered page %1 lines...")
                                   .arg(Preferences::editorLinesPerPage));

        loadPagedContent();

        emit lpubAlert->messageSig(LOG_STATUS,QString());
        QApplication::processEvents();
    }
}

/* NOT USED FOR THE MOMENT */
void EditWindow::loadContentBlocks(const QStringList &content, bool firstBlock) {
    QElapsedTimer t; t.start();
    int lineCount     = content.size();
    int blockLineCount= 0;
    int blockIndx     = 0;
    int linesPerBlock = 100;
    int blocks        = lineCount / linesPerBlock;
    int remain        = lineCount % linesPerBlock;
    if (remain) {
        if (blocks)
            blocks++;
        else
            blocks = 1;
    }
    if (blocks) {
        for (int i = 0; i < blocks ; i++) {
            int nextBlockIndx = qMin((linesPerBlock - (firstBlock ? 1 : 0)), (lineCount - blockIndx) - 1);
            int maxBlockIndx  = blockIndx + nextBlockIndx;
            const QString block = content.mid(blockIndx, nextBlockIndx).join('\n');
            blockLineCount  = block.count("\n") + (firstBlock ? 2 : 1);
            if (firstBlock) {
                firstBlock = false;
                _textEdit->setPlainText(block);
            } else {
                _textEdit->append(block);
            }
#ifdef QT_DEBUG_MODE
        emit lpubAlert->messageSig(LOG_DEBUG,QString("Load content block %1, lines %2 - %3")
                                   .arg(i)
                                   .arg(blockLineCount)
                                   .arg(lpubAlert->elapsedTime(t.elapsed())));
#endif
            blockIndx = maxBlockIndx;
            QApplication::processEvents();
        }
    }
}

void EditWindow::loadPagedContent()
{
   _contentLoading = true;

   QElapsedTimer t; t.start();
#ifdef QT_DEBUG_MODE
   emit lpubAlert->messageSig(LOG_DEBUG,QString("Load paged content %1 lines - start...")
                              .arg(_pageContent.size()));
#endif

   bool initialLoad   = _textEdit->document()->isEmpty();
   int linesPerPage   = Preferences::editorLinesPerPage - (initialLoad ? 1 : 0);
   int nextIndx       = qMin(linesPerPage, (_pageContent.size() - _pageIndx) - 1);
   int maxPageIndx    = _pageIndx + nextIndx;
   const QString page = _pageContent.mid(_pageIndx, nextIndx).join('\n');
   int pageLineCount  = page.count("\n") + (initialLoad ? 2 : 1);

#ifdef QT_DEBUG_MODE
   emit lpubAlert->messageSig(LOG_DEBUG,QString("Load page line count %1 - %2")
                              .arg(pageLineCount)
                              .arg(lpubAlert->elapsedTime(t.elapsed())));
#endif

   disconnect(_textEdit->document(),SIGNAL(contentsChange(int,int,int)),
              this,                 SLOT(  contentsChange(int,int,int)));
   disconnect(_textEdit,            SIGNAL(cursorPositionChanged()),
              this,                 SLOT(  highlightCurrentLine()));
   disconnect(_textEdit,            SIGNAL(textChanged()),
              this,                 SLOT(  enableSave()));
   disconnect(_textEdit,            SIGNAL(textChanged()),
              _textEdit,            SLOT(  updateLineNumberArea()));
   disconnect(_textEdit,            SIGNAL(cursorPositionChanged()),
              _textEdit,            SLOT(  updateLineNumberArea()));

   verticalScrollBar->setMaximum(verticalScrollBar->maximum() + nextIndx);

//   loadContentBlocks(page.split('\n'), initialLoad);
// *
   if (initialLoad) {
       _textEdit->setPlainText(page);
#ifdef QT_DEBUG_MODE
   emit lpubAlert->messageSig(LOG_DEBUG,QString("Load page set %1 plain text lines - %2")
                              .arg(pageLineCount)
                              .arg(lpubAlert->elapsedTime(t.elapsed())));
#endif
   } else {
       _textEdit->append(page);
#ifdef QT_DEBUG_MODE
   emit lpubAlert->messageSig(LOG_DEBUG,QString("Load page append %1 text lines - %2")
                              .arg(pageLineCount)
                              .arg(lpubAlert->elapsedTime(t.elapsed())));
#endif
   }
// */

   _contentLoaded = maxPageIndx >= _pageContent.size() - 1;

   emit lpubAlert->messageSig(LOG_TRACE,QString("Load page of %1 lines from %2 to %3, content lines %4, final page: %5 - %6")
                              .arg(pageLineCount)
                              .arg(_pageIndx + 1)
                              .arg(maxPageIndx + 1)
                              .arg(_pageContent.size())
                              .arg(_contentLoaded ? "Yes" : "No")
                              .arg(lpubAlert->elapsedTime(t.elapsed())));

   _pageIndx = maxPageIndx;

   connect(_textEdit->document(),   SIGNAL(contentsChange(int,int,int)),
           this,                    SLOT(  contentsChange(int,int,int)));
   connect(_textEdit,               SIGNAL(cursorPositionChanged()),
           this,                    SLOT(  highlightCurrentLine()));
   connect(_textEdit,               SIGNAL(textChanged()),
           this,                    SLOT(  enableSave()));
   connect(_textEdit,               SIGNAL(textChanged()),
           _textEdit,               SLOT(  updateLineNumberArea()));
   connect(_textEdit,               SIGNAL(cursorPositionChanged()),
           _textEdit,               SLOT(  updateLineNumberArea()));

   _contentLoading = false;
}

/*
 *
 * Text Editor section
 *
 */

QTextEditor::QTextEditor(bool detachedEdit, QWidget *parent) :
    QTextEdit(parent),
    completer(nullptr),
    completion_minchars(1),
    completion_max(0),
    detachedEdit(detachedEdit),
    _fileIsUTF8(false)
{
    lineNumberArea = new QLineNumberArea(this);

    connect(this->document(), SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateLineNumberArea/*_2*/(int)));
    connect(this, SIGNAL(textChanged()), this, SLOT(updateLineNumberArea()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateLineNumberArea()));
    //connect(this, SIGNAL(selectionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    //highlightCurrentLine();
}

void QTextEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
    if (modelFileEdit()) {
        if ( event->button() == Qt::LeftButton && Preferences::editorPreviewOnDoubleClick) {
            emit triggerPreviewLine();
        }
    }
    return;
}

void QTextEditor::setCompleter(QCompleter *comp)
{
    if (completer)
        QObject::disconnect(completer, nullptr, this, nullptr);

    completer = comp;

    if (!completer)
        return;

    completer->setWidget(this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(completer, SIGNAL(activated(QString)),
                     this, SLOT(autocomplete(QString)));
    QObject::connect(completer, SIGNAL(highlighted(QString)),
                     this, SLOT(autocomplete(QString)));
}

void QTextEditor::setCompleterMinChars(int min_chars) {
    completion_minchars = min_chars;
}

void QTextEditor::setCompleterMaxSuggestions(int max) {
    completion_max = max;
}

void QTextEditor::setCompleterPrefix(const QString& prefix)
{
    completion_prefix = prefix;
}

int QTextEditor::wordStart() const
{
    // lastIndexOf returns the index of the last space, new line or -1 if there are no spaces
    // or new lines so that + 1 returns the index of the character starting the word or 0
    QTextCursor tc = textCursor();
    int start_pos = toPlainText().leftRef(tc.position()).lastIndexOf(' ') + 1;
    int after_new_line = 0;
    if ((after_new_line = toPlainText().leftRef(tc.position()).lastIndexOf('\n') + 1) > start_pos)
        start_pos = after_new_line;
    if (toPlainText().rightRef(toPlainText().size()-start_pos).startsWith(completion_prefix))
        start_pos += completion_prefix.size();
    return start_pos;
}

QString QTextEditor::currentWord() const
{
    QTextCursor tc = textCursor();
    int completion_index = wordStart();
    return toPlainText().mid(completion_index, tc.position() - completion_index);
}

void QTextEditor::autocomplete(const QString& completion)
{
    if (completer->widget() != this)
        return;

    QTextCursor tc = textCursor();
    int startIndex = wordStart();
    setText(toPlainText().replace(
            startIndex, tc.position() - startIndex,
            completion));
    tc.setPosition(startIndex + completion.size());
    setTextCursor(tc);
}

QString QTextEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void QTextEditor::focusInEvent(QFocusEvent *e)
{
    if (completer)
        completer->setWidget(this);
    QTextEdit::focusInEvent(e);
}

void QTextEditor::keyPressEvent(QKeyEvent *e)
{
    if (completer && completer->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (!completer || !isShortcut) // do not process the shortcut when we have a completer
        QTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!completer || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut &&
       (hasModifier ||
        e->text().isEmpty() ||
        completionPrefix.length() < completion_minchars ||
        eow.contains(e->text().right(1)) ||
       (completion_max > 0 && completer->completionCount() > completion_max))) {
        completer->popup()->hide();
        return;
    } else {
        if (completionPrefix != completer->completionPrefix()) {
            completer->setCompletionPrefix(completionPrefix);
        }
        QRect cr = cursorRect();
        cr.setWidth(completer->popup()->sizeHintForColumn(0)
                    + completer->popup()->verticalScrollBar()->sizeHint().width());
        completer->complete(cr); // popup it up!
    }
}

void QTextEditor::mouseReleaseEvent(QMouseEvent *event)
{
  QWidget::mouseReleaseEvent(event);
  if (event->button() == Qt::LeftButton) {
     emit updateSelectedParts();
  }
}

void QTextEditor::toggleComment(){

    int current = 0;
    int selectedLines = 0;

    QTextCursor cursor = textCursor();
    if(cursor.selection().isEmpty())
        cursor.select(QTextCursor::LineUnderCursor);

    QString str = cursor.selection().toPlainText();
    selectedLines = str.count("\n")+1;

    cursor.beginEditBlock();

    while (true)
    {
        if (current == selectedLines) {
           break;
        }

        bool result = false;
        QString replaceText;

        cursor.select(QTextCursor::LineUnderCursor);
        QString selection = cursor.selectedText();
        moveCursor(QTextCursor::StartOfLine);

        if (!selection.isEmpty())
        {
            QString findText;
            QTextDocument::FindFlags flags;
            // remove comments
            if (selection.startsWith("0 //1")) {
                findText = "0 //1";
                replaceText = "1";
            } else
            if (selection.startsWith("0 // ")) {
                findText = "0 //";
                replaceText = "0";
            } else
            if (selection.startsWith("0 //")) { // no space after comment
                findText = "0 //";
                replaceText = "0 ";             // add space after 0
            } else
            // add comments
            if (selection.startsWith("1")) {
                findText = "1";
                replaceText = "0 //1";
            } else
            if (selection.startsWith("0 ")) {
                findText = "0 ";
                replaceText = "0 //";
            }
            result = find(findText, flags);
        } else {
            break;
        }

        if (result) {
            textCursor().insertText(replaceText);
            if (++current < selectedLines) {
                cursor.movePosition(QTextCursor::Down);
                setTextCursor(cursor);
            }
        }
    }

    cursor.endEditBlock();
}

void QTextEditor::showAllCharacters(bool show){
    if (show){
        showCharacters(" ","\u002E");
#ifdef Q_OS_WIN
        showCharacters("\t","\u003E");
#else
        showCharacters("\t","\u2192");
#endif
    } else {
        showCharacters("\u002E"," ");
#ifdef Q_OS_WIN
        showCharacters("\u003E","\t");
#else
        showCharacters("\u2192","\t");
#endif
    }
}

int QTextEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 13 +  fontMetrics().width(QLatin1Char('9')) * (digits);

    return space;
}

void QTextEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void QTextEditor::updateLineNumberArea(QRectF /*rect_f*/)
{
    QTextEditor::updateLineNumberArea();
}

void QTextEditor::updateLineNumberArea(int /*slider_pos*/)
{
    QTextEditor::updateLineNumberArea();
}

void QTextEditor::updateLineNumberArea()
{
    /*
     * When the signal is emitted, the sliderPosition has been adjusted according to the action,
     * but the value has not yet been propagated (meaning the valueChanged() signal was not yet emitted),
     * and the visual display has not been updated. In slots connected to this signal you can thus safely
     * adjust any action by calling setSliderPosition() yourself, based on both the action and the
     * slider's value.
     */
    // Make sure the sliderPosition triggers one last time the valueChanged() signal with the actual value !!!!
    this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->sliderPosition());

    // Since "QTextEdit" does not have an "updateRequest(...)" signal, we chose
    // to grab the imformations from "sliderPosition()" and "contentsRect()".
    // See the necessary connections used (Class constructor implementation part).

    QRect rect =  this->contentsRect();
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    updateLineNumberAreaWidth(0);
    //----------
    int dy = this->verticalScrollBar()->sliderPosition();
    if (dy > -1) {
        lineNumberArea->scroll(0, dy);
    }

    // Addjust slider to alway see the number of the currently being edited line...
    int first_block_id = getFirstVisibleBlockId();
    if (first_block_id == 0 || this->textCursor().block().blockNumber() == first_block_id-1)
        this->verticalScrollBar()->setSliderPosition(dy-this->document()->documentMargin());

//    // Snap to first line (TODO...)
//    if (first_block_id > 0)
//    {
//        int slider_pos = this->verticalScrollBar()->sliderPosition();
//        int prev_block_height = (int) this->document()->documentLayout()->blockBoundingRect(this->document()->findBlockByNumber(first_block_id-1)).height();
//        if (dy <= this->document()->documentMargin() + prev_block_height)
//            this->verticalScrollBar()->setSliderPosition(slider_pos - (this->document()->documentMargin() + prev_block_height));
//    }

}

int QTextEditor::getFirstVisibleBlockId()
{
    // Detect the first block for which bounding rect - once translated
    // in absolute coordinated - is contained by the editor's text area

    // Costly way of doing but since "blockBoundingGeometry(...)" doesn't
    // exists for "QTextEdit"...

    QTextCursor curs = QTextCursor(this->document());
    curs.movePosition(QTextCursor::Start);
    for(int i=0; i < this->document()->blockCount(); ++i)
    {
        QTextBlock block = curs.block();

        QRect r1 = this->viewport()->geometry();
        QRect r2 = this->document()->documentLayout()->blockBoundingRect(block).translated(
                    this->viewport()->geometry().x(), this->viewport()->geometry().y() - (
                        this->verticalScrollBar()->sliderPosition()
                        ) ).toRect();

        if (r1.contains(r2, true)) { return i; }

        curs.movePosition(QTextCursor::NextBlock);
    }

    return 0;
}

void QTextEditor::resizeEvent(QResizeEvent *e)
{
    QTextEdit::resizeEvent(e);

    QRect cr = this->contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void QTextEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->sliderPosition());

    QPainter painter(lineNumberArea);

    // line number colors
    QColor col_1(Qt::magenta);     // Current line
    QColor col_0(Qt::darkGray);    // Other lines

    QColor numAreaColor;
    if (Preferences::displayTheme == THEME_DEFAULT) {
        numAreaColor = QColor(Qt::gray).lighter(150);
      }
    else
      if (Preferences::displayTheme == THEME_DARK) {
          numAreaColor = QColor(Preferences::themeColors[THEME_DARK_EDIT_MARGIN]);
          col_0 = QColor(Qt::darkGray).darker(150);
      }

    painter.fillRect(event->rect(),numAreaColor);
    int blockNumber = this->getFirstVisibleBlockId();

    QTextBlock block = this->document()->findBlockByNumber(blockNumber);
    QTextBlock prev_block = (blockNumber > 0) ? this->document()->findBlockByNumber(blockNumber-1) : block;
    int translate_y = (blockNumber > 0) ? -this->verticalScrollBar()->sliderPosition() : 0;

    int top = this->viewport()->geometry().top();

    // Adjust text position according to the previous "non entirely visible" block
    // if applicable. Also takes in consideration the document's margin offset.
    int additional_margin;
    if (blockNumber == 0)
        // Simply adjust to document's margin
        additional_margin = (int) this->document()->documentMargin() -1 - this->verticalScrollBar()->sliderPosition();
    else
        // Getting the height of the visible part of the previous "non entirely visible" block
        additional_margin = (int) this->document()->documentLayout()->blockBoundingRect(prev_block)
                .translated(0, translate_y).intersected(this->viewport()->geometry()).height();

    // Shift the starting point
    top += additional_margin;

    int bottom = top + (int) this->document()->documentLayout()->blockBoundingRect(block).height();

    // Draw the numbers (displaying the current line number in green)
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen((this->textCursor().blockNumber() == blockNumber) ? col_1 : col_0);
            painter.drawText(-5, top,
                             lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) this->document()->documentLayout()->blockBoundingRect(block).height();
        ++blockNumber;
    }

}

void QTextEditor::showCharacters(
    QString findString,
    QString replaceString)
{
    QTextDocument *doc = document();
    QTextCursor cursor = textCursor();

    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Start);

    QTextCursor newCursor = cursor;
    quint64 count = 0;

    QTextDocument::FindFlags options;

    if (!findString.isEmpty())
    {
        while (true)
        {
            newCursor = doc->find(findString, newCursor, options);

            if (!newCursor.isNull())
            {
                if (newCursor.hasSelection())
                {
                    newCursor.insertText(replaceString);
                    count++;
                }
            }
            else
            {
                break;
            }
        }
    }
    cursor.endEditBlock();
}

void QTextEditor::findDialog()
{
    QTextCursor cursor = textCursor();
    QString selection = cursor.selectedText();
    if (selection.isEmpty()) {
        cursor.select(QTextCursor::WordUnderCursor);
        selection = cursor.selectedText();
    }
    popUp = new QFindReplace(this,selection);
    popUp->show();
}

QFindReplace::QFindReplace(
    QTextEditor *textEdit,
    const QString &selectedText,
    QWidget *parent)
    : QDialog(parent)
{
    setWindowIcon(QIcon(":/resources/LPub32.png"));
    setWindowTitle("LDraw File Editor Find");

    find = new QFindReplaceCtrls(textEdit,this);
    find->textFind->setText(selectedText);

    completer = new QCompleter(this);
    completer->setModel(modelFromFile(":/resources/autocomplete.lst"));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    find->textFind->setWordCompleter(completer);

    findReplace = new QFindReplaceCtrls(textEdit,this);
    findReplace->textFind->setText(selectedText);

    completer = new QCompleter(this);
    completer->setModel(modelFromFile(":/resources/autocomplete.lst"));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    findReplace->textFind->setWordCompleter(completer);

    connect(find, SIGNAL(popUpClose()), this, SLOT(popUpClose()));
    connect(findReplace, SIGNAL(popUpClose()), this, SLOT(popUpClose()));
    connect(this, SIGNAL(accepted()), this, SLOT(popUpClose()));
    connect(this, SIGNAL(rejected()), this, SLOT(popUpClose()));

    readFindReplaceSettings(find);
    readFindReplaceSettings(findReplace);

    QTabWidget  *tabWidget = new QTabWidget;
    QVBoxLayout *vlayout = new QVBoxLayout;
    QHBoxLayout *hlayout = new QHBoxLayout;
    QGridLayout *gropuLayout;

    setLayout(vlayout);
    vlayout->addWidget(tabWidget);

    QWidget *widget;
    QGridLayout *grid;
    QGroupBox *box;

    widget = new QWidget;
    grid = new QGridLayout;
    widget->setLayout(grid);

    box = new QGroupBox("String to find");
    grid->addWidget(box);
    gropuLayout = new QGridLayout;
    box->setLayout(gropuLayout);

    gropuLayout->addWidget(find->textFind,0,0,1,5);

    gropuLayout->addWidget(find->buttonFind,1,0,1,1);
    gropuLayout->addWidget(find->buttonFindNext,1,1,1,1);
    gropuLayout->addWidget(find->buttonFindPrevious,1,2,1,1);
    gropuLayout->addWidget(find->buttonFindAll,1,3,1,1);
    gropuLayout->addWidget(find->buttonFindClear,1,4,1,1);

    box = new QGroupBox();
    box->setLayout(hlayout);
    gropuLayout->addWidget(box,2,0,1,5);
    hlayout->addWidget(find->checkboxCase);
    hlayout->addWidget(find->checkboxWord);
    hlayout->addWidget(find->checkboxRegExp);

    gropuLayout->addWidget(find->labelMessage,3,0,1,5);

    tabWidget->addTab(widget,"Find");

    widget = new QWidget;
    grid = new QGridLayout;
    widget->setLayout(grid);

    box = new QGroupBox("String to find and replace");
    grid->addWidget(box);
    gropuLayout = new QGridLayout;
    box->setLayout(gropuLayout);

    findReplace->label = new QLabel("Find: ");
    gropuLayout->addWidget(findReplace->label,0,0,1,1);
    gropuLayout->addWidget(findReplace->textFind,0,1,1,4);

    findReplace->label = new QLabel("Replace: ");
    gropuLayout->addWidget(findReplace->label,1,0,1,1);
    gropuLayout->addWidget(findReplace->textReplace,1,1,1,4);

    gropuLayout->addWidget(findReplace->buttonFind,2,0,1,1);
    gropuLayout->addWidget(findReplace->buttonFindNext,2,1,1,1);
    gropuLayout->addWidget(findReplace->buttonFindPrevious,2,2,1,1);
    gropuLayout->addWidget(findReplace->buttonFindAll,2,3,1,1);
    gropuLayout->addWidget(findReplace->buttonFindClear,2,4,1,1);

    gropuLayout->addWidget(findReplace->buttonReplace,3,0,1,1);
    gropuLayout->addWidget(findReplace->buttonReplaceAndFind,3,1,1,1);
    gropuLayout->addWidget(findReplace->buttonReplaceAll,3,2,1,1);
    gropuLayout->addWidget(findReplace->buttonReplaceClear,3,3,1,1);

    hlayout = new QHBoxLayout;
    box = new QGroupBox();
    box->setLayout(hlayout);
    gropuLayout->addWidget(box,4,0,1,5);
    hlayout->addWidget(findReplace->checkboxCase);
    hlayout->addWidget(findReplace->checkboxWord);
    hlayout->addWidget(findReplace->checkboxRegExp);

    gropuLayout->addWidget(findReplace->labelMessage,5,0,1,5);

    tabWidget->addTab(widget,"Replace");

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(findReplace->buttonCancel, QDialogButtonBox::ActionRole);
    vlayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QTimer::singleShot(0, find->textFind, SLOT(setFocus()));

    setMinimumSize(100,80);
}

QAbstractItemModel *QFindReplace::modelFromFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
            words << line.trimmed();
    }

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    return new QStringListModel(words, completer);
}

void  QFindReplace::popUpClose()
{
    QFindReplaceCtrls *fr = qobject_cast<QFindReplaceCtrls *>(sender());
    if (fr) {
        writeFindReplaceSettings(findReplace);
        writeFindReplaceSettings(find);
        if (fr->_findall){
            fr->_textEdit->document()->undo();
        }
    }
    close();
}

void QFindReplace::readFindReplaceSettings(QFindReplaceCtrls *fr) {
    QSettings settings;
    settings.beginGroup(FINDREPLACEWINDOW);
    fr->checkboxCase->setChecked(settings.value(CASE_CHECK, false).toBool());
    fr->checkboxWord->setChecked(settings.value(WORD_CHECK, false).toBool());
    fr->checkboxRegExp->setChecked(settings.value(REGEXP_CHECK, false).toBool());
    settings.endGroup();
}

void QFindReplace::writeFindReplaceSettings(QFindReplaceCtrls *fr) {
    QSettings settings;
    settings.beginGroup(FINDREPLACEWINDOW);
    settings.setValue(CASE_CHECK, fr->checkboxCase->isChecked());
    settings.setValue(WORD_CHECK, fr->checkboxWord->isChecked());
    settings.setValue(REGEXP_CHECK, fr->checkboxRegExp->isChecked());
    settings.endGroup();
}

QFindReplaceCtrls::QFindReplaceCtrls(QTextEditor *textEdit, QWidget *parent)
    : QWidget(parent),_textEdit(textEdit)
{
    // find items
    textFind    = new HistoryLineEdit/*QLineEdit*/;

    buttonFind  = new QPushButton("Find");
    buttonFindNext = new QPushButton("Find Next");
    buttonFindPrevious = new QPushButton("Find Previous");
    buttonFindAll = new QPushButton("Find All");
    buttonFindClear = new QPushButton("Clear");

    // options
    checkboxCase = new QCheckBox("Case Senstive");
    checkboxWord = new QCheckBox("Whole Words");
    checkboxRegExp = new QCheckBox("Regular Expression");

    // replace items
    textReplace = new HistoryLineEdit/*QLineEdit*/;

    buttonReplace = new QPushButton("Replace");
    buttonReplaceAndFind = new QPushButton("Replace && Find");
    buttonReplaceAll = new QPushButton("Replace All");
    buttonReplaceClear = new QPushButton("Clear");

    // message
    labelMessage = new QLabel;

    // cancel button
    buttonCancel = new QPushButton("Cancel");

    // events
    connect(textFind, SIGNAL(textChanged(QString)), this, SLOT(textFindChanged()));
    connect(textFind, SIGNAL(textChanged(QString)), this, SLOT(validateRegExp(QString)));
    connect(textReplace, SIGNAL(textChanged(QString)), this, SLOT(textReplaceChanged()));

    connect(checkboxRegExp, SIGNAL(toggled(bool)), this, SLOT(regexpSelected(bool)));

    connect(buttonFind, SIGNAL(clicked()), this,SLOT(findInText()));
    connect(buttonFindNext,SIGNAL(clicked()),this,SLOT(findInTextNext()));
    connect(buttonFindPrevious,SIGNAL(clicked()),this,SLOT(findInTextPrevious()));
    connect(buttonFindAll,SIGNAL(clicked()),this,SLOT(findInTextAll()));
    connect(buttonFindClear, SIGNAL(clicked()), this, SLOT(findClear()));

    connect(buttonReplace,SIGNAL(clicked()),this,SLOT(replaceInText()));
    connect(buttonReplaceAndFind,SIGNAL(clicked()),this,SLOT(replaceInTextFind()));
    connect(buttonReplaceAll,SIGNAL(clicked()),this,SLOT(replaceInTextAll()));
    connect(buttonReplaceClear, SIGNAL(clicked()), this, SLOT(replaceClear()));

    connect(buttonCancel, SIGNAL(clicked()), this, SIGNAL(popUpClose()));

    disableButtons();
}

void QFindReplaceCtrls::findInText(){
    find();
}

void QFindReplaceCtrls::findInTextNext()
{
    find(NEXT);
}

void QFindReplaceCtrls::findInTextPrevious()
{
    find(PREVIOUS);
}

void QFindReplaceCtrls::findInTextAll() {
    if (!_textEdit)
        return; // TODO: show some warning?

    const QString &toSearch = textFind->text();

    // undo previous change (if any)
    _textEdit->document()->undo();

    bool result = false;
    bool useRegExp = checkboxRegExp->isChecked();

    QTextDocument::FindFlags flags;

    if (checkboxCase->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (checkboxWord->isChecked())
        flags |= QTextDocument::FindWholeWords;

    if (toSearch.isEmpty()) {
        showError("The search field is empty. Enter a word and click Find.");
    } else {
        QTextCursor highlightCursor(_textEdit->document());
        QTextCursor textCursor(_textEdit->document());

        textCursor.beginEditBlock();

        QTextCharFormat plainFormat(highlightCursor.charFormat());
        QTextCharFormat colorFormat = plainFormat;
        colorFormat.setBackground(Qt::yellow);

        QRegExp reg;
        if (useRegExp) {
            reg = QRegExp(toSearch,
                         (checkboxCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive));
        }

        while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {

            highlightCursor = useRegExp ? _textEdit->document()->find(reg, highlightCursor, flags) :
                                          _textEdit->document()->find(toSearch, highlightCursor, flags);

            if (!highlightCursor.isNull()) {
                result = true;
                highlightCursor.movePosition(QTextCursor::WordRight,
                                             QTextCursor::KeepAnchor);
                highlightCursor.mergeCharFormat(colorFormat);
            }
        }

        textCursor.endEditBlock();

        if (result) {
            _findall = true;
            showError("");
        } else {
            showError(tr("no match found for '%1'").arg(textFind->text()));
            highlightCursor.setPosition(0); // move to the beginning of the document for the next find
            _textEdit->setTextCursor(highlightCursor);
        }
    }
}

void QFindReplaceCtrls::find(int direction) {
    if (!_textEdit)
        return; // TODO: show some warning?

    const QString &toSearch = textFind->text();

    // undo previous change (if any)
    if (_findall){
        _textEdit->document()->undo();
        _findall = false;
    }

    bool result = false;

    QTextDocument::FindFlags flags;

    if (direction == PREVIOUS)
        flags |= QTextDocument::FindBackward;
    if (checkboxCase->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (checkboxWord->isChecked())
        flags |= QTextDocument::FindWholeWords;

    if (toSearch.isEmpty()) {
        showError("The search field is empty. Enter a word and click Find.");
    } else {
        QTextCursor textCursor(_textEdit->document());

        textCursor.beginEditBlock();

        if (direction == NEXT && buttonFindPrevious->isEnabled() == false)
        {
             _textEdit->moveCursor(QTextCursor::Start);
        }

        if (checkboxRegExp->isChecked()) {
            QRegExp reg(toSearch,
                        (checkboxCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive));
            textCursor = _textEdit->document()->find(reg, textCursor, flags);
            _textEdit->setTextCursor(textCursor);
            result = (!textCursor.isNull());
        } else {
            result = _textEdit->find(toSearch, flags);
        }

        textCursor.endEditBlock();

        if (result) {
            if (direction == NEXT && buttonFindPrevious->isEnabled() == false) {
                buttonFindPrevious->setEnabled(true);
            }
            showError("");
        } else {
            if (direction == DEFAULT) {
                showError(tr("no match found for '%1'").arg(textFind->text()));
            } else {
                showError(tr("no more items found for '%1'").arg(textFind->text()));
            }
            textCursor.setPosition(0); // move to the beginning of the document for the next find
            _textEdit->setTextCursor(textCursor);
        }
    }
}

void QFindReplaceCtrls::replaceInText() {
    if (!_textEdit->textCursor().hasSelection()) {
        find();
    } else {
        _textEdit->textCursor().insertText(textReplace->text());
    }
}

void QFindReplaceCtrls::replaceInTextFind() {
    if (!_textEdit->textCursor().hasSelection()) {
        find();
    } else {
        _textEdit->textCursor().insertText(textReplace->text());
        find();
    }
}

void QFindReplaceCtrls::replaceInTextAll() {
    if (!_textEdit)
        return; // TODO: show some warning?

    const QString &toSearch = textFind->text();
    const QString &toReplace = textReplace->text();

    if (toReplace.isEmpty()) {
        showError("The replace field is empty. Enter a word and click Replace.");
    } else if (toSearch.isEmpty()) {
        showError("The search field is empty. Enter a word and click Replace.");
    } else {
        QTextCursor textCursor = _textEdit->textCursor();

        textCursor.beginEditBlock();

        textCursor.movePosition(QTextCursor::Start);
        QTextCursor newCursor = textCursor;
        quint64 count = 0;

        QTextDocument::FindFlags flags;

        if (checkboxCase->isChecked())
            flags |= QTextDocument::FindCaseSensitively;
        if (checkboxWord->isChecked())
            flags |= QTextDocument::FindWholeWords;

        while (true)
        {
            if (checkboxRegExp->isChecked()) {
                QRegExp reg(toSearch,
                            (checkboxCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive));
                newCursor = _textEdit->document()->find(reg, newCursor, flags);
            } else {
                newCursor = _textEdit->document()->find(toSearch, newCursor, flags);
            }

            if (!newCursor.isNull())
            {
                if (newCursor.hasSelection())
                {
                    newCursor.insertText(toReplace);
                    count++;
                }
            }
            else
            {
                break;
            }
        }

        textCursor.endEditBlock();
        showMessage( tr("%1 occurrence(s) were replaced.").arg(count));
    }
}

void QFindReplaceCtrls::regexpSelected(bool sel) {
    if (sel)
        validateRegExp(textFind->text());
    else
        validateRegExp("");
}

void QFindReplaceCtrls::validateRegExp(const QString &text) {
    if (!checkboxRegExp->isChecked() || text.size() == 0) {
        labelMessage->clear();
        return; // nothing to validate
    }

    QRegExp reg(text,
                (checkboxCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive));

    if (reg.isValid()) {
        showError("");
    } else {
        showError(reg.errorString());
    }
}

void QFindReplaceCtrls::showError(const QString &error) {
    if (error == "") {
        labelMessage->clear();
    } else {
        labelMessage->setText("<span style=\" font-weight:200; color:#ff0000;\">" + error + "</span>");
    }
}

void QFindReplaceCtrls::showMessage(const QString &message) {
    if (message == "") {
        labelMessage->clear();
    } else {
        labelMessage->setText("<span style=\" font-weight:200; color:green;\">" + message + "</span>");
    }
}

void QFindReplaceCtrls::textReplaceChanged() {
    bool enable = textReplace->text().size() > 0;
    buttonReplace->setEnabled(enable);
    buttonReplaceAndFind->setEnabled(enable);
    buttonReplaceAll->setEnabled(enable);
    buttonReplaceClear->setEnabled(enable);
}

void QFindReplaceCtrls::textFindChanged() {
    bool enable = textFind->text().size() > 0;
    buttonFind->setEnabled(enable);
    buttonFindNext->setEnabled(enable);
    buttonFindAll->setEnabled(enable);
    buttonFindClear->setEnabled(enable);
}

void  QFindReplaceCtrls::disableButtons()
{
    bool enable = false;

    buttonFind->setEnabled(enable);
    buttonFindNext->setEnabled(enable);
    buttonFindPrevious->setEnabled(enable);
    buttonFindAll->setEnabled(enable);
    buttonFindClear->setEnabled(enable);

    buttonReplace->setEnabled(enable);
    buttonReplaceAndFind->setEnabled(enable);
    buttonReplaceAll->setEnabled(enable);
    buttonReplaceClear->setEnabled(enable);
}

void QFindReplaceCtrls::findClear() {
    _textEdit->document()->undo();
    _textEdit->moveCursor(QTextCursor::Start);
    textFind->clear();
    labelMessage->clear();
}

void QFindReplaceCtrls::replaceClear() {
    _textEdit->document()->undo();
    _textEdit->moveCursor(QTextCursor::Start);
    textReplace->clear();
    labelMessage->clear();
}
