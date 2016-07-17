/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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
 * The editwindow is used to display the freeform annotations
 * and colour parts fade listings to the user.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef PARMSWINDOW_H
#define PARMSWINDOW_H

#include "version.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include <QMainWindow>
#include <QTextCursor>

class QPlainTextEdit;
class LDrawFile;
class ParmsHighlighter;
class QString;
class QAction;
class QMenu;
class QUndoStack;

class QPaintEvent;
class QResizeEvent;
class QSize;

class LineNumberArea;
class TextEditor;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class ParmsWindow : public QMainWindow
{
    Q_OBJECT

public:
    ParmsWindow(QMainWindow *parent = 0);

protected:

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void readSettings();
    void writeSettings();

    TextEditor       *_textEdit;
    bool              _parmsChanged;
    QString           title;
    ParmsHighlighter *highlighter;
    QString           fileName;  // file currently being displayed

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
    QAction  *findAct;

signals:
    bool fileModified(bool);

private slots:
    void enableSave();
    bool maybeSave();
    bool saveFile();
    void closeEvent(QCloseEvent *event);

public slots:
    void displayParmsFile(const QString &fileName);

public:
    TextEditor *textEdit() { return _textEdit; }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit TextEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void parmsOpen(int &opt);
    bool parmsSave(int &opt);
    QLineEdit   *textFind;
    QLabel      *labelMessage;
    QPushButton *buttonFind;
    QPushButton *buttonFindNext;
    QPushButton *buttonFindPrevious;
    QGridLayout *layout;
    QWidget     *popUp;

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void findDialog();
    void findInText();
    void findInTextNext();
    void findInTextPrevious();

private:
    QWidget *lineNumberArea;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEditor *editor) : QWidget(editor) {
        textEditor = editor;
    }

    QSize sizeHint() const {
        return QSize(textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        textEditor->lineNumberAreaPaintEvent(event);
    }

private:
    TextEditor *textEditor;
};

#endif
