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

#include <QtWidgets>

#include <QCompleter>
#include <QApplication>
#include <QTextBlock>
#include <QTextStream>
#include <QAbstractItemView>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QKeyEvent>
#include <QScrollBar>

#include "commandstextedit.h"
#include "findreplace.h"
#include "highlighter.h"

#include "declarations.h"
#include "version.h"
#include "lpub_preferences.h"
#include "lpub_object.h"
#include "lpub_qtcompat.h"

#include <jsonfile.h>
#include <commands/snippets/jsonsnippettranslatorfactory.h>
#include <commands/snippets/snippetcollection.h>
#include <commands/snippets/snippetcompleter.h>

class ScrollBarFix : public QScrollBar
{
public:
    ScrollBarFix(Qt::Orientation orient, QWidget *parent=0)
        : QScrollBar(orient, parent) {}

protected:
    void sliderChange(SliderChange change) {
        if (signalsBlocked() && change == QAbstractSlider::SliderValueChange)
            blockSignals(false);
        QScrollBar::sliderChange(change);
    }
};

CommandsTextEdit::CommandsTextEdit(QWidget *parent) :
    QPlainTextEdit(parent),
    popUp(nullptr),
    ac(nullptr),
    sc(nullptr),
    lineNumberArea(new LineNumberArea(this)),
    showHardLinebreaks(false)
{
    QPalette lineNumberPalette = lineNumberArea->palette();
    lineNumberPalette.setCurrentColorGroup(QPalette::Active);
    lineNumberPalette.setColor(QPalette::Text,QColor(Qt::darkGray));
    lineNumberPalette.setColor(QPalette::Highlight,QColor(Qt::magenta));
    if (Preferences::darkTheme) {
        lineNumberPalette.setColor(QPalette::Text,QColor(Qt::darkGray).darker(150));
        lineNumberPalette.setColor(QPalette::Window,QColor(Preferences::themeColors[THEME_DARK_EDIT_MARGIN]));
    } else
        lineNumberPalette.setColor(QPalette::Window,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]).lighter(130));

    lineNumberArea->setPalette(lineNumberPalette);

    setAutoCompleter(new QCompleter(metaCommandModel(this), this));
    setSnippetCompleter(new SnippetCompleter(lpub->snippetCollection, this));

    setEditorFont();

    setVerticalScrollBar(new ScrollBarFix(Qt::Vertical, this));

    setLineWrapMode(QPlainTextEdit::NoWrap);

    setContextMenuPolicy(Qt::CustomContextMenu);

    createActions();

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));

    connect(this, SIGNAL(blockCountChanged(int)),
            this, SLOT(  updateLineNumberAreaWidth(int)));

    connect(this, SIGNAL(blockCountChanged(int)),
            this, SLOT(  enableActions(int)));

    connect(this, SIGNAL(updateRequest(QRect, int)),
            this, SLOT(  updateLineNumberArea(QRect, int)));

    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(  highlightCurrentLine()));

    updateLineNumberAreaWidth(0);

    QAction * actionComplete = new QAction(tr("Snippet Completer"), this);
    actionComplete->setObjectName("snippetCompleterAct.7");
    actionComplete->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Space));
    lpub->actions.insert(actionComplete->objectName(), Action(QStringLiteral("Edit.Snippet Completer"), actionComplete));
    addAction(actionComplete);
    connect(actionComplete, SIGNAL(triggered()),
            this,           SLOT(  performCompletion()));

    setTextEditHighlighter();
}

