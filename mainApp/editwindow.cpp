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
#include <QTextCursor>
#include <QtConcurrent>

#include <QCompleter>
#include <QApplication>
#include <QTextBlock>
#include <QTextStream>
#include <QAbstractItemView>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QKeyEvent>
#include <QScrollBar>

#include "editwindow.h"
#include "lpub_object.h"
#include "highlighter.h"
#include "highlightersimple.h"

#include "ldrawfiles.h"
#include "findreplace.h"
#include "substitutepartdialog.h"
#include "ldrawpartdialog.h"
#include "ldrawcolordialog.h"

#include "lc_global.h"
#include "lc_math.h"
#include "version.h"
#include "paths.h"
#include "lpub_preferences.h"
#include "pli.h"
#include "step.h"
#include "color.h"
#include "messageboxresizable.h"
#include "waitingspinnerwidget.h"
#include "threadworkers.h"
#include "lpub_qtcompat.h"
#include "commonmenus.h"

#include "lc_mainwindow.h"
#include "lc_viewwidget.h"
#include "lc_previewwidget.h"
#include "lc_colors.h"
#include "pieceinf.h"

#include <jsonfile.h>
#include <commands/commandsdialog.h>
#include <commands/snippets/jsonsnippettranslatorfactory.h>
#include <commands/snippets/snippetcollection.h>
#include <commands/snippets/snippetcompleter.h>

class ScrollBarFix : public QScrollBar
{
public:
    ScrollBarFix(Qt::Orientation orient, QWidget *parent=0)
        : QScrollBar(orient, parent) {}

protected:
    void sliderChange(SliderChange change) {
        if (signalsBlocked() && change == QAbstractSlider::SliderValueChange)
            blockSignals(false);
        QScrollBar::sliderChange(change);
    }
};

#define SUB_PLACEHOLDER "@@~|~@@"

enum CurrentStepType { INVALID_CURRENT_STEP, EXISTING_CURRENT_STEP, NEW_CURRENT_STEP };
enum DecorationType { SIMPLE_DECORATION, STANDARD_DECORATION };

EditWindow *cmdEditor;
EditWindow *cmdModEditor;

EditWindow::EditWindow(QMainWindow *parent, bool _modelFileEdit_) :
  QMainWindow(parent),isIncludeFile(false),_modelFileEdit(_modelFileEdit_),_pageIndx(0)
{
    qRegisterMetaType<CurrentStepType>("CurrentStepType");
    qRegisterMetaType<DecorationType>("DecorationType");

    _textEdit = new TextEditor(_modelFileEdit, this);

    loadModelWorker = new LoadModelWorker(_modelFileEdit);

    verticalScrollBar = _textEdit->verticalScrollBar();

    lpub->snippetCollection = new SnippetCollection(this);

    _textEdit->setAutoCompleter(new QCompleter(metaCommandModel(this), this));
    _textEdit->setSnippetCompleter(new SnippetCompleter(lpub->snippetCollection, _textEdit));

    _textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    _textEdit->setContextMenuPolicy(Qt::CustomContextMenu);

    showLineType    = LINE_HIGHLIGHT;
    isReadOnly      = false;
    visualEditorVisible = false;
    _isUndo = _isRedo = false;

    setTextEditHighlighter();

    createActions();
    updateOpenWithActions();
    createToolBars();

    if (Preferences::editorBufferedPaging) {
        _textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        connect(verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(verticalScrollValueChanged(int)));
    }

    connect(&futureWatcher, &QFutureWatcher<int>::finished, this, &EditWindow::contentLoaded);
    connect(_textEdit, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
    connect(_textEdit, SIGNAL(cursorPositionChanged()),  this, SLOT(highlightCurrentLine()));
    connect(_textEdit, SIGNAL(highlightSelectedParts()),  this, SLOT(highlightSelectedParts()));
    connect(_textEdit, SIGNAL(triggerPreviewLine()),  this,  SLOT(triggerPreviewLine()));

    setCentralWidget(_textEdit);

    if (_modelFileEdit) {
        _saveSubfileIndex = 0;
        QObject::connect(&fileWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(modelFileChanged(const QString&)));
        this->statusBar()->show();
        readSettings();
        cmdModEditor = this;
    } else {
        cmdEditor = this;
    }
}

EditWindow::~EditWindow()
{
    if (modelFileEdit())
        cmdModEditor = nullptr;
    else
        cmdEditor = nullptr;
}

void EditWindow::commandsDialog()
{
    CommandsDialog::showCommandsDialog();
}

void EditWindow::gotoLine()
{
    const int STEP = 1;
    const int MIN_VALUE = 1;

    QTextCursor cursor = _textEdit->textCursor();
    int currentLine = cursor.blockNumber()+1;
    int maxValue = _textEdit->document()->blockCount();

    bool ok;
    int line = QInputDialog::getInt(this, tr("Go to..."),
                                          tr("Line: ", "Line number in the command editor"), currentLine, MIN_VALUE, maxValue, STEP, &ok);
    if (!ok) return;
    _textEdit->gotoLine(line);
}

QAbstractItemModel *EditWindow::metaCommandModel(QObject *parent)
{
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif

    QStandardItemModel *model = new QStandardItemModel(parent);
    QString commandIcon = QStringLiteral(":/resources/command16.png");
    if (Preferences::displayTheme == THEME_DARK)
        commandIcon = QStringLiteral(":/resources/command_dark16.png");
    foreach (const QString &keyword, lpub->metaKeywords)
        model->appendRow(new QStandardItem(QIcon(commandIcon), keyword));

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    return model;
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
    if (Preferences::editorDecoration == SIMPLE_DECORATION)
      highlighterSimple = new HighlighterSimple(_textEdit->document());
    else
      highlighter = new Highlighter(_textEdit->document());

    setSelectionHighlighter();
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
        lpub->raisePreviewDockWindow();
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

    emit lpub->messageSig(LOG_ERROR, tr("Part preview for %1 failed.").arg(partType));
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

        programEntries = Settings.value(QString("%1/%2").arg(SETTINGS,openWithProgramListKey)).toStringList();

        numOpenWithPrograms = qMin(programEntries.size(), Preferences::maxOpenWithPrograms);

        emit lpub->messageSig(LOG_DEBUG, tr("1. Number of Programs: %1").arg(numOpenWithPrograms));

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
                    emit lpub->messageSig(LOG_INFO,tr("Could not save program file icon: %1").arg(iconFile));
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
            } else {
                programEntries.removeOne(programEntries.at(i));
                --numOpenWithPrograms;
            }
        }

        // add system editor if exits
        if (!Preferences::systemEditor.isEmpty()) {
          QFileInfo fileInfo(Preferences::systemEditor);
          if (fileInfo.exists() && fileInfo.isFile()) {
            QString arguments;
            if (Preferences::usingNPP)
              arguments = QLatin1String(WINDOWS_NPP_LPUB3D_UDL_ARG);
            const int i = numOpenWithPrograms;
            programPath = fileInfo.absoluteFilePath();
            programName = fileInfo.completeBaseName();
            programName.replace(programName[0],programName[0].toUpper());
            programData = QString("'%1' %2").arg(programPath).arg(arguments);
            QString text = programName;
            if (text.isEmpty())
                text = tr("&%1 %2").arg(i + 1).arg(fileInfo.fileName());
            openWithActList[i]->setText(text);
            openWithActList[i]->setData(programData); // includes arguments
            openWithActList[i]->setIcon(getProgramIcon());
            openWithActList[i]->setStatusTip(tr("Open current file with %2").arg(fileInfo.fileName()));
            openWithActList[i]->setVisible(true);
            programEntries.append(QString("%1|%2").arg(programName).arg(programData));
            numOpenWithPrograms = programEntries.size();
          }
        }

        emit lpub->messageSig(LOG_DEBUG, tr("2. Number of Programs: %1").arg(numOpenWithPrograms));

        // hide empty program actions - redundant
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
    QStringList list = program.split(quoteRx, SkipEmptyParts);            // Split by " or '
    if (list.size() == 1) {
        program = list.first();
    } else {
        QStringList values;
        for (QString &item : list) {
            if (inside) {                                                 // If 's' is inside quotes ...
                values.append(item);                                      // ... get the whole string
            } else {                                                      // If 's' is outside quotes ...
                values.append(item.split(" ",SkipEmptyParts));            // ... get the split string
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
    QStringList arguments = QStringList() << QDir::toNativeSeparators(fileName);
    QString program;
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
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
                emit lpub->messageSig(LOG_ERROR, tr("No program specified. Cannot launch %1.")
                                                    .arg(QFileInfo(fileName).fileName()));
            }
#endif
        } else {
            openWithProgramAndArgs(program,arguments);
        }
        qint64 pid;
        QString workingDirectory = QDir::currentPath() + QDir::separator();
        QProcess::startDetached(program, arguments, workingDirectory, &pid);
        emit lpub->messageSig(LOG_INFO, tr("Launched %1 with pid=%2 %3%4...")
                                           .arg(QFileInfo(fileName).fileName()).arg(pid)
                                           .arg(QFileInfo(program).fileName())
                                           .arg(arguments.size() ? QString(" %1").arg(arguments.join(" ")) : ""));
    }
}

