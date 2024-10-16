/****************************************************************************
**
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

/****************************************************************************
 *
 * The editwindow is used to display all editable parameter files
 * and logs to the user.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QtWidgets>
#include <QDesktopWidget>

#include "parmswindow.h"
#include "findreplace.h"
#include "parmshighlighter.h"
#include "messageboxresizable.h"

#include "lpub.h"
#include "declarations.h"
#include "version.h"
#include "lpub_preferences.h"

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

ParmsWindow *parmsWindow;

ParmsWindow::ParmsWindow(QWidget *parent) :
  QMainWindow(parent)
{
    _textEdit   = new ParmEditor(this);

    highlighter = new ParmsHighlighter(_textEdit->document());

    setSelectionHighlighter();

    _textEdit->setLineWrapMode(ParmEditor::NoWrap);
    _textEdit->setUndoRedoEnabled(true);
    _textEdit->setContextMenuPolicy(Qt::CustomContextMenu);

    _textEdit->popUp = nullptr;
    _fadeStepFile      = false;
    _fileModified      = false;
    _restartRequired   = true;

    createActions();
    createToolBars();

    connect(_textEdit, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));

    setCentralWidget(_textEdit);

    readSettings();

    statusBar()->show();

    parmsWindow  = this;
}

void ParmsWindow::gotoLine()
{
    const int STEP = 1;
    const int MIN_VALUE = 1;

    QTextCursor cursor = _textEdit->textCursor();
    int currentLine = cursor.blockNumber()+1;
    int maxValue = _textEdit->document()->blockCount();

    bool ok;
    int line = QInputDialog::getInt(this, tr("Go to..."),
                                          tr("Line: ", "Line number in the parameter editor"), currentLine, MIN_VALUE, maxValue, STEP, &ok);
    if (!ok) return;
    _textEdit->gotoLine(line);
}

void ParmsWindow::setSelectionHighlighter()
{
    QColor highlightColor;
    if (Preferences::displayTheme == THEME_DEFAULT)
        highlightColor = QColor(LPUB3D_DEFAULT_COLOUR);
    else if (Preferences::darkTheme)
        highlightColor = QColor(Preferences::themeColors[THEME_DARK_PALETTE_HILIGHT_TEXT]);
    highlightColor.setAlpha(30);

    auto palette = _textEdit->palette();
    palette.setBrush(QPalette::Highlight, highlightColor);
    palette.setBrush(QPalette::HighlightedText, QBrush(Qt::NoBrush));

    _textEdit->setPalette(palette);
}

void ParmsWindow::createActions()
{
    exitAct = new QAction(QIcon(":/resources/exit.png"),tr("E&xit"), this);
    exitAct->setObjectName("exitAct.5");
    exitAct->setShortcut(QStringLiteral("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit this window"));
    lpub->actions.insert(exitAct->objectName(), Action(QStringLiteral("Edit.Exit"), exitAct));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    openAct = new QAction(QIcon(":/resources/open.png"), tr("&Open File"), this);
    openAct->setObjectName("openAct.5");
    openAct->setShortcut(QStringLiteral("Ctrl+O"));
    openAct->setStatusTip(tr("Open stdout or stderr log file for loaded model"));
    lpub->actions.insert(openAct->objectName(), Action(QStringLiteral("Edit.Open File"), openAct));
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));

    refreshAct = new QAction(QIcon(":/resources/redraw.png"), tr("&Refresh File"), this);
    refreshAct->setObjectName("refreshAct.5");
    refreshAct->setShortcut(QStringLiteral("Ctrl+Alt+O"));
    refreshAct->setStatusTip(tr("Reload the current file to see updated content"));
    lpub->actions.insert(refreshAct->objectName(), Action(QStringLiteral("Edit.Refresh File"), refreshAct));
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refreshFile()));

    cutAct = new QAction(QIcon(":/resources/cut.png"), tr("Cu&t"), this);
    cutAct->setObjectName("cutAct.5");
    cutAct->setShortcut(QStringLiteral("Ctrl+X"));
    cutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    lpub->actions.insert(cutAct->objectName(), Action(QStringLiteral("Edit.Cut"), cutAct));
    connect(cutAct, SIGNAL(triggered()), _textEdit, SLOT(cut()));

    copyAct = new QAction(QIcon(":/resources/copy.png"), tr("&Copy"), this);
    copyAct->setObjectName("copyAct.5");
    copyAct->setShortcut(QStringLiteral("Ctrl+C"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    lpub->actions.insert(copyAct->objectName(), Action(QStringLiteral("Edit.Copy"), copyAct));
    connect(copyAct, SIGNAL(triggered()), _textEdit, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/resources/paste.png"), tr("&Paste"), this);
    pasteAct->setObjectName("pasteAct.5");
    pasteAct->setShortcut(QStringLiteral("Ctrl+V"));
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    lpub->actions.insert(pasteAct->objectName(), Action(QStringLiteral("Edit.Paste"), pasteAct));
    connect(pasteAct, SIGNAL(triggered()), _textEdit, SLOT(paste()));

    findAct = new QAction(QIcon(":/resources/find.png"), tr("&Find"), this);
    findAct->setObjectName("findAct.5");
    findAct->setShortcut(QStringLiteral("Ctrl+F"));
    findAct->setStatusTip(tr("Find object"));
    lpub->actions.insert(findAct->objectName(), Action(QStringLiteral("Edit.Find"), findAct));
    connect(findAct, SIGNAL(triggered()), _textEdit, SLOT(findDialog()));

    gotoLineAct = new QAction(QIcon(":/resources/gotoline.png"), tr("&Go to Line"), this);
    gotoLineAct->setObjectName("gotoLineAct.5");
    gotoLineAct->setShortcut(QStringLiteral("Ctrl+G"));
    gotoLineAct->setStatusTip(tr("Go to line..."));
    lpub->actions.insert(gotoLineAct->objectName(), Action(QStringLiteral("Edit.Go To Line"), gotoLineAct));
    connect(gotoLineAct, SIGNAL(triggered()), this, SLOT(gotoLine()));

    saveAct = new QAction(QIcon(":/resources/save.png"), tr("&Save"), this);
    saveAct->setObjectName("saveAct.5");
    saveAct->setShortcut(QStringLiteral("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    lpub->actions.insert(saveAct->objectName(), Action(QStringLiteral("Edit.Save"), saveAct));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));

    saveCopyAsAct = new QAction(QIcon(":/resources/saveas.png"), tr("Save Copy &As"), this);
    saveCopyAsAct->setObjectName("saveCopyAsAct.5");
    saveCopyAsAct->setShortcut(QStringLiteral("Ctrl+Alt+S"));
    saveCopyAsAct->setStatusTip(tr("Save a copy of the document as... to disk"));
    lpub->actions.insert(saveCopyAsAct->objectName(), Action(QStringLiteral("Edit.Save Copy As"), saveCopyAsAct));
    connect(saveCopyAsAct, SIGNAL(triggered()), this, SLOT(saveCopyAsFile()));

    topAct = new QAction(QIcon(":/resources/topofdocument.png"), tr("Top of Document"), this);
    topAct->setObjectName("topAct.5");
    topAct->setShortcut(QStringLiteral("Ctrl+T"));
    topAct->setStatusTip(tr("Go to the top of document"));
    lpub->actions.insert(topAct->objectName(), Action(QStringLiteral("File.Top Of Document"), topAct));
    connect(topAct, SIGNAL(triggered()), this, SLOT(topOfDocument()));

    bottomAct = new QAction(QIcon(":/resources/bottomofdocument.png"), tr("Bottom of Document"), this);
    bottomAct->setObjectName("bottomAct.5");
    bottomAct->setShortcut(QStringLiteral("Ctrl+B"));
    bottomAct->setStatusTip(tr("Go to the bottom of document"));
    lpub->actions.insert(bottomAct->objectName(), Action(QStringLiteral("File.Bottom Of Document"), bottomAct));
    connect(bottomAct, SIGNAL(triggered()), this, SLOT(bottomOfDocument()));

    delAct = new QAction(QIcon(":/resources/delete.png"), tr("&Delete"), this);
    delAct->setObjectName("delAct.5");
    delAct->setShortcut(QStringLiteral("DEL"));
    delAct->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    delAct->setStatusTip(tr("Delete the selection"));
    lpub->actions.insert(delAct->objectName(), Action(QStringLiteral("File.Delete"), delAct));
    connect(delAct, SIGNAL(triggered()), _textEdit, SLOT(cut()));

    QIcon systemEditorIcon;
    systemEditorIcon.addFile(":/resources/systemeditor.png");
    systemEditorIcon.addFile(":/resources/systemeditor16.png");
    systemEditorAct = new QAction(systemEditorIcon, tr("Open With System Editor"), this);
    systemEditorAct->setObjectName("systemEditorAct.5");
    systemEditorAct->setStatusTip(tr("Open this file with the system editor"));
    lpub->actions.insert(systemEditorAct->objectName(), Action(QStringLiteral("Edit.Open With System Editor"), systemEditorAct));
    connect(systemEditorAct, SIGNAL(triggered()), this, SLOT(systemEditor()));

    selAllAct = new QAction(QIcon(":/resources/selectall.png"), tr("&Select All"), this);
    selAllAct->setObjectName("selAllAct.5");
    selAllAct->setShortcut(QStringLiteral("Ctrl+A"));
    selAllAct->setStatusTip(tr("Select all page content"));
    lpub->actions.insert(selAllAct->objectName(), Action(QStringLiteral("File.Select All"), selAllAct));
    connect(selAllAct, SIGNAL(triggered()), _textEdit, SLOT(selectAll()));

    showAllCharsAct = new QAction(QIcon(":/resources/showallcharacters.png"), tr("Show All Characters"), this);
    showAllCharsAct->setObjectName("showAllCharsAct.5");
    showAllCharsAct->setShortcut(QStringLiteral("Ctrl+J"));
    showAllCharsAct->setStatusTip(tr("Show all characters"));
    showAllCharsAct->setCheckable(true);
    lpub->actions.insert(showAllCharsAct->objectName(), Action(QStringLiteral("File.Show All Characters"), showAllCharsAct));
    connect(showAllCharsAct, SIGNAL(triggered()), this, SLOT(showAllCharacters()));

    undoAct = new QAction(QIcon(":/resources/editundo.png"), tr("Undo"), this);
    undoAct->setObjectName("undoAct.5");
    undoAct->setShortcut(QStringLiteral("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last change"));
    undoAct->setEnabled(false);
    lpub->actions.insert(undoAct->objectName(), Action(QStringLiteral("Edit.Undo"), undoAct));
    connect(undoAct, SIGNAL(triggered()), _textEdit, SLOT(undo()));

    redoAct = new QAction(QIcon(":/resources/editredo.png"), tr("Redo"), this);
    redoAct->setObjectName("redoAct.5");
#ifdef __APPLE__
    redoAct->setShortcut(QStringLiteral("Ctrl+Shift+Z"));
    redoAct->setStatusTip(tr("Redo last change"));
#else
    redoAct->setShortcut(QStringLiteral("Ctrl+Y"));
    redoAct->setStatusTip(tr("Redo last change"));
#endif
    redoAct->setEnabled(false);
    lpub->actions.insert(redoAct->objectName(), Action(QStringLiteral("Edit.Redo"), redoAct));
    connect(redoAct, SIGNAL(triggered()), _textEdit, SLOT(redo()));

    openAct->setVisible(false);

    refreshAct->setVisible(false);
    saveAct->setEnabled(false);
    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    delAct->setEnabled(false);
    systemEditorAct->setEnabled(false);
    selAllAct->setEnabled(false);
    findAct->setEnabled(false);
    gotoLineAct->setEnabled(false);
    topAct->setEnabled(false);
    bottomAct->setEnabled(false);
    showAllCharsAct->setEnabled(false);

    connect(_textEdit, SIGNAL(copyAvailable(bool)),
            cutAct,    SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(copyAvailable(bool)),
            copyAct,   SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(copyAvailable(bool)),
             delAct,   SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(undoAvailable(bool)),
             undoAct,  SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(redoAvailable(bool)),
             redoAct,  SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(textChanged()),
             this,     SLOT(enableSave()));
}

void ParmsWindow::createToolBars()
{
    editToolBar = addToolBar(tr("Edit"));
    editToolBar->setObjectName("editToolbar");
    editToolBar->addAction(exitAct);
    editToolBar->addSeparator();
    editToolBar->addAction(topAct);
    editToolBar->addAction(bottomAct);
    editToolBar->addSeparator();
    editToolBar->addAction(openAct);
    editToolBar->addAction(saveAct);
    editToolBar->addAction(saveCopyAsAct);
    editToolBar->addAction(systemEditorAct);
    editToolBar->addAction(selAllAct);
    editToolBar->addAction(showAllCharsAct);
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(findAct);
    editToolBar->addAction(gotoLineAct);
    editToolBar->addAction(refreshAct);
    editToolBar->addAction(delAct);

    undoRedoToolBar = addToolBar(tr("Undo Redo"));
    undoRedoToolBar->setObjectName("undoRedoToolbar");
    undoRedoToolBar->addAction(undoAct);
    undoRedoToolBar->addAction(redoAct);
}

void ParmsWindow::showContextMenu(const QPoint &pt)
{
    QMenu *menu = _textEdit->createStandardContextMenu();
    menu->addSeparator();
    menu->addAction(topAct);
    menu->addAction(bottomAct);
    menu->addAction(findAct);
    menu->addAction(gotoLineAct);
    menu->addSeparator();
    menu->addAction(systemEditorAct);
    menu->exec(_textEdit->mapToGlobal(pt));
    delete menu;
}

void ParmsWindow::systemEditor()
{
    QRegExp ldrawExtRx("\\.mpd$|\\.ldr$|\\.dat$|\\.lst", Qt::CaseInsensitive);
    bool hasLDrawExtension = fileName.contains(ldrawExtRx);
    if (hasLDrawExtension && !Preferences::systemEditor.isEmpty()) {
        QString const program = Preferences::systemEditor;
        QStringList arguments = QStringList() << fileName;
        if (Preferences::usingNPP)
            arguments << QLatin1String("-udl=LPUB3D");
        qint64 pid;
        QString workingDirectory = QDir::currentPath() + QDir::separator();
        QProcess::startDetached(Preferences::systemEditor, arguments, workingDirectory, &pid);
        emit lpub->messageSig(LOG_INFO, tr("Launched %1 with pid=%2 %3%4...")
                              .arg(QFileInfo(fileName).fileName()).arg(pid)
                              .arg(QFileInfo(program).fileName())
                              .arg(arguments.size() ? " "+arguments.join(" ") : ""));
    } else {
        QDesktopServices::openUrl(QUrl("file:///"+fileName, QUrl::TolerantMode));
    }
}

void ParmsWindow::displayParmsFile(
  const QString &_fileName)
{
    // Automatically hide open file action - show for logs only
    if (openAct->isVisible())
      openAct->setVisible(false);

    bool showAllCharsAction = false;

    fileName = _fileName;

    QFile file(fileName);

    // Use QFileInfo to get the base-name and extension
    QFileInfo fileInfo(file.fileName());

    if (fileInfo.fileName() == Preferences::validPliSubstituteParts)
      title = "PLI/BOM Substitute Parts";
    else if (fileInfo.fileName() == Preferences::validLDrawColorParts)
      {
        title = "Fade Steps Color Parts";
        _fadeStepFile = true;
      }
    else if (fileInfo.fileName() == VER_TITLE_ANNOTATIONS_FILE)
      title = "Title Annotation";
    else if (fileInfo.fileName() == VER_EXCLUDED_PARTS_FILE)
      title = "Excluded Parts";
    else if (fileInfo.fileName() == VER_STICKER_PARTS_FILE)
      title = "Sticker Parts";
    else if (fileInfo.fileName() == VER_FREEFOM_ANNOTATIONS_FILE)
      title = "Freeform";
    else if (fileInfo.fileName() == VER_LDVIEW_INI_FILE) {
      title = "LDView INI Settings";
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == VER_LDVIEW_POV_INI_FILE) {
      title = "LDView POV File Generation ini";
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == VER_POVRAY_INI_FILE) {
      title = "Raytracer (POV-Ray) ini";
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == VER_POVRAY_CONF_FILE) {
      title = "Raytracer (POV-Ray) conf";
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == VER_LDGLITE_INI_FILE) {
      title = "LDGLite ini";
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == VER_NATIVE_EXPORT_INI_FILE) {
      title = "Native Export ini";
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == QString("%1Log.txt").arg(VER_PRODUCTNAME_STR)) {
      title = QString("%1 Log").arg(VER_PRODUCTNAME_STR);
      viewLogWindowSettings();
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == VER_LPUB3D_LD2BLCODESXREF_FILE) {
        title = "LDraw to Bricklink Desgn ID cross-reference";
        showAllCharsAction = true;
        _restartRequired = true;
    }
    else if (fileInfo.fileName() == Preferences::validAnnotationStyles) {
        title = "Part Annotation Style reference";
        showAllCharsAction = true;
        _restartRequired = true;
    }
    else if (fileInfo.fileName() == VER_LPUB3D_LD2BLCOLORSXREF_FILE) {
        title = "LDraw to Bricklink Color Code cross-reference";
        showAllCharsAction = true;
        _restartRequired = true;
    }
    else if (fileInfo.fileName() == VER_LPUB3D_BLCOLORS_FILE) {
        title = "Bricklink Color ID reference";
        showAllCharsAction = true;
        _restartRequired = true;
    }
    else if (fileInfo.fileName() == "stderr-povray") {
      title = "Standard error - Raytracer (POV-Ray)";
      viewLogWindowSettings();
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == "stdout-povray") {
      title = "Standard output - Raytracer (POV-Ray)";
      viewLogWindowSettings();
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == "stderr-ldglite") {
      title = "Standard error - LDGlite";
      viewLogWindowSettings();
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == "stdout-ldglite") {
      title = "Standard output - LDGLite";
      viewLogWindowSettings();
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == "stderr-ldviewpov") {
      title = "Standard error - LDView POV Generation";
      viewLogWindowSettings();
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == "stdout-ldviewpov") {
      title = "Standard output - LDView POV Generation";
      viewLogWindowSettings();
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == "stderr-ldview") {
      title = "Standard error - LDView";
      viewLogWindowSettings();
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == "stdout-ldview") {
      title = "Standard output - LDView";
      viewLogWindowSettings();
      _restartRequired = false;
    }
    else {
      title = fileInfo.fileName();
      _restartRequired = false;
    }

    if (! file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr,
                 QMessageBox::tr("Parameter Editor"),
                 QMessageBox::tr("Cannot read parameter file %1:\n%2.")
                 .arg(fileName)
                 .arg(file.errorString()));

        _textEdit->document()->clear();
        return;
    }

    // check file encoding
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    bool isUTF8 = LDrawFile::_currFileIsUTF8;
    _textEdit->setIsUTF8(isUTF8);

    QTextStream in(&file);
    in.setCodec(_textEdit->getIsUTF8() ? codec : QTextCodec::codecForName("System"));

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    _textEdit->blockSignals(true);
    _textEdit->setPlainText(in.readAll());
    _textEdit->blockSignals(false);
    _textEdit->document()->setModified(false);

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    refreshAct->setVisible(true);
    selAllAct->setEnabled(true);
    findAct->setEnabled(true);
    topAct->setEnabled(true);
    bottomAct->setEnabled(true);
    gotoLineAct->setEnabled(true);
    systemEditorAct->setEnabled(true);
    showAllCharsAct->setEnabled(true);
    if (showAllCharsAction)
        showAllCharsAct->setVisible(true);
    else
        showAllCharsAct->setVisible(false);

    saveAct->setStatusTip(tr("Save %1 to disc").arg(fileInfo.absoluteFilePath()));
    systemEditorAct->setStatusTip(tr("Open %1 with the system editor").arg(fileInfo.absoluteFilePath()));

    statusBar()->showMessage(tr("File %1 loaded").arg(file.fileName()), 2000);
}

bool ParmsWindow::maybeSave()
{
  bool rc = true;

  if (_textEdit->document()->isModified()) {
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
    box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
    box.setDefaultButton   (QMessageBox::Yes);

    if (box.exec() == QMessageBox::Yes) {
      rc = saveFile();
    }
  }
  return rc;
}

bool ParmsWindow::saveFile(bool force)
{
    bool rc = false;
    // check for dirty editor
    if (_textEdit->document()->isModified() || force)
    {
        QFile file(fileName);
        if (! file.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(nullptr,
                                 QMessageBox::tr("%1 Editor")
                                 .arg(title),
                                 QMessageBox::tr("Cannot write file %1:\n%2.")
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

        if (rc) {
            if (!force) {
              saveAct->setEnabled(false);
              _textEdit->document()->setModified(false);
              _fileModified = true;
            }
            statusBar()->showMessage(tr("File %1 saved").arg(fileName), 2000);
        }

        if (showAllCharsAct->isChecked()) {
            _textEdit->showAllCharacters(true);
            _textEdit->blockSignals(false);
        }
    }

  return rc;
}

bool ParmsWindow::saveCopyAsFile()
{
    bool rc = false;
    // provide a file name
    QFileInfo fileInfo(fileName);
    QString saveCopyAsName = QString("%1/%2_%3.txt").arg(fileInfo.absolutePath()).arg(fileInfo.completeBaseName()).arg(QDateTime::currentDateTime().toString(QLatin1String("yyyyMMdd-hhmmss")));
    QString filter(QFileDialog::tr("All Files (*.*)"));
    QString saveCopyAsAbsoluteFilePath = QFileDialog::getSaveFileName(
                                                       nullptr,
                                                       QFileDialog::tr("Save %1 log").arg(VER_PRODUCTNAME_STR),
                                                       QDir::toNativeSeparators(saveCopyAsName),
                                                       filter);
    if (saveCopyAsAbsoluteFilePath.isEmpty())
      return rc;

    fileName = saveCopyAsAbsoluteFilePath;
    rc = saveFile(true /*force*/);
    fileName = fileInfo.absoluteFilePath();
    return rc;
}

