/****************************************************************************
**
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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
#include "lpub_preferences.h"
#include "lpub.h"
#include "name.h"
#include "version.h"


ParmsWindow *parmsWindow;

ParmsWindow::ParmsWindow(QMainWindow *parent) :
  QMainWindow(parent)
{
    parmsWindow  = this;
    parmsWindow->statusBar()->show();

    _textEdit        = new TextEditor;
    _fadeStepFile    = false;
    _fileModified    = false;
    _restartRequired = true;

    highlighter = new ParmsHighlighter(_textEdit->document());
    _textEdit->setLineWrapMode(TextEditor::NoWrap);
    _textEdit->setUndoRedoEnabled(true);

    createActions();
    createToolBars();

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

    delAct = new QAction(QIcon(":/resources/delete.png"), tr("&Delete"), this);
    delAct->setShortcut(tr("DEL"));
    delAct->setStatusTip(tr("Delete the selection - DEL"));
    connect(delAct, SIGNAL(triggered()), _textEdit, SLOT(cut()));

    selAllAct = new QAction(QIcon(":/resources/selectall.png"), tr("&Select All"), this);
    selAllAct->setShortcut(tr("Ctrl+A"));
    selAllAct->setStatusTip(tr("Select all page content - Ctrl+A"));
    connect(selAllAct, SIGNAL(triggered()), _textEdit, SLOT(selectAll()));

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
    editToolBar->addAction(openAct);
    editToolBar->addAction(saveAct);
    editToolBar->addAction(saveCopyAsAct);
    editToolBar->addAction(selAllAct);
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

void ParmsWindow::displayParmsFile(
  const QString &_fileName)
{
    // Automatically hide open file action - show for logs only
    if (openAct->isVisible())
      openAct->setVisible(false);
    if (refreshAct->isVisible())
      refreshAct->setVisible(false);

    fileName = _fileName;

    QFile file(fileName);

    // Use QFileInfo to get the base-name and extension
    QFileInfo fileInfo(file.fileName());

    if (fileInfo.fileName() == "pliSubstituteParts.lst")
      title = "PLI/BOM Substitute Parts";
    else if (fileInfo.fileName() == VER_LDRAW_COLOR_PARTS_FILE)
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
      title = "LDView ini";
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
    else if (fileInfo.fileName() == VER_NATIVE_POV_INI_FILE) {
      title = "Native POV File Generation ini";
      _restartRequired = false;
    }
    else if (fileInfo.fileName() == QString("%1Log.txt").arg(VER_PRODUCTNAME_STR)) {
      title = QString("%1 Log").arg(VER_PRODUCTNAME_STR);
      viewLogWindowSettings();
      _restartRequired = false;
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
    } else {
      title = fileInfo.fileName();
      _restartRequired = false;
    }

    if (! file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::warning(NULL,
                 QMessageBox::tr("Parmeter Editor"),
                 QMessageBox::tr("Cannot read file %1:\n%2.")
                 .arg(fileName)
                 .arg(file.errorString()));

        _textEdit->document()->clear();
        return;
    }

    QTextStream ss(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    _textEdit->blockSignals(true);
    _textEdit->setPlainText(ss.readAll());
    _textEdit->blockSignals(false);
    _textEdit->document()->setModified(false);

    file.close();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    selAllAct->setEnabled(true);
    findAct->setEnabled(true);

    statusBar()->showMessage(tr("File %1 loaded").arg(file.fileName()), 2000);
}

bool ParmsWindow::maybeSave()
{
  bool rc = true;

  if (_textEdit->document()->isModified()) {
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Parmeter Editor"),
            tr("The document has been modified.\n"
                "Do you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save) {
      saveFile();
      return rc;
    } else if (ret == QMessageBox::Cancel) {
      return ! rc;
    }
  }
  return rc;
}

bool ParmsWindow::saveFile()
{
    bool rc = false;
    // check for dirty editor
    if (_textEdit->document()->isModified())
    {
        QFile file(fileName);
        if (! file.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(NULL,
                                 QMessageBox::tr("%1 Editor")
                                 .arg(title),
                                 QMessageBox::tr("Cannot write file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return rc;
        }

        QTextDocumentWriter writer(fileName, "plaintext");
        rc = writer.write(_textEdit->document());

        if (rc){
            saveAct->setEnabled(false);
            _textEdit->document()->setModified(false);
            _fileModified = true;
            statusBar()->showMessage(tr("File saved"), 2000);
        }
    }

  return rc;
}

bool ParmsWindow::saveCopyAsFile()
{
    bool rc = false;
    // provide a file name
    QFileInfo fileInfo(fileName);
    QString fileSaveName = QString("%1_%2.txt").arg(fileInfo.baseName()).arg(QDateTime::currentDateTime().toString(QLatin1String("yyyyMMdd-hhmmsszzz")));
    QString filter(QFileDialog::tr("All Files (*.*)"));
    QString saveCopyAsFileName = QFileDialog::getSaveFileName(NULL,
                                 QFileDialog::tr("Save %1 log").arg(VER_PRODUCTNAME_STR),
                                 fileSaveName,
                                 filter);
    if (saveCopyAsFileName.isEmpty())
      return rc;

    QFile file(saveCopyAsFileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(saveCopyAsFileName),
                                  file.errorString()));
        return rc;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out << _textEdit->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    _textEdit->document()->setModified(false);
    setWindowModified(false);

    if (fileName.isEmpty())
        fileName = "untitled.txt";
    setWindowFilePath(fileName);

    statusBar()->showMessage(tr("File %1 saved").arg(fileSaveName), 2000);

    return true;
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
    QString filter(QFileDialog::tr("stderr (stderr-*);;stdout (stdout-*)"));
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(NULL,
                           QFileDialog::tr("Select stderr or stdout file"),
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

void ParmsWindow::viewLogWindowSettings(){
  // customize the menu for logging

  if (! openAct->isVisible())
    openAct->setVisible(true);
  if (! refreshAct->isVisible())
    refreshAct->setVisible(true);

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
              if (! fileLoaded){
                  args << QString("%1").arg(gui->getCurFile());
                  QSettings Settings;
                  Settings.setValue(QString("%1/%2").arg(DEFAULTS,SAVE_DISPLAY_PAGE_NUM),gui->displayPageNum);
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

void ParmsWindow::setWindowTitle(const QString &title){
    QMainWindow::setWindowTitle(title);

    _textEdit->windowTitle = title;
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

/*
 *
 * Text Editor section
 *
 */