void EditWindow::createOpenWithActions()
{
    // adjust for separator and system editor
    const int systemEditor = Preferences::systemEditor.isEmpty() ? 0 : 1;
    const int maxOpenWithPrograms = Preferences::maxOpenWithPrograms + systemEditor;
    for (int i = 0; i < maxOpenWithPrograms; i++) {
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

    editModelFileAct = new QAction(QIcon(":/resources/editldraw.png"),tr("Edit Current Model"), this);
    editModelFileAct->setObjectName("editModelFileAct.2");
    editModelFileAct->setStatusTip(tr("Edit loaded LDraw file in detached LDraw Editor"));
    lpub->actions.insert(editModelFileAct->objectName(), Action(QStringLiteral("Edit.Edit Current Model"), editModelFileAct));
    connect(editModelFileAct, SIGNAL(triggered()), this, SIGNAL(editModelFileSig()));

    previewLineAct = new QAction(QIcon(":/resources/previewpart.png"),tr("Preview Highlighted Line..."), this);
    previewLineAct->setObjectName("previewLineAct.2");
    previewLineAct->setStatusTip(tr("Display the part on the highlighted line in a popup 3D viewer"));
    lpub->actions.insert(previewLineAct->objectName(), Action(QStringLiteral("Edit.Preview Highlighted Line"), previewLineAct));
    connect(previewLineAct, SIGNAL(triggered()), this, SLOT(previewLine()));

#ifdef QT_DEBUG_MODE
    previewViewerFileAct = new QAction(QIcon(":/resources/previewpart.png"),tr("Preview Current Model..."), this);
    previewViewerFileAct->setObjectName("previewViewerFileAct.2");
    previewViewerFileAct->setStatusTip(tr("Display Visual Editor file in a popup 3D viewer"));
    lpub->actions.insert(previewViewerFileAct->objectName(), Action(QStringLiteral("Edit.Preview Current Model"), previewViewerFileAct));
    connect(previewViewerFileAct, SIGNAL(triggered()), this, SLOT(previewViewerFile()));
#endif

    cutAct = new QAction(QIcon(":/resources/cut.png"), tr("Cu&t"), this);
    cutAct->setObjectName("cutAct.2");
    cutAct->setShortcut(QStringLiteral("Ctrl+X"));
    cutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    lpub->actions.insert(cutAct->objectName(), Action(QStringLiteral("Edit.Cut"), cutAct));
    connect(cutAct, SIGNAL(triggered()), _textEdit, SLOT(cut()));

    copyAct = new QAction(QIcon(":/resources/copy.png"), tr("&Copy"), this);
    copyAct->setObjectName("copyAct.2");
    copyAct->setShortcut(QStringLiteral("Ctrl+C"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    lpub->actions.insert(copyAct->objectName(), Action(QStringLiteral("Edit.Copy"), copyAct));
    connect(copyAct, SIGNAL(triggered()), _textEdit, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/resources/paste.png"), tr("&Paste"), this);
    pasteAct->setObjectName("pasteAct.2");
    pasteAct->setShortcut(QStringLiteral("Ctrl+V"));
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    lpub->actions.insert(pasteAct->objectName(), Action(QStringLiteral("Edit.Paste"), pasteAct));
    connect(pasteAct, SIGNAL(triggered()), _textEdit, SLOT(paste()));

    findAct = new QAction(QIcon(":/resources/find.png"), tr("&Find"), this);
    findAct->setObjectName("findAct.2");
    findAct->setShortcut(QStringLiteral("Ctrl+F"));
    findAct->setStatusTip(tr("Find object"));
    lpub->actions.insert(findAct->objectName(), Action(QStringLiteral("Edit.Find"), findAct));
    connect(findAct, SIGNAL(triggered()), _textEdit, SLOT(findDialog()));

    gotoLineAct = new QAction(QIcon(":/resources/gotoline.png"), tr("&Go to Line"), this);
    gotoLineAct->setObjectName("gotoLineAct.2");
    gotoLineAct->setShortcut(QStringLiteral("Ctrl+G"));
    gotoLineAct->setStatusTip(tr("Go to line"));
    lpub->actions.insert(gotoLineAct->objectName(), Action(QStringLiteral("Edit.Go To Line"), gotoLineAct));
    connect(gotoLineAct, SIGNAL(triggered()), this, SLOT(gotoLine()));

    redrawAct = new QAction(QIcon(":/resources/redraw.png"), tr("&Redraw"), this);
    redrawAct->setObjectName("redrawAct.2");
    redrawAct->setShortcut(QStringLiteral("Ctrl+R"));
    redrawAct->setStatusTip(tr("Redraw page, reset model caches"));
    lpub->actions.insert(redrawAct->objectName(), Action(QStringLiteral("Edit.Redraw"), redrawAct));
    connect(redrawAct, SIGNAL(triggered()), this, SLOT(redraw()));

    updateAct = new QAction(QIcon(":/resources/update.png"), tr("&Update"), this);
    updateAct->setObjectName("updateAct.2");
    updateAct->setShortcut(QStringLiteral("Ctrl+U"));
    updateAct->setStatusTip(tr("Update page"));
    lpub->actions.insert(updateAct->objectName(), Action(QStringLiteral("Edit.Update"), updateAct));
    connect(updateAct, SIGNAL(triggered(bool)), this, SLOT(update(bool)));

    delAct = new QAction(QIcon(":/resources/delete.png"), tr("&Delete"), this);
    delAct->setObjectName("delAct.2");
    delAct->setShortcut(QStringLiteral("DEL"));
    delAct->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    delAct->setStatusTip(tr("Delete the selection"));
    lpub->actions.insert(delAct->objectName(), Action(QStringLiteral("File.Delete"), delAct));
    connect(delAct, SIGNAL(triggered()), this, SLOT(deleteSelection()));

    selAllAct = new QAction(QIcon(":/resources/selectall.png"), tr("&Select All"), this);
    selAllAct->setObjectName("selAllAct.2");
    selAllAct->setShortcut(QStringLiteral("Ctrl+A"));
    selAllAct->setStatusTip(tr("Select all page content"));
    lpub->actions.insert(selAllAct->objectName(), Action(QStringLiteral("File.Select All"), selAllAct));
    connect(selAllAct, SIGNAL(triggered()), _textEdit, SLOT(selectAll()));

    showAllCharsAct = new QAction(QIcon(":/resources/showallcharacters.png"), tr("Show All Characters"), this);
    showAllCharsAct->setObjectName("showAllCharsAct.2");
    showAllCharsAct->setShortcut(QStringLiteral("Ctrl+J"));
    showAllCharsAct->setStatusTip(tr("Show all characters"));
    showAllCharsAct->setCheckable(true);
    lpub->actions.insert(showAllCharsAct->objectName(), Action(QStringLiteral("File.Show All Characters"), showAllCharsAct));
    connect(showAllCharsAct, SIGNAL(triggered()), this, SLOT(showAllCharacters()));

    toggleCmmentAct = new QAction(QIcon(":/resources/togglecomment.png"), tr("Toggle Line Comment"), this);
    toggleCmmentAct->setObjectName("toggleCmmentAct.2");
    toggleCmmentAct->setShortcut(QStringLiteral("Ctrl+D"));
    toggleCmmentAct->setStatusTip(tr("Add or remove comment from selected line"));
    lpub->actions.insert(toggleCmmentAct->objectName(), Action(QStringLiteral("Edit.Toggle Line Comment"), toggleCmmentAct));
    connect(toggleCmmentAct, SIGNAL(triggered()), _textEdit, SLOT(toggleComment()));

    topAct = new QAction(QIcon(":/resources/topofdocument.png"), tr("Top of Document"), this);
    topAct->setObjectName("topAct.2");
    topAct->setShortcut(QStringLiteral("Ctrl+T"));
    topAct->setStatusTip(tr("Go to the top of document"));
    lpub->actions.insert(topAct->objectName(), Action(QStringLiteral("File.Top Of Document"), topAct));
    connect(topAct, SIGNAL(triggered()), this, SLOT(topOfDocument()));

    bottomAct = new QAction(QIcon(":/resources/bottomofdocument.png"), tr("Bottom of Document"), this);
    bottomAct->setObjectName("bottomAct.2");
    bottomAct->setShortcut(QStringLiteral("Ctrl+B"));
    bottomAct->setStatusTip(tr("Go to the bottom of document"));
    lpub->actions.insert(bottomAct->objectName(), Action(QStringLiteral("File.Bottom Of Document"), bottomAct));
    connect(bottomAct, SIGNAL(triggered()), this, SLOT(bottomOfDocument()));

    connect(_textEdit, SIGNAL(copyAvailable(bool)),
            cutAct,    SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(copyAvailable(bool)),
            copyAct,   SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(copyAvailable(bool)),
             delAct,   SLOT(setEnabled(bool)));

    // edit model file
    exitAct = new QAction(QIcon(":/resources/exit.png"),tr("E&xit"), this);
    exitAct->setObjectName("exitAct.2");
    exitAct->setShortcut(QStringLiteral("Ctrl+Q"));
    exitAct->setStatusTip(tr("Close this window"));
    lpub->actions.insert(exitAct->objectName(), Action(QStringLiteral("File.Exit"), exitAct));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    saveAct = new QAction(QIcon(":/resources/save.png"), tr("&Save"), this);
    saveAct->setObjectName("saveAct.2");
    saveAct->setShortcut(QStringLiteral("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    lpub->actions.insert(saveAct->objectName(), Action(QStringLiteral("File.Save"), saveAct));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));

    saveCopyAct = new QAction(QIcon(":/resources/savecopy.png"),tr("Save A Copy As..."), this);
    saveCopyAct->setObjectName("saveCopyAct.2");
    saveCopyAct->setShortcut(QStringLiteral("Ctrl+Shift+C"));
    saveCopyAct->setStatusTip(tr("Save a copy of the document under a new name"));
    saveCopyAct->setEnabled(false);
    lpub->actions.insert(saveCopyAct->objectName(), Action(QStringLiteral("File.Save A Copy As"), saveCopyAct));
    connect(saveCopyAct, SIGNAL(triggered()), this, SLOT(saveFileCopy()));

    undoAct = new QAction(QIcon(":/resources/editundo.png"), tr("Undo"), this);
    undoAct->setObjectName("undoAct.2");
    undoAct->setShortcut(QStringLiteral("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last change"));
    lpub->actions.insert(undoAct->objectName(), Action(QStringLiteral("Edit.Undo"), undoAct));
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

    redoAct = new QAction(QIcon(":/resources/editredo.png"), tr("Redo"), this);
    redoAct->setObjectName("redoAct.2");
#ifdef __APPLE__
    redoAct->setShortcut(QStringLiteral("Ctrl+Shift+Z"));
    redoAct->setStatusTip(tr("Redo last change"));
#else
    redoAct->setShortcut(QStringLiteral("Ctrl+Y"));
    redoAct->setStatusTip(tr("Redo last change"));
#endif
    lpub->actions.insert(redoAct->objectName(), Action(QStringLiteral("Edit.Redo"), redoAct));
    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

    preferencesAct = new QAction(QIcon(":/resources/preferences.png"),tr("Preferences..."), this);
    preferencesAct->setObjectName("preferencesAct.2");
    preferencesAct->setStatusTip(tr("Set your preferences for the LDraw Command editor"));
    lpub->actions.insert(preferencesAct->objectName(), Action(QStringLiteral("Edit.Preferences"), preferencesAct));
    connect(preferencesAct, SIGNAL(triggered()), this, SLOT(preferences()));

    openFolderAct = new QAction(QIcon(":/resources/openworkingfolder.png"),tr("Open Working Folder"), this);
    openFolderAct->setObjectName("openFolderAct.2");
    openFolderAct->setShortcut(QStringLiteral("Alt+Shift+2"));
    openFolderAct->setStatusTip(tr("Open file working folder"));
    lpub->actions.insert(openFolderAct->objectName(), Action(QStringLiteral("File.Open Working Folder"), openFolderAct));
    connect(openFolderAct, SIGNAL(triggered()), this, SLOT(openFolder()));

    copyFullPathToClipboardAct = new QAction(QIcon(":/resources/copytoclipboard.png"),tr("Full Path to Clipboard"), this);
    copyFullPathToClipboardAct->setObjectName("copyFullPathToClipboardAct.2");
    copyFullPathToClipboardAct->setShortcut(QStringLiteral("Alt+Shift+3"));
    copyFullPathToClipboardAct->setStatusTip(tr("Copy full file path to clipboard"));
    lpub->actions.insert(copyFullPathToClipboardAct->objectName(), Action(QStringLiteral("File.Full Path To Clipboard"), copyFullPathToClipboardAct));
    connect(copyFullPathToClipboardAct, SIGNAL(triggered()), this, SLOT(updateClipboard()));

    copyFileNameToClipboardAct = new QAction(QIcon(":/resources/copytoclipboard.png"),tr("File Name to Clipboard"), this);
    copyFileNameToClipboardAct->setObjectName("copyFileNameToClipboardAct.2");
    copyFileNameToClipboardAct->setShortcut(QStringLiteral("Alt+Shift+0"));
    copyFileNameToClipboardAct->setStatusTip(tr("Copy file name to clipboard"));
    lpub->actions.insert(copyFileNameToClipboardAct->objectName(), Action(QStringLiteral("File.File Name To Clipboard"), copyFileNameToClipboardAct));
    connect(copyFileNameToClipboardAct, SIGNAL(triggered()), this, SLOT(updateClipboard()));

    QIcon commandsDialogIcon;
    if (Preferences::displayTheme == THEME_DARK) {
        commandsDialogIcon.addFile(":/resources/command_dark32.png");
        commandsDialogIcon.addFile(":/resources/command_dark16.png");
    } else {
        commandsDialogIcon.addFile(":/resources/command32.png");
        commandsDialogIcon.addFile(":/resources/command16.png");
    }
    commandsDialogAct = new QAction(commandsDialogIcon,tr("Manage &LPub Metacommands"), this);
    commandsDialogAct->setObjectName("commandsDialogAct.2");
    commandsDialogAct->setStatusTip(tr("View LPub meta commands and customize command descriptions"));
    commandsDialogAct->setShortcut(QStringLiteral("Ctrl+K"));
    lpub->actions.insert(commandsDialogAct->objectName(), Action(QStringLiteral("Help.Manage LPub Metacommands"), commandsDialogAct));
    connect(commandsDialogAct, SIGNAL(triggered()), this, SLOT(commandsDialog()));

    openWithToolbarAct = new QAction(QIcon(":/resources/openwith.png"), tr("Open With..."), this);
    openWithToolbarAct->setObjectName("openWithToolbarAct.2");
    openWithToolbarAct->setStatusTip(tr("Open model file with selected application"));
    lpub->actions.insert(openWithToolbarAct->objectName(), Action(QStringLiteral("File.Open With"), openWithToolbarAct));

    editColorAct = new QAction(QIcon(":/resources/editcolor.png"),tr("Change Part Color..."), this);
    editColorAct->setObjectName("editColorAct.2");
    editColorAct->setStatusTip(tr("Edit the part color"));
    lpub->actions.insert(editColorAct->objectName(), Action(QStringLiteral("Tools.Change Part Color"), editColorAct));
    connect(editColorAct, SIGNAL(triggered()), this, SLOT(editLineItem()));

    editPartAct = new QAction(QIcon(":/resources/editpart.png"),tr("Change Part..."), this);
    editPartAct->setObjectName("editPartAct.2");
    editPartAct->setStatusTip(tr("Edit this part"));
    lpub->actions.insert(editPartAct->objectName(), Action(QStringLiteral("Tools.Change Part"), editPartAct));
    connect(editPartAct, SIGNAL(triggered()), this, SLOT(editLineItem()));

    substitutePartAct = new QAction(QIcon(":/resources/editplisubstituteparts.png"),tr("Substitute Part..."), this);
    substitutePartAct->setObjectName("substitutePartAct.2");
    substitutePartAct->setStatusTip(tr("Substitute this part"));
    lpub->actions.insert(substitutePartAct->objectName(), Action(QStringLiteral("Tools.Substitute Part"), substitutePartAct));
    connect(substitutePartAct, SIGNAL(triggered()), this, SLOT(editLineItem()));

    removeSubstitutePartAct = new QAction(QIcon(":/resources/removesubstitutepart.png"),tr("Remove Substitute..."), this);
    removeSubstitutePartAct->setObjectName("removeSubstitutePartAct.2");
    removeSubstitutePartAct->setStatusTip(tr("Replace this substitute part with the original part."));
    lpub->actions.insert(removeSubstitutePartAct->objectName(), Action(QStringLiteral("Tools.Remove Substitute"), removeSubstitutePartAct));
    connect(removeSubstitutePartAct, SIGNAL(triggered()), this, SLOT(editLineItem()));

    connect(_textEdit, SIGNAL(undoAvailable(bool)),
             undoAct,  SLOT(setEnabled(bool)));

    connect(_textEdit, SIGNAL(redoAvailable(bool)),
             redoAct,  SLOT(setEnabled(bool)));

    // This is only triggered when Undo/Redo buttons are visible
    connect(_textEdit, SIGNAL(undoKeySequence()),
             this,     SLOT(undoKeySequence()));

    // This is only triggered when Undo/Redo buttons are visible
    connect(_textEdit, SIGNAL(redoKeySequence()),
             this,     SLOT(redoKeySequence()));

    // This is triggerd by the mainwindow when Ctrl+Z key sequence is detected
    connect(this,      SIGNAL(triggerUndoSig()),
            this,      SLOT(undo()));

    // This is triggerd by the mainwindow when Ctrl+Y key sequence is detected
    connect(this,      SIGNAL(triggerRedoSig()),
            this,      SLOT(redo()));

    connect(saveAct,   SIGNAL(triggered(bool)),
             this,     SLOT(  updateDisabled(bool)));

    connect(_textEdit, SIGNAL(textChanged()),
             this,     SLOT(enableSave()));
}

void EditWindow::undoKeySequence()
{
    _isUndo = true;
}

// These are only triggered when Undo/Redo buttons are visible
void EditWindow::redoKeySequence()
{
    _isRedo = true;
}

void EditWindow::undo()
{
    _isUndo = true;
    if (_textEdit->document()->isUndoAvailable())
        _textEdit->undo();
}

void EditWindow::redo()
{
    _isRedo = true;
    if (_textEdit->document()->isRedoAvailable())
        _textEdit->redo();
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
    gotoLineAct->setEnabled(false);
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

    if (modelFileEdit())
        previewLineAct->setEnabled(false);
#ifdef QT_DEBUG_MODE
    previewViewerFileAct->setEnabled(false);
#endif
    editColorAct->setEnabled(false);
    editPartAct->setEnabled(false);

    if (removeMenu)
        removeMenu = nullptr;
    substitutePartAct->setEnabled(false);
    removeSubstitutePartAct->setEnabled(false);
}

void EditWindow::enableActions()
{
    editModelFileAct->setEnabled(true);

    redrawAct->setEnabled(true);
    selAllAct->setEnabled(true);
    findAct->setEnabled(true);
    gotoLineAct->setEnabled(true);
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
    editToolBar->setObjectName("editorEditToolbar");
    if (modelFileEdit()) {
        mpdCombo = new QComboBox(this);
        mpdCombo->setMinimumContentsLength(25);
        mpdCombo->setInsertPolicy(QComboBox::InsertAtBottom);
        mpdCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
        mpdCombo->setToolTip(tr("Go to submodel"));
        mpdCombo->setStatusTip("Use dropdown to go to submodel");
        connect(mpdCombo,SIGNAL(activated(int)),
                this,    SLOT(mpdComboChanged(int)));
        editToolBar->addSeparator();
        editToolBar->addAction(exitAct);
        editToolBar->addAction(saveAct);
        editToolBar->addAction(saveCopyAct);
        editToolBar->addSeparator();
        editToolBar->addAction(undoAct);
        editToolBar->addAction(redoAct);

        mpdComboSeparatorAct = editToolBar->addSeparator();
        mpdComboSeparatorAct->setObjectName("mpdComboSeparatorAct.2");
        lpub->actions.insert(mpdComboSeparatorAct->objectName(), Action(QStringLiteral(""), mpdComboSeparatorAct));

        mpdComboAct = editToolBar->addWidget(mpdCombo);
        mpdComboAct->setObjectName("mpdComboAct.2");
        lpub->actions.insert(mpdComboAct->objectName(), Action(QStringLiteral(""), mpdComboAct));

#ifndef QT_NO_CLIPBOARD
        editToolBar->addSeparator();
        editToolBar->addAction(copyFileNameToClipboardAct);
        editToolBar->addAction(copyFullPathToClipboardAct);
#endif
        editToolBar->addSeparator();
        editToolBar->addAction(openFolderAct);
        QMenu *openWithMenu = new QMenu(tr("Open With Menu"), this);
        openWithMenu->setEnabled(false);
        openWithToolbarAct->setMenu(openWithMenu);
        if (numOpenWithPrograms) {
            openWithMenu->setEnabled(true);
            const int systemEditor = Preferences::systemEditor.isEmpty() ? 0 : 1;
            const int maxOpenWithPrograms = Preferences::maxOpenWithPrograms + systemEditor;
            for (int i = 0; i < maxOpenWithPrograms; i++) {
                if (i == Preferences::maxOpenWithPrograms)
                    openWithMenu->addSeparator();
                openWithMenu->addAction(openWithActList.at(i));
            }
        }
        editToolBar->addAction(openWithToolbarAct);
        editToolBar->addSeparator();
    }
    editToolBar->addAction(updateAct);
    editToolBar->addAction(redrawAct);
    editToolBar->addAction(toggleCmmentAct);
    //editToolBar->addAction(showAllCharsAct);
    editToolBar->addAction(selAllAct);
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(findAct);
    editToolBar->addAction(gotoLineAct);
    editToolBar->addAction(delAct);
    editToolBar->addAction(topAct);
    editToolBar->addAction(bottomAct);
    editToolBar->addAction(preferencesAct);

    toolsToolBar = addToolBar(tr("Editor Tools Toolbar"));
    toolsToolBar->setObjectName("editorToolsToolbar");
    toolsToolBar->setEnabled(false);

    if (!isReadOnly) {
        toolsToolBar->addAction(editColorAct);
        toolsToolBar->addAction(editPartAct);
        toolsToolBar->addAction(substitutePartAct);
    }

    if (modelFileEdit())
        toolsToolBar->addAction(previewLineAct);
#ifdef QT_DEBUG_MODE
    toolsToolBar->addSeparator();
    toolsToolBar->addAction(previewViewerFileAct);
#endif

    disableActions();
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

int EditWindow::setCurrentStep(const int lineNumber, bool inScope)
{
    // limit the scope to the current page
    if (inScope) {
        if (!stepLines.isInScope(lineNumber))
            return INVALID_CURRENT_STEP;
    }

    const Where &top = lpub->page.top;
    const Where &bottom = lpub->page.bottom;
    const TypeLine here(fileOrderIndex, lineNumber);
    const QString stepKey = lpub->ldrawFile.getViewerStepKeyFromRange(here.modelIndex, here.lineIndex, top.modelIndex,top.lineNumber, bottom.modelIndex, bottom.lineNumber);

    if (!stepKey.isEmpty()) {
        if (lpub->currentStep && lpub->currentStep->viewerStepKey.startsWith(&stepKey))
            return EXISTING_CURRENT_STEP;

        // set current step
        lpub->setCurrentStep(stepKey);

        if (lpub->currentStep) {
#ifdef QT_DEBUG_MODE
            emit lpub->messageSig(LOG_DEBUG,tr("Loaded step for line %1 model: %2, page: %3, step: %4, line scope: %5-%6")
                                  .arg(here.lineIndex)
                                  .arg(lpub->ldrawFile.getSubmodelName(here.modelIndex,false))
                                  .arg(lpub->page.meta.LPub.page.number.number)
                                  .arg(lpub->currentStep->stepNumber.number)
                                  .arg(top.lineNumber + 1/*adjust for 0-start index*/)
                                  .arg(bottom.lineNumber + 1 /*adjust for 0-index*/));
#endif
            return NEW_CURRENT_STEP;
        }
#ifdef QT_DEBUG_MODE
        else
            emit lpub->messageSig(LOG_DEBUG,tr("Failed to set Current Step for key [%1], line: %2, model: %3")
                                  .arg(stepKey).arg(here.lineIndex).arg(lpub->ldrawFile.getSubmodelName(here.modelIndex,false)));
#endif
    }
#ifdef QT_DEBUG_MODE
    else
        emit lpub->messageSig(LOG_DEBUG,tr("Failed to get Viewer Step Key for line: %1, model: %2")
                              .arg(here.lineIndex).arg(lpub->ldrawFile.getSubmodelName(here.modelIndex,false)));
#endif
    return INVALID_CURRENT_STEP;
}

bool EditWindow::setValidPartLine()
{
    QTextCursor cursor = _textEdit->textCursor();
    cursor.select(QTextCursor::LineUnderCursor);
    QString selection = cursor.selection().toPlainText();
    QStringList list;
    QString partType, titleType = QLatin1String("part");
    int validCode = -1;
    int colorCode = LDRAW_MATERIAL_COLOUR;
    bool colorOk = false;
    bool isSubstitute = false;
    bool isSubstituteAlt = false;
    bool isDisplayType = lpub->currentStep->displayStep != DT_DEFAULT;
    bool isPliControlFile = modelFileEdit() && fileName == Preferences::pliControlFile;

    toolsToolBar->setEnabled(false);
    if (isReadOnly) {
        editColorAct->setVisible(false);
        editPartAct->setVisible(false);
        substitutePartAct->setVisible(false);
    } else {
        editColorAct->setText(tr("Edit Color"));
        editPartAct->setText(tr("Edit Part"));
        if (isPliControlFile)
            substitutePartAct->setEnabled(false);
        else if (isDisplayType)
            substitutePartAct->setEnabled(false);
        else
            substitutePartAct->setText(tr("Substitute Part"));
    }

    if (!isPliControlFile)
        copyFullPathToClipboardAct->setEnabled(false);
    copyFileNameToClipboardAct->setEnabled(false);

    if (selection.startsWith("1 ")) {
        list = selection.split(" ", SkipEmptyParts);

        validCode = list[1].toInt(&colorOk);

        // 0 1           2 3 4 5 6 7 8 9 10 11 12 13 14
        // 1 <colorCode> 0 0 0 0 0 1 1 0 0  0  1  0  <part type>
        for (int i = 14; i < list.size(); i++)
            partType += (list[i]+" ");

    } else if (selection.contains(QRegExp("\\sBEGIN\\sSUB\\s"))) {
        // 0 1     2   3     4   5           6
        // 0 !LPUB PLI BEGIN SUB <part type> <colorCode>
        list = selection.split(" ", SkipEmptyParts);

        if (list.size() > sSubPart)
            partType  = list[5];

        if (list.size() > sSubColor)
            validCode = list[6].toInt(&colorOk);

        isSubstitute = true;
    } else {
        return false;
    }

    if (partType.isEmpty())
        return false;

    if (colorOk)
        colorCode = validCode;
    else
        return false;

    const int lineNumber = cursor.blockNumber();
    const bool stepSet = modelFileEdit() ? false : setCurrentStep(lineNumber) != INVALID_CURRENT_STEP;

    // substitute partKey
    QString subPartKey = QString("%1|%2").arg(QFileInfo(partType).completeBaseName()).arg(QString::number(colorCode));

    // set substitute flag
    if (stepSet && !isSubstitute) {
        const PliPart* pliPart = lpub->currentStep->pli.getPart(QString(subPartKey).replace("|","_"));
        if (pliPart)
            isSubstituteAlt = pliPart->subType;
        // we have a partType that is not in the PLI so check if it is an excluded part
        else
            // if the partType is not excluded, likely is being substituted - this check is not 100%
            isSubstituteAlt = !ExcludedParts::isExcludedPart(partType);
    }

    partType = partType.trimmed();

#ifdef QT_DEBUG_MODE
    emit lpub->messageSig(LOG_DEBUG,
                               QString("Editor PartType: %1, ColorCode: %2, Line: %3")
                               .arg(partType).arg(colorCode).arg(selection));
#endif

    // partType is valid if we get here so check color to enable tools
    //toolsToolBar->setEnabled(true);

    const QString elidedPartType = partType.size() > 20 ? QString(partType.left(17) + "..." + partType.right(3)) : partType;

    lcPreferences& Preferences = lcGetPreferences();

    if (modelFileEdit()) {
        if (Preferences.mPreviewEnabled && !isIncludeFile) {
            previewLineAct->setText(tr("Preview %1 %2...").arg(titleType).arg(elidedPartType));
            previewLineAct->setData(QString("%1|%2").arg(colorCode).arg(partType));
            previewLineAct->setEnabled(true);
        }
    }

    if (_subFileList.contains(partType.toLower())) {
        if (modelFileEdit()) {
            titleType = "Subfile";
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

    if (!isReadOnly) {

        if (colorCode != LDRAW_MATERIAL_COLOUR) {
            editColorAct->setText(tr("Edit Color %1 (%2)...").arg(gColorList[lcGetColorIndex(colorCode)].Name).arg(colorCode));
            editColorAct->setData(QString("%1|%2").arg(colorCode).arg(selection));
        }
        editColorAct->setEnabled(colorCode != LDRAW_MATERIAL_COLOUR);

        editPartAct->setText(tr("Edit %1 %2...").arg(titleType).arg(elidedPartType));
        editPartAct->setData(QString("%1|%2").arg(partType).arg(colorCode));
        editPartAct->setEnabled(true);

        const QString actionText = tr("Substitute  %1...").arg(elidedPartType);

        if (!isPliControlFile && !isDisplayType) {
            if (isSubstitute || isSubstituteAlt) {
                removeSubstitutePartAct->setText(tr("Remove %1").arg(actionText));
                removeSubstitutePartAct->setData(QString("%1|%2").arg(subPartKey).arg(isSubstitute ? sRemove : sRemoveAlt));
                removeSubstitutePartAct->setEnabled(stepSet || modelFileEdit());

                removeMenu = new QMenu(tr("Remove %1").arg(actionText), this);
                removeMenu->setIcon(QIcon(":/resources/removesubstitutepart.png"));
                removeMenu->addAction(removeSubstitutePartAct);

                substitutePartAct->setText(tr("Change %1").arg(actionText));
                substitutePartAct->setMenu(removeMenu);
                subPartKey.append(QString("|%1").arg(sUpdate));
            } else {
                if (removeMenu) {
                    delete removeMenu;
                    removeMenu = nullptr;
                }
                substitutePartAct->setText(actionText);
                subPartKey.append(QString("|%1").arg(sSubstitute));
            }
            substitutePartAct->setData(subPartKey);
            substitutePartAct->setEnabled(stepSet || modelFileEdit());
        }

        if (numOpenWithPrograms)
            openWithToolbarAct->setEnabled(true);
    } // !isReadOnly

    return true;
}

void EditWindow::showContextMenu(const QPoint &pt)
{
    QMenu *menu = _textEdit->createStandardContextMenu();

    if (!fileName.isEmpty()) {
        bool isPliControlFile = modelFileEdit() && fileName == Preferences::pliControlFile;
        bool isDisplayType = lpub->currentStep->displayStep != DT_DEFAULT;
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
            menu->addSeparator();
            QMenu *openWithMenu = new QMenu(tr("Open With..."), this);
            openWithMenu->setIcon(QIcon(":/resources/openwith.png"));
            menu->addMenu(openWithMenu);
            if (numOpenWithPrograms) {
                for (int i = 0; i < numOpenWithPrograms; i++) {
                    QFileInfo fileInfo(programEntries.at(i).split("|").last());
                    openWithActList[i]->setStatusTip(tr("Open %1 with %2")
                                                     .arg(QFileInfo(fileName).fileName())
                                                     .arg(fileInfo.fileName()));
                    openWithMenu->addAction(openWithActList.at(i));
                }
            }
        }

        menu->addSeparator();
        QMenu *toolsMenu = new QMenu(tr("Tools..."), this);
        toolsMenu->setIcon(QIcon(":/resources/tools.png"));
        menu->addMenu(toolsMenu);
        toolsMenu->addAction(commandsDialogAct);

        if (setValidPartLine()) {
            toolsMenu->addAction(editColorAct);
            toolsMenu->addAction(editPartAct);
            if (!isPliControlFile && !isDisplayType)
                toolsMenu->addAction(substitutePartAct);
            if (modelFileEdit())
                toolsMenu->addAction(previewLineAct);
        }
        menu->addSeparator();
    }

    menu->addAction(updateAct);
    menu->addAction(redrawAct);
    menu->addAction(toggleCmmentAct);
    menu->addAction(findAct);
    menu->addAction(gotoLineAct);
    menu->addAction(topAct);
    menu->addAction(bottomAct);
    menu->addAction(showAllCharsAct);
    menu->exec(_textEdit->mapToGlobal(pt));
    delete menu;
}

void EditWindow::editLineItem()
{
    QString findText;
    QString replaceText;
    QStringList elements, items;

    int lineNumber = 0;
    int currentLine = 0;
    int selectedLines = 0;
    int action = sSubstitute;

    if (sender() == editColorAct) {
        elements = editColorAct->data().toString().split("|");
        int colorCode = elements.first().toInt();
        int newColorIndex = -1;
        items = elements.at(1).split(" ", SkipEmptyParts);
        QColor qcolor = lcQColorFromVector4(gColorList[lcGetColorIndex(colorCode)].Value);
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
    else if (sender() == substitutePartAct) {
        elements = substitutePartAct->data().toString().split("|");
        if (elements.size() == 3) {
            action = elements.at(2).toInt();
            if (!substitutePLIPart(replaceText, action, elements))
                return;
        } else {
            emit lpub->messageSig(LOG_ERROR, QString("Failed to retrieve substitue part key from action data [%1].").arg(elements.join(" ")));
            return;
        }
    }
    else if (sender() == removeSubstitutePartAct) {
        elements = removeSubstitutePartAct->data().toString().split("|");
        if (elements.size() == 3) {
            action = elements.at(2).toInt();
        } else {
            emit lpub->messageSig(LOG_ERROR, QString("Failed to retrieve substitue part key from action data [%1].").arg(elements.join(" ")));
            return;
        }
    }

    auto removeLine = [] (QTextCursor &cursor, int lineNumber)
    {
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.removeSelectedText();
        cursor.deleteChar();
    };

    QTextCursor cursor = _textEdit->textCursor();
    if(cursor.selection().isEmpty())
        cursor.select(QTextCursor::LineUnderCursor);

    QString str = cursor.selection().toPlainText();
    selectedLines = str.count("\n") + 1;

    if (action == sRemove)
        lineNumber = cursor.blockNumber();

    QTextCursor::MoveOperation nextLine = cursor.anchor() < cursor.position() ? QTextCursor::Up : QTextCursor::Down;

    cursor.beginEditBlock();

    while (currentLine < selectedLines)
    {
        cursor.select(QTextCursor::LineUnderCursor);
        QString selection = cursor.selectedText();

        bool result  = !selection.isEmpty();

        if (result) {
            if (sender() == editPartAct) {
                // set cursor selection to the part type
                QTextDocument::FindFlags flags;
                if ((result = _textEdit->find(findText, flags))) {
                    cursor.setPosition(_textEdit->textCursor().anchor(),QTextCursor::MoveAnchor);
                    cursor.setPosition(_textEdit->textCursor().position(), QTextCursor::KeepAnchor);
                }
            } else {
                if (action == sSubstitute)
                    replaceText.replace(SUB_PLACEHOLDER, selection);
            }
        } else {
            break;
        }

        if (result) {
            // remove substitute lines from bottom-up
            if (action == sRemove) {
                removeLine(cursor, lineNumber + 4);
                removeLine(cursor, lineNumber + 3);
                removeLine(cursor, lineNumber + 1);
                removeLine(cursor, lineNumber);
            } else if (action == sRemoveAlt) {
                removeLine(cursor, lineNumber + 2);
                removeLine(cursor, lineNumber + 1);
                removeLine(cursor, lineNumber - 1);
                removeLine(cursor, lineNumber - 2);
            } else {
                cursor.insertText(replaceText);
            }

            if (++currentLine < selectedLines) {
                cursor.movePosition(nextLine, QTextCursor::MoveAnchor);
            }

            _textEdit->setTextCursor(cursor);
            _textEdit->ensureCursorVisible();
        }
    }

    cursor.endEditBlock();
}

bool EditWindow::substitutePLIPart(QString &replaceText, const int action, const QStringList &elements)
{
    QStringList attributes;
    if (action == sRemove) {
        attributes.append(elements.at(sType));
        attributes.append(elements.at(sColorCode));
        replaceText = attributes.join(" ");
        return true;
    }

    QMessageBox box;
    box.setTextFormat (Qt::RichText);
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    box.setStandardButtons (QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    box.setDefaultButton   (QMessageBox::Save);
    box.setWindowTitle("Substitute PLI Part");

    Step* step = lpub->currentStep;
    if (step) {
        const QString key = QString("%1_%2").arg(elements.at(sType)).arg(elements.at(sColorCode));
        const PliPart* pliPart = step->pli.getPart(key);
        if (pliPart) {
            QStringList defaultList;
            if (action == sUpdate) {
                const float modelScale = step->pli.pliMeta.modelScale.value();
                const bool customViewpoint = step->pli.pliMeta.cameraAngles.customViewpoint();
                const bool noCA = !customViewpoint && step->pli.pliMeta.rotStep.value().type.toUpper() == QLatin1String("ABS");
                defaultList.append(QString::number(double(modelScale)));
                defaultList.append(QString::number(double(step->pli.pliMeta.cameraFoV.value())));
                defaultList.append(QString::number(noCA ? double(0.0f) : double(step->pli.pliMeta.cameraAngles.value(0))));
                defaultList.append(QString::number(noCA ? double(0.0f) : double(step->pli.pliMeta.cameraAngles.value(1))));
                defaultList.append(QString(QString("%1 %2 %3")
                                           .arg(double(step->pli.pliMeta.target.x()))
                                           .arg(double(step->pli.pliMeta.target.y()))
                                           .arg(double(step->pli.pliMeta.target.z()))).split(" "));
                defaultList.append(QString(renderer->getRotstepMeta(step->pli.pliMeta.rotStep,true)).split("_"));
            }
            // treat parts with '_' in the name - encode
            if (pliPart->type.count("_")) {
                QString nameKey = pliPart->nameKey;
                const QString type = QFileInfo(pliPart->type).completeBaseName();
                nameKey.replace(type, QString(type).replace("_", ";"));
                attributes = nameKey.split("_");
                attributes.replace(0,type);
            } else {
                attributes = pliPart->nameKey.split("_");
            }
            attributes.removeAt(nResType);
            attributes.removeAt(nResolution);
            attributes.removeAt(nPageWidth);
            attributes.replace(nType,pliPart->type);
            if (attributes.size() == nAdjustedBaseAttributes        /*BaseAttributes - removals*/)
                attributes.append(QString("0 0 0 0 0 0 REL").split(" "));
            else if (attributes.size() == nAdjustedTarget           /*Target - removals*/)
                attributes.append(QString("0 0 0 REL").split(" ")); /*13 items total without substituted part [new substitution]*/
            if (!pliPart->subOriginalType.isEmpty())
                attributes.append(pliPart->subOriginalType);        /*14 items total with substituted part [update substitution]*/
            if (SubstitutePartDialog::getSubstitutePart(attributes,this,action,defaultList)) {
                if (action == sUpdate)
                    replaceText = QString("0 !LPUB PLI BEGIN SUB %1").arg(attributes.join(" "));
                else
                    replaceText = QString("0 !LPUB PLI BEGIN SUB %1\n"
                                          "0 !LPUB PART BEGIN IGN\n"
                                          "%2\n"
                                          "0 !LPUB PLI END\n"
                                          "0 !LPUB PART END").arg(attributes.join(" ")).arg(SUB_PLACEHOLDER);
                return true;
            } else
                return false;
        } else {
            const int colorCode = elements.at(sColorCode).toInt();
            const QString type = elements.at(sType);
            const QString colorName = gColorList[lcGetColorIndex(colorCode)].Name;
            if (_textEdit->document()->isModified()) {
                box.setWindowIcon(QIcon());
                box.setIconPixmap (QPixmap(":/icons/lpub96.png"));
                const QString title = "<b>" + QMessageBox::tr ("Unsaved substitute part unpdates detected.") + "</b>";
                const QString text = QMessageBox::tr("<br>Do you want to save your updates for part [%1], color %2 (%3)...")
                                                     .arg(type).arg(colorName).arg(colorCode);
                box.setText (title);
                box.setInformativeText (text);
                if (box.exec() == QMessageBox::Save)
                    emit saveAct->triggered();
                else if (box.exec() == QMessageBox::Discard)
                    emit undoAct->triggered();
                return false;
            } else {
                emit lpub->messageSig(LOG_ERROR, tr("Failed to retrieve part [%1], color %2 (%3)...")
                                                    .arg(type).arg(colorName).arg(colorCode));
                return false;
            }
        }
    } // step

    emit lpub->messageSig(LOG_ERROR, tr("Failed to get current Step for selected line."));
    return false;
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
        pageUpDown(QTextCursor::Up, QTextCursor::KeepAnchor);
        if (!_textEdit->find(findText))
            statusBar()->showMessage(tr("Did not find submodel '%1'").arg(findText));
        else {
            QTextCursor cursor = _textEdit->textCursor();
            int lineNumber = cursor.blockNumber();
            _textEdit->moveCursor(QTextCursor::Start,QTextCursor::MoveAnchor);
            for (int i = 0; i < lineNumber; i++)
              _textEdit->moveCursor(QTextCursor::Down/*QTextCursor::EndOfLine*/,QTextCursor::MoveAnchor/*QTextCursor::KeepAnchor*/);
            pageUpDown(QTextCursor::Up, QTextCursor::KeepAnchor);
        }
    }
}

void EditWindow::contentsChange(
  int position,
  int charsRemoved,
  int charsAdded)
{
  bool contentsChanged = false;
  QString addedChars,removedChars;

  if (charsAdded || charsRemoved) {
    addedChars = _textEdit->toPlainText();

    if (addedChars.size() == 0)
      return;

    addedChars = addedChars.mid(position,charsAdded);
    removedChars = lpub->ldrawFile.contents(fileName).join("\n");
    removedChars = removedChars.mid(position,charsRemoved);
    contentsChanged = addedChars != removedChars;

    if (!contentsChanged)
        return;
  }

  bool isUndo = _isUndo;
  bool isRedo = _isRedo;
  _isUndo = _isRedo = false;

  if (!Preferences::saveOnUpdate)
     updateDisabled(false);

  emit contentsChangeSig(fileName, isUndo, isRedo, position, charsRemoved, addedChars);
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
            m->showMessage(tr("Failed to open folder!\n%1").arg(path));
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
            m->showMessage(tr("Failed to open working folder!\n%1").arg(path));
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
            emit lpub->messageSig(LOG_ERROR, tr("Copy to clipboard - Sender: %1, No data detected")
                                               .arg(sender()->metaObject()->className()));
            return;
        }

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(data, QClipboard::Clipboard);

        if (clipboard->supportsSelection())
            clipboard->setText(data, QClipboard::Selection);

#if defined(Q_OS_LINUX)
        QThread::msleep(1); //workaround for copied text not being available...
#endif

        QString efn =QFileInfo(data).fileName();
        // Text elided to 20 chars
        QString _fileName = QString("File '%1' %2")
                           .arg(efn.size() > 20 ?
                                efn.left(17) + "..." +
                                efn.right(3) : efn)
                           .arg(fullPath ? "full path" : "name");

        emit lpub->messageSig(LOG_INFO_STATUS, tr("%1 copied to clipboard.").arg(_fileName));
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

void EditWindow::highlightSelectedParts() {

    if (isIncludeFile || isReadOnly)
        return;

    toolsToolBar->setEnabled(setValidPartLine());

    // stop here if in detached editor
    if (modelFileEdit())
        return;

    if (!visualEditorVisible)
        return;

    QTextCursor cursor = _textEdit->textCursor();

    bool setSelection = false;
    if(!cursor.hasSelection()) {
        setSelection = true;
        cursor.select(QTextCursor::LineUnderCursor);
    }

    QStringList content = cursor.selection().toPlainText().split("\n");

    int selectedLines = content.size();

    if (!selectedLines)
        return;

    QTextCursor::MoveOperation nextLine = cursor.anchor() < cursor.position() ? QTextCursor::Up : QTextCursor::Down;

    int currentLine = 0;

    bool performHighlight = false;
    int firstLine = cursor.blockNumber();
    QVector<LineHighlight> highlightSelection;
    QVector<TypeLine> lineTypeIndexes;

    while (currentLine < selectedLines)
    {
        // only process lines that are in the currently displayed page
        int lineNumber = cursor.blockNumber();

        if (!stepLines.isInScope(lineNumber)) {
            emit lpub->messageSig(LOG_NOTICE,
                                  QString("Line index %1 is out of the current page line scope [%2-%3]: %4")
                                  .arg(lineNumber)
                                  .arg(stepLines.top)
                                  .arg(stepLines.bottom)
                                  .arg(content.at(currentLine)));
            break;
        }

        // display step in viewer enabled
        if (Preferences::editorLoadSelectionStep) {
            // submit the last selected line to update the viewer
            if (currentLine == stepLines.bottom || currentLine + 1 == selectedLines) {
                // display new step in the viewer i.e. not the initial display step, e.g. a step group step etc...
                if (selectedLines > 1 && setCurrentStep(lineNumber) == NEW_CURRENT_STEP) {
                    // for now, clear any saved selection highlighting when displaying a new step
                    if (Preferences::editorHighlightLines)
                        clearEditorHighlightLines();
                    emit setStepForLineSig();
                }
            }
        }

        if (content.at(currentLine).startsWith("1 ") || content.at(currentLine).contains(" PLI BEGIN SUB ")) {
            TypeLine tl(fileOrderIndex, lineNumber);
            lineTypeIndexes.append(tl);
            performHighlight = Preferences::editorHighlightLines;
            if (performHighlight) {
                bool clearSelection = savedSelection.contains(lineNumber);
                LineHighlight lh(lineNumber, clearSelection ? HIGHLIGHT_CLEAR : HIGHLIGHT_SELECTION);
                highlightSelection.append(lh);
                if (clearSelection)
                    savedSelection.removeAll(lineNumber);
                else
                    savedSelection.append(lineNumber);
            }
        }
        cursor.movePosition(nextLine);
        cursor.select(QTextCursor::LineUnderCursor);
        currentLine++;
    }

    // triggered when type 0 line selected but items (from editor) are selected in the viewer
    if (highlightSelection.isEmpty() && savedSelection.size())
    {
        performHighlight = true;
        LineHighlight lh(firstLine,HIGHLIGHT_CURRENT);
        highlightSelection.append(lh);

        for (int line : savedSelection) {
            LineHighlight lh(line,HIGHLIGHT_SELECTION);
            highlightSelection.append(lh);
        }

        auto lt = [] (const LineHighlight &lh1, const LineHighlight &lh2) { return lh1.line < lh2.line; };
        std::sort(highlightSelection.begin(), highlightSelection.end(),lt);
    }

    if (setSelection)
        cursor.clearSelection();

    if (performHighlight)
        highlightSelectedLines(highlightSelection, true/*isEditor*/);

    if (lineTypeIndexes.size())
        emit SelectedPartLinesSig(lineTypeIndexes);
}

void EditWindow::clearEditorHighlightLines(bool currentLine)
{
    QVector<LineHighlight> selection;
    if (currentLine) {
        QTextCursor cursor = _textEdit->textCursor();
        if (!cursor.isNull()) {
            LineHighlight hl(cursor.blockNumber(),HIGHLIGHT_CLEAR);
            selection.append(hl);
        }
    } else if (savedSelection.size()) {
        for (int line : savedSelection) {
            LineHighlight lh(line,HIGHLIGHT_CLEAR);
            selection.append(lh);
        }
        savedSelection.clear();
    }

    highlightSelectedLines(selection, true/*isEditor*/);
}

void EditWindow::highlightSelectedLines(QVector<int> &lines, bool clear)
{
    QVector<LineHighlight> selection;
    for (int line : lines) {
        LineHighlight lh(line, clear ? HIGHLIGHT_CLEAR : HIGHLIGHT_SELECTION);
        selection.append(lh);
    }

    highlightSelectedLines(selection, false/*isEditor*/);
}

void EditWindow::highlightSelectedLines(QVector<LineHighlight> &lines, bool isEditor)
{
    if (isReadOnly)
        return;

    auto formatSelection = [this, &isEditor] (QVector<LineHighlight> &lines)
    {
        QList<QTextEdit::ExtraSelection> extraSelections;

        auto lineColor = [&isEditor] (LineHighlightType action)
        {
            QColor color = QColor(Qt::transparent);

            switch(action)
            {
                case HIGHLIGHT_CLEAR:
                    if (Preferences::displayTheme == THEME_DARK)
                        color = QColor(Preferences::themeColors[THEME_DARK_PALETTE_BASE]);
                    else
                        color = QColor(Preferences::themeColors[THEME_DEFAULT_VIEWER_BACKGROUND_COLOR]);
                    break;
                case HIGHLIGHT_SELECTION:
                    if (Preferences::displayTheme == THEME_DARK) {
                        color = QColor(isEditor ?
                                           Preferences::themeColors[THEME_DARK_LINE_HIGHLIGHT_EDITOR_SELECT] :
                                           Preferences::themeColors[THEME_DARK_LINE_HIGHLIGHT_VIEWER_SELECT]);
                        color.setAlpha(100); // make 60% transparent
                    } else {
                        color = QColor(isEditor ?
                                           Preferences::themeColors[THEME_DEFAULT_LINE_HIGHLIGHT_EDITOR_SELECT] :
                                           Preferences::themeColors[THEME_DEFAULT_LINE_HIGHLIGHT_VIEWER_SELECT]);
                    }
                    color = color.lighter(180);
                    break;
                case HIGHLIGHT_CURRENT:
                    if (Preferences::displayTheme == THEME_DARK)
                        color = QColor(Preferences::themeColors[THEME_DARK_LINE_HIGHLIGHT]);
                    else
                        color = QColor(Preferences::themeColors[THEME_DEFAULT_LINE_HIGHLIGHT]);
                    break;
                default:
                    break;
            }
            return color;
        };

        if (lines.size()) {
            for (int i = 0; i < lines.size(); ++i) {
                QTextCursor cursor(_textEdit->document()->findBlockByNumber(lines.at(i).line));
                if (!cursor.isNull()) {
                    QTextEdit::ExtraSelection selection;
                    selection.format.setBackground(lineColor(lines.at(i).action));
                    if (lines.at(i).action == HIGHLIGHT_CURRENT) {
                        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
                        cursor.clearSelection();
                    } else {
                        cursor.select(QTextCursor::LineUnderCursor);
                    }
                    selection.cursor = cursor;
                    extraSelections.append(selection);
                }
            }
        } else {
            for (int i = stepLines.top; i <= stepLines.bottom; ++i) {
                QTextCursor cursor(_textEdit->document()->findBlockByNumber(i));
                if (!cursor.isNull()) {
                    cursor.select(QTextCursor::LineUnderCursor);
                    const QString selection = cursor.selection().toPlainText();
                    if (selection.startsWith("1 ")) {
                        QTextEdit::ExtraSelection selection;
                        selection.format.setBackground(lineColor(HIGHLIGHT_CLEAR));
                        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
                        selection.cursor = cursor;
                        selection.cursor.clearSelection();
                        extraSelections.append(selection);
                    }
                }
            }
        }
        _textEdit->setExtraSelections(extraSelections);
    };

    // clear or highlight saved selection - if any
    QVector<LineHighlight> selection;
    QVector<int> savedLines;

    if (savedSelection.size()) {
        if (!isEditor) {
            // copy saved selection lines and clear saved selection
            savedLines = savedSelection;
            savedSelection.clear();
        } else {
            auto foundInLines = [&lines] (int line) {
                for (const LineHighlight &lh : lines)
                    if (line == lh.line)
                        return true;
                return false;
            };
            // add saved selection lines to selected lines
            for (int savedLine : savedSelection)
                if (!foundInLines(savedLine))
                   savedLines.append(savedLine);
        }
    }

    // add selected lines to lines
    if (savedLines.size()) {
        for (int line : savedLines) {
            LineHighlight lh(line, isEditor ? HIGHLIGHT_SELECTION : HIGHLIGHT_CLEAR);
            selection.append(lh);
        }

        // merge lines into selection
        for (LineHighlight lh : lines)
            selection.append(lh);

        // sort selection
        auto lt = [] (const LineHighlight &lh1, const LineHighlight &lh2) { return lh1.line < lh2.line; };
        std::sort(selection.begin(), selection.end(),lt);
    }

    // format selection
    formatSelection(savedLines.size() ? selection : lines);
}

void EditWindow::highlightCurrentLine()
{
    QTextCursor cursor = _textEdit->textCursor();

    if (isIncludeFile) {
        cursor.select(QTextCursor::LineUnderCursor);
        const QString selection = cursor.selection().toPlainText();
        if (selection.startsWith("1 "))
            showLineType = LINE_ERROR;
        else
            showLineType = LINE_HIGHLIGHT;
    } else if (Preferences::editorHighlightLines && !isReadOnly) {
        const int line = cursor.blockNumber();
        if (savedSelection.size() && line >= stepLines.top && line <= stepLines.bottom)
            return;
    }

    QList<QTextEdit::ExtraSelection> extraSelections;

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
    selection.cursor = cursor;
    selection.cursor.clearSelection();
    extraSelections.append(selection);

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
          this,              SLOT(  verticalScrollValueChanged(int)));

  _textEdit->ensureCursorVisible();
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

          emit lpub->messageSig(LOG_INFO_STATUS,QString("Show Line %1 - Loading buffered page %2 lines...")
                                     .arg(lineNumber).arg(linesNeeded));

          for (int i = 0; i < pages && !_contentLoading; i++) {
              QApplication::processEvents();
              loadPagedContent();
          }

          emit lpub->messageSig(LOG_STATUS,QString());
#ifdef QT_DEBUG_MODE
          emit lpub->messageSig(LOG_DEBUG,QString("ShowLine add %1 %2 to line %3 from line %4.")
                                     .arg(pages).arg(pages == 1 ? "page" : "pages").arg(lineNumber).arg(_pageIndx + 1));
#endif
          waitingSpinnerStop();
      }
  }

  _textEdit->moveCursor(QTextCursor::Start,QTextCursor::MoveAnchor);
  for (int i = 0; i < showLineNumber; i++)
    _textEdit->moveCursor(QTextCursor::Down/*QTextCursor::EndOfLine*/,QTextCursor::MoveAnchor/*QTextCursor::KeepAnchor*/);
  //_textEdit->ensureCursorVisible();

  pageUpDown(QTextCursor::Up, QTextCursor::KeepAnchor);
}

bool EditWindow::updateEnabled()
{
    return updateAct->isEnabled();
}

void EditWindow::updateDisabled(bool state)
{
    if (sender() == saveAct) {
        updateAct->setDisabled(true);
    } else {
        updateAct->setDisabled(state);
        if (sender() == updateAct && !Preferences::saveOnUpdate) {
            if (!modelFileEdit())
                emit updateDisabledSig(state);
        }
    }
}

void EditWindow::setSubFiles(const QStringList& subFiles) {
    _subFileList = subFiles;
    _subFileListPending = false;
}

void EditWindow::setPagedContent(const QStringList & content)
{
#ifdef QT_DEBUG_MODE
    emit lpub->messageSig(LOG_DEBUG,QString("Set paged content line count: %1, content size %2")
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
    emit lpub->messageSig(LOG_DEBUG,QString("Set plain text line count: %1, content size %2")
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

void EditWindow::setVisualEditorVisible(bool value)
{
    visualEditorVisible = value;
#ifdef QT_DEBUG_MODE
        emit lpub->messageSig(LOG_DEBUG,QMessageBox::tr("Visual Editor (from Command Editor) visible: %1").arg(value ? "TRUE" : "FALSE"));
#endif
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
  if (!Preferences::modeGUI)
      return;

  disableActions();

  reloaded        = _fileName == fileName;
  fileName        = _fileName;
  lineCount       = 0;
  _pageIndx       = 0;
  _contentLoaded  = false;
  _waitingSpinner = nullptr;
  displayTimer.start();

  disconnect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
             this,                  SLOT(  contentsChange(int,int,int)));

  if (ldrawFile) {
    _subFileListPending = true;
    fileOrderIndex = ldrawFile->getSubmodelIndex(_fileName);
    isIncludeFile  = ldrawFile->isIncludeFile(_fileName);
    stepLines      = lineScope;
    clearEditorHighlightLines();
  } else if (!modelFileEdit())
      return;

#ifdef QT_DEBUG_MODE
    emit lpub->messageSig(LOG_DEBUG,QString("1. Editor Load Starting..."));
#endif

  if (fileName.isEmpty()) {

    _textEdit->document()->clear();

  } else if (modelFileEdit()) { // Detached Editor

    if (fileName == Preferences::pliControlFile) {
        redrawAct->setVisible(false);
        updateAct->setVisible(false);
        substitutePartAct->setVisible(false);
    }

    if (!ldrawFile && !QFileInfo(fileName).exists()) {
      _textEdit->document()->setModified(false);
      connect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
              this,                  SLOT(  contentsChange(int,int,int)));
      return;
    }

    waitingSpinnerStart();

    fileWatcher.removePath(fileName);

    disconnect(_textEdit, SIGNAL(textChanged()),
               this,      SLOT(  enableSave()));

    _textEdit->document()->clear();
    _textEdit->setIsUTF8(ldrawFile ? LDrawFile::_currFileIsUTF8 : true);

    QFuture<int> loadFuture = QtConcurrent::run(loadModelWorker->loadModel, ldrawFile, fileName);
    futureWatcher.setFuture(loadFuture);

  } // Detached Editor
  else
  {
    if (!ldrawFile)
        return;

    waitingSpinnerStart();

    lineCount = ldrawFile->size(fileName);

    if (Preferences::editorBufferedPaging && lineCount > Preferences::editorLinesPerPage) {
#ifdef QT_DEBUG_MODE
      emit lpub->messageSig(LOG_DEBUG,QString("3. Editor Load Paged Text Started..."));
#endif
      _pageContent = ldrawFile->contents(fileName);

      loadPagedContent();

    } else {
#ifdef QT_DEBUG_MODE
      emit lpub->messageSig(LOG_DEBUG,QString("3. Editor Load Plain Text Started..."));
#endif
      // loadContentBlocks(ldrawFile->contents(fileName),true/ *initial load* /);

      _textEdit->setPlainText(ldrawFile->contents(fileName).join("\n"));
    }

    loadFinished();
  }  // Docked Editor
}

void EditWindow::loadFinished()
{
    waitingSpinnerStop();

    const QString message = tr("%1 File %2: %3, %4 lines - %5")
            .arg(isIncludeFile ? "Include" : "Model")
            .arg(reloaded ? "Updated" : "Loaded")
            .arg(QFileInfo(fileName).fileName())
            .arg(lineCount)
            .arg(lpub->elapsedTime(displayTimer.elapsed()));

    if (modelFileEdit()) {  // Detached Editor
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
                this,       SLOT(  enableSave()));

        exitAct->setEnabled(true);

        statusBar()->showMessage(message);

        if (modelFileEdit()) {
            saveAct->setStatusTip(tr("Save %1").arg(QDir::toNativeSeparators(fileName)));
            if(QFileInfo(fileName).exists())
                fileWatcher.addPath(fileName);
        }
    } // Detached Editor

    _contentLoaded = true;

    _textEdit->document()->setModified(false);

    connect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
            this,                  SLOT(  contentsChange(int,int,int)));

    enableActions();

#ifdef QT_DEBUG_MODE
    previewViewerFileAct->setEnabled(true);
    emit lpub->messageSig(LOG_DEBUG,QString("5. %1").arg(message));
#endif
}

void EditWindow::contentLoaded()
{
    if (futureWatcher.future().result()) {
        _textEdit->document()->clear();
        _textEdit->document()->setModified(false);

        connect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
                this,                  SLOT(  contentsChange(int,int,int)));

        waitingSpinnerStop();

        emit lpub->messageSig(LOG_ERROR, QString("Editor load failed for %1").arg(fileName));

        return;
    }

    loadFinished();
}

void EditWindow::waitingSpinnerStart()
{
    if (!Preferences::modeGUI)
        return;

    if (_waitingSpinner) {
        if (_waitingSpinner->isSpinning())
            _waitingSpinner->stop();
    }

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
    _waitingSpinner->start();

#ifdef QT_DEBUG_MODE
    emit lpub->messageSig(LOG_DEBUG,QString("2. Waiting Spinner Started"));
#endif

    //QApplication::processEvents();
}

void EditWindow::waitingSpinnerStop()
{
  if (!Preferences::modeGUI)
      return;

  if (_waitingSpinner && _waitingSpinner->isSpinning()) {
    _waitingSpinner->stop();

#ifdef QT_DEBUG_MODE
    emit lpub->messageSig(LOG_DEBUG,QString("4. Waiting Spinner Stopped"));
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

void EditWindow::deleteSelection() {
    _textEdit->cut();
    _textEdit->textCursor().deleteChar();
}

void EditWindow::clearWindow() {
  _textEdit->document()->clear();
  _textEdit->document()->setModified(false);
  clearEditorHighlightLines();
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

/*
 *
 * Editor options section
 *
 */

void EditWindow::preferences()
{
    const QString windowTitle       = QString("Editor Preferences");
    int editorDecoration            = Preferences::editorDecoration;
    int editorLinesPerPage          = Preferences::editorLinesPerPage;
    int editorFontSize              = Preferences::editorFontSize;
    bool editorBufferedPaging       = Preferences::editorBufferedPaging;
    bool editorHighlightLines       = Preferences::editorHighlightLines;
    bool editorLoadSelectionStep    = Preferences::editorLoadSelectionStep;
    QString editorFont              = Preferences::editorFont;
    QString change;
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
    dialog->setWhatsThis(lpubWT(WT_DIALOG_EDITOR_PREFERENCES, windowTitle));
    QFormLayout *form = new QFormLayout(dialog);

    // options - editor font
    QGroupBox *editorFontGrpBox = new QGroupBox(tr("Editor Font"));
    form->addWidget(editorFontGrpBox);
    QFormLayout *editorFontSubform = new QFormLayout(editorFontGrpBox);

    QLabel    *editorFontLabel = new QLabel(tr("Family:"), dialog);
    QFontComboBox *editorFontCombo = new QFontComboBox(dialog);
    editorFontCombo->setCurrentFont(QFont(editorFont, editorFontSize));
    editorFontSubform->addRow(editorFontLabel, editorFontCombo);

    QLabel    *editorFontSizeLabel = new QLabel(tr("Size:"), dialog);
    QComboBox *editorFontSizeCombo = new QComboBox(dialog);
    foreach (int size, QFontDatabase::standardSizes())
        editorFontSizeCombo->addItem(QString().setNum(size));
    editorFontSizeCombo->setCurrentText(QString().setNum(editorFontSize));
    editorFontSubform->addRow(editorFontSizeLabel, editorFontSizeCombo);

    // options - editor decoration
    QGroupBox *editorDecorationGrpBox = new QGroupBox(tr("Editor Text Decoration"));
    form->addWidget(editorDecorationGrpBox);
    QFormLayout *editorDecorationSubform = new QFormLayout(editorDecorationGrpBox);

    QLabel    *editorDecorationLabel = new QLabel(tr("Text Decoration:"), dialog);
    QComboBox *editorDecorationCombo = new QComboBox(dialog);
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

        editorLoadSelectionStepBox = new QCheckBox(tr("Load Selection Step in Visual Editor"), dialog);
        editorLoadSelectionStepBox->setToolTip(tr("Load the first step (on multi-line select) of selected lines in the Visual Editor"));
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
        bool fontChanged = false;
        QSettings Settings;
        Preferences::editorFont           = editorFontCombo->currentFont().family();
        if (editorFont != Preferences::editorFont) {
            fontChanged |= true;
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"EditorFont"),Preferences::editorFont);
            emit lpub->messageSig(LOG_INFO,tr("LDraw editor text font changed to %1").arg(Preferences::editorFont));
        }
        Preferences::editorFontSize       = editorFontSizeCombo->currentText().toInt();
        if (editorFontSize != Preferences::editorFontSize) {
            fontChanged |= true;
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"EditorFontSize"),Preferences::editorFontSize);
            emit lpub->messageSig(LOG_INFO,tr("LDraw editor text font size changed to %1").arg(Preferences::editorFontSize));
        }
        Preferences::editorDecoration     = editorDecorationCombo->currentIndex();
        if (editorDecoration != Preferences::editorDecoration) {
            if (Preferences::editorDecoration == SIMPLE_DECORATION) {
                if (highlighterSimple)
                    highlighterSimple = nullptr;
                highlighterSimple = new HighlighterSimple(_textEdit->document());
                highlighterSimple->rehighlight();
            } else {
                if (highlighter)
                    highlighter = nullptr;
                highlighter = new Highlighter(_textEdit->document());
                highlighter->rehighlight();
            }
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorDecoration"),Preferences::editorDecoration);
            emit lpub->messageSig(LOG_INFO,tr("LDraw editor text decoration changed to %1").arg(Preferences::editorDecoration == SIMPLE_DECORATION ? "Simple" : "Standard"));
        }
        Preferences::editorBufferedPaging = editorBufferedPagingGrpBox->isChecked();
        if (editorBufferedPaging != Preferences::editorBufferedPaging) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorBufferedPaging"),Preferences::editorBufferedPaging);
            emit lpub->messageSig(LOG_INFO,QString("Editor buffered paging is %1").arg(Preferences::editorBufferedPaging ? "On" : "Off"));
        }
        Preferences::editorLinesPerPage   = editorLinesPerPageSpin->text().toInt();
        if (editorLinesPerPage != Preferences::editorLinesPerPage) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorLinesPerPage"),Preferences::editorLinesPerPage);
            emit lpub->messageSig(LOG_INFO,tr("Buffered lines par page changed from %1 to %2").arg(editorLinesPerPage).arg(Preferences::editorLinesPerPage));
        }

        if (! modelFileEdit()) {
            Preferences::editorHighlightLines   = editorHighlightLinesBox->isChecked();
            if (editorHighlightLines != Preferences::editorHighlightLines) {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorHighlightLines"),Preferences::editorHighlightLines);
                emit lpub->messageSig(LOG_INFO,tr("Highlight selected lines changed from %1 to %2").arg(editorHighlightLines).arg(Preferences::editorLinesPerPage));
            }

            Preferences::editorLoadSelectionStep   = editorLoadSelectionStepBox->isChecked();
            if (editorLoadSelectionStep != Preferences::editorLoadSelectionStep) {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorLoadSelectionStep"),Preferences::editorLoadSelectionStep);
                emit lpub->messageSig(LOG_INFO,tr("Load selection step in Visual Editor changed from %1 to %2").arg(editorLoadSelectionStep).arg(Preferences::editorLoadSelectionStep));
            }
        } // ! modelFileEdit()
        else {
            Preferences::editorPreviewOnDoubleClick = editorPreviewOnDoubleClickBox->isChecked();
            if (editorPreviewOnDoubleClick != Preferences::editorPreviewOnDoubleClick) {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"EditorPreviewOnDoubleClick"),Preferences::editorPreviewOnDoubleClick);
                emit lpub->messageSig(LOG_INFO,tr("Launch floating preview part double click changed from %1 to %2").arg(editorPreviewOnDoubleClick).arg(Preferences::editorPreviewOnDoubleClick));
            }
        } // modelFileEdit()

        if (fontChanged)
            _textEdit->setEditorFont();

        if (!change.isEmpty())
            showMessage(tr("%1 editor %2 change").arg(VER_PRODUCTNAME_STR).arg(change));
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
        emit lpub->messageSig(LOG_INFO_STATUS,tr("Loading buffered page %1 lines...")
                                   .arg(Preferences::editorLinesPerPage));

        loadPagedContent();

        emit lpub->messageSig(LOG_STATUS,QString());
        QApplication::processEvents();
    }
}

