/****************************************************************************
**
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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
class FindReplace;
class FindReplaceCtrls;
class LineNumberArea;
class TextEditor;

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

    TextEditor       *_textEdit;
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
    QAction  *selAllAct;
    QAction  *findAct;
    QAction  *topAct;
    QAction  *bottomAct;
    QAction  *showAllCharsAct;

private slots:
    void openFile();
    void refreshFile();
    void enableSave();
    void toggleClear();
    void viewLogWindowSettings();
    bool maybeSave();
    bool saveFile(bool force = false);
    bool saveCopyAsFile();
    void topOfDocument();
    void bottomOfDocument();
    void showAllCharacters();
    void showContextMenu(const QPoint &pt);
    void closeEvent(QCloseEvent *event);

public slots:
    void displayParmsFile(const QString &fileName);

public:
    TextEditor *textEdit() { return _textEdit; }
    void setWindowTitle(const QString &title);
};

extern class ParmsWindow *parmsWindow;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit TextEditor(QWidget *parent = nullptr);

    void showAllCharacters(bool show);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void setIsUTF8(bool isUTF8) { _fileIsUTF8 = isUTF8; }
    bool getIsUTF8() { return _fileIsUTF8; }
    int lineNumberAreaWidth();
    void parmsOpen(int &opt);
    bool parmsSave(int &opt);
    QWidget     *popUp;

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void findDialog();
    void showCharacters(
         QString findString,
         QString replaceString);

private:
    bool _fileIsUTF8;
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class FindReplace : public QDialog
{
    Q_OBJECT

public:
    explicit FindReplace(TextEditor *textEdit, const QString &selectedText, QWidget *parent = nullptr);

protected slots:
    void popUpClose();

protected:
    FindReplaceCtrls *find;
    FindReplaceCtrls *findReplace;
    void readFindReplaceSettings(FindReplaceCtrls *fr);
    void writeFindReplaceSettings(FindReplaceCtrls *fr);
};

class FindReplaceCtrls : public QWidget
{
    Q_OBJECT

public:
    explicit FindReplaceCtrls(TextEditor *textEdit, QWidget *parent = nullptr);
    TextEditor *_textEdit;
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
