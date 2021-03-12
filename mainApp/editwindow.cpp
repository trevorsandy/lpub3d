/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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

#include "lc_global.h"
#include "editwindow.h"
#include "lpubalert.h"
#include "highlighter.h"
#include "highlightersimple.h"
#include "ldrawfiles.h"
#include "messageboxresizable.h"

#include "version.h"
#include "paths.h"
#include "lpub_preferences.h"

#include "lc_mainwindow.h"

#include "lc_qglwidget.h"
#include "previewwidget.h"

EditWindow *editWindow;

EditWindow::EditWindow(QMainWindow *parent, bool _modelFileEdit_) :
  QMainWindow(parent),isIncludeFile(false),_modelFileEdit(_modelFileEdit_),_pageIndx(0)
{
    editWindow  = this;

    _textEdit   = new QTextEditor(this);

    verticalScrollBar = _textEdit->verticalScrollBar();

    setTextEditHighlighter();

    _textEdit->setLineWrapMode(QTextEditor::NoWrap);
    _textEdit->setUndoRedoEnabled(true);
    _textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    _textEdit->popUp = nullptr;

    completer = new QCompleter(this);
    completer->setModel(modelFromFile(":/resources/autocomplete.lst"));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    _textEdit->setCompleter(completer);

    showLineType = LINE_HIGHLIGHT;

    createActions();
    createToolBars();
    highlightCurrentLine();

    if (Preferences::editorBufferedPaging) {
        _textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        connect(verticalScrollBar, SIGNAL(valueChanged(int)),
                     this, SLOT(verticalScrollValueChanged(int)));
    }

    connect(_textEdit, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
    connect(_textEdit, SIGNAL(cursorPositionChanged()),  this, SLOT(highlightCurrentLine()));
    connect(_textEdit, SIGNAL(updateSelectedParts()),   this, SLOT(updateSelectedParts()));
    connect(_textEdit, SIGNAL(triggerPreviewLine()),   this,  SLOT(triggerPreviewLine()));

    setCentralWidget(_textEdit);

    updateOpenWithActions();

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

void EditWindow::setTextEditHighlighter()
{
    if (Preferences::editorDecoration == SIMPLE)
      highlighterSimple = new HighlighterSimple(_textEdit->document());
    else
      highlighter = new Highlighter(_textEdit->document());
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

    if (Preferences.mPreviewPosition != lcPreviewPosition::Floating) {
        emit previewPieceSig(partType, colorCode);
        return;
    }

    PreviewWidget *Preview = new PreviewWidget();

    lcQGLWidget *ViewWidget = new lcQGLWidget(nullptr, Preview, true/*isView*/, true/*isPreview*/);

    if (Preview && ViewWidget) {
        ViewWidget->setAttribute(Qt::WA_DeleteOnClose, true);
        if (!Preview->SetCurrentPiece(partType, colorCode))
            emit lpubAlert->messageSig(LOG_ERROR, QString("Part preview for %1 failed.").arg(partType));
        ViewWidget->SetPreviewPosition(rect());
    } else {
        emit lpubAlert->messageSig(LOG_ERROR, QString("Preview %1 failed.").arg(partType));
    }
}

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

    openFolderActAct = new QAction(QIcon(":/resources/openworkingfolder.png"),tr("Open Working Folder"), this);
    openFolderActAct->setShortcut(tr("Alt+Shift+2"));
    openFolderActAct->setStatusTip(tr("Open file working folder - Alt+Shift+2"));
    connect(openFolderActAct, SIGNAL(triggered()), this, SLOT(openFolder()));

    copyFullPathToClipboardAct = new QAction(QIcon(":/resources/copytoclipboard.png"),tr("Full Path to Clipboard"), this);
    copyFullPathToClipboardAct->setShortcut(tr("Alt+Shift+3"));
    copyFullPathToClipboardAct->setStatusTip(tr("Copy full file path to clipboard - Alt+Shift+3"));
    connect(copyFullPathToClipboardAct, SIGNAL(triggered()), this, SLOT(updateClipboard()));

    copyFileNameToClipboardAct = new QAction(QIcon(":/resources/copytoclipboard.png"),tr("File Name to Clipboard"), this);
    copyFileNameToClipboardAct->setShortcut(tr("Alt+Shift+0"));
    copyFileNameToClipboardAct->setStatusTip(tr("Copy file name to clipboard - Alt+Shift+0"));
    connect(copyFileNameToClipboardAct, SIGNAL(triggered()), this, SLOT(updateClipboard()));

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
    previewLineAct->setEnabled(false);

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

    openFolderActAct->setEnabled(false);
    copyFullPathToClipboardAct->setEnabled(false);
    copyFileNameToClipboardAct->setEnabled(false);

    undoAct->setEnabled(false);
    redoAct->setEnabled(false);
    saveAct->setEnabled(false);
}

void EditWindow::enableActions()
{
    editModelFileAct->setEnabled(true);

    redrawAct->setEnabled(true);
    selAllAct->setEnabled(true);
    findAct->setEnabled(true);
    topAct->setEnabled(true);
    toggleCmmentAct->setEnabled(true);
    bottomAct->setEnabled(true);
    showAllCharsAct->setEnabled(true);
    openFolderActAct->setEnabled(true);
}

void EditWindow::clearEditorWindow()
{
    fileName.clear();
    disableActions();
}

void EditWindow::createToolBars()
{
    editToolBar = addToolBar(tr("LDraw Editor Toolbar"));
    editToolBar->setObjectName("EditToolbar");
    if (modelFileEdit()) {
        mpdCombo = new QComboBox(this);
        mpdCombo->setMinimumContentsLength(25);
        mpdCombo->setInsertPolicy(QComboBox::InsertAtBottom);
        mpdCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
        mpdCombo->setToolTip(tr("Go to submodel"));
        mpdCombo->setStatusTip("Use dropdown to go to submodel");
        connect(mpdCombo,SIGNAL(activated(int)),
                this,    SLOT(mpdComboChanged(int)));

        editToolBar->addAction(exitAct);
        editToolBar->addAction(saveAct);
        editToolBar->addSeparator();
        editToolBar->addAction(undoAct);
        editToolBar->addAction(redoAct);
        editToolBar->addSeparator();
        editToolBar->addWidget(mpdCombo);
        editToolBar->addSeparator();
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
    editToolBar->addSeparator();
    editToolBar->addAction(preferencesAct);
}

bool EditWindow::validPartLine ()
{
    QString partType;
    int validCode, colorCode = LDRAW_MATERIAL_COLOUR;
    QTextCursor cursor = _textEdit->textCursor();
    cursor.select(QTextCursor::LineUnderCursor);
    QString selection = cursor.selection().toPlainText();
    QStringList list;
    bool colorOk = false;

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

    } else {
        return false;
    }

    if (partType.isEmpty())
        return false;

    if (colorOk)
        colorCode = validCode;

    partType = partType.trimmed();

    emit lpubAlert->messageSig(LOG_DEBUG,
                               QString("Editor PartType: %1, ColorCode: %2, Line: %3")
                               .arg(partType).arg(colorCode).arg(selection));

    QString partKey = QString("%1|%2").arg(colorCode).arg(partType);

    if (_subFileList.contains(partType.toLower())) {
        previewLineAct->setText(tr("Preview highlighted subfile..."));
        previewLineAct->setStatusTip(tr("Display the subfile on the highlighted line in a popup 3D viewer"));

        copyFullPathToClipboardAct->setEnabled(true);
        copyFullPathToClipboardAct->setData(partType);
    }

    copyFileNameToClipboardAct->setEnabled(true);
    copyFileNameToClipboardAct->setData(partType);

    previewLineAct->setData(partKey);
    previewLineAct->setEnabled(true);

    return true;
}

void EditWindow::showContextMenu(const QPoint &pt)
{
    QMenu *menu = _textEdit->createStandardContextMenu();

    bool validLine = false;
    previewLineAct->setEnabled(validLine);
    copyFullPathToClipboardAct->setEnabled(validLine);
    copyFileNameToClipboardAct->setEnabled(validLine);

    if (!fileName.isEmpty()) {
        if (_subFileListPending) {
            emit getSubFileListSig();
            while (_subFileListPending)
                QApplication::processEvents();
        }
        validLine = validPartLine();
        menu->addSeparator();
        menu->addAction(openFolderActAct);
#ifndef QT_NO_CLIPBOARD
        menu->addAction(copyFileNameToClipboardAct);
        menu->addAction(copyFullPathToClipboardAct);
#endif
        menu->addSeparator();
        menu->addAction(previewLineAct);

        lcPreferences& Preferences = lcGetPreferences();
        if (Preferences.mPreviewEnabled && !isIncludeFile)
            previewLineAct->setEnabled(validLine);

        if (!modelFileEdit()) {
            editModelFileAct->setText(tr("Edit %1").arg(QFileInfo(fileName).fileName()));
            editModelFileAct->setStatusTip(tr("Edit %1 in detached LDraw Editor").arg(QFileInfo(fileName).fileName()));
            menu->addAction(editModelFileAct);
        }

        QMenu *openWithMenu;
        openWithMenu = menu->addMenu(tr("Open With..."));
        openWithMenu->setIcon(QIcon(":/resources/openwith.png"));
        openWithMenu->setStatusTip(tr("Open model file with selected application"));
        openWithMenu->setEnabled(false);
        if (numOpenWithPrograms) {
            openWithMenu->setEnabled(true);
            for (int i = 0; i < numOpenWithPrograms; i++)
                openWithMenu->addAction(openWithActList.at(i));
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

void EditWindow::triggerPreviewLine()
{
    if (!isIncludeFile) {
        if (_subFileListPending) {
            emit getSubFileListSig();
            while (_subFileListPending)
                QApplication::processEvents();
        }
        if (validPartLine())
            emit previewLineAct->triggered();
    }
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
        const QString path = QDir::toNativeSeparators(QDir::currentPath() + QDir::separator() + Paths::tmpDir);
        openFolderSelect(path + QDir::separator() + fileName);
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
        if (_modelFileEdit && !fileName.isEmpty())
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

        if (_modelFileEdit && !fileName.isEmpty())
            fileWatcher.addPath(fileName);
    }

  return rc;
}

void EditWindow::highlightSelectedLines(QVector<int> &lines, bool clear)
{
    highlightSelectedLines(lines, clear, false/*editorSelection*/);
}

void EditWindow::highlightSelectedLines(QVector<int> &lines, bool clear, bool editorSelection)
{
    // Remove duplicate editor (saved) lines and viewer lines
    if (!editorSelection && savedSelection.size()) {
        for (int line : lines) {
            if (savedSelection.contains(line))
                savedSelection.removeAll(line);
        }
    }

    auto highlightLines = [this, &editorSelection] (QVector<int> &linesToFormat, bool clear)
    {
        QTextCursor highlightCursor(_textEdit->document());

        QTextCursor textCursor(_textEdit->document());

        textCursor.beginEditBlock();

        if (!_textEdit->isReadOnly()) {

            QColor lineColor = QColor(Qt::transparent);

            bool applyFormat = linesToFormat.size() || (editorSelection && savedSelection.size());
            if (applyFormat) {
                if (clear) {
                    if (Preferences::displayTheme == THEME_DEFAULT) {
                        lineColor = QColor(Qt::white);
                    } else if (Preferences::displayTheme == THEME_DARK) {
                        lineColor = QColor(THEME_SCENE_BGCOLOR_DARK);
                    }
                } else {
                    if (Preferences::displayTheme == THEME_DEFAULT) {
                        lineColor = QColor(Qt::yellow).lighter(180);
                    } else if (Preferences::displayTheme == THEME_DARK) {
                        lineColor = QColor(Qt::yellow).lighter(180);
                        lineColor.setAlpha(100); // make 60% transparent
                    }
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

        textCursor.endEditBlock();
    };

    disconnect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
               this,                  SLOT(  contentsChange(int,int,int)));

    // apply previously selected lines if any
    if (savedSelection.size())
        highlightLines(savedSelection, false/*clear*/);

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
        if (Preferences::displayTheme == THEME_DEFAULT) {
            if (showLineType == LINE_ERROR)
                lineColor = QColor(THEME_LINE_ERROR_DEFAULT);
             else
                lineColor = QColor(THEME_LINE_HIGHLIGHT_DEFAULT);
          }
        else
        if (Preferences::displayTheme == THEME_DARK) {
            if (showLineType == LINE_ERROR)
                lineColor = QColor(THEME_LINE_ERROR_DARK).lighter(180);
            else
                lineColor = QColor(THEME_LINE_HIGHLIGHT_DARK);
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
}

void EditWindow::updateSelectedParts() {
    if (modelFileEdit())
        return;

    if (!gMainWindow->isVisible())
        return;

    if (isIncludeFile)
        return;

    int currentLine = 0;
    int selectedLines = 0;
    bool clearSelection = false;

    QVector<TypeLine> lineTypeIndexes;
    QVector<int> toggleLines;

    QTextCursor cursor = _textEdit->textCursor();

    QTextCursor saveCursor = cursor;

    if(!cursor.hasSelection())
        cursor.select(QTextCursor::LineUnderCursor);

    QStringList content = cursor.selection().toPlainText().split("\n");

    selectedLines = content.size();

    if (!selectedLines)
        return;

    auto getSelectedLineNumber = [&cursor] () {
        int lineNumber = 0;

        QTextCursor _cursor = cursor;
        _cursor.select(QTextCursor::LineUnderCursor);

        while(_cursor.positionInBlock()>0) {
            _cursor.movePosition(QTextCursor::Up);
            lineNumber++;
        }

        QTextBlock block = _cursor.block().previous();

        while(block.isValid()) {
            lineNumber += block.lineCount();
            block = block.previous();
        }
        return lineNumber;
    };

    cursor.beginEditBlock();

    while (currentLine < selectedLines)
    {
        QString selection = content.at(currentLine);
        if (content.at(currentLine).startsWith("1") ||
            content.at(currentLine).contains(" PLI BEGIN SUB "))
        {
            int lineNumber = getSelectedLineNumber();
            TypeLine typeLine(fileOrderIndex,lineNumber);
            lineTypeIndexes.append(typeLine);
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
        // set next selected line
        cursor.movePosition(QTextCursor::Down);
        _textEdit->setTextCursor(cursor);
       currentLine++;
    }

    // restore selection
   _textEdit->setTextCursor(saveCursor);

    cursor.endEditBlock();

    emit SelectedPartLinesSig(lineTypeIndexes);
}

void EditWindow::showLine(int lineNumber, int lineType)
{
  showLineType = lineType;
  showLineNumber = lineNumber;
  if (Preferences::editorBufferedPaging &&
      showLineNumber > Preferences::editorLinesPerPage &&
      showLineNumber > _pageIndx) {
      int linesNeeded = (showLineNumber - _pageIndx );
      int jump = linesNeeded / Preferences::editorLinesPerPage;
      if (!(linesNeeded % Preferences::editorLinesPerPage))
          jump++;
      for (int i = 0; i < jump; i++)
          loadPagedContent();
      emit lpubAlert->messageSig(LOG_DEBUG,QString("ShowLine jump %1 %2 to line %3 from line %4.")
                                 .arg(jump).arg(jump == 1 ? "page" : "pages").arg(lineNumber).arg(_pageIndx + 1));
  }

  _textEdit->moveCursor(QTextCursor::Start,QTextCursor::MoveAnchor);
  for (int i = 0; i < showLineNumber; i++)
    _textEdit->moveCursor(QTextCursor::Down,QTextCursor::MoveAnchor);
//  _textEdit->moveCursor(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
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
  bool reloaded  = _fileName == fileName;
  fileName       = _fileName;
  fileOrderIndex = ldrawFile->getSubmodelIndex(_fileName);
  isIncludeFile  = ldrawFile->isIncludeFile(_fileName);
  _contentLoaded = false;
  _subFileListPending = true;
  stepLines      = lineScope;
  savedSelection.clear();

  if (modelFileEdit() && !fileName.isEmpty())
      fileWatcher.removePath(fileName);

  disconnect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
             this,                  SLOT(  contentsChange(int,int,int)));

  if (fileName == "") {
    _textEdit->document()->clear();
  } else {
      if (modelFileEdit()) {
          QFile file(fileName);
          if (!file.open(QFile::ReadOnly | QFile::Text)) {
              QMessageBox::warning(nullptr,
                       QMessageBox::tr("Detached LDraw Editor"),
                       QMessageBox::tr("Cannot read editor display file %1:\n%2.")
                       .arg(fileName)
                       .arg(file.errorString()));

              _textEdit->document()->clear();
              return;
          }

          bool setProgressDialog = file.size() > 256000;  /*in Bytes = 250KB*/

          QProgressDialog *progressDialog = nullptr;

          if (setProgressDialog) {
              QString message = QString("Loading '%1', please wait...").arg(QFileInfo(fileName).fileName());
              progressDialog = new QProgressDialog(message, "Cancel",0,0,this);
              progressDialog->setWindowModality(Qt::WindowModal);
              // hide cancel button
              QList<QPushButton *> buttonList=progressDialog->findChildren<QPushButton *>();
              if (buttonList.size())
                  buttonList.at(0)->hide();
              progressDialog->exec();
          }

          mpdCombo->setMaxCount(0);
          mpdCombo->setMaxCount(1000);
          if (isIncludeFile) {
              mpdCombo->addItem(QFileInfo(fileName).fileName());
          } else {
              mpdCombo->addItems(ldrawFile->subFileOrder());
          }
          if(_saveSubfileIndex) {
              mpdCombo->setCurrentIndex(_saveSubfileIndex);
              _saveSubfileIndex = 0;
          }
          mpdCombo->setEnabled(!isIncludeFile);

          // check file encoding
          QTextCodec *codec = QTextCodec::codecForName("UTF-8");
          bool isUTF8 = LDrawFile::_currFileIsUTF8;
          _textEdit->setIsUTF8(isUTF8);

          QFileInfo   fileInfo(fileName);
          QTextStream in(&file);
          in.setCodec(_textEdit->getIsUTF8() ? codec : QTextCodec::codecForName("System"));

          disconnect(_textEdit, SIGNAL(textChanged()),
                     this,      SLOT(enableSave()));

          int lineCount = 0;
          if (Preferences::editorBufferedPaging && lineCount >= Preferences::editorLinesPerPage) {
              _pageIndx = 0;
              while(!in.atEnd()) {
                  _pageContent.append(in.readLine());
                  lineCount++;
              }
              loadPagedContent();
          }
          else
          {
              _textEdit->setPlainText(in.readAll());
          }

          if (setProgressDialog)
              progressDialog->reset();

          connect(_textEdit,  SIGNAL(textChanged()),
                   this,      SLOT(enableSave()));
          file.close();

          exitAct->setEnabled(true);
          statusBar()->showMessage(tr("%1 file %2 %3")
                                   .arg(isIncludeFile ? "Include" : "Model").arg(fileName).arg(reloaded ? "updated" : "loaded"), 2000);
      } else {
          if (Preferences::editorBufferedPaging && ldrawFile->size(fileName) > Preferences::editorLinesPerPage) {
              _pageIndx    = 0;
              _pageContent = ldrawFile->contents(fileName);
              loadPagedContent();
          } else {
             _textEdit->setPlainText(ldrawFile->contents(fileName).join("\n"));
          }
      }
  }

  _textEdit->document()->setModified(false);

  connect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
          this,                  SLOT(  contentsChange(int,int,int)));

  enableActions();

  if (modelFileEdit() && !fileName.isEmpty())
      fileWatcher.addPath(fileName);
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
    Settings.beginGroup(PARMSWINDOW);
    restoreGeometry(Settings.value("Geometry").toByteArray());
    restoreState(Settings.value("State").toByteArray());
    QSize size = Settings.value("Size", QDesktopWidget().availableGeometry(this).size()*0.5).toSize();
    resize(size);
    Settings.endGroup();
}

void EditWindow::writeSettings()
{
    QSettings Settings;
    Settings.beginGroup(PARMSWINDOW);
    Settings.setValue("Geometry", saveGeometry());
    Settings.setValue("State", saveState());
    Settings.setValue("Size", size());
    Settings.endGroup();
}

void EditWindow::closeEvent(QCloseEvent *_event)
{
    if (!modelFileEdit())
        return;

    if (!fileName.isEmpty())
        fileWatcher.removePath(fileName);

    writeSettings();

    mpdCombo->setMaxCount(0);
    mpdCombo->setMaxCount(1000);

    if (maybeSave()){
        _event->accept();
    } else {
        _event->ignore();
    }
}

void EditWindow::preferences()
{
    QString windowTitle       = QString("Editor Preferences");
    int editorDecoration      = Preferences::editorDecoration;
    int editorLinesPerPage    = Preferences::editorLinesPerPage;
    bool editorBufferedPaging = Preferences::editorBufferedPaging;

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
    QGroupBox *editorDecorationGrpBox = new QGroupBox(tr("Editor text decoration"));
    form->addWidget(editorDecorationGrpBox);
    QFormLayout *editorDecorationSubform = new QFormLayout(editorDecorationGrpBox);

    QLabel    *editorDecorationLabel = new QLabel("Text Decoration:", dialog);
    QComboBox * editorDecorationCombo = new QComboBox(dialog);
    editorDecorationCombo->addItem(tr("Simple"));
    editorDecorationCombo->addItem(tr("Standard"));
    editorDecorationCombo->setCurrentIndex(editorDecoration);
    editorDecorationCombo->setToolTip("Set text decoration. Fancy decoration will slow-down loading very large models");
    editorDecorationCombo->setStatusTip("Use dropdown to select LDraw editor text decoration");
    editorDecorationSubform->addRow(editorDecorationLabel, editorDecorationCombo);

    // options - buffered paging
    QGroupBox *editorBufferedPagingGrpBox = new QGroupBox(tr("Buffered paging"));
    editorBufferedPagingGrpBox->setCheckable(true);
    editorBufferedPagingGrpBox->setChecked(editorBufferedPaging);
    editorBufferedPagingGrpBox->setToolTip("Set buffered paging. Improve the loading times for very large models");
    form->addWidget(editorBufferedPagingGrpBox);
    QFormLayout *editorBufferedPagingSubform = new QFormLayout(editorBufferedPagingGrpBox);

    QLabel   *editorLinesPerPageLabel = new QLabel("Lines Per Buffered Page:", dialog);
    QSpinBox *editorLinesPerPageSpin  = new QSpinBox(dialog);
    editorLinesPerPageSpin->setRange(EDITOR_MIN_LINES_DEFAULT,EDITOR_MAX_LINES_DEFAULT);
    editorLinesPerPageSpin->setValue(editorLinesPerPage);
    editorLinesPerPageSpin->setToolTip("Set lines per page to optimize scrolling.");
    editorLinesPerPageSpin->setStatusTip(QString("Set the desired buffered lines per page between %1 and %2.").arg(EDITOR_MIN_LINES_DEFAULT).arg(EDITOR_MAX_LINES_DEFAULT));
    editorBufferedPagingSubform->addRow(editorLinesPerPageLabel, editorLinesPerPageSpin);

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
    }
}

/*
 *
 * Buffered Paging section
 *
 */

void  EditWindow::verticalScrollValueChanged(int value)
{
    if (_contentLoaded)
        return;

    int scrollMaximum = verticalScrollBar->maximum();

    // we load a new page at 90 percent of the page scroll
    if (value > (scrollMaximum * 0.90))
        loadPagedContent();
}

void EditWindow::loadPagedContent()
{
   int maxPageLines = _pageIndx + Preferences::editorLinesPerPage;
   int contentSize  = qMin(maxPageLines, _pageContent.size());

   QString part;
   for (; _pageIndx < contentSize; _pageIndx++)
       part.append(_pageContent.at(_pageIndx)+'\n');

   if (modelFileEdit() && !fileName.isEmpty())
       fileWatcher.removePath(fileName);

   disconnect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
              this,                  SLOT(  contentsChange(int,int,int)));
   disconnect(_textEdit, SIGNAL(textChanged()),
              this,      SLOT(enableSave()));

   _textEdit->insertPlainText(part);

   connect(_textEdit,  SIGNAL(textChanged()),
           this,       SLOT(enableSave()));
   connect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
           this,                  SLOT(  contentsChange(int,int,int)));

   if (modelFileEdit() && !fileName.isEmpty())
       fileWatcher.addPath(fileName);

   _contentLoaded = maxPageLines > _pageContent.size();

   emit lpubAlert->messageSig(LOG_TRACE,QString("Paged content at index: [%1], total lines: [%2], max lines: [%3], load completed: [%4]")
                                                .arg(_pageIndx).arg(_pageContent.size()).arg(maxPageLines).arg(_contentLoaded ? "Yes" : "No"));
}

/*
 *
 * Text Editor section
 *
 */

QTextEditor::QTextEditor(QWidget *parent) :
    QTextEdit(parent),
    completer(nullptr),
    completion_minchars(1),
    completion_max(0),
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
    if ( event->button() == Qt::LeftButton ) {
        emit triggerPreviewLine();
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
  if (event->button() == Qt::LeftButton) {
     emit updateSelectedParts();
  }
  QWidget::mouseReleaseEvent(event);
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
          numAreaColor = QColor(THEME_EDIT_MARGIN_DARK);
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
    QGridLayout *gropuLayout = new QGridLayout;

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