/* NOT USED FOR THE MOMENT */
void EditWindow::loadContentBlocks(const QStringList &content, bool firstBlock) {
    QElapsedTimer t; t.start();
    int lineCount     = content.size();
    int blockIndx     = 0;
    int linesPerBlock = 100;
    int blocks        = lineCount / linesPerBlock;
    int remain        = lineCount % linesPerBlock;
#ifdef QT_DEBUG_MODE
    int blockLineCount= 0;
#endif
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
#ifdef QT_DEBUG_MODE
            blockLineCount  = block.count("\n") + (firstBlock ? 2 : 1);
#endif
            if (firstBlock) {
                firstBlock = false;
                _textEdit->setPlainText(block);
            } else {
                _textEdit->appendPlainText(block);
            }
#ifdef QT_DEBUG_MODE
        emit lpub->messageSig(LOG_DEBUG,tr("Load content block %1, lines %2 - %3")
                                   .arg(i)
                                   .arg(blockLineCount)
                                   .arg(lpub->elapsedTime(t.elapsed())));
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
   emit lpub->messageSig(LOG_DEBUG,tr("Load paged content %1 lines - start...")
                              .arg(_pageContent.size()));
#endif

   bool initialLoad   = _textEdit->document()->isEmpty();
   int linesPerPage   = Preferences::editorLinesPerPage - (initialLoad ? 1 : 0);
   int nextIndx       = qMin(linesPerPage, (_pageContent.size() - _pageIndx) - 1);
   int maxPageIndx    = _pageIndx + nextIndx;
   const QString page = _pageContent.mid(_pageIndx, nextIndx).join('\n');
   int pageLineCount  = page.count("\n") + (initialLoad ? 2 : 1);

#ifdef QT_DEBUG_MODE
   emit lpub->messageSig(LOG_DEBUG,tr("Load page line count %1 - %2")
                              .arg(pageLineCount)
                              .arg(lpub->elapsedTime(t.elapsed())));
#endif

   const bool wasBlocked = _textEdit->blockSignals(true);

   verticalScrollBar->setMaximum(verticalScrollBar->maximum() + nextIndx);

//   loadContentBlocks(page.split('\n'), initialLoad);
// *
   if (initialLoad) {
       _textEdit->setPlainText(page);
#ifdef QT_DEBUG_MODE
   emit lpub->messageSig(LOG_DEBUG,tr("Load page set %1 plain text lines - %2")
                              .arg(pageLineCount)
                              .arg(lpub->elapsedTime(t.elapsed())));
#endif
   } else {
       _textEdit->appendPlainText(page);
#ifdef QT_DEBUG_MODE
   emit lpub->messageSig(LOG_DEBUG,QString("Load page append %1 text lines - %2")
                              .arg(pageLineCount)
                              .arg(lpub->elapsedTime(t.elapsed())));
#endif
   }
// */

   _contentLoaded = maxPageIndx >= _pageContent.size() - 1;

   emit lpub->messageSig(LOG_TRACE,tr("Load page of %1 lines from %2 to %3, content lines %4, final page: %5 - %6")
                              .arg(pageLineCount)
                              .arg(_pageIndx + 1)
                              .arg(maxPageIndx + 1)
                              .arg(_pageContent.size())
                              .arg(_contentLoaded ? "Yes" : "No")
                              .arg(lpub->elapsedTime(t.elapsed())));

   _pageIndx = maxPageIndx;

   _textEdit->blockSignals(wasBlocked);

   _contentLoading = false;
}

