/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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

#include <QtGui>

#include "parmswindow.h"
#include "highlighter.h"

ParmsWindow *parmsWindow;

ParmsWindow::ParmsWindow()
{
    parmsWindow  = this;
    parmsWindow->statusBar()->show();

    _textEdit   = new QTextEdit;

    highlighter = new Highlighter(_textEdit->document());
    _textEdit->setLineWrapMode(QTextEdit::NoWrap);
    _textEdit->setUndoRedoEnabled(true);

    createActions();
    createToolBars();

    setCentralWidget(_textEdit);

    resize(QSize(600, 800));
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
    connect(_textEdit, SIGNAL(undoAvailable(bool)),
             saveAct,  SLOT(setEnabled(bool)));
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
    editToolBar->addAction(delAct);

    undoRedoToolBar = addToolBar(tr("Undo Redo"));
    undoRedoToolBar->setObjectName("UndoRedoToolbar");
    undoRedoToolBar->addAction(undoAct);
    undoRedoToolBar->addAction(redoAct);
}

void ParmsWindow::pageUpDown(
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
      _textEdit->verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
    } else {
      cursor.movePosition(QTextCursor::Up, moveMode);
      _textEdit->verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
    }
  }
}

void ParmsWindow::displayParmsFile(
  const QString &_fileName)
{
    fileName = _fileName;
    QFile file(fileName);

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

    _textEdit->setPlainText(ss.readAll());
    _textEdit->document()->setModified(false);

    file.close();

    selAllAct->setEnabled(true);
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
                                 QMessageBox::tr("Parmeter Editor"),
                                 QMessageBox::tr("Cannot write file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return ! rc;
        }

        QTextDocumentWriter writer(fileName, "plaintext");
        rc = writer.write(_textEdit->document());

        if (rc){
            _textEdit->document()->setModified(false);
            saveAct->setEnabled(false);
            statusBar()->showMessage(tr("File saved"), 2000);
        }
    }

  return rc;
}

void ParmsWindow::closeEvent(QCloseEvent *event)
{
  if (maybeSave()) {
    event->accept();
  } else {
    event->ignore();
  }
}