TextEditor::TextEditor(QWidget *parent) :
    QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int TextEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
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

void TextEditor::findDialog(){

    popUp = new QWidget;
    popUp->setWindowTitle(tr("%1 Find").arg(windowTitle));

    layout = new QGridLayout;

    buttonFind  = new QPushButton("Find");
    buttonFindNext = new QPushButton("Find Next");
    buttonFindPrevious = new QPushButton("Find Previous");
    buttonFindClear = new QPushButton("Clear");

    connect(buttonFind, SIGNAL(clicked()), this,SLOT(findInText()));
    connect(buttonFindNext,SIGNAL(clicked()),this,SLOT(findInTextNext()));
    connect(buttonFindPrevious,SIGNAL(clicked()),this,SLOT(findInTextPrevious()));
    connect(buttonFindClear, SIGNAL(clicked()), this, SLOT(findClear()));

    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);

    textFind    = new QLineEdit;
    textFind->setMinimumWidth(250);

    labelMessage = new QLabel;
    labelMessage->setMinimumWidth(250);
    textFind->setText(cursor.selectedText());

    layout->addWidget(textFind,0,0,1,4);

    layout->addWidget(buttonFind,1,0,1,1);
    layout->addWidget(buttonFindNext,1,1,1,1);
    layout->addWidget(buttonFindPrevious,1,2,1,1);
    layout->addWidget(buttonFindClear,1,3,1,1);
    layout->addWidget(labelMessage,2,0,1,4);

    popUp->setLayout(layout);
    popUp->resize(300,50);

    popUp->show();

}

void TextEditor::findInText(){
    //Set Cursor position to the start of document
    //to let find() method make search in whole text,
    //otherwise it search from cursor position to the end.
    moveCursor(QTextCursor::Start);
    if (!find(textFind->text()))
        labelMessage->setText("Did not find the text '" + textFind->text() + "'");
    else
        labelMessage->clear();
}

void TextEditor::findInTextNext()
{
  if(buttonFindPrevious->isEnabled() == false)
  {
      moveCursor(QTextCursor::Start);
  }

  if(find(textFind->text()))
  {
      buttonFindPrevious->setEnabled(true);
      labelMessage->clear();
  } else {
      labelMessage->setText("No more items.");
  }
}

void TextEditor::findInTextPrevious()
{
  if(! find(textFind->text(), QTextDocument::FindBackward))
      labelMessage->setText("No more items.");
  else
      labelMessage->clear();
}

void TextEditor::findClear(){
  textFind->clear();
  labelMessage->clear();
}