/*
 *
 * Text Editor section
 *
 */

TextEditor::TextEditor(bool detachedEdit, QWidget *parent) :
    QPlainTextEdit(parent),
    popUp(nullptr),
    ac(nullptr),
    sc(nullptr),
    lineNumberArea(new LineNumberArea(this)),
    detachedEdit(detachedEdit),
    showHardLinebreaks(false),
    _fileIsUTF8(false)
{
    QPalette lineNumberPalette = lineNumberArea->palette();
    lineNumberPalette.setCurrentColorGroup(QPalette::Active);
    lineNumberPalette.setColor(QPalette::Highlight,QColor(Qt::magenta));
    if (Preferences::displayTheme == THEME_DARK) {
        lineNumberPalette.setColor(QPalette::Text,QColor(Qt::darkGray).darker(150));
        lineNumberPalette.setColor(QPalette::Background,QColor(Preferences::themeColors[THEME_DARK_EDIT_MARGIN]));
    } else {
        lineNumberPalette.setColor(QPalette::Text,QColor(Qt::darkGray));
        lineNumberPalette.setColor(QPalette::Background,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]).lighter(130));
    }

    lineNumberArea->setPalette(lineNumberPalette);

    setEditorFont();

    setVerticalScrollBar(new ScrollBarFix(Qt::Vertical, this));

    connect(this, SIGNAL(blockCountChanged(int)),
            this, SLOT(  updateLineNumberAreaWidth(int)));

    connect(this, SIGNAL(updateRequest(QRect, int)),
            this, SLOT(  updateLineNumberArea(QRect, int)));

    updateLineNumberAreaWidth(0);

    QAction * actionComplete = new QAction(tr("Snippet Completer"), this);
    actionComplete->setObjectName("snippetCompleterAct.2");
    actionComplete->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Space));
    lpub->actions.insert(actionComplete->objectName(), Action("Edit.Snippet Completer", actionComplete));
    connect(actionComplete, SIGNAL(triggered()),
            this,           SLOT(  performCompletion()));
    this->addAction(actionComplete);
}

