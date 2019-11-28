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

QString Gui::topLevelFile()
{
  return ldrawFile.topLevelFile();
}

void Gui::insertLine(const Where &here, const QString &line, QUndoCommand *parent)
{
  if (ldrawFile.contains(here.modelName)) {
    undoStack->push(new InsertLineCommand(&ldrawFile,here,line,parent));
  }
}

void Gui::appendLine(const Where &here, const QString &line, QUndoCommand *parent)
{
  if (ldrawFile.contains(here.modelName)) {
    undoStack->push(new AppendLineCommand(&ldrawFile,here,line,parent));
  }
}
      
void Gui::replaceLine(const Where &here, const QString &line, QUndoCommand *parent)
{
  if (ldrawFile.contains(here.modelName) && 
      here.lineNumber < ldrawFile.size(here.modelName)) {

    undoStack->push(new ReplaceLineCommand(&ldrawFile,here,line,parent));
  }
}

void Gui::deleteLine(const Where &here, QUndoCommand *parent)
{
  if (ldrawFile.contains(here.modelName) && 
      here.lineNumber < ldrawFile.size(here.modelName)) {
    undoStack->push(new DeleteLineCommand(&ldrawFile,here,parent));
  }
}

QString Gui::readLine(const Where &here)
{
  return ldrawFile.readLine(here.modelName,here.lineNumber);
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
  displayPage();
}

void Gui::contentsChange(
  const QString &fileName,
  int      position,
  int      _charsRemoved,
  const QString &charsAdded)
{
  QString  charsRemoved;

  /* Calculate the characters removed from the LDrawFile */

  if (_charsRemoved && ldrawFile.contains(fileName)) {

    QString contents = ldrawFile.contents(fileName).join("\n");

    charsRemoved = contents.mid(position,_charsRemoved);
  }
  
  undoStack->push(new ContentsChangeCommand(&ldrawFile,
                                            fileName,
                                            position,
                                            charsRemoved,
                                            charsAdded));
}

void Gui::undo()
{
  macroNesting++;
  undoStack->undo();
  macroNesting--;
  displayPage();
}

void Gui::redo()
{
  macroNesting++;
  undoStack->redo();
  macroNesting--;
  displayPage();
}

void Gui::canRedoChanged(bool enabled)
{
  redoAct->setEnabled(enabled);
}

void Gui::canUndoChanged(bool enabled)
{
  undoAct->setEnabled(enabled);
}
void Gui::cleanChanged(bool cleanState)
{
  saveAct->setDisabled( cleanState);
}

void Gui::normalizeHeader(const Where &here)
{
  Where current = here;

  int numLines = ldrawFile.size(current.modelName);
  for ( ; current.lineNumber < numLines; current.lineNumber++) {
      QString line = gui->readLine(current);
      int p;
      for (p = 0; p < line.size(); ++p) {
          if (line[p] != ' ') {
              break;
            }
        }
      if (line[p] >= '1' && line[p] <= '5') {
          if (current.lineNumber == 0) {
              QString attribute = QString("0 Name: %1").arg(current.modelName);
              gui->insertLine(current,attribute,nullptr);
              attribute = QString("0 Author: %1").arg(Preferences::defaultAuthor.isEmpty() ? "Undefined" :
                                                      Preferences::defaultAuthor);
              current.lineNumber++;
              gui->insertLine(current,attribute,nullptr);
            } else if (current > 0) {
              --current;
            }
          break;
        } else if ( ! isHeader(line)) {
          if (current.lineNumber != 0) {
              --current;
              break;
            }
        }
    }
}

void Gui::scanPast(Where &topOfStep, const QRegExp &lineRx)
{
  Where walk     = topOfStep + 1;
  Where lastPos  = topOfStep;
  int  numLines  = gui->subFileSize(walk.modelName);
  if (walk < numLines) {
    QString line = gui->readLine(walk);
    if (line.contains(lineRx) || isHeader(line)) {
      for ( ++walk; walk < numLines; ++walk) {
        line = gui->readLine(walk);
        lastPos = line.contains(lineRx) ? walk : lastPos;
        if ( ! line.contains(lineRx) && ! isHeader(line)) {
          topOfStep = lastPos;
          if (line.contains("0 STEP") || line.contains("0 ROTSTEP")){
            break;
          }
        }
      }
    }
  }
}

bool Gui::stepContains(Where &here, QRegExp &lineRx, QString &result, int capGrp) {
    bool found = false;
    if ((found = stepContains(here,lineRx))){
        if (capGrp)
            result = lineRx.cap(capGrp);
        else
            result = "true";
    }
    return found;
}

bool Gui::stepContains(Where &topOfStep, QRegExp &lineRx)
{
  bool found = false;
  Where walk    = topOfStep;
  int  numLines = gui->subFileSize(walk.modelName);
  for (; walk < numLines; ++walk) {
    QString line = gui->readLine(walk);
    if ((found = line.contains(lineRx)))
      topOfStep = walk;
    if (found || line.contains("0 STEP") || line.contains("0 ROTSTEP"))
      break;
  }
  return found;
}
