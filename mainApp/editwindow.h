/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QFileSystemWatcher>
#include <QSortFilterProxyModel>
#include <QAction>
#include <atomic>

#include "declarations.h"

class QAbstractItemModel;
class QCompleter;
class QComboBox;
class QTextCursor;

class Highlighter;
class HighlighterSimple;
class FindReplace;
class SnippetCompleter;

class TextEditor;
class LDrawFile;
class WaitingSpinnerWidget;
class LoadModelWorker;

class SnippetCollection;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class EditWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditWindow(QMainWindow *parent = nullptr, bool modelFileEdit = false);
    ~EditWindow();
    TextEditor *textEdit()
    {
        return _textEdit;
    }
    bool modelFileEdit()
    {
        return _modelFileEdit;
    }
    QString &getCurrentFile()
    {
        return fileName;
    }
    bool contentLoading()
    {
        return _contentLoading;
    }
    int linesTop()
    {
        return stepLines.top;
    }
    int linesBottom()
    {
        return stepLines.bottom;
    }

    bool updateEnabled()
    {
        return updateAct->isEnabled();
    }

    void setUpdateEnabled(bool b)
    {
        updateAct->setEnabled(b);
    }

    QToolBar *fileToolBar;
    QToolBar *standardToolBar;
    QToolBar *editToolBar;
    QToolBar *toolsToolBar;

signals:
    void contentsChangeSig(const QString &fileName,
                                 bool isUndo,
                                 bool isRedo,
                                 int  position,
                                 int  charsRemoved,
                           const QString &charsAdded);
    void refreshModelFileSig();
    void getSubFileListSig();
    void redrawSig();
    void updateSig();
    void enableWatcherSig();
    void disableWatcherSig();
    void updateDisabledSig(bool);
    void editModelFileSig();
    void SelectedPartLinesSig(QVector<TypeLine>&, PartSource = EDITOR_LINE);
    void setStepForLineSig();
    void waitingSpinnerStopSig();
    void triggerUndoSig();
    void triggerRedoSig();

    void comboFilterChanged();

public slots:
    void displayFile(LDrawFile *, const QString &fileName, const StepLines& lineScope);
    void displayFile(LDrawFile *, const QString &fileName);
    void setLineScope(const StepLines& lineScope);
    void setSubFiles(const QStringList& subFiles);
    void modelFileChanged(const QString &fileName);
    void showLine(int, int);
    void highlightSelectedLines(QVector<int> &lines, bool clear);
    void triggerPreviewLine();
    void updateDisabled(bool);
    void clearEditorWindow();
    void setTextEditHighlighter();
    void setSelectionHighlighter();
    void pageUpDown(QTextCursor::MoveOperation op, QTextCursor::MoveMode moveMode);
    void setReadOnly(bool enabled);
    void setSubmodel(int index);
    void loadContentBlocks(const QStringList &, bool); /* NOT USED FOR THE MOMENT */
    void loadPagedContent();
    void setPagedContent(const QStringList &);
    void setPlainText(const QString &);
    void setLineCount(int);
    void clearWindow();
    bool maybeSave();
    bool saveFile();

private slots:
    void openWith();
    void openWithChoice();
    void gotoLine();
    void contentsChange(int position, int charsRemoved, int charsAdded);
    bool saveFileCopy();
    void redraw();
    void update(bool state);
    void deleteSelection();
    void enableSave();
    void highlightCurrentLine();
    void topOfDocument();
    void editLineItem();
    void previewLine();
    void editorTabLock();
#ifdef QT_DEBUG_MODE
    void previewCurrentModel();
#endif
    void commandsDialog();
    void insertCommand(const QString &, bool);
    void bottomOfDocument();
    void showAllCharacters();
    void mpdComboChanged(int index);
    void showContextMenu(const QPoint &pt);
    void highlightSelectedParts();
    void preferences();
    void verticalScrollValueChanged(int action);
    void setVisualEditorVisible(bool);
    void openFolder();
#ifndef QT_NO_CLIPBOARD
    void updateClipboard();
#endif
    void waitingSpinnerStart();
    void waitingSpinnerStop();
    void contentLoaded();
    void loadFinished();

    void undo();
    void redo();
    void undoKeySequence();
    void redoKeySequence();

    void comboFilterTriggered();
    void comboFilterTextChanged(const QString&);

    void moveSelection(bool = true);
    void moveLineUp() { moveSelection(); }
    void moveLineDown() { moveSelection(false); }

