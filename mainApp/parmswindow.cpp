/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

/****************************************************************************
 *
 * The editwindow is used to display the freeform annotations
 * and colour parts fade listings to the user.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "version.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include <QDesktopWidget>
#include "parmshighlighter.h"
#include "parmswindow.h"
#include "lpub_preferences.h"
#include "lpub.h"
#include "name.h"


ParmsWindow *parmsWindow;

ParmsWindow::ParmsWindow(QMainWindow *parent) :
  QMainWindow(parent)
{
    parmsWindow  = this;
    parmsWindow->statusBar()->show();

    _textEdit   = new TextEditor;

    highlighter = new ParmsHighlighter(_textEdit->document());
    _textEdit->setLineWrapMode(TextEditor::NoWrap);
    _textEdit->setUndoRedoEnabled(true);
    _parmsChanged = false;

    createActions();
    createToolBars();

    setCentralWidget(_textEdit);

    readSettings();
}

void ParmsWindow::createActions()
{
    cutAct = new QAction(QIcon(":/resources/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcut(tr("Ctrl+X"));
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), _textEdit, SLOT(cut()));

    copyAct = new QAction(QIcon(":/resources/copy.png"), tr("&Copy"), this);
    copyAct->setShortcut(tr("Ctrl+C"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), _textEdit, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/resources/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcut(tr("Ctrl+V"));
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), _textEdit, SLOT(paste()));

    findAct = new QAction(QIcon(":/resources/find.png"), tr("&Find"), this);
    findAct->setShortcut(tr("Ctrl+F"));
    findAct->setStatusTip(tr("Find object "));
    connect(findAct, SIGNAL(triggered()), _textEdit, SLOT(findDialog()));

    saveAct = new QAction(QIcon(":/resources/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));

    delAct = new QAction(QIcon(":/resources/delete.png"), tr("&Delete"), this);
    delAct->setShortcut(tr("DEL"));
    delAct->setStatusTip(tr("Delete the selection"));
    connect(delAct, SIGNAL(triggered()), _textEdit, SLOT(cut()));

    selAllAct = new QAction(QIcon(":/resources/selectall.png"), tr("&Select All"), this);
    selAllAct->setShortcut(tr("Ctrl+A"));
    selAllAct->setStatusTip(tr("Select all page content"));
    connect(selAllAct, SIGNAL(triggered()), _textEdit, SLOT(selectAll()));

    undoAct = new QAction(QIcon(":/resources/editundo.png"), tr("Undo"), this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last change"));
    undoAct->setEnabled(false);
    connect(undoAct, SIGNAL(triggered()), _textEdit, SLOT(undo()));
    redoAct = new QAction(QIcon(":/resources/editredo.png"), tr("Redo"), this);
#ifdef __APPLE__
    redoAct->setShortcut(tr("Ctrl+Shift+Z"));
#else
    redoAct->setShortcut(tr("Ctrl+Y"));
#endif
    redoAct->setStatusTip(tr("Redo last change"));
    redoAct->setEnabled(false);
    connect(redoAct, SIGNAL(triggered()), _textEdit, SLOT(redo()));

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
    editToolBar->addAction(saveAct);
    editToolBar->addAction(selAllAct);
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(findAct);
    editToolBar->addAction(delAct);

    undoRedoToolBar = addToolBar(tr("Undo Redo"));
    undoRedoToolBar->setObjectName("UndoRedoToolbar");
    undoRedoToolBar->addAction(undoAct);
    undoRedoToolBar->addAction(redoAct);
}

void ParmsWindow::displayParmsFile(
  const QString &_fileName)
{
    fileName = _fileName;
    QFile file(fileName);
    QFileInfo fileInfo(file.fileName());

    if (fileInfo.fileName() == "pliSubstituteParts.lst")
      title = "PLI/BOM Substitute Parts";
    else if (fileInfo.fileName() == "fadeStepColorParts.lst")
      title = "Fade Step Color Parts";
    else if (fileInfo.fileName() == "titleAnnotations.lst")
      title = "Title Annotation";
    else //freeformAnnotations.lst
      title = "Freeform";

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

    _textEdit->blockSignals(true);
    _textEdit->setPlainText(ss.readAll());
    _textEdit->blockSignals(false);
    _textEdit->document()->setModified(false);

    file.close();

    selAllAct->setEnabled(true);
    findAct->setEnabled(true);
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
            return ! rc;
        }

        QTextDocumentWriter writer(fileName, "plaintext");
        rc = writer.write(_textEdit->document());

        if (rc){
            _parmsChanged = true;
            saveAct->setEnabled(false);
            _textEdit->document()->setModified(false);
            emit fileModified(_textEdit->document()->isModified());
            statusBar()->showMessage(tr("File saved"), 2000);
        }
    }

  return rc;
}

void ParmsWindow::enableSave()
{
  if (_textEdit->document()->isModified())
    {
      saveAct->setEnabled(true);
    }
}

void ParmsWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave() || _parmsChanged ) {
        writeSettings();
        if (_parmsChanged){
            bool fileLoaded = false;
            if (!gui->getCurFile().isEmpty())
                fileLoaded = true;

            QMessageBox box;
            box.setIcon (QMessageBox::Question);
            box.setDefaultButton   (QMessageBox::Ok);
            box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);
            box.setText (tr("You must close and restart %1\nfor changes to take effect.")
                         .arg(QString::fromLatin1(VER_PRODUCTNAME_STR)));
            box.setInformativeText (tr (fileLoaded ? "Click \"OK\" to close and restart %1" : "Click \"OK\" to close %1")
                                    .arg(QString::fromLatin1(VER_PRODUCTNAME_STR)));

            if (box.exec() == QMessageBox::Ok) {
                if (fileLoaded) {
                    QStringList args = QApplication::arguments();
                    args << tr ("%1").arg(gui->getCurFile());
                    args.removeFirst();
                    QProcess::startDetached(QApplication::applicationFilePath(), args);
                    qDebug() << "exe:" << QApplication::applicationFilePath() << ", args:" << args;
                }
                event->accept();
                QCoreApplication::quit();
            }
        }
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

        QColor lineColor = QColor(Qt::blue).lighter(180);

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

    QColor col_1(Qt::magenta);   // Current line
    QColor col_0(Qt::darkGray);    // Other lines

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
    popUp->setWindowTitle("Find");

    layout = new QGridLayout;

    buttonFind  = new QPushButton("Find");
    buttonFindNext = new QPushButton("Find Next");
    buttonFindPrevious = new QPushButton("Find Previous");

    connect(buttonFind, SIGNAL(clicked()), this,SLOT(findInText()));
    connect(buttonFindNext,SIGNAL(clicked()),this,SLOT(findInTextNext()));
    connect(buttonFindPrevious,SIGNAL(clicked()),this,SLOT(findInTextPrevious()));

    textFind    = new QLineEdit;
    textFind->setMinimumWidth(250);

    labelMessage = new QLabel;
    labelMessage->setMinimumWidth(250);

    layout->addWidget(textFind,0,0,1,3);

    layout->addWidget(buttonFind,1,0,1,1);
    layout->addWidget(buttonFindNext,1,1,1,1);
    layout->addWidget(buttonFindPrevious,1,2,1,1);
    layout->addWidget(labelMessage,2,0,1,3);

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

