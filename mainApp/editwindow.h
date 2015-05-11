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

#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include <QMainWindow>
#include <QTextCursor>

class QTextEdit;
class LDrawFile;
class Highlighter;
class QString;
class QAction;
class QMenu;

class EditWindow : public QMainWindow
{
    Q_OBJECT

public:
    EditWindow();

protected:

private:
    void createActions();
    void createMenus();
    void createToolBars();

    QDockWidget *dock1;
    QDockWidget *dock2;

    QTextEdit   *_textEdit;
    Highlighter *highlighter;
    QString      fileName;  // of file currently being displayed

    QMenu    *editMenu;
    QToolBar *editToolBar;
    QAction  *cutAct;
    QAction  *copyAct;
    QAction  *pasteAct;
    QAction  *redrawAct;

signals:
    void contentsChange(const QString &, int position, int charsRemoved, const QString &charsAdded);
    void redrawSig();

private slots:
    void contentsChange(int position, int charsRemoved, int charsAdded);
    // Maybe this helps resizing the editwindow (Jaco)
    void redraw();

public slots:
    void displayFile(LDrawFile *, const QString &fileName);
    void showLine(int);
    void pageUpDown(
      QTextCursor::MoveOperation op,
      QTextCursor::MoveMode      moveMode);

public:
    QTextEdit *textEdit() { return _textEdit; }
};

#endif