void CommandsTextEdit::createActions()
{
    cutAct = new QAction(QIcon(":/resources/cut.png"), tr("Cu&t"), this);
    cutAct->setObjectName("cutAct.7");
    cutAct->setShortcut(QStringLiteral("Ctrl+X"));
    cutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard - Ctrl+X"));
    lpub->actions.insert(cutAct->objectName(), Action(QStringLiteral("Edit.Cut"), cutAct));
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

    copyAct = new QAction(QIcon(":/resources/copy.png"), tr("&Copy"), this);
    copyAct->setObjectName("copyAct.7");
    copyAct->setShortcut(QStringLiteral("Ctrl+C"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard - Ctrl+C"));
    lpub->actions.insert(copyAct->objectName(), Action(QStringLiteral("Edit.Copy"), copyAct));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/resources/paste.png"), tr("&Paste"), this);
    pasteAct->setObjectName("pasteAct.7");
    pasteAct->setShortcut(QStringLiteral("Ctrl+V"));
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection - Ctrl+V"));
    lpub->actions.insert(pasteAct->objectName(), Action(QStringLiteral("Edit.Paste"), pasteAct));
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

    findAct = new QAction(QIcon(":/resources/find.png"), tr("&Find"), this);
    findAct->setObjectName("findAct.7");
    findAct->setShortcut(QStringLiteral("Ctrl+F"));
    findAct->setStatusTip(tr("Find object - Ctrl+F"));
    lpub->actions.insert(findAct->objectName(), Action(QStringLiteral("File.Find"), findAct));
    connect(findAct, SIGNAL(triggered()), this, SLOT(findDialog()));

    gotoLineAct = new QAction(QIcon(":/resources/gotoline.png"), tr("&Go To Line"), this);
    gotoLineAct->setObjectName("gotoLineAct.7");
    gotoLineAct->setShortcut(QStringLiteral("Ctrl+G"));
    gotoLineAct->setStatusTip(tr("Go to line... - Ctrl+G"));
    lpub->actions.insert(gotoLineAct->objectName(), Action(QStringLiteral("File.Go To Line"), gotoLineAct));
    connect(gotoLineAct, SIGNAL(triggered()), this, SLOT(gotoLine()));

    lineWrapAct = new QAction(QIcon(":/resources/linewrap16.png"), tr("&Line Wrap"), this);
    lineWrapAct->setObjectName("lineWrapAct.7");
    lineWrapAct->setShortcut(QStringLiteral("Ctrl+W"));
    lineWrapAct->setStatusTip(tr("Allow long sentences to be broken and wrap onto the next line - Ctrl+W"));
    lineWrapAct->setCheckable(true);
    lpub->actions.insert(lineWrapAct->objectName(), Action(QStringLiteral("File.Line Wrap"), lineWrapAct));
    connect(lineWrapAct, SIGNAL(triggered()), this, SLOT(lineWrap()));

    topAct = new QAction(QIcon(":/resources/topofdocument.png"), tr("Top of Document"), this);
    topAct->setObjectName("topAct.7");
    topAct->setShortcut(QStringLiteral("Ctrl+T"));
    topAct->setStatusTip(tr("Go to the top of document - Ctrl+T"));
    lpub->actions.insert(topAct->objectName(), Action(QStringLiteral("File.Top"), topAct));
    connect(topAct, SIGNAL(triggered()), this, SLOT(topOfDocument()));

    bottomAct = new QAction(QIcon(":/resources/bottomofdocument.png"), tr("Bottom of Document"), this);
    bottomAct->setObjectName("bottomAct.7");
    bottomAct->setShortcut(QStringLiteral("Ctrl+B"));
    bottomAct->setStatusTip(tr("Go to the bottom of document - Ctrl+B"));
    lpub->actions.insert(bottomAct->objectName(), Action(QStringLiteral("File.Bottom"), bottomAct));
    connect(bottomAct, SIGNAL(triggered()), this, SLOT(bottomOfDocument()));

    delAct = new QAction(QIcon(":/resources/delete.png"), tr("&Delete"), this);
    delAct->setObjectName("delAct.7");
    delAct->setShortcut(QStringLiteral("DEL"));
    delAct->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    delAct->setStatusTip(tr("Delete the selection - DEL"));
    lpub->actions.insert(delAct->objectName(), Action(QStringLiteral("File.Delete"), delAct));
    connect(delAct, SIGNAL(triggered()), this, SLOT(cut()));

    selAllAct = new QAction(QIcon(":/resources/selectall.png"), tr("&Select All"), this);
    selAllAct->setObjectName("selAllAct.7");
    selAllAct->setShortcut(QStringLiteral("Ctrl+A"));
    selAllAct->setStatusTip(tr("Select all page content - Ctrl+A"));
    lpub->actions.insert(selAllAct->objectName(), Action(QStringLiteral("File.Delete"), selAllAct));
    connect(selAllAct, SIGNAL(triggered()), this, SLOT(selectAll()));

    showAllCharsAct = new QAction(QIcon(":/resources/showallcharacters.png"), tr("Show All Characters"), this);
    showAllCharsAct->setObjectName("showAllCharsAct.7");
    showAllCharsAct->setShortcut(QStringLiteral("Ctrl+J"));
    showAllCharsAct->setStatusTip(tr("Show all characters - Ctrl+J"));
    showAllCharsAct->setCheckable(true);
    lpub->actions.insert(showAllCharsAct->objectName(), Action(QStringLiteral("File.Show All Characters"), showAllCharsAct));
    connect(showAllCharsAct, SIGNAL(triggered()), this, SLOT(showAllCharacters()));

    undoAct = new QAction(QIcon(":/resources/editundo.png"), tr("Undo"), this);
    undoAct->setObjectName("undoAct.7");
    undoAct->setShortcut(QStringLiteral("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last change - Ctrl+Z"));
    undoAct->setEnabled(false);
    lpub->actions.insert(undoAct->objectName(), Action(QStringLiteral("Edit.Undo"), undoAct));
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));
    redoAct = new QAction(QIcon(":/resources/editredo.png"), tr("Redo"), this);
    redoAct->setObjectName("redoAct.7");
#ifdef __APPLE__
    redoAct->setShortcut(QStringLiteral("Ctrl+Shift+Z"));
    redoAct->setStatusTip(tr("Redo last change - Ctrl+Shift+Z"));
#else
    redoAct->setShortcut(QStringLiteral("Ctrl+Y"));
    redoAct->setStatusTip(tr("Redo last change - Ctrl+Y"));
#endif
    redoAct->setEnabled(false);
    lpub->actions.insert(redoAct->objectName(), Action(QStringLiteral("Edit.Redo"), redoAct));
    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

    enableActions(0, false);

    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    delAct->setEnabled(false);
    undoAct->setEnabled(false);
    redoAct->setEnabled(false);

    connect(this,    SIGNAL(copyAvailable(bool)),
            cutAct,  SLOT(  setEnabled(bool)));

    connect(this,    SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(  setEnabled(bool)));

    connect(this,    SIGNAL(copyAvailable(bool)),
            delAct,  SLOT(  setEnabled(bool)));

    connect(this,    SIGNAL(undoAvailable(bool)),
            undoAct, SLOT(  setEnabled(bool)));

    connect(this,    SIGNAL(redoAvailable(bool)),
            redoAct, SLOT(  setEnabled(bool)));
}

