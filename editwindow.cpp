/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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
 * possible.  It does work the the syntax highlighter implemented in
 * highlighter.(h,cpp)
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QtGui>

#include "editwindow.h"
#include "highlighter.h"
#include "ldrawfiles.h"

EditWindow *editWindow;

EditWindow::EditWindow()
{
    editWindow  = this;
    _textEdit   = new QTextEdit;

    highlighter = new Highlighter(_textEdit->document());
    _textEdit->setLineWrapMode(QTextEdit::NoWrap);
    _textEdit->setUndoRedoEnabled(true);

    createActions();
    createToolBars();

    setCentralWidget(_textEdit);

    resize(800,600);
}

void EditWindow::createActions()
{
    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcut(tr("Ctrl+X"));
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), _textEdit, SLOT(cut()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcut(tr("Ctrl+C"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), _textEdit, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcut(tr("Ctrl+V"));
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), _textEdit, SLOT(paste()));

    redrawAct = new QAction(QIcon(":/images/redraw.png"), tr("&Redraw"), this);
    redrawAct->setShortcut(tr("Ctrl+R"));
    redrawAct->setStatusTip(tr("Redraw page"));
    connect(redrawAct, SIGNAL(triggered()), this, SLOT(redraw()));

    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(_textEdit, SIGNAL(copyAvailable(bool)),
            cutAct,    SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(copyAvailable(bool)),
            copyAct,   SLOT(setEnabled(bool)));
}

void EditWindow::createToolBars()
{
    editToolBar = addToolBar(tr("Edit"));
    editToolBar->setObjectName("EditToolbar");
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(redrawAct);
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
      _textEdit->verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
    } else {
      cursor.movePosition(QTextCursor::Up, moveMode);
      _textEdit->verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
    }
  }  
}

void EditWindow::showLine(int lineNumber)
{
  _textEdit->moveCursor(QTextCursor::Start,QTextCursor::MoveAnchor);
  for (int i = 0; i < lineNumber; i++) {
    _textEdit->moveCursor(QTextCursor::Down,QTextCursor::MoveAnchor);
  }
  _textEdit->moveCursor(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
  _textEdit->ensureCursorVisible();
  
  pageUpDown(QTextCursor::Up, QTextCursor::KeepAnchor);
}

void EditWindow::displayFile(
  LDrawFile     *ldrawFile,
  const QString &_fileName)
{
  fileName = _fileName;
  disconnect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
             this,                  SLOT(  contentsChange(int,int,int)));
  if (fileName == "") {
    _textEdit->document()->clear();
  } else {
    _textEdit->setPlainText(ldrawFile->contents(fileName).join("\n"));
  }
  _textEdit->document()->setModified(false);
  connect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
          this,                  SLOT(  contentsChange(int,int,int)));
  

}
// Jaco is trying to get the edit window to resize...

void EditWindow::redraw()
{
  redrawSig();
// QT manual says
// void QWidget::adjustSize ()
// Adjusts the size of the widget to fit its contents.
  adjustSize(); //Does not work.
}

