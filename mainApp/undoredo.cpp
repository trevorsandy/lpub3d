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

#include "lpub.h"
#include "commands.h"
#include "editwindow.h"
#include "pairdialog.h"

#include "lc_view.h"
#include "lc_model.h"

QString Gui::topLevelFile()
{
  return lpub->ldrawFile.topLevelFile();
}

void Gui::insertLine(const Where &here, const QString &line, QUndoCommand *parent)
{
  if (lpub->ldrawFile.contains(here.modelName)) {
    undoStack->push(new InsertLineCommand(&lpub->ldrawFile,here,line,parent));
  }
}

void Gui::appendLine(const Where &here, const QString &line, QUndoCommand *parent)
{
  if (lpub->ldrawFile.contains(here.modelName)) {
    undoStack->push(new AppendLineCommand(&lpub->ldrawFile,here,line,parent));
  }
}
      
void Gui::replaceLine(const Where &here, const QString &line, QUndoCommand *parent)
{
  if (lpub->ldrawFile.contains(here.modelName) &&
      here.lineNumber < lpub->ldrawFile.size(here.modelName)) {

    undoStack->push(new ReplaceLineCommand(&lpub->ldrawFile,here,line,parent));
  }
}

void Gui::deleteLine(const Where &here, QUndoCommand *parent)
{
  if (lpub->ldrawFile.contains(here.modelName) &&
      here.lineNumber < lpub->ldrawFile.size(here.modelName)) {
    undoStack->push(new DeleteLineCommand(&lpub->ldrawFile,here,parent));
  }
}

QString Gui::readLine(const Where &here)
{
  return lpub->ldrawFile.readLine(here.modelName,here.lineNumber);
}

void Gui::beginMacro(QString name)
{
  ++macroNesting;
  undoStack->beginMacro(name);
}

void Gui::endMacro()
{
  undoStack->endMacro();
  --macroNesting;
  if (!mloadingFile)
    cyclePageDisplay(displayPageNum);
}

void Gui::contentsChange(
  const QString &fileName,
        bool     isUndo,
        bool     isRedo,
        int      position,
        int      _charsRemoved,
  const QString &charsAdded)
{
  // Undo or redo the last 'userTyped' command and quit.
  // Do not push undo or redo content to the stack (ldrawFile)
  // as this content is already in the undo or redo command on the stack.
  if (isUndo && undoStack->undoText() == "userTyped") {
      undoStack->undo();
      return;
  } else if (isRedo && undoStack->redoText() == "userTyped") {
      undoStack->redo();
      return;
  }

  QString  charsRemoved;

  /* Calculate the characters removed from the LDrawFile */

  if (_charsRemoved && lpub->ldrawFile.contains(fileName)) {

    QString contents = lpub->ldrawFile.contents(fileName).join("\n");

    charsRemoved = contents.mid(position,_charsRemoved);
  }
  
  undoStack->push(new ContentsChangeCommand(&lpub->ldrawFile,
                                            fileName,
                                            position,
                                            charsRemoved,
                                            charsAdded));
}

void Gui::setBuildModClearStepKey(const QString &text)
{
    if (text.startsWith("BuildMod")) {
        buildModClearStepKey = text.split("|").last();
#ifdef QT_DEBUG_MODE
        emit messageSig(LOG_DEBUG, QString("BuildMod - reset images trigger: %1").arg(text));
#endif
    }
}

void Gui::undo()
{
  if (undoStack->undoText() == "userTyped" && editWindow->updateEnabled()) {
      emit editWindow->triggerUndoSig();
      return;
  }
  if (viewerUndo) {
    lcView* ActiveView = GetActiveView();
    lcModel* ActiveModel = ActiveView ? ActiveView->GetActiveModel() : nullptr;
    if (ActiveModel)
      ActiveModel->UndoAction();
  } else {
    setBuildModClearStepKey(undoStack->undoText());
    macroNesting++;
    undoStack->undo();
    macroNesting--;
    displayPage();
  }
}