void CommandsTextEdit::setCommandText(const QString &content)
{
    blockSignals(true);
    setPlainText(content);
    blockSignals(false);
    document()->setModified(false);
    enableActions(document()->blockCount(), true);
}

void CommandsTextEdit::enableActions(int lines, bool b)
{
    bool enable = b && lines > 1;

    selAllAct->setEnabled(b);
    findAct->setEnabled(b);
    gotoLineAct->setEnabled(enable);
    topAct->setEnabled(enable);
    bottomAct->setEnabled(enable);
    lineWrapAct->setEnabled(b);
    showAllCharsAct->setEnabled(b);
}

void CommandsTextEdit::showContextMenu(const QPoint &pt)
{
    QMenu *menu = this->createStandardContextMenu();
    menu->addSeparator();
    menu->addAction(findAct);
    menu->addAction(gotoLineAct);
    menu->addAction(topAct);
    menu->addAction(bottomAct);
    menu->addAction(lineWrapAct);
    menu->addAction(showAllCharsAct);
    menu->addSeparator();
    menu->exec(this->mapToGlobal(pt));
    delete menu;
}

void CommandsTextEdit::showAllCharacters() {

    const bool enabled = showAllCharsAct->isChecked();

    showHardLinebreaks = enabled;

    QTextOption textOption = document()->defaultTextOption();
    QTextOption::Flags optionFlags = textOption.flags();
    if (enabled) {
        optionFlags |= QTextOption::ShowLineAndParagraphSeparators;
        optionFlags |= QTextOption::ShowTabsAndSpaces;
    } else {
        optionFlags &= ~QTextOption::ShowLineAndParagraphSeparators;
        optionFlags &= ~QTextOption::ShowTabsAndSpaces;
    }
    textOption.setFlags(optionFlags);

    document()->setDefaultTextOption(textOption);

    // repaint
    viewport()->update();
}

