/****************************************************************************
**
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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

#include <QDialog>
#include <QMainWindow>
#include <QPlainTextEdit>

#ifndef WATCHER
#define WATCHER
#endif

class LDrawFile;
class ParmsHighlighter;
class ParmFindReplace;
class ParmFindReplaceCtrls;
class ParmLineNumberArea;
class ParmEditor;

class QString;
class QAction;
class QMenu;

class QPlainTextEdit;
class QPaintEvent;
class QResizeEvent;
class QSize;
class QCheckBox;

class QLineEdit;
class QPushButton;
class QLabel;


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
    QAction  *exitAct;
    QAction  *openAct;
    QAction  *refreshAct;
    QAction  *cutAct;
    QAction  *copyAct;
    QAction  *pasteAct;
    QAction  *saveAct;
    QAction  *saveCopyAsAct;
    QAction  *undoAct;
    QAction  *redoAct;
    QAction  *delAct;
    QAction  *systemeditorAct;
    QAction  *selAllAct;
    QAction  *gotoLineAct;
    QAction  *findAct;
    QAction  *topAct;
    QAction  *bottomAct;
    QAction  *showAllCharsAct;

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
    void systemeditor();
    void showAllCharacters();
    void showContextMenu(const QPoint &pt);
    void setSelectionHighlighter();
    void closeEvent(QCloseEvent *event);

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
    void resizeEvent(QResizeEvent *event);

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

    QSize sizeHint() const
    {
        return QSize(textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        textEditor->lineNumberAreaPaintEvent(event);
    }

private:
    ParmEditor *textEditor;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class ParmFindReplace : public QDialog
{
    Q_OBJECT

public:
    explicit ParmFindReplace(ParmEditor *textEdit, const QString &selectedText, QWidget *parent = nullptr);

protected slots:
    void popUpClose();

protected:
    ParmFindReplaceCtrls *find;
    ParmFindReplaceCtrls *findReplace;
    void readFindReplaceSettings(ParmFindReplaceCtrls *fr);
    void writeFindReplaceSettings(ParmFindReplaceCtrls *fr);
};

class ParmFindReplaceCtrls : public QWidget
{
    Q_OBJECT

public:
    explicit ParmFindReplaceCtrls(ParmEditor *textEdit, QWidget *parent = nullptr);
    ParmEditor  *_textEdit;
    QLineEdit   *textFind;
    QLineEdit   *textReplace;
    QLabel      *labelMessage;

    QPushButton *buttonFind;
    QPushButton *buttonFindNext;
    QPushButton *buttonFindPrevious;
    QPushButton *buttonFindAll;
    QPushButton *buttonFindClear;

    QPushButton *buttonReplace;
    QPushButton *buttonReplaceAndFind;
    QPushButton *buttonReplaceAll;
    QPushButton *buttonReplaceClear;

    QLabel      *label;

    QCheckBox   *checkboxCase;
    QCheckBox   *checkboxWord;
    QCheckBox   *checkboxRegExp;

    QPushButton *buttonCancel;
    bool        _findall;

public slots:
    void find(int direction = 0);

signals:
    void popUpClose();

protected slots:
    void findInText();
    void findInTextNext();
    void findInTextPrevious();
    void findInTextAll();

    void findClear();

    void replaceInText();
    void replaceInTextFind();
    void replaceInTextAll();

    void replaceClear();

    void textFindChanged();
    void textReplaceChanged();
    void validateRegExp(const QString &text);
    void regexpSelected(bool sel);

protected:
    void disableButtons();
    void showError(const QString &error);
    void showMessage(const QString &message);
};

#endif