void ParmsWindow::enableSave()
{
  if (_textEdit->document()->isModified())
    {
      saveAct->setEnabled(true);
    }
}

void ParmsWindow::openFile()
{
    QFileInfo fileInfo(gui->getCurFile());
    qDebug() << fileInfo.absoluteFilePath();
    QString filter(QFileDialog::tr("log (LPub3DLog*);;stderr (stderr-*);;stdout (stdout-*)"));
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(nullptr,
                           QFileDialog::tr("Select log, stderr or stdout file"),
                           fileInfo.absolutePath(),
                           filter);

        if (!fileName.isEmpty())
            displayParmsFile(fileName);
    }
}

void ParmsWindow::refreshFile() {
  QFileInfo fileInfo(fileName);
  if (fileInfo.exists())
    displayParmsFile(fileName);
  _textEdit->moveCursor(QTextCursor::End);
}

void ParmsWindow::toggleClear() {
  if (_textEdit->document()->isModified())
    {
      delAct->setEnabled(true);
    } else {
      delAct->setEnabled(false);
    }
}

void ParmsWindow::topOfDocument() {
    _textEdit->moveCursor(QTextCursor::Start);
}

void ParmsWindow::bottomOfDocument() {
    _textEdit->moveCursor(QTextCursor::End);
}