void CommandsTextEdit::gotoLine()
{
    const int STEP = 1;
    const int MIN_VALUE = 1;

    QTextCursor cursor = textCursor();
    int currentLine = cursor.blockNumber()+1;
    int maxValue = document()->blockCount();

    bool ok;
    int line = QInputDialog::getInt(this, tr("Go to..."),
                                          tr("Line: ", "Line number in the parameter editor"), currentLine, MIN_VALUE, maxValue, STEP, &ok);
    if (!ok) return;

    QTextCursor goToCursor(document()->findBlockByNumber(line-1));
    setTextCursor(goToCursor);
}

void CommandsTextEdit::lineWrap()
{
    if (lineWrapAct->isChecked())
        setLineWrapMode(QPlainTextEdit::WidgetWidth);
    else
        setLineWrapMode(QPlainTextEdit::NoWrap);
}

void CommandsTextEdit::topOfDocument() {
    moveCursor(QTextCursor::Start);
}

void CommandsTextEdit::bottomOfDocument() {
    moveCursor(QTextCursor::End);
}

QAbstractItemModel *CommandsTextEdit::metaCommandModel(QObject *parent)
{
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif

    QStandardItemModel *model = new QStandardItemModel(parent);
    QString commandIcon = QStringLiteral(":/resources/command16.png");
    if (Preferences::darkTheme)
        commandIcon = QStringLiteral(":/resources/command_dark16.png");
    foreach (const QString &keyword, lpub->metaKeywords)
        model->appendRow(new QStandardItem(QIcon(commandIcon), keyword));

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    return model;
}

void CommandsTextEdit::setSelectionHighlighter()
{
    QColor highlightColor;
    if (Preferences::darkTheme) {
        highlightColor = QColor(Preferences::themeColors[THEME_DARK_LINE_SELECT]);
        highlightColor.setAlpha(50);
    } else {
        highlightColor = QColor(Preferences::themeColors[THEME_DEFAULT_LINE_SELECT]);
        highlightColor.setAlpha(30);
    }

    auto highlightPalette = palette();
    highlightPalette.setBrush(QPalette::Highlight, highlightColor);
    highlightPalette.setBrush(QPalette::HighlightedText, QBrush(Qt::NoBrush));

    setPalette(highlightPalette);
}

void CommandsTextEdit::setTextEditHighlighter()
{
    highlighter = new Highlighter(document());

    setSelectionHighlighter();
}

// Snippet commands
void CommandsTextEdit::setSnippetCompleter(SnippetCompleter *completer)
{
    if (sc)
        sc->disconnect(this);

    sc = completer;

    if (!sc)
        return;

    connect(completer,  SIGNAL(snippetSelected(QString,QString, int)),
            this,       SLOT(  insertSnippet(QString,QString, int)));
}

void CommandsTextEdit::performCompletion()
{
    if (!sc)
        return;

   QRect popupRect = cursorRect();
   popupRect.setLeft(popupRect.left() + lineNumberAreaWidth());

   QStringList words = extractDistinctWordsFromDocument();
   sc->performCompletion(retrieveTextUnderCursor(), words, popupRect);
}

void CommandsTextEdit::insertSnippet(const QString &completionPrefix, const QString &completion, int newCursorPos)
{
   QTextCursor cursor = this->textCursor();

   // select the completion prefix
   cursor.clearSelection();
   cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, completionPrefix.length());

   int pos = cursor.position();

   // replace completion prefix with snippet
   cursor.insertText(completion);

   // move cursor to requested position
   cursor.setPosition(pos);
   cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, newCursorPos);

   this->setTextCursor(cursor);
}

