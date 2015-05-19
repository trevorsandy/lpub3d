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

#ifndef PARMSWINDOW_H
#define PARMSWINDOW_H

#define WINDOW   "MainWindow"

#include <QMainWindow>
#include <QTextCursor>

class QTextEdit;
class LDrawFile;
class Highlighter;
class QString;
class QAction;
class QMenu;
class QUndoStack;

class ParmsWindow : public QMainWindow
{
    Q_OBJECT

public:
    ParmsWindow();

protected:

private:
    void createActions();
    void createMenus();
    void createToolBars();

    QTextEdit   *_textEdit;
    Highlighter *highlighter;
    QString      fileName;  // file currently being displayed

    QMenu    *editMenu;
    QToolBar *editToolBar;
    QToolBar *undoRedoToolBar;
    QAction  *cutAct;
    QAction  *copyAct;
    QAction  *pasteAct;
    QAction  *saveAct;
    QAction  *undoAct;
    QAction  *redoAct;
    QAction  *delAct;
    QAction  *selAllAct;

signals:

private slots:
    bool maybeSave();
    bool saveFile();
    void closeEvent(QCloseEvent *event);

public slots:
    void displayParmsFile(const QString &fileName);
    void pageUpDown(
      QTextCursor::MoveOperation op,
      QTextCursor::MoveMode      moveMode);

public:
    QTextEdit *textEdit() { return _textEdit; }
};

#endif