// Snippet commands
void TextEditor::setSnippetCompleter(SnippetCompleter *completer)
{
    if (sc)
        sc->disconnect(this);

    sc = completer;

    if (!sc)
        return;

    connect(completer,  SIGNAL(snippetSelected(QString,QString, int)),
            this,       SLOT(  insertSnippet(QString,QString, int)));
}

void TextEditor::performCompletion()
{
    if (!sc)
        return;

   QRect popupRect = cursorRect();
   popupRect.setLeft(popupRect.left() + lineNumberAreaWidth());

   QStringList words = extractDistinctWordsFromDocument();
   sc->performCompletion(retrieveTextUnderCursor(), words, popupRect);
}

void TextEditor::insertSnippet(const QString &completionPrefix, const QString &completion, int newCursorPos)
{
   QTextCursor cursor = this->textCursor();

   // select the completion prefix
   cursor.clearSelection();
   cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, completionPrefix.length());

   int pos = cursor.position();

   // replace completion prefix with snippet
   cursor.insertText(completion);

   // move cursor to requested position
   cursor.setPosition(pos);
   cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, newCursorPos);

   this->setTextCursor(cursor);
}

QStringList TextEditor::extractDistinctWordsFromDocument() const
{
    QStringList allWords = retrieveAllWordsFromDocument();
    allWords.removeDuplicates();

    QStringList words = filterWordList(allWords, [](const QString &word){
        return word.length() > EDITOR_FILTER_MIN_WORD_LENGTH;
    });
    words.sort(Qt::CaseInsensitive);

    return words;
}