void ParmsWindow::showAllCharacters() {
    _textEdit->blockSignals(true);
    _textEdit->showAllCharacters(showAllCharsAct->isChecked());
    _textEdit->blockSignals(false);
    _textEdit->document()->setModified(false);
}

void ParmsWindow::viewLogWindowSettings() {
  // customize the menu for logging

  if (! openAct->isVisible())
    openAct->setVisible(true);

  disconnect(delAct, SIGNAL(triggered()),
             _textEdit,        SLOT(cut()));
  disconnect(_textEdit,        SIGNAL(copyAvailable(bool)),
             delAct, SLOT(setEnabled(bool)));
  connect(delAct,    SIGNAL(triggered()),
          _textEdit,           SLOT(clear()));
  connect(_textEdit,           SIGNAL(textChanged()),
           this,               SLOT(toggleClear()));
  delAct->setEnabled(true);
  delAct->setIconText(tr("&Clear"));
  delAct->setStatusTip(tr("Clear the %1 log. This action cannot be undone.").arg(VER_PRODUCTNAME_STR));
  pasteAct->setVisible(false);
}

void ParmsWindow::closeEvent(QCloseEvent *event)
{
  writeSettings();

  bool acceptEvent = false;

  if (maybeSave()) {
      acceptEvent = true;
    }

  if (_fileModified) {

      // load LDGLite settings if modified
      QFileInfo fileInfo(fileName);
      if ((Render::getRenderer() == RENDERER_LDGLITE) && (fileInfo.fileName() == QString(VER_LDGLITE_INI_FILE)))
          Preferences::setLDGLiteIniParams();

      // is there anything loaded - to take advantage of our change?
      bool fileLoaded = !gui->getCurFile().isEmpty();
      if ((fileLoaded || _fadeStepFile) && _restartRequired) {

          QMessageBox box;
          box.setIcon (QMessageBox::Question);
          box.setDefaultButton   (QMessageBox::Ok);
          box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);
          box.setText (tr("You must close and restart %1\nfor changes to take effect.")
                       .arg(QString::fromLatin1(VER_PRODUCTNAME_STR)));
          box.setInformativeText (tr (fileLoaded ? "Click \"OK\" to close and restart %1" : "Click \"OK\" to close %1")
                                  .arg(QString::fromLatin1(VER_PRODUCTNAME_STR)));

          if (box.exec() == QMessageBox::Ok) {
              QStringList args = QApplication::arguments();
              if (fileLoaded) {
                  if (!args.contains(gui->getCurFile(),Qt::CaseInsensitive))
                      args << QString("%1").arg(gui->getCurFile());
                  QSettings Settings;
                  Settings.setValue(QString("%1/%2").arg(DEFAULTS,SAVE_DISPLAY_PAGE_NUM_KEY),Gui::displayPageNum);
                } else {
                  args << QString();
                }
              args.removeFirst();
              QProcess::startDetached(QApplication::applicationFilePath(), args);

              logDebug() << tr("Restarted %1 using:").arg(VER_PRODUCTNAME_STR) << QApplication::applicationFilePath() << ", args:" << args;

              event->accept();
              QCoreApplication::quit();
            }
        }
    }

  if (acceptEvent) {
      event->accept();
    } else {
      event->ignore();
    }
}

