/****************************************************************************
**
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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
 * The editwindow is used to display all editable parameter files
 * and logs to the user.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef PARMSWINDOW_H
#define PARMSWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>

#ifndef WATCHER
#define WATCHER
#endif

class ParmsHighlighter;
class ParmEditor;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class ParmsWindow : public QMainWindow
{
    Q_OBJECT

public:
  ParmsWindow(QMainWindow *parent = nullptr);

protected:

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void readSettings();
    void writeSettings();

    ParmEditor       *_textEdit;
    bool              _fadeStepFile;
    bool              _fileModified;
    bool              _restartRequired;
    QString           title;
    ParmsHighlighter *highlighter;
    QString           fileName;  // file currently being displayed

    QMenu    *editMenu;
    QToolBar *editToolBar;
    QToolBar *undoRedoToolBar;

    QAction *exitAct;
    QAction *openAct;
    QAction *refreshAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *findAct;
    QAction *gotoLineAct;
    QAction *saveAct;
    QAction *saveCopyAsAct;
    QAction *topAct;
    QAction *bottomAct;
    QAction *delAct;
    QAction *systemEditorAct;
    QAction *selAllAct;
    QAction *showAllCharsAct;
    QAction *undoAct;
    QAction *redoAct;

private slots:
    void openFile();
    void gotoLine();
    void refreshFile();
    void enableSave();
    void toggleClear();
    void viewLogWindowSettings();
    bool maybeSave();
    bool saveFile(bool force = false);
    bool saveCopyAsFile();
    void topOfDocument();
    void bottomOfDocument();
    void systemEditor();
    void showAllCharacters();
    void showContextMenu(const QPoint &pt);
    void setSelectionHighlighter();
    void closeEvent(QCloseEvent *event) override;

public slots:
    void displayParmsFile(const QString &fileName);

public:
    ParmEditor *textEdit() { return _textEdit; }
    void setWindowTitle(const QString &title);
};

extern class ParmsWindow *parmsWindow;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class ParmEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit ParmEditor(QWidget *parent = nullptr);

    void showAllCharacters(bool show);
    void setIsUTF8(bool isUTF8) { _fileIsUTF8 = isUTF8; }
    bool getIsUTF8() { return _fileIsUTF8; }
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int  lineNumberAreaWidth();
    void parmsOpen(int &opt);
    bool parmsSave(int &opt);
    void gotoLine(int line);
    QWidget     *popUp;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &, int);
    void highlightCurrentLine();
    void findDialog();
    void showCharacters(
         QString findString,
         QString replaceString);

private:
    QWidget *lineNumberArea;
    bool _fileIsUTF8;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class ParmLineNumberArea : public QWidget
{
public:
    ParmLineNumberArea(ParmEditor *editor) : QWidget(editor)
    {
        textEditor = editor;
    }

    QSize sizeHint() const override
    {
        return QSize(textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        textEditor->lineNumberAreaPaintEvent(event);
    }

private:
    ParmEditor *textEditor;
};

#endif