QStringList TextEditor::retrieveAllWordsFromDocument() const
{
    return toPlainText().split(QRegExp("\\W+"), QString::SkipEmptyParts);
}

template <class UnaryPredicate>
QStringList TextEditor::filterWordList(const QStringList &words, UnaryPredicate predicate) const
{
    QStringList filteredWordList;

    foreach (const QString &word, words) {
        if (predicate(word))
        {
           filteredWordList << word;
        }
    }

    return filteredWordList;
}

QString TextEditor::retrieveTextUnderCursor() const
{
   QTextCursor cursor = this->textCursor();
   QTextDocument *document = this->document();

   // empty text if cursor at start of line
   if (cursor.atBlockStart()) {
       return QString();
   }

   cursor.clearSelection();

   // move left until we find a space or reach the start of line
   while(!document->characterAt(cursor.position()-1).isSpace() && !cursor.atBlockStart()) {
       cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
   }

   return cursor.selectedText();
}

// Auto complete
void TextEditor::setAutoCompleter(QCompleter *completer)
{
    if (ac)
        ac->disconnect(this);

    ac = completer;

    if (!ac)
        return;

    ac->setWidget(this);
    ac->setCompletionMode(QCompleter::PopupCompletion);
    ac->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    ac->setCaseSensitivity(Qt::CaseInsensitive);
    ac->setWrapAround(false);
    QObject::connect(ac,   SIGNAL(activated(const QString&)),
                     this, SLOT(  insertCompletion(const QString&)));
}

