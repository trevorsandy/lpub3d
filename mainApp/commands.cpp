/****************************************************************************  
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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

#include "commands.h"
#include "ldrawfiles.h"
#include "lpub.h"

InsertLineCommand::InsertLineCommand(
  LDrawFile     *_ldrawFile,
  const Where   &_here,
  const QString &_line,
  QUndoCommand *parent)
  : QUndoCommand(parent)
{
  setText("insertLine");
  ldrawFile  = _ldrawFile;
  here       = _here;
  line       = _line;
}

void InsertLineCommand::undo()
{
  ldrawFile->deleteLine(here.modelName,here.lineNumber);
  gui->showLine(here);
}

void InsertLineCommand::redo()
{
  ldrawFile->insertLine(here.modelName,here.lineNumber,line);
  gui->showLine(here);
  if (!isHeader(line))
     Gui::displayPage();
}

AppendLineCommand::AppendLineCommand(
  LDrawFile    *_ldrawFile,
  const Where  &_here,
  const QString &_line,
  QUndoCommand *parent)
  : QUndoCommand(parent)
{
  setText("appendLine");
  ldrawFile = _ldrawFile;
  line      = _line;
  here      = _here;
}

void AppendLineCommand::undo()
{
  ldrawFile->deleteLine(here.modelName,here.lineNumber+1);
  gui->showLine(here);
  Gui::displayPage();
}

void AppendLineCommand::redo()
{
  ldrawFile->insertLine(here.modelName,here.lineNumber+1,line);
  gui->showLine(here);
  Gui::displayPage();
}

DeleteLineCommand::DeleteLineCommand(
  LDrawFile    *_ldrawFile,
  const Where  &_here,
  QUndoCommand *parent)
  : QUndoCommand(parent)
{
  setText("deleteLine");
  ldrawFile = _ldrawFile;
  here      = _here;
}

void DeleteLineCommand::undo()
{
  ldrawFile->insertLine(here.modelName,here.lineNumber,deletedLine);
  gui->showLine(here);
  Gui::displayPage();
}

void DeleteLineCommand::redo()
{
  deletedLine = ldrawFile->readLine(here.modelName, here.lineNumber);
  ldrawFile->deleteLine(here.modelName,here.lineNumber);
  gui->showLine(here);
  Gui::displayPage();
}

ReplaceLineCommand::ReplaceLineCommand(
  LDrawFile     *_ldrawFile,
  const Where   &_here,
  const QString &_newLine,
  QUndoCommand  *parent)
  : QUndoCommand(parent)
{
  setText("replaceLine");
  ldrawFile  = _ldrawFile;
  here       = _here;
  newLine    = _newLine;
  oldLine    = ldrawFile->readLine(here.modelName,here.lineNumber);
}

void ReplaceLineCommand::undo()
{
  ldrawFile->replaceLine(here.modelName,here.lineNumber,oldLine);
  gui->showLine(here);
  Gui::displayPage();
}

void ReplaceLineCommand::redo()
{
  ldrawFile->replaceLine(here.modelName,
                         here.lineNumber,
                         newLine);
  gui->showLine(here);
  Gui::displayPage();
}

ContentsChangeCommand::ContentsChangeCommand(
  LDrawFile     *_ldrawFile,
  const QString &_modelName,
  int            _position,
  const QString &_removedChars,
  const QString &_addedChars,
  QUndoCommand  *parent)
  : QUndoCommand(parent)
{
  setText("commandEdit");
  ldrawFile    = _ldrawFile;
  modelName    = _modelName;
  position     = _position;
  removedChars = _removedChars;
  addedChars   = _addedChars;
  isRedo       = false;
}

void ContentsChangeCommand::redo()
{
  ldrawFile->changeContents(
    modelName,
    position,
    removedChars.size(),
    addedChars);

  if ( !isRedo) {
    isRedo = true;
  } else {
    Gui::maxPages = -1;
    Gui::displayPage();
  }
}

void ContentsChangeCommand::undo()
{
  Gui::maxPages = -1;

  ldrawFile->changeContents(
    modelName,
    position,
    addedChars.size(),
    removedChars);
  Gui::displayPage();
}