QStringList CommandsTextEdit::extractDistinctWordsFromDocument() const
{
    QStringList allWords = retrieveAllWordsFromDocument();
    allWords.removeDuplicates();

    QStringList words = filterWordList(allWords, [](const QString &word) {
        return word.length() > EDITOR_FILTER_MIN_WORD_LENGTH;
    });
    words.sort(Qt::CaseInsensitive);

    return words;
}

QStringList CommandsTextEdit::retrieveAllWordsFromDocument() const
{
    return toPlainText().split(QRegExp("\\W+"), SkipEmptyParts);
}

template <class UnaryPredicate>
QStringList CommandsTextEdit::filterWordList(const QStringList &words, UnaryPredicate predicate) const
{
    QStringList filteredWordList;

    foreach (const QString &word, words) {
        if (predicate(word))
        {
           filteredWordList << word;
        }
    }

    return filteredWordList;
}

QString CommandsTextEdit::retrieveTextUnderCursor() const
{
   QTextCursor cursor = this->textCursor();
   QTextDocument *document = this->document();

   // empty text if cursor at start of line
   if (cursor.atBlockStart()) {
       return QString();
   }

   cursor.clearSelection();

   // move left until we find a space or reach the start of line
   while(!document->characterAt(cursor.position()-1).isSpace() && !cursor.atBlockStart()) {
       cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
   }

   return cursor.selectedText();
}

// Auto complete
void CommandsTextEdit::setAutoCompleter(QCompleter *completer)
{
    if (ac)
        ac->disconnect(this);

    ac = completer;

    if (!ac)
        return;

    ac->setWidget(this);
    ac->setCompletionMode(QCompleter::PopupCompletion);
    ac->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    ac->setCaseSensitivity(Qt::CaseInsensitive);
    ac->setWrapAround(false);
    QObject::connect(ac,   SIGNAL(activated(const QString&)),
                     this, SLOT(  insertCompletion(const QString&)));
}