void TextEditor::keyPressEvent(QKeyEvent *e)
{
    // these will only be triggered if the undo/redo buttons are visible
    // otherwise, the MainWindow will trap the key sequence
    if (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_Z)
        emit undoKeySequence();
    else if (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_Y)
        emit redoKeySequence();

    if ((ac && ac->popup()->isVisible()) || (sc && sc->isPopupVisible())) {
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

    const bool isSnippetShortcut = (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_Space); // CTRL+Space
    if (isSnippetShortcut) { // do not popup the autocompleter when we have a snippet completer command
      if (sc)
          sc->hidePopup();
      QPlainTextEdit::keyPressEvent(e);
      return;
    }

    const bool isShortcut = (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (!ac || !isShortcut) // do not process the shortcut when we have an auto completer
        QPlainTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers().testFlag(Qt::ControlModifier) ||
                             e->modifiers().testFlag(Qt::ShiftModifier);
    if (!ac || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty() || completionPrefix.length() < 3
                      || eow.contains(e->text().right(1)))) {
        ac->popup()->hide();
        return;
    }

    if (completionPrefix != ac->completionPrefix()) {
        ac->setCompletionPrefix(completionPrefix);
        ac->popup()->setCurrentIndex(ac->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(ac->popup()->sizeHintForColumn(0)
                + ac->popup()->verticalScrollBar()->sizeHint().width());

    ac->complete(cr); // popup it up!
}

void TextEditor::focusInEvent(QFocusEvent *e)
{
    if (ac)
        ac->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void TextEditor::paintEvent(QPaintEvent *e)
{
    QPlainTextEdit::paintEvent(e);

    // draw line end markers if enabled
    if (showHardLinebreaks) {
        drawLineEndMarker(e);
    }
}

void TextEditor::drawLineEndMarker(QPaintEvent *e)
{
    QPainter painter(viewport());

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int leftMargin = qRound(fontMetrics().horizontalAdvance(" ") / 2.0);
    int lineEndCharWidth = fontMetrics().horizontalAdvance("\u00B6");
#else
    int leftMargin = qRound(fontMetrics().width(" ") / 2.0);
    int lineEndCharWidth = fontMetrics().width("\u00B6");
#endif

    int fontHeight = fontMetrics().height();

    QColor markerColor;
    if (Preferences::displayTheme == THEME_DARK)
        markerColor = QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_HEADER_VALUE]);
    else
        markerColor = QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_HEADER_VALUE]);

    QTextBlock block = firstVisibleBlock();
    while (block.isValid()) {
        QRectF blockGeometry = blockBoundingGeometry(block).translated(contentOffset());
        if (blockGeometry.top() > e->rect().bottom())
            break;

        if (block.isVisible() && blockGeometry.toRect().intersects(e->rect())) {
            QString text = block.text();
            if (text.endsWith("  ")) {
                painter.setPen(markerColor);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
                painter.drawText(blockGeometry.left() + fontMetrics().horizontalAdvance(text) + leftMargin,
                                 blockGeometry.top(),
                                 lineEndCharWidth,
                                 fontHeight,
                                 Qt::AlignLeft | Qt::AlignVCenter,
                                 "\u00B6");
#else
                painter.drawText(blockGeometry.left() + fontMetrics().width(text) + leftMargin,
                                 blockGeometry.top(),
                                 lineEndCharWidth,
                                 fontHeight,
                                 Qt::AlignLeft | Qt::AlignVCenter,
                                 "\u00B6");
#endif
            }
        }

        block = block.next();
    }
}