void Gui::redo()
{
  if (undoStack->redoText() == "userTyped" && !editWindow->updateEnabled()) {
      emit editWindow->triggerRedoSig();
      return;
  }
  if (viewerRedo) {
      lcView* ActiveView = GetActiveView();
      lcModel* ActiveModel = ActiveView ? ActiveView->GetActiveModel() : nullptr;
      if (ActiveModel)
          ActiveModel->RedoAction();
  } else {
    setBuildModClearStepKey(undoStack->redoText());
    macroNesting++;
    undoStack->redo();
    macroNesting--;
    displayPage();
  }
}

void Gui::canRedoChanged(bool enabled)
{
  redoAct->setEnabled(enabled);
  if (!undoStack->redoText().isEmpty())
  {
    redoAct->setText(QString(tr("&Redo %1")).arg(undoStack->redoText()));
#ifdef __APPLE__
    redoAct->setStatusTip(tr("Redo %1 - Ctrl+Shift+Z").arg(undoStack->redoText()));
#else
    redoAct->setStatusTip(tr("Redo %1 - Ctrl+Y").arg(undoStack->redoText()));
#endif
  }
  else
  {
    redoAct->setText(tr("&Redo"));
#ifdef __APPLE__
    redoAct->setStatusTip(tr("Redo last change - Ctrl+Shift+Z"));
#else
    redoAct->setStatusTip(tr("Redo last change - Ctrl+Y"));
#endif
  }
}

void Gui::canUndoChanged(bool enabled)
{
  undoAct->setEnabled(enabled);
  if (!undoStack->undoText().isEmpty())
  {
    undoAct->setText(QString(tr("&Undo %1")).arg(undoStack->undoText()));
    undoAct->setStatusTip(tr("Undo %1 - Ctrl+Z").arg(undoStack->undoText()));
  }
  else
  {
    undoAct->setText(tr("&Undo"));
    undoAct->setStatusTip(tr("Undo last change - Ctrl+Z"));
  }
}

void Gui::cleanChanged(bool cleanState)
{
  getAct("saveAct.1")->setDisabled( cleanState);
}

void Gui::scanPast(Where &topOfStep, const QRegExp &lineRx)
{
  Where walk    = lpub->ldrawFile.readLine(topOfStep.modelName,topOfStep.lineNumber) == "0 STEP" ? topOfStep + 1 : topOfStep;
  Where lastPos = topOfStep;
  int  numLines = lpub->ldrawFile.size(walk.modelName);
  QRegExp endRx("^0 STEP$|^0 ROTSTEP|^0 ROTATION|^1 ");
  if (walk < numLines) {
    QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
    for ( ++walk; walk < numLines; ++walk) {
      line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
      lastPos = line.contains(lineRx) ? walk : lastPos;
      if ( ! line.contains(lineRx) && ! isHeader(line)) {
        topOfStep = lastPos;
        if (line.contains(endRx)) {
          ++topOfStep;
          break;
        }
      }
    }
  }
}

// special case - used in setting fade step from command meta
bool Gui::stepContains(Where &here, QRegExp &lineRx, QString &result, int capGrp) {
    bool found = false;
    if ((found = Gui::stepContains(here,lineRx))){
        if (capGrp)
            result = lineRx.cap(capGrp);
        else
            result = "true";
    }
    return found;
}

// general case string
bool Gui::stepContains(Where &topOfStep, const QString value)
{
    QRegExp lineRx(value, Qt::CaseInsensitive);
    return Gui::stepContains(topOfStep, lineRx);
}

// general case regex
bool Gui::stepContains(Where &topOfStep, QRegExp &lineRx)
{
  bool found = false;
  Where walk    = topOfStep;
  int  numLines = lpub->ldrawFile.size(walk.modelName);
  QRegExp endRx("^0 STEP$|^0 ROTSTEP|^0 NOFILE$|^0 FILE");
  for (; walk < numLines; ++walk) {
    QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
    if ((found = line.contains(lineRx)))
      topOfStep = walk;
    if (found || line.contains(endRx))
      break;
  }
  topOfStep.setModelIndex(lpub->ldrawFile.getSubmodelIndex(topOfStep.modelName));
  return found;
}