void ParmsWindow::readSettings()
{
    QSettings Settings;
    Settings.beginGroup(PARMSWINDOW);
    restoreGeometry(Settings.value("Geometry").toByteArray());
    restoreState(Settings.value("State").toByteArray());
    QSize size = Settings.value("Size", QDesktopWidget().availableGeometry(this).size()*0.5).toSize();
    resize(size);
    Settings.endGroup();
}

void ParmsWindow::writeSettings()
{
    QSettings Settings;
    Settings.beginGroup(PARMSWINDOW);
    Settings.setValue("Geometry", saveGeometry());
    Settings.setValue("State", saveState());
    Settings.setValue("Size", size());
    Settings.endGroup();
}

void ParmsWindow::setWindowTitle(const QString &title) {
    QMainWindow::setWindowTitle(title);
}

/*
 *
 * Text Editor section
 *
 */

ParmEditor::ParmEditor(QWidget *parent) :
    QPlainTextEdit(parent),lineNumberArea(new ParmLineNumberArea(this)),_fileIsUTF8(false)
{
    QPalette lineNumberPalette = lineNumberArea->palette();
    lineNumberPalette.setCurrentColorGroup(QPalette::Active);
    lineNumberPalette.setColor(QPalette::Text,QColor(Qt::darkGray));
    lineNumberPalette.setColor(QPalette::Highlight,QColor(Qt::magenta));
    if (Preferences::darkTheme) {
        lineNumberPalette.setColor(QPalette::Text,QColor(Qt::darkGray).darker(150));
        lineNumberPalette.setColor(QPalette::Background,QColor(Preferences::themeColors[THEME_DARK_EDIT_MARGIN]));
    } else
        lineNumberPalette.setColor(QPalette::Background,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]).lighter(130));

    lineNumberArea->setPalette(lineNumberPalette);

    setVerticalScrollBar(new ScrollBarFix(Qt::Vertical, this));

    connect(this, SIGNAL(blockCountChanged(int)),
            this, SLOT(updateLineNumberAreaWidth(int)));

    connect(this, SIGNAL(updateRequest(QRect,int)),
            this, SLOT(updateLineNumberArea(QRect,int)));

    connect(this, SIGNAL(cursorPositionChanged()),
            this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);

    highlightCurrentLine();
}

