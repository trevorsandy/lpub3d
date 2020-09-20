/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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

#include <QDialog>
#include <QMainWindow>
#include <QTextEdit>
#include <QTextCursor>
#include <QFileSystemWatcher>

#include "name.h"
#include "historylineedit.h"

class LDrawFile;
class Highlighter;
class QFindReplace;
class QFindReplaceCtrls;
class QLineNumberArea;
class QTextEditor;

class QString;
class QAction;
class QMenu;
class QPaintEvent;
class QResizeEvent;
class QSize;
class QCheckBox;

class QLineEdit;
class QPushButton;
class QLabel;
class QComboBox;


class QAbstractItemModel;
class QCompleter;
class QProgressBar;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class EditWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditWindow(QMainWindow *parent = nullptr, bool modelFileEdit = false);
    QToolBar *editToolBar;

protected:

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void readSettings();
    void writeSettings();

    QTextEditor  *_textEdit;
    Highlighter  *highlighter;
    QComboBox    *mpdCombo;
    QString       fileName;            // of model file currently being displayed
    int           fileOrderIndex;
    bool          _modelFileEdit;
    QString       _curSubFile;         // currently displayed submodel
    int           _saveSubfileIndex;
    QFileSystemWatcher fileWatcher;

    QMenu    *editMenu;
    QAction  *topAct;
    QAction  *bottomAct;
    QAction  *cutAct;
    QAction  *copyAct;
    QAction  *pasteAct;
    QAction  *redrawAct;
    QAction  *updateAct;
    QAction  *delAct;
    QAction  *selAllAct;
    QAction  *findAct;
    QAction  *toggleCmmentAct;
    QAction  *showAllCharsAct;

    QAction  *exitAct;
    QAction  *saveAct;
    QAction  *undoAct;
    QAction  *redoAct;

signals:
    void contentsChange(const QString &, int position, int charsRemoved, const QString &charsAdded);
    void refreshModelFileSig();
    void redrawSig();
    void updateSig();
    void enableWatcherSig();
    void disableWatcherSig();
    void updateDisabledSig(bool);
     void SelectedPartLinesSig(QVector<TypeLine>&, PartSource = EDITOR_LINE);

private slots:
    void contentsChange(int position, int charsRemoved, int charsAdded);
    bool saveFile();
    bool maybeSave();
    void redraw();
    void update(bool state);
    void enableSave();
    void highlightCurrentLine();
    void topOfDocument();
    void bottomOfDocument();
    void showAllCharacters();
    void mpdComboChanged(int index);
    void showContextMenu(const QPoint &pt);
    void updateSelectedParts();
    void closeEvent(QCloseEvent *event);

public slots:
    void displayFile(LDrawFile *, const QString &fileName);
    void modelFileChanged(const QString &fileName);
    void showLine(int);
    void highlightSelectedLines(QVector<int> &lines);
    void updateDisabled(bool);
    void disableActions();
    void pageUpDown(
      QTextCursor::MoveOperation op,
      QTextCursor::MoveMode      moveMode);

public:
    QTextEditor *textEdit() { return _textEdit; }
    bool modelFileEdit() { return _modelFileEdit; }

private:
    QAbstractItemModel *modelFromFile(const QString& fileName);
    QCompleter *completer;
};

extern class EditWindow *editWindow;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class QTextEditor : public QTextEdit
{
    Q_OBJECT

public:
    explicit QTextEditor(QWidget *parent = nullptr);

    ~QTextEditor()override{}

    void showAllCharacters(bool show);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void setIsUTF8(bool isUTF8) { _fileIsUTF8 = isUTF8; }
    bool getIsUTF8() { return _fileIsUTF8; }
    int getFirstVisibleBlockId();
    int lineNumberAreaWidth();
    QFindReplace *popUp;

    void setCompleter(QCompleter *c);
    void setCompleterMinChars(int min_chars);
    void setCompleterMaxSuggestions(int max);
    void setCompleterPrefix(const QString& prefix);

signals:
    void updateSelectedParts();

public slots:
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(QRectF /*rect_f*/);
    void updateLineNumberArea(int /*slider_pos*/);
    void updateLineNumberArea(); 
    void findDialog();
    void toggleComment();
    void showCharacters(
         QString findString,
         QString replaceString);
    void autocomplete(const QString &completion);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void focusInEvent(QFocusEvent *e) override;
    QString currentWord() const;

private:
    QString textUnderCursor() const;
    int wordStart() const;
    QCompleter *completer;
    int         completion_minchars;
    int         completion_max;
    QString     completion_prefix;
    bool        _fileIsUTF8;
    QWidget    *lineNumberArea;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class QLineNumberArea : public QWidget
{
public:
    QLineNumberArea(QTextEditor *editor) : QWidget(editor) {
        textEditor = editor;}

    QSize sizeHint() const {
        return QSize(textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event){
        textEditor->lineNumberAreaPaintEvent(event);
    }

private:
    QTextEditor *textEditor;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class QFindReplace : public QDialog
{
    Q_OBJECT

public:
    explicit QFindReplace(QTextEditor *textEdit, const QString &selectedText, QWidget *parent = nullptr);

protected slots:
    void popUpClose();

protected:
    QFindReplaceCtrls *find;
    QFindReplaceCtrls *findReplace;
    void readFindReplaceSettings(QFindReplaceCtrls *fr);
    void writeFindReplaceSettings(QFindReplaceCtrls *fr);
private:
    QAbstractItemModel *modelFromFile(const QString& fileName);
    QCompleter *completer;
};

class QFindReplaceCtrls : public QWidget
{
    Q_OBJECT

public:
    explicit QFindReplaceCtrls(QTextEditor *textEdit, QWidget *parent = nullptr);
    QTextEditor *_textEdit;
    HistoryLineEdit *textFind;
    HistoryLineEdit *textReplace;
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