void TextEditor::insertCompletion(const QString &completion)
{
    if (ac->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - ac->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString TextEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void TextEditor::gotoLine(int line)
{
    QTextCursor cursor(document()->findBlockByNumber(line-1));
    this->setTextCursor(cursor);
}

void TextEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
    if (modelFileEdit()) {
        if ( event->button() == Qt::LeftButton && Preferences::editorPreviewOnDoubleClick) {
            emit triggerPreviewLine();
        }
    }
    return;
}

void TextEditor::mouseReleaseEvent(QMouseEvent *event)
{
  QWidget::mouseReleaseEvent(event);
  if (event->button() == Qt::LeftButton) {
     emit highlightSelectedParts();
  }
}

void TextEditor::toggleComment(){

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

void TextEditor::showAllCharacters(bool enabled) {
    showHardLinebreaks = enabled;

    QTextOption textOption = document()->defaultTextOption();
    QTextOption::Flags optionFlags = textOption.flags();
    if (enabled) {
        optionFlags |= QTextOption::ShowLineAndParagraphSeparators;
        optionFlags |= QTextOption::ShowTabsAndSpaces;
    } else {
        optionFlags &= ~QTextOption::ShowLineAndParagraphSeparators;
        optionFlags &= ~QTextOption::ShowTabsAndSpaces;
    }
    textOption.setFlags(optionFlags);

    document()->setDefaultTextOption(textOption);

    // repaint
    viewport()->update();
}

int TextEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    QFont font = lineNumberArea->font();
    const QFontMetrics linefmt(font);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int space = 10 + linefmt.horizontalAdvance(QLatin1Char('9')) * digits;
#else
    int space = 10 + linefmt.width(QLatin1Char('9')) * digits;
#endif
    return space;
}

void TextEditor::setEditorFont()
{
    const int editorFontSize = Preferences::editorFontSize;
    const QString editorFont = Preferences::editorFont;
    QFont font(editorFont, editorFontSize);
    font.setStyleHint(QFont::System);

    lineNumberArea->setFont(font);
    setFont(font);
}

void TextEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void TextEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = this->contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);

    int selStart = textCursor().selectionStart();
    int selEnd = textCursor().selectionEnd();

    QPalette palette = lineNumberArea->palette();

    painter.fillRect(event->rect(), palette.color(QPalette::Background));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    qreal top = blockBoundingGeometry(block).translated(contentOffset()).top();
    qreal bottom = top;

    while (block.isValid() && top <= event->rect().bottom()) {
        top = bottom;

        const qreal height = blockBoundingRect(block).height();
        bottom = top + height;

        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.setPen(palette.color(QPalette::Text));

            bool selected = (
                                (selStart < block.position() + block.length() && selEnd > block.position())
                                || (selStart == selEnd && selStart == block.position())
                            );

            if (selected) {
                painter.save();
                painter.setPen(palette.color(QPalette::Highlight));
            }

            const QString number = QString::number(blockNumber + 1);
            painter.drawText(0, top, lineNumberArea->width() - 4, height, Qt::AlignRight, number);

            if (selected)
                painter.restore();
        }

        block = block.next();
        ++blockNumber;
    }
}

void TextEditor::showCharacters(
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

void TextEditor::findDialog()
{
    QTextCursor cursor = textCursor();
    QString selection = cursor.selectedText();
    if (selection.isEmpty()) {
        cursor.select(QTextCursor::WordUnderCursor);
        selection = cursor.selectedText();
    }
    popUp = new FindReplace(this, selection);
    popUp->show();
}
