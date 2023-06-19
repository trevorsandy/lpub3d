/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2018 - 2023 Trevor SANDY. All rights reserved.
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
  if (macroNesting == 0) {
    if (!suspendFileDisplay)
      cyclePageDisplay(displayPageNum);
  }
}

void Gui::contentsChange(
  const QString &fileName,
        bool     isUndo,
        bool     isRedo,
        int      position,
        int      _charsRemoved,
  const QString &charsAdded)
{
  // Undo or redo the last 'commandEdit' and quit.
  // Do not push undo or redo content to the stack (ldrawFile)
  // as this content is already in the undo or redo command on the stack.
  if (isUndo && undoStack->undoText() == "commandEdit") {
      undoStack->undo();
      return;
  } else if (isRedo && undoStack->redoText() == "commandEdit") {
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
  if (undoStack->undoText() == "commandEdit" && editWindow->updateEnabled()) {
      emit editWindow->triggerUndoSig();
      return;
  }
  if (Preferences::buildModEnabled)
    setUndoRedoBuildModAction();
  if (visualEditUndo) {
    lcView* ActiveView = GetActiveView();
    lcModel* ActiveModel = ActiveView ? ActiveView->GetActiveModel() : nullptr;
    if (ActiveModel)
      ActiveModel->UndoAction();
    clearVisualEditUndoRedoText();
    enableVisualBuildModActions();
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
  if (undoStack->redoText() == "commandEdit" && !editWindow->updateEnabled()) {
      emit editWindow->triggerRedoSig();
      return;
  }
  if (Preferences::buildModEnabled)
    setUndoRedoBuildModAction(false/*Undo*/);
  if (visualEditRedo) {
      lcView* ActiveView = GetActiveView();
      lcModel* ActiveModel = ActiveView ? ActiveView->GetActiveModel() : nullptr;
      if (ActiveModel)
          ActiveModel->RedoAction();
      clearVisualEditUndoRedoText();
      enableVisualBuildModActions();
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
    redoAct->setStatusTip(tr("Redo %1").arg(undoStack->redoText()));
#else
    redoAct->setStatusTip(tr("Redo %1").arg(undoStack->redoText()));
#endif
  }
  else
  {
    redoAct->setText(tr("&Redo"));
#ifdef __APPLE__
    redoAct->setStatusTip(tr("Redo last change"));
#else
    redoAct->setStatusTip(tr("Redo last change"));
#endif
  }
}

void Gui::canUndoChanged(bool enabled)
{
  undoAct->setEnabled(enabled);
  if (!undoStack->undoText().isEmpty())
  {
    undoAct->setText(QString(tr("&Undo %1")).arg(undoStack->undoText()));
    undoAct->setStatusTip(tr("Undo %1").arg(undoStack->undoText()));
  }
  else
  {
    undoAct->setText(tr("&Undo"));
    undoAct->setStatusTip(tr("Undo last change"));
  }
}

void Gui::cleanChanged(bool cleanState)
{
  getAct("saveAct.1")->setDisabled(cleanState);
  if (cleanState)
      visualEditUndoRedoText.clear();
}

void Gui::scanPast(Where &topOfStep, const QRegExp &lineRx)
{
  const bool isScanPastGlobal = lineRx.pattern() == QStringLiteral(GLOBAL_META_RX);
  const bool onStepMeta = lpub->ldrawFile.readLine(topOfStep.modelName,topOfStep.lineNumber) == QStringLiteral("0 STEP");
  QRegExp endRx("^[1-5] |^0 ROTATION|^0 STEP$|^0 ROTSTEP");
  if (isScanPastGlobal) {
    if (onStepMeta)
      return;
    else
      endRx.setPattern("^[1-5] |^0 ROTATION|^0 STEP|^0 ROTSTEP|^0 !?LPUB MULTI_STEP BEGIN|^0 !?LPUB CALLOUT BEGIN|^0 !?LPUB INSERT");
  }
  Where walk    = onStepMeta ? topOfStep + 1 : topOfStep;
  Where lastPos = topOfStep;
  int  numLines = lpub->ldrawFile.size(walk.modelName);
  if (walk < numLines) {
    QString line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
    if (isScanPastGlobal && line.contains(endRx))
      return;
    for ( ++walk; walk < numLines; ++walk) {
      line = lpub->ldrawFile.readLine(walk.modelName,walk.lineNumber);
      lastPos = line.contains(lineRx) ? walk : lastPos;
      if ( ! line.contains(lineRx) && ! isHeader(line)) {
        topOfStep = lastPos;
        if (line.contains(endRx)) {
            if (! isScanPastGlobal)
              ++topOfStep;
          break;
        }
      }
    }
  }
}

// special case - return specified capture group in result
bool Gui::stepContains(Where &here, const QRegExp &lineRx, QString &result, int capGrp, bool displayModel) {
    bool found = false;
    if (Gui::stepContains(here,lineRx,displayModel))
        found = true;
    if (capGrp)
        result = lineRx.cap(capGrp).trimmed();
    return found;
}

// general case string
bool Gui::stepContains(Where &topOfStep, const QString &value)
{
    QRegExp lineRx(value, Qt::CaseInsensitive);
    return Gui::stepContains(topOfStep, lineRx);
}

// general case regex
bool Gui::stepContains(Where &topOfStep, const QRegExp &lineRx, bool displayModel)
{
  bool found = false;
  Where walk = topOfStep;
  LDrawFile &ldrawFile = lpub->ldrawFile;
  int  numLines = ldrawFile.size(walk.modelName);
  QRegExp endRx("^0\\s+STEP$|^0\\s+ROTSTEP|^0\\s+NOFILE$|^0\\s+FILE|^0\\s+!DATA");
  for (; walk < numLines; ++walk) {
    QString line = ldrawFile.readLine(walk.modelName,walk.lineNumber);
    if (displayModel) {
      if (line.contains(lineRx)) {
        // Can consolidate multiple illegal displayModel commands in a single Rx
        if ((found = lineRx.cap(1).contains("BEGIN SUB"))) {
          topOfStep = walk;
        }
      }
    } else if ((found = line.contains(lineRx))) {
      topOfStep = walk;
    }
    if (found || line.contains(endRx)) {
      break;
    }
  }
  if (topOfStep.modelIndex == BM_INVALID_INDEX) {
    topOfStep.setModelIndex(ldrawFile.getSubmodelIndex(topOfStep.modelName));
  }
  return found;
}