void CommandsTextEdit::keyPressEvent(QKeyEvent *e)
{
    if ((ac && ac->popup()->isVisible()) || (sc && sc->isPopupVisible())) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    const bool isSnippetShortcut = (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_Space); // CTRL+Space
    if (isSnippetShortcut) { // do not popup the autocompleter when we have a snippet completer command
      if (sc)
          sc->hidePopup();
      QPlainTextEdit::keyPressEvent(e);
      return;
    }

    const bool isShortcut = (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (!ac || !isShortcut) // do not process the shortcut when we have an auto completer
        QPlainTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers().testFlag(Qt::ControlModifier) ||
                             e->modifiers().testFlag(Qt::ShiftModifier);
    if (!ac || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty() || completionPrefix.length() < 3
                      || eow.contains(e->text().right(1)))) {
        ac->popup()->hide();
        return;
    }

    if (completionPrefix != ac->completionPrefix()) {
        ac->setCompletionPrefix(completionPrefix);
        ac->popup()->setCurrentIndex(ac->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(ac->popup()->sizeHintForColumn(0)
                + ac->popup()->verticalScrollBar()->sizeHint().width());

    ac->complete(cr); // popup it up!
}

void CommandsTextEdit::focusInEvent(QFocusEvent *e)
{
    if (ac)
        ac->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void CommandsTextEdit::paintEvent(QPaintEvent *e)
{
    QPlainTextEdit::paintEvent(e);

    // draw line end markers if enabled
    if (showHardLinebreaks) {
        drawLineEndMarker(e);
    }
}

void CommandsTextEdit::drawLineEndMarker(QPaintEvent *e)
{
    QPainter painter(viewport());

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int leftMargin = qRound(fontMetrics().horizontalAdvance(" ") / 2.0);
    int lineEndCharWidth = fontMetrics().horizontalAdvance("\u00B6");
#else
    int leftMargin = qRound(fontMetrics().width(" ") / 2.0);
    int lineEndCharWidth = fontMetrics().width("\u00B6");
#endif
    int fontHeight = fontMetrics().height();

    QColor markerColor;
    if (Preferences::darkTheme)
        markerColor = QColor(Qt::darkGray).darker(150);
    else
        markerColor = QColor(Qt::darkGray);

    QTextBlock block = firstVisibleBlock();
    while (block.isValid()) {
        QRectF blockGeometry = blockBoundingGeometry(block).translated(contentOffset());
        if (blockGeometry.top() > e->rect().bottom())
            break;

        if (block.isVisible() && blockGeometry.toRect().intersects(e->rect())) {
            QString text = block.text();
            if (text.endsWith("  ")) {
                painter.setPen(markerColor);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
                painter.drawText(blockGeometry.left() + fontMetrics().horizontalAdvance(text) + leftMargin,
                                 blockGeometry.top(),
                                 lineEndCharWidth,
                                 fontHeight,
                                 Qt::AlignLeft | Qt::AlignVCenter,
                                 "\u00B6");
#else
                painter.drawText(blockGeometry.left() + fontMetrics().width(text) + leftMargin,
                                 blockGeometry.top(),
                                 lineEndCharWidth,
                                 fontHeight,
                                 Qt::AlignLeft | Qt::AlignVCenter,
                                 "\u00B6");
#endif
            }
        }

        block = block.next();
    }
}

void CommandsTextEdit::insertCompletion(const QString &completion)
{
    if (ac->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - ac->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString CommandsTextEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CommandsTextEdit::gotoLine(int line)
{
    QTextCursor cursor(document()->findBlockByNumber(line-1));
    this->setTextCursor(cursor);
}

int CommandsTextEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    QFont font = lineNumberArea->font();
    const QFontMetrics linefmt(font);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int space = 10 + linefmt.horizontalAdvance(QLatin1Char('9')) * digits;
#else
    int space = 10 + linefmt.width(QLatin1Char('9')) * digits;
#endif
    return space;
}

void CommandsTextEdit::setEditorFont()
{
    const int editorFontSize = Preferences::editorFontSize;
    const QString editorFont = Preferences::editorFont;
    QFont font(editorFont, editorFontSize);
    font.setStyleHint(QFont::System);

    lineNumberArea->setFont(font);
    setFont(font);
}

void CommandsTextEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CommandsTextEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CommandsTextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = this->contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CommandsTextEdit::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor;
        if (Preferences::displayTheme == THEME_DEFAULT) {
            lineColor = QColor(Preferences::themeColors[THEME_DEFAULT_LINE_HIGHLIGHT]);
          }
        else
        if (Preferences::darkTheme) {
            lineColor = QColor(Preferences::themeColors[THEME_DARK_LINE_HIGHLIGHT]);
        }

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CommandsTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);

    int selStart = textCursor().selectionStart();
    int selEnd = textCursor().selectionEnd();

    QPalette palette = lineNumberArea->palette();

    painter.fillRect(event->rect(), palette.color(QPalette::Window));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    qreal top = blockBoundingGeometry(block).translated(contentOffset()).top();
    qreal bottom = top;

    while (block.isValid() && top <= event->rect().bottom()) {
        top = bottom;

        const qreal height = blockBoundingRect(block).height();
        bottom = top + height;

        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.setPen(palette.windowText().color());

            bool selected = (
                                (selStart < block.position() + block.length() && selEnd > block.position())
                                || (selStart == selEnd && selStart == block.position())
                            );

            if (selected) {
                painter.save();
                painter.setPen(palette.highlight().color());
            }

            const QString number = QString::number(blockNumber + 1);
            painter.drawText(0, top, lineNumberArea->width() - 4, height, Qt::AlignRight, number);

            if (selected)
                painter.restore();
        }

        block = block.next();
        ++blockNumber;
    }
}

void CommandsTextEdit::findDialog()
{
    QTextCursor cursor = textCursor();
    QString selection = cursor.selectedText();
    if (selection.isEmpty()) {
        cursor.select(QTextCursor::WordUnderCursor);
        selection = cursor.selectedText();
    }
    popUp = new FindReplace(this, selection);
    popUp->show();
}
