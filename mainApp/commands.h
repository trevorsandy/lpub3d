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
#ifndef commands_h
#define commands_h

/***************************************************************************
 *
 * This is the design for the undo/redo mechanism, as well as the activity
 * trace that lets users send debuggers the activity trace, and everything
 * that the user did can be recreated
 *
 * Gui has a QUndoStack named undoStack.  All the undo/redo things are
 * kept on the undo/redo stack.
 *
 * All the user changes that cause the LDraw file(s) to get updated are
 * stored on the undo/redo stack.
 *
 * All of these changes are implemented in metaitem.cpp. They issue all the
 * commands using readLine, insertLine, replaceLine and deleteLine.  These
 * activities all have direct maps to Undo/Redo command types and are easier
 * to track.
 *
 * The special case has to do with users manually editing the LDraw file
 * in the editWindow's textEdit.  Changes to the document can show up as
 * document->contentsChange(int position, int charsRemoved, int charsAdded);
 *
 * By itself, this information is not enough to be able to undo and redo.
 * The contentsChange event is handled locally by the editWindow, and
 * converted to 
 *
 * signal void contentsChange(int position, 
 *                            int charsRemoved, 
 *                            QString charsAdded);
 *
 * This contentsChange signal is connected to Gui.  To be able to undo/redo
 * gui needs to change int charsRemoved to QString charsRemoved, at which
 * time it can be put on the undo/redo stack.
 *
 * contentsChange also need to be reflected to the ldraw files, so that the
 * LDraw file matches the textEdit->document context.
 *
 * Changes made by metaitem.cpp that change the file being displayed by
 * textEdit, need to be reflected in the textEdit document.  Anytime a
 * change is made to the ldraw file, and that LDraw file is being displayed
 * by the editWindow, the gui signals editWindow using this:
 *
 * void displayFile(LDrawFile *ldrawFile, QString subFile);
 *
 * The editWindow disconnects contentsChange events, updates the document's
 * plain text, and then reconnects contentsChange.
 * 
 ***************************************************************************/

#include <QUndoCommand>
#include "where.h"

class LDrawFile;

class InsertLineCommand : public QUndoCommand

{
public:

  InsertLineCommand(LDrawFile     *ldrawFile,
                    const Where   &here,
                    const QString &line,
                    QUndoCommand  *parent = 0);

  void undo();
  void redo();

private:

  LDrawFile *ldrawFile;
  Where      here;
  QString    line;

};

class AppendLineCommand : public QUndoCommand

{
public:

  AppendLineCommand(LDrawFile     *ldrawFile,
                    const Where   &here,
                    const QString &line,
                    QUndoCommand  *parent = 0);

  void undo();
  void redo();

private:

  LDrawFile *ldrawFile;
  Where      here;
  QString    line;

};

class DeleteLineCommand : public QUndoCommand

{
public:

  DeleteLineCommand(LDrawFile    *ldrawFile,
                    const Where  &here,
                    QUndoCommand *parent = 0);

  void undo();
  void redo();

private:

  LDrawFile *ldrawFile;
  Where      here;
  QString    deletedLine;

};

class ReplaceLineCommand : public QUndoCommand

{
public:

  ReplaceLineCommand(LDrawFile     *ldrawFile,
                     const Where   &here,
                     const QString &newLine,
                     QUndoCommand  *parent = 0);

  void undo();
  void redo();

private:

  LDrawFile *ldrawFile;
  Where      here;
  QString    newLine;
  QString    oldLine;

};


class ContentsChangeCommand : public QUndoCommand

{
public:

  ContentsChangeCommand(LDrawFile     *ldrawFile,

                        const QString &modelName,
                        int            position,
                        const QString &removedChars,
                        const QString &addedChars,
                        QUndoCommand  *parent = 0);

  void undo();
  void redo();

private:

  LDrawFile *ldrawFile;
  QString    modelName;
  int        position;
  QString    removedChars;
  QString    addedChars;
  bool       isRedo;
};



#endif

