/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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

#include "version.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "editwindow.h"
#include "highlighter.h"
#include "ldrawfiles.h"

EditWindow *editWindow;

EditWindow::EditWindow(QMainWindow *parent) :
  QMainWindow(parent)
{
    editWindow  = this;

    _textEdit   = new QTextEditor;

    highlighter = new Highlighter(_textEdit->document());
    _textEdit->setLineWrapMode(QTextEditor::NoWrap);
    _textEdit->setUndoRedoEnabled(true);

    createActions();
    createToolBars();

    connect(_textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    highlightCurrentLine();

    setCentralWidget(_textEdit);

    resize(QDesktopWidget().availableGeometry(this).size()*0.6);
}

void EditWindow::createActions()
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

    redrawAct = new QAction(QIcon(":/resources/redraw.png"), tr("&Redraw"), this);
    redrawAct->setShortcut(tr("Ctrl+R"));
    redrawAct->setStatusTip(tr("Redraw page"));
    connect(redrawAct, SIGNAL(triggered()), this, SLOT(redraw()));

    delAct = new QAction(QIcon(":/resources/delete.png"), tr("&Delete"), this);
    delAct->setShortcut(tr("DEL"));
    delAct->setStatusTip(tr("Delete the selection"));
    connect(delAct, SIGNAL(triggered()), _textEdit, SLOT(cut()));

    selAllAct = new QAction(QIcon(":/resources/selectall.png"), tr("&Select All"), this);
    selAllAct->setShortcut(tr("Ctrl+A"));
    selAllAct->setStatusTip(tr("Select all page content"));
    connect(selAllAct, SIGNAL(triggered()), _textEdit, SLOT(selectAll()));

    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    delAct->setEnabled(false);
    redrawAct->setEnabled(false);
    selAllAct->setEnabled(false);
    connect(_textEdit, SIGNAL(copyAvailable(bool)),
            cutAct,    SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(copyAvailable(bool)),
            copyAct,   SLOT(setEnabled(bool)));
    connect(_textEdit, SIGNAL(copyAvailable(bool)),
             delAct,   SLOT(setEnabled(bool)));
}

void EditWindow::createToolBars()
{
    editToolBar = addToolBar(tr("Edit"));
    editToolBar->setObjectName("EditToolbar");
    editToolBar->addAction(selAllAct);
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(delAct);
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

void EditWindow::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!_textEdit->isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::blue).lighter(180);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = _textEdit->textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

     _textEdit->setExtraSelections(extraSelections);
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

  selAllAct->setEnabled(true);
  redrawAct->setEnabled(true);

}
// Jaco is trying to get the edit window to resize...

void EditWindow::redraw()
{
  redrawSig();
// QT manual says
// void QWidget::adjustSize ()
// Adjusts the size of the widget to fit its contents.
//  adjustSize(); //Does not work.
}

QTextEditor::QTextEditor(QWidget *parent) :
    QTextEdit(parent)
{
    lineNumberArea = new QLineNumberArea(this);

    connect(this->document(), SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateLineNumberArea/*_2*/(int)));
    connect(this, SIGNAL(textChanged()), this, SLOT(updateLineNumberArea()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateLineNumberArea()));
    //connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    //highlightCurrentLine();
}

int QTextEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, this->document()->blockCount());
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
    painter.fillRect(event->rect(), QColor(Qt::gray).lighter(150));
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

    QColor col_1(Qt::magenta);     // Current line
    QColor col_0(Qt::darkGray);    // Other lines

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
