/****************************************************************************
**
** Copyright (C) 2022 - 2025 Trevor SANDY. All rights reserved.
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
 * The commandstextedit is used to edit LPUB meta commands.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef COMMANDSTEXTEDIT_H
#define COMMANDSTEXTEDIT_H

#include <QPlainTextEdit>

class QAbstractItemModel;
class QCompleter;
class QComboBox;
class QTextCursor;

class Highlighter;
class FindReplace;
class SnippetCompleter;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class CommandsTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CommandsTextEdit(QWidget *parent = nullptr);

    ~CommandsTextEdit()override{}

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void setAutoCompleter(QCompleter *completer);
    void setSnippetCompleter(SnippetCompleter *completer);
    void gotoLine(int line);

    FindReplace *popUp;

public slots:
    void setEditorFont();
    void setTextEditHighlighter();
    void setSelectionHighlighter();
    void setCommandText(const QString &text);

    void resizeEvent(QResizeEvent *e) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

    void insertCompletion(const QString &completion);
    void insertSnippet(const QString &completionPrefix, const QString &completion, int newCursorPos);
    void performCompletion();

    void enableActions(int /*lines*/, bool = false);

    void showContextMenu(const QPoint &pt);
    void showAllCharacters();
    void topOfDocument();
    void bottomOfDocument();
    void lineWrap();
    void gotoLine();

    void highlightCurrentLine();
    void findDialog();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    QAbstractItemModel *metaCommandModel(QObject *parent);
    QStringList extractDistinctWordsFromDocument() const;
    QStringList retrieveAllWordsFromDocument() const;
    template <class UnaryPredicate>
    QStringList filterWordList(const QStringList &words, UnaryPredicate predicate) const;
    QString retrieveTextUnderCursor() const;
    QString textUnderCursor() const;

    void drawLineEndMarker(QPaintEvent *e);
    void createActions();

    QCompleter        *ac;
    SnippetCompleter  *sc;
    QWidget           *lineNumberArea;
    Highlighter       *highlighter;

    QAction  *undoAct;
    QAction  *redoAct;
    QAction  *cutAct;
    QAction  *copyAct;
    QAction  *pasteAct;
    QAction  *findAct;
    QAction  *gotoLineAct;
    QAction  *lineWrapAct;
    QAction  *topAct;
    QAction  *bottomAct;
    QAction  *delAct;
    QAction  *selAllAct;
    QAction  *showAllCharsAct;

    bool      showHardLinebreaks;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CommandsTextEdit *editor) : QWidget(editor)
    {
        commandsTextEdit = editor;
    }

    QSize sizeHint() const override
    {
        return QSize(commandsTextEdit->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        commandsTextEdit->lineNumberAreaPaintEvent(event);
    }

private:
    CommandsTextEdit *commandsTextEdit;
};

#endif // COMMANDSTEXTEDIT_H
