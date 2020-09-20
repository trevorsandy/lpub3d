/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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

#include "editwindow.h"
#include "highlighter.h"
#include "ldrawfiles.h"
#include "messageboxresizable.h"

#include "name.h"
#include "version.h"
#include "lpub_preferences.h"

EditWindow *editWindow;

EditWindow::EditWindow(QMainWindow *parent, bool _modelFileEdit_) :
  QMainWindow(parent),_modelFileEdit(_modelFileEdit_)
{
    editWindow  = this;

    _textEdit   = new QTextEditor(this);

    highlighter = new Highlighter(_textEdit->document());
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

    createActions();
    createToolBars();

    connect(_textEdit, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
    connect(_textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    highlightCurrentLine();

    setCentralWidget(_textEdit);

    if (modelFileEdit()) {
        _saveSubfileIndex = 0;
        QObject::connect(&fileWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(modelFileChanged(const QString&)));
        editWindow->statusBar()->show();
        readSettings();
    }
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

void EditWindow::createActions()
{
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
    connect(updateAct, SIGNAL(triggered()), this, SLOT(update()));

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

    undoAct->setEnabled(false);
    redoAct->setEnabled(false);
    saveAct->setEnabled(false);

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
}

void EditWindow::showContextMenu(const QPoint &pt)
{
    QMenu *menu = _textEdit->createStandardContextMenu();
    menu->addSeparator();
    menu->addAction(topAct);
    menu->addAction(bottomAct);
    menu->addAction(toggleCmmentAct);
    menu->addAction(findAct);
    menu->addAction(updateAct);
    menu->addAction(redrawAct);
    menu->exec(_textEdit->mapToGlobal(pt));
    delete menu;
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
}

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
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Model File Editor"),
            tr("The model file has been modified.\n"
                "Do you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save) {
      rc = saveFile();
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

void EditWindow::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!_textEdit->isReadOnly()) {
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

void EditWindow::updateDisabled(bool state){
    QAction *action = qobject_cast<QAction *>(sender());
    if ((action && action == saveAct))
    {
        updateAct->setDisabled(true);
    } else {
        updateAct->setDisabled(state);
    }
}

void EditWindow::displayFile(
  LDrawFile     *ldrawFile,
  const QString &_fileName)
{
  bool reloaded = _fileName == fileName;

  fileName = _fileName;

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
                       QMessageBox::tr("Model File Editor"),
                       QMessageBox::tr("Cannot read file %1:\n%2.")
                       .arg(fileName)
                       .arg(file.errorString()));

              _textEdit->document()->clear();
              return;
          }

          mpdCombo->setMaxCount(0);
          mpdCombo->setMaxCount(1000);
          mpdCombo->addItems(ldrawFile->subFileOrder());
          if(_saveSubfileIndex) {
              mpdCombo->setCurrentIndex(_saveSubfileIndex);
              _saveSubfileIndex = 0;
          }

          // check file encoding
          QTextCodec *codec = QTextCodec::codecForName("UTF-8");
          bool isUTF8 = LDrawFile::_currFileIsUTF8;
          _textEdit->setIsUTF8(isUTF8);

          QFileInfo   fileInfo(fileName);
          QTextStream in(&file);
          in.setCodec(_textEdit->getIsUTF8() ? codec : QTextCodec::codecForName("System"));

          disconnect(_textEdit, SIGNAL(textChanged()),
                     this,      SLOT(enableSave()));

          _textEdit->setPlainText(in.readAll());

          connect(_textEdit,  SIGNAL(textChanged()),
                   this,      SLOT(enableSave()));
          file.close();

          exitAct->setEnabled(true);
          statusBar()->showMessage(tr("Model File %1 %2")
                                   .arg(fileName).arg(reloaded ? "updated" : "loaded"), 2000);
      } else {
          _textEdit->setPlainText(ldrawFile->contents(fileName).join("\n"));
      }
  }

  _textEdit->document()->setModified(false);

  connect(_textEdit->document(), SIGNAL(contentsChange(int,int,int)),
          this,                  SLOT(  contentsChange(int,int,int)));

  selAllAct->setEnabled(true);
  showAllCharsAct->setEnabled(true);
  redrawAct->setEnabled(true);
  findAct->setEnabled(true);
  toggleCmmentAct->setEnabled(true);
  topAct->setEnabled(true);
  bottomAct->setEnabled(true);

  if (modelFileEdit() && !fileName.isEmpty())
      fileWatcher.addPath(fileName);
}

void EditWindow::redraw()
{
  if (modelFileEdit() && Preferences::saveOnRedraw)
      saveFile();
  redrawSig();
}

void EditWindow::update()
{
  if (modelFileEdit() && Preferences::saveOnUpdate)
      saveFile();
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

void EditWindow::closeEvent(QCloseEvent *event)
{
    if (!modelFileEdit())
        return;

    if (!fileName.isEmpty())
        fileWatcher.removePath(fileName);

    writeSettings();

    mpdCombo->setMaxCount(0);
    mpdCombo->setMaxCount(1000);

    if (maybeSave()){
        event->accept();
    } else {
        event->ignore();
    }
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
            };
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