protected:
    void createActions();
    void createMenus();
    void createToolBars();
    void createOpenWithActions();
    void readSettings();
    void writeSettings();
    void clearEditorHighlightLines(bool = false);
    void openFolderSelect(const QString &absoluteFilePath);
    void highlightSelectedLines(QVector<LineHighlight> &lines, bool isEditor);
    void setOpenWithProgramAndArgs(QString &program, QStringList &arguments);
    void updateOpenWithActions();
    void disableActions();
    void enableActions();
    void enablePartLineTools(bool b);

    int setCurrentStep(const int lineNumber, bool inScope = true);

    bool setValidPartLine();
    bool substitutePLIPart(QString &replaceText, const int action, const QStringList &elements);

    QAbstractItemModel *metaCommandModel(QObject *parent = nullptr);

    void configureMpdCombo();
    Qt::CaseSensitivity comboCaseSensitivity() const;
    void setComboCaseSensitivity(Qt::CaseSensitivity);
    QRegExp::PatternSyntax comboPatternSyntax() const;
    void setComboPatternSyntax(QRegExp::PatternSyntax);

    void closeEvent(QCloseEvent*_event) override;

    WaitingSpinnerWidget *_waitingSpinner;
    TextEditor        *_textEdit;
    LoadModelWorker   *loadModelWorker;
    Highlighter       *highlighter;
    HighlighterSimple *highlighterSimple;

    QComboBox         *mpdCombo;
    QAction           *mComboCaseSensitivityAction;
    QAction           *mComboFilterAction;
    QActionGroup      *mComboPatternGroup;
    QString            mComboDefaultText;
    QSortFilterProxyModel *mComboFilterModel;

    QFutureWatcher<int> futureWatcher;
    QFileSystemWatcher fileWatcher;
    QElapsedTimer      displayTimer;
    StepLines          stepLines;
    QVector<int>       savedSelection;
    QStringList        programEntries;
    QString            fileName;            // of model file currently being displayed
    int                numOpenWithPrograms;
    int                showLineType;
    int                showLineNumber;
    int                fileOrderIndex;
    QAtomicInt         lineCount;
    bool               isIncludeFile;
    bool               visualEditorVisible;
    bool               isReadOnly;
    bool               reloaded;
    bool               _modelFileEdit;
    bool               _subFileListPending;
    bool               _contentLoaded;
    bool               _contentLoading;
    bool               _isUndo;
    bool               _isRedo;
    QString            _curSubFile;         // currently displayed submodel
    QStringList        _subFileList;
    QStringList        _pageContent;
    int                _pageIndx;
    int                _saveSubfileIndex;

    QMenu         *removeMenu;
    QScrollBar    *verticalScrollBar;
    QList<QAction *> openWithActList;

    QAction  *openWithChoiceAct;
    QAction  *openWithToolbarAct;
    QAction  *editModelFileAct;
    QAction  *previewLineAct;
    QAction  *editCommandAct;
    QAction  *editColorAct;
    QAction  *editPartAct;

    QAction  *mpdComboSeparatorAct;
    QAction  *mpdComboAct;

    QAction  *substitutePartAct;
    QAction  *removeSubstitutePartAct;

#ifdef QT_DEBUG_MODE
    QAction  *previewCurrentModelAct;
#endif
    QAction  *topAct;
    QAction  *bottomAct;
    QAction  *moveUpAct;
    QAction  *moveDownAct;
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
    QAction  *gotoLineAct;
    QAction  *editorTabLockAct;
    QAction  *preferencesAct;
    QAction  *openFolderAct;
    QAction  *copyFullPathToClipboardAct;
    QAction  *copyFileNameToClipboardAct;

    QAction  *exitAct;
    QAction  *saveAct;
    QAction  *saveCopyAct;
    QAction  *undoAct;
    QAction  *redoAct;
};

extern class EditWindow *cmdEditor;
extern class EditWindow *cmdModEditor;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit TextEditor(bool modelFileEdit = false, QWidget *parent = nullptr);

    ~TextEditor()override{}

    void showAllCharacters(bool enabled);
    void setIsUTF8(bool isUTF8) { _fileIsUTF8 = isUTF8; }
    bool getIsUTF8() { return _fileIsUTF8; }

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void setAutoCompleter(QCompleter *c);
    void setSnippetCompleter(SnippetCompleter *completer);

    void gotoLine(int line);
    bool modelFileEdit()
    {
        return detachedEdit;
    }

    FindReplace *popUp;

signals:
    void highlightSelectedParts();
    void triggerPreviewLine();
    void undoKeySequence();
    void redoKeySequence();

public slots:
    void setEditorFont();

    void resizeEvent(QResizeEvent *e) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

    void insertCompletion(const QString &completion);
    void insertSnippet(const QString &completionPrefix, const QString &completion, int newCursorPos);
    void performCompletion();

    void showCharacters(QString findString, QString replaceString);

    void findDialog();
    void toggleComment();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;

    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    QStringList extractDistinctWordsFromDocument() const;
    QStringList retrieveAllWordsFromDocument() const;
    template <class UnaryPredicate>
    QStringList filterWordList(const QStringList &words, UnaryPredicate predicate) const;
    QString retrieveTextUnderCursor() const;
    QString textUnderCursor() const;
    void drawLineEndMarker(QPaintEvent *e);

    QCompleter *ac;
    SnippetCompleter *sc;
    QWidget    *lineNumberArea;
    bool        detachedEdit;
    bool        showHardLinebreaks;
    std::atomic<bool> _fileIsUTF8;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEditor *editor) : QWidget(editor)
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
    TextEditor *textEditor;
};

#endif
