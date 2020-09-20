/****************************************************************************
**
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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

#include "parmshighlighter.h"
#include "parmswindow.h"
#include "messageboxresizable.h"

#include "lpub.h"
#include "name.h"
#include "version.h"
#include "lpub_preferences.h"

ParmsWindow *parmsWindow;

ParmsWindow::ParmsWindow(QMainWindow *parent) :
  QMainWindow(parent)
{
    parmsWindow  = this;
    parmsWindow->statusBar()->show();

    _textEdit          = new TextEditor(this);

    highlighter = new ParmsHighlighter(_textEdit->document());
    _textEdit->setLineWrapMode(TextEditor::NoWrap);
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
}

void ParmsWindow::createActions()
{
    exitAct = new QAction(QIcon(":/resources/exit.png"),tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application - Ctrl+Q"));

    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    openAct = new QAction(QIcon(":/resources/open.png"), tr("&Open Stderr or Stdout log"), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open stdout or stderr log file for loaded model - Ctrl+O"));

    connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));

    refreshAct = new QAction(QIcon(":/resources/redraw.png"), tr("&Refresh file"), this);
    refreshAct->setShortcut(tr("Ctrl+Alt+O"));
    refreshAct->setStatusTip(tr("Reload the current file to see updated content - Ctrl+Alt+O"));

    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refreshFile()));

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

    saveAct = new QAction(QIcon(":/resources/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk - Ctrl+S"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));

    saveCopyAsAct = new QAction(QIcon(":/resources/saveas.png"), tr("Save Copy &As"), this);
    saveCopyAsAct->setShortcut(tr("Ctrl+Alt+S"));
    saveCopyAsAct->setStatusTip(tr("Save a copy of the document as... to disk - Ctrl+Alt+S"));
    connect(saveCopyAsAct, SIGNAL(triggered()), this, SLOT(saveCopyAsFile()));

    topAct = new QAction(QIcon(":/resources/topofdocument.png"), tr("Top of Document"), this);
    topAct->setShortcut(tr("Ctrl+T"));
    topAct->setStatusTip(tr("Go to the top of document - Ctrl+T"));
    connect(topAct, SIGNAL(triggered()), this, SLOT(topOfDocument()));

    bottomAct = new QAction(QIcon(":/resources/bottomofdocument.png"), tr("Bottom of Document"), this);
    bottomAct->setShortcut(tr("Ctrl+B"));
    bottomAct->setStatusTip(tr("Go to the bottom of document - Ctrl+B"));
    connect(bottomAct, SIGNAL(triggered()), this, SLOT(bottomOfDocument()));

    delAct = new QAction(QIcon(":/resources/delete.png"), tr("&Delete"), this);
    delAct->setShortcut(tr("DEL"));
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

    undoAct = new QAction(QIcon(":/resources/editundo.png"), tr("Undo"), this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last change - Ctrl+Z"));
    undoAct->setEnabled(false);
    connect(undoAct, SIGNAL(triggered()), _textEdit, SLOT(undo()));
    redoAct = new QAction(QIcon(":/resources/editredo.png"), tr("Redo"), this);
#ifdef __APPLE__
    redoAct->setShortcut(tr("Ctrl+Shift+Z"));
    redoAct->setStatusTip(tr("Redo last change - Ctrl+Shift+Z"));
#else
    redoAct->setShortcut(tr("Ctrl+Y"));
    redoAct->setStatusTip(tr("Redo last change - Ctrl+Y"));
#endif
    redoAct->setEnabled(false);
    connect(redoAct, SIGNAL(triggered()), _textEdit, SLOT(redo()));

    openAct->setVisible(false);

    refreshAct->setVisible(false);
    saveAct->setEnabled(false);
    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    delAct->setEnabled(false);
    selAllAct->setEnabled(false);
    findAct->setEnabled(false);
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
    editToolBar->setObjectName("EditToolbar");
    editToolBar->addAction(exitAct);
    editToolBar->addSeparator();
    editToolBar->addAction(topAct);
    editToolBar->addAction(bottomAct);
    editToolBar->addSeparator();
    editToolBar->addAction(openAct);
    editToolBar->addAction(saveAct);
    editToolBar->addAction(saveCopyAsAct);
    editToolBar->addAction(selAllAct);
    editToolBar->addAction(showAllCharsAct);
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(findAct);
    editToolBar->addAction(refreshAct);
    editToolBar->addAction(delAct);

    undoRedoToolBar = addToolBar(tr("Undo Redo"));
    undoRedoToolBar->setObjectName("UndoRedoToolbar");
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
    menu->exec(_textEdit->mapToGlobal(pt));
    delete menu;
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

    if (fileInfo.fileName() == "pliSubstituteParts.lst")
      title = "PLI/BOM Substitute Parts";
    else if (fileInfo.fileName() == Preferences::validLDrawColorParts)
      {
        title = "Fade Step Color Parts";
        _fadeStepFile = true;
      }
    else if (fileInfo.fileName() == VER_TITLE_ANNOTATIONS_FILE)
      title = "Title Annotation";
    else if (fileInfo.fileName() == VER_EXCLUDED_PARTS_FILE)
      title = "Excluded Parts";
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
    else if (fileInfo.fileName() == Preferences::validAnnotationStyleFile) {
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
                 QMessageBox::tr("Cannot read file %1:\n%2.")
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
    showAllCharsAct->setEnabled(true);
    if (showAllCharsAction)
        showAllCharsAct->setVisible(true);
    else
        showAllCharsAct->setVisible(false);

    statusBar()->showMessage(tr("File %1 loaded").arg(file.fileName()), 2000);
}

bool ParmsWindow::maybeSave()
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

        if (rc){
            if (!force){
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
    QString fileSaveName = QString("%1_%2.txt").arg(fileInfo.completeBaseName()).arg(QDateTime::currentDateTime().toString(QLatin1String("yyyyMMdd-hhmmss")));
    QString filter(QFileDialog::tr("All Files (*.*)"));
    QString saveCopyAsFileName = QFileDialog::getSaveFileName(nullptr,
                                 QFileDialog::tr("Save %1 log").arg(VER_PRODUCTNAME_STR),
                                 fileSaveName,
                                 filter);
    if (saveCopyAsFileName.isEmpty())
      return rc;

    QString saveFileName = fileName;
    fileName = QDir::toNativeSeparators(fileInfo.absolutePath()+"/"+saveCopyAsFileName);
    bool force = true;
    rc = saveFile(force);
    fileName = saveFileName;
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

void ParmsWindow::refreshFile(){
  QFileInfo fileInfo(fileName);
  if (fileInfo.exists())
    displayParmsFile(fileName);
  _textEdit->moveCursor(QTextCursor::End);
}

void ParmsWindow::toggleClear(){
  if (_textEdit->document()->isModified())
    {
      delAct->setEnabled(true);
    } else {
      delAct->setEnabled(false);
    }
}

void ParmsWindow::topOfDocument(){
    _textEdit->moveCursor(QTextCursor::Start);
}

void ParmsWindow::bottomOfDocument(){
    _textEdit->moveCursor(QTextCursor::End);
}

void ParmsWindow::showAllCharacters(){
    _textEdit->blockSignals(true);
    _textEdit->showAllCharacters(showAllCharsAct->isChecked());
    _textEdit->blockSignals(false);
    _textEdit->document()->setModified(false);
}

void ParmsWindow::viewLogWindowSettings(){
  // customize the menu for logging

  if (! openAct->isVisible())
    openAct->setVisible(true);

  disconnect(delAct, SIGNAL(triggered()),
             _textEdit, SLOT(cut()));
  disconnect(_textEdit, SIGNAL(copyAvailable(bool)),
             delAct,   SLOT(setEnabled(bool)));
  connect(delAct, SIGNAL(triggered()),
          _textEdit, SLOT(clear()));
  connect(_textEdit, SIGNAL(textChanged()),
           this,     SLOT(toggleClear()));
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

  if (_fileModified){

      // load LDGLite settings if modified
      QFileInfo fileInfo(fileName);
      QString renderer = Render::getRenderer();
      if ((renderer == RENDERER_LDGLITE) && (fileInfo.fileName() == QString(VER_LDGLITE_INI_FILE)))
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
              if (fileLoaded){
                  if (!args.contains(gui->getCurFile(),Qt::CaseInsensitive))
                      args << QString("%1").arg(gui->getCurFile());
                  QSettings Settings;
                  Settings.setValue(QString("%1/%2").arg(DEFAULTS,SAVE_DISPLAY_PAGE_NUM_KEY),gui->displayPageNum);
                } else {
                  args << QString();
                }
              args.removeFirst();
              QProcess::startDetached(QApplication::applicationFilePath(), args);
              logDebug() << "Restarted LPub3D using:" << QApplication::applicationFilePath() << ", args:" << args;

              event->accept();
              QCoreApplication::quit();
            }
        }
    }

  if (acceptEvent){
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

void ParmsWindow::setWindowTitle(const QString &title){
    QMainWindow::setWindowTitle(title);
}

/*
 *
 * Text Editor section
 *
 */