void ParmEditor::gotoLine(int line)
{
    QTextCursor cursor(document()->findBlockByNumber(line-1));
    this->setTextCursor(cursor);
}

void ParmEditor::showAllCharacters(bool enabled) {
    if (enabled) {
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

int ParmEditor::lineNumberAreaWidth()
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

void ParmEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ParmEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void ParmEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ParmEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor;
        if (Preferences::displayTheme == THEME_DEFAULT) {
            lineColor = QColor(Preferences::themeColors[THEME_DEFAULT_LINE_HIGHLIGHT]);
          }
        else
        if (Preferences::darkTheme) {
            lineColor = QColor(Preferences::themeColors[THEME_DARK_LINE_HIGHLIGHT]);
        }

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void ParmEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
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
            painter.setPen(palette.windowText().color());

            bool selected = (
                                (selStart < block.position() + block.length() && selEnd > block.position())
                                || (selStart == selEnd && selStart == block.position())
                            );

            if (selected) {
                painter.save();
                painter.setPen(palette.highlight().color());
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

void ParmEditor::showCharacters(
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

void ParmEditor::findDialog()
{
    QTextCursor cursor = textCursor();
    QString selection = cursor.selectedText();
    if (selection.isEmpty()) {
        cursor.select(QTextCursor::WordUnderCursor);
        selection = cursor.selectedText();
    }
    popUp = new FindReplace(this,selection);
    popUp->show();
}