TextEditor::TextEditor(QWidget *parent) :
    QPlainTextEdit(parent),_fileIsUTF8(false)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

void TextEditor::showAllCharacters(bool show){
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

int TextEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
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

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor;
        if (Preferences::displayTheme == THEME_DEFAULT) {
            lineColor = QColor(Qt::blue).lighter(180);
          }
        else
        if (Preferences::displayTheme == THEME_DARK) {
            lineColor = QColor(THEME_EDITWINDOW_LINE_DARK);
        }

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void TextEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(Qt::cyan).lighter(180));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    // line number colors
    QColor col_1(Qt::magenta);   // Current line
    QColor col_0(Qt::darkGray);    // Other lines
    if (Preferences::displayTheme == THEME_DARK) {
       col_0 = QColor(Qt::darkGray).darker(150);    // Other lines
    }

    // Draw the numbers (displaying the current line number in green)
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen((this->textCursor().blockNumber() == blockNumber) ? col_1 : col_0);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
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
    popUp = new FindReplace(this,selection);
    popUp->show();
}

FindReplace::FindReplace(
    TextEditor *textEdit,
    const QString &selectedText,
    QWidget *parent)
    : QDialog(parent)
{
    setWindowIcon(QIcon(":/resources/LPub32.png"));
    setWindowTitle("Parameter File Editor Find");

    find = new FindReplaceCtrls(textEdit,this);
    find->textFind->setText(selectedText);

    findReplace = new FindReplaceCtrls(textEdit,this);
    findReplace->textFind->setText(selectedText);

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

void  FindReplace::popUpClose()
{
    FindReplaceCtrls *fr = qobject_cast<FindReplaceCtrls *>(sender());
    if (fr) {
        writeFindReplaceSettings(findReplace);
        writeFindReplaceSettings(find);
        if (fr->_findall){
            fr->_textEdit->document()->undo();
        }
    }
    close();
}

void FindReplace::readFindReplaceSettings(FindReplaceCtrls *fr) {
    QSettings settings;
    settings.beginGroup(FINDREPLACEWINDOW);
    fr->checkboxCase->setChecked(settings.value(CASE_CHECK, false).toBool());
    fr->checkboxWord->setChecked(settings.value(WORD_CHECK, false).toBool());
    fr->checkboxRegExp->setChecked(settings.value(REGEXP_CHECK, false).toBool());
    settings.endGroup();
}

void FindReplace::writeFindReplaceSettings(FindReplaceCtrls *fr) {
    QSettings settings;
    settings.beginGroup(FINDREPLACEWINDOW);
    settings.setValue(CASE_CHECK, fr->checkboxCase->isChecked());
    settings.setValue(WORD_CHECK, fr->checkboxWord->isChecked());
    settings.setValue(REGEXP_CHECK, fr->checkboxRegExp->isChecked());
    settings.endGroup();
}

FindReplaceCtrls::FindReplaceCtrls(TextEditor *textEdit, QWidget *parent)
    : QWidget(parent),_textEdit(textEdit)
{
    // find items
    textFind    = new QLineEdit;

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
    textReplace = new QLineEdit;

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

void FindReplaceCtrls::findInText(){
    find();
}

void FindReplaceCtrls::findInTextNext()
{
    find(NEXT);
}

void FindReplaceCtrls::findInTextPrevious()
{
    find(PREVIOUS);
}

void FindReplaceCtrls::findInTextAll() {
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

void FindReplaceCtrls::find(int direction) {
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

void FindReplaceCtrls::replaceInText() {
    if (!_textEdit->textCursor().hasSelection()) {
        find();
    } else {
        _textEdit->textCursor().insertText(textReplace->text());
    }
}

void FindReplaceCtrls::replaceInTextFind() {
    if (!_textEdit->textCursor().hasSelection()) {
        find();
    } else {
        _textEdit->textCursor().insertText(textReplace->text());
        find();
    }
}

void FindReplaceCtrls::replaceInTextAll() {
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

void FindReplaceCtrls::regexpSelected(bool sel) {
    if (sel)
        validateRegExp(textFind->text());
    else
        validateRegExp("");
}

void FindReplaceCtrls::validateRegExp(const QString &text) {
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

void FindReplaceCtrls::showError(const QString &error) {
    if (error == "") {
        labelMessage->clear();
    } else {
        labelMessage->setText("<span style=\" font-weight:200; color:#ff0000;\">" + error + "</span>");
    }
}

void FindReplaceCtrls::showMessage(const QString &message) {
    if (message == "") {
        labelMessage->clear();
    } else {
        labelMessage->setText("<span style=\" font-weight:200; color:green;\">" + message + "</span>");
    }
}

void FindReplaceCtrls::textReplaceChanged() {
    bool enable = textReplace->text().size() > 0;
    buttonReplace->setEnabled(enable);
    buttonReplaceAndFind->setEnabled(enable);
    buttonReplaceAll->setEnabled(enable);
    buttonReplaceClear->setEnabled(enable);
}

void FindReplaceCtrls::textFindChanged() {
    bool enable = textFind->text().size() > 0;
    buttonFind->setEnabled(enable);
    buttonFindNext->setEnabled(enable);
    buttonFindAll->setEnabled(enable);
    buttonFindClear->setEnabled(enable);
}

void FindReplaceCtrls::disableButtons()
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

void FindReplaceCtrls::findClear() {
    _textEdit->document()->undo();
    _textEdit->moveCursor(QTextCursor::Start);
    textFind->clear();
    labelMessage->clear();
}

void FindReplaceCtrls::replaceClear() {
    _textEdit->document()->undo();
    _textEdit->moveCursor(QTextCursor::Start);
    textReplace->clear();
    labelMessage->clear();
}
