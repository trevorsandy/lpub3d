/****************************************************************************
**
** Copyright (C) 2022 - 2024 Trevor SANDY. All rights reserved.
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
 * The find and replace dialog is used to facilitate find and replace functions
 * with specific focus on LPub commands.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/
 
#include <QtWidgets>
#include <QPlainTextEdit>

#include "findreplace.h"
#include "lpub_object.h"
#include "historylineedit.h"
#include "commonmenus.h"
 
 FindReplace::FindReplace(QPlainTextEdit *textEdit,
    const QString &selectedText,
    QWidget *parent)
    : QDialog(parent)
{
    setWindowIcon(QIcon(":/resources/LPub32.png"));

    setWindowTitle(tr("Command Editor Find"));

    setWhatsThis(lpubWT(WT_DIALOG_FIND_REPLACE,windowTitle()));

    completer = new QCompleter(this);
    completer->setModel(metaCommandModel(completer));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);

    find = new FindReplaceCtrls(textEdit,this);
    find->textFind->setText(selectedText);
    find->textFind->setClearButtonEnabled(true);
    find->textFind->setWordCompleter(completer);
    connect(find, SIGNAL(popUpClose()), this, SLOT(popUpClose()));

    findReplace = new FindReplaceCtrls(textEdit,this);
    findReplace->textFind->setText(selectedText);
    findReplace->textFind->setClearButtonEnabled(true);
    findReplace->textFind->setWordCompleter(completer);
    connect(findReplace, SIGNAL(popUpClose()), this, SLOT(popUpClose()));

    connect(this, SIGNAL(accepted()), this, SLOT(popUpClose()));
    connect(this, SIGNAL(rejected()), this, SLOT(popUpClose()));

    readFindReplaceSettings(find);
    readFindReplaceSettings(findReplace);

    QTabWidget  *tabWidget = new QTabWidget;
    QVBoxLayout *vlayout = new QVBoxLayout;
    QHBoxLayout *hlayout = new QHBoxLayout;
    QGridLayout *gropuLayout;

    setLayout(vlayout);
    vlayout->addWidget(tabWidget);

    QWidget *widget;
    QGridLayout *grid;
    QGroupBox *box;

    widget = new QWidget;
    grid = new QGridLayout;
    widget->setLayout(grid);

    box = new QGroupBox("String to find");
    grid->addWidget(box);
    gropuLayout = new QGridLayout;
    box->setLayout(gropuLayout);

    gropuLayout->addWidget(find->textFind,0,0,1,5);

    gropuLayout->addWidget(find->buttonFind,1,0,1,1);
    gropuLayout->addWidget(find->buttonFindNext,1,1,1,1);
    gropuLayout->addWidget(find->buttonFindPrevious,1,2,1,1);
    gropuLayout->addWidget(find->buttonFindAll,1,3,1,1);
    gropuLayout->addWidget(find->buttonFindClear,1,4,1,1);

    box = new QGroupBox();
    box->setLayout(hlayout);
    gropuLayout->addWidget(box,2,0,1,5);
    hlayout->addWidget(find->checkboxCase);
    hlayout->addWidget(find->checkboxWord);
    hlayout->addWidget(find->checkboxRegExp);

    gropuLayout->addWidget(find->labelMessage,3,0,1,5);

    tabWidget->addTab(widget,"Find");

    widget = new QWidget;
    grid = new QGridLayout;
    widget->setLayout(grid);

    box = new QGroupBox("String to find and replace");
    grid->addWidget(box);
    gropuLayout = new QGridLayout;
    box->setLayout(gropuLayout);

    findReplace->label = new QLabel("Find: ");
    gropuLayout->addWidget(findReplace->label,0,0,1,1);
    gropuLayout->addWidget(findReplace->textFind,0,1,1,4);

    findReplace->label = new QLabel("Replace: ");
    gropuLayout->addWidget(findReplace->label,1,0,1,1);
    gropuLayout->addWidget(findReplace->textReplace,1,1,1,4);

    gropuLayout->addWidget(findReplace->buttonFind,2,0,1,1);
    gropuLayout->addWidget(findReplace->buttonFindNext,2,1,1,1);
    gropuLayout->addWidget(findReplace->buttonFindPrevious,2,2,1,1);
    gropuLayout->addWidget(findReplace->buttonFindAll,2,3,1,1);
    gropuLayout->addWidget(findReplace->buttonFindClear,2,4,1,1);

    gropuLayout->addWidget(findReplace->buttonReplace,3,0,1,1);
    gropuLayout->addWidget(findReplace->buttonReplaceAndFind,3,1,1,1);
    gropuLayout->addWidget(findReplace->buttonReplaceAll,3,2,1,1);
    gropuLayout->addWidget(findReplace->buttonReplaceClear,3,3,1,1);

    hlayout = new QHBoxLayout;
    box = new QGroupBox();
    box->setLayout(hlayout);
    gropuLayout->addWidget(box,4,0,1,5);
    hlayout->addWidget(findReplace->checkboxCase);
    hlayout->addWidget(findReplace->checkboxWord);
    hlayout->addWidget(findReplace->checkboxRegExp);

    gropuLayout->addWidget(findReplace->labelMessage,5,0,1,5);

    tabWidget->addTab(widget,"Replace");

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(findReplace->buttonCancel, QDialogButtonBox::ActionRole);
    vlayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QTimer::singleShot(0, find->textFind, SLOT(setFocus()));

    setMinimumSize(100,80);
}

QAbstractItemModel *FindReplace::metaCommandModel(QObject *parent)
{
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif

    QStandardItemModel *model = new QStandardItemModel(parent);
    QString commandIcon = QStringLiteral(":/resources/command16.png");
    if (Preferences::displayTheme == THEME_DARK)
        commandIcon = QStringLiteral(":/resources/command_dark16.png");
    foreach (const QString &keyword, lpub->metaKeywords)
        model->appendRow(new QStandardItem(QIcon(commandIcon), keyword));

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    return model;
}

void  FindReplace::popUpClose()
{
    FindReplaceCtrls *fr = qobject_cast<FindReplaceCtrls *>(sender());
    if (fr) {
        writeFindReplaceSettings(findReplace);
        writeFindReplaceSettings(find);
        if (fr->_findall){
            const bool wasBlocked = fr->_textEdit->blockSignals(true);
            fr->_textEdit->document()->undo();
            fr->_textEdit->blockSignals(wasBlocked);
        }
    }
    close();
}

void FindReplace::readFindReplaceSettings(FindReplaceCtrls *fr) {
    QSettings settings;
    settings.beginGroup(FINDREPLACEWINDOW);
    fr->checkboxCase->setChecked(settings.value(CASE_CHECK, false).toBool());
    fr->checkboxWord->setChecked(settings.value(WORD_CHECK, false).toBool());
    fr->checkboxRegExp->setChecked(settings.value(REGEXP_CHECK, false).toBool());
    settings.endGroup();
}

void FindReplace::writeFindReplaceSettings(FindReplaceCtrls *fr) {
    QSettings settings;
    settings.beginGroup(FINDREPLACEWINDOW);
    settings.setValue(CASE_CHECK, fr->checkboxCase->isChecked());
    settings.setValue(WORD_CHECK, fr->checkboxWord->isChecked());
    settings.setValue(REGEXP_CHECK, fr->checkboxRegExp->isChecked());
    settings.endGroup();
}

FindReplaceCtrls::FindReplaceCtrls(QPlainTextEdit *textEdit, QWidget *parent)
    : QWidget(parent),_textEdit(textEdit)
{
    // find items
    textFind    = new HistoryLineEdit/*QLineEdit*/;

    buttonFind  = new QPushButton("Find");
    buttonFindNext = new QPushButton("Find Next");
    buttonFindPrevious = new QPushButton("Find Previous");
    buttonFindAll = new QPushButton("Find All");
    buttonFindClear = new QPushButton("Clear");

    // options
    checkboxCase = new QCheckBox("Case Senstive");
    checkboxWord = new QCheckBox("Whole Words");
    checkboxRegExp = new QCheckBox("Regular Expression");

    // replace items
    textReplace = new HistoryLineEdit/*QLineEdit*/;

    buttonReplace = new QPushButton("Replace");
    buttonReplaceAndFind = new QPushButton("Replace && Find");
    buttonReplaceAll = new QPushButton("Replace All");
    buttonReplaceClear = new QPushButton("Clear");

    // message
    labelMessage = new QLabel;

    // cancel button
    buttonCancel = new QPushButton("Cancel");

    // events
    connect(textFind, SIGNAL(textChanged(QString)), this, SLOT(textFindChanged()));
    connect(textFind, SIGNAL(textChanged(QString)), this, SLOT(validateRegExp(QString)));
    connect(textReplace, SIGNAL(textChanged(QString)), this, SLOT(textReplaceChanged()));

    connect(checkboxRegExp, SIGNAL(toggled(bool)), this, SLOT(regexpSelected(bool)));

    connect(buttonFind, SIGNAL(clicked()), this,SLOT(findInText()));
    connect(buttonFindNext,SIGNAL(clicked()),this,SLOT(findInTextNext()));
    connect(buttonFindPrevious,SIGNAL(clicked()),this,SLOT(findInTextPrevious()));
    connect(buttonFindAll,SIGNAL(clicked()),this,SLOT(findInTextAll()));
    connect(buttonFindClear, SIGNAL(clicked()), this, SLOT(findClear()));

    connect(buttonReplace,SIGNAL(clicked()),this,SLOT(replaceInText()));
    connect(buttonReplaceAndFind,SIGNAL(clicked()),this,SLOT(replaceInTextFind()));
    connect(buttonReplaceAll,SIGNAL(clicked()),this,SLOT(replaceInTextAll()));
    connect(buttonReplaceClear, SIGNAL(clicked()), this, SLOT(replaceClear()));

    connect(buttonCancel, SIGNAL(clicked()), this, SIGNAL(popUpClose()));

    disableButtons();
}

void FindReplaceCtrls::findInText(){
    find();
}

void FindReplaceCtrls::findInTextNext()
{
    find(NEXT);
}

void FindReplaceCtrls::findInTextPrevious()
{
    find(PREVIOUS);
}

void FindReplaceCtrls::findInTextAll() {
    if (!_textEdit) {
        lpub->messageSig(LOG_ERROR, tr("Document is null."));
        return;
    }

    const bool wasBlocked = _textEdit->blockSignals(true);

    const QString &toSearch = textFind->text();

    // undo previous change (if any)
    _textEdit->document()->undo();

    bool result = false;
    bool useRegExp = checkboxRegExp->isChecked();

    QTextDocument::FindFlags flags;

    if (checkboxCase->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (checkboxWord->isChecked())
        flags |= QTextDocument::FindWholeWords;

    if (toSearch.isEmpty()) {
        showError(tr("The search field is empty. Enter a word and click Find."));
    } else {

        QTextCursor highlightCursor(_textEdit->document());
        QTextCursor textCursor(_textEdit->document());

        textCursor.beginEditBlock();

        QTextCharFormat plainFormat(highlightCursor.charFormat());
        QTextCharFormat colorFormat = plainFormat;
        colorFormat.setBackground(Qt::yellow);

        QRegExp reg;
        if (useRegExp) {
            reg = QRegExp(toSearch,
                         (checkboxCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive));
        }

        while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {

            highlightCursor = useRegExp ? _textEdit->document()->find(reg, highlightCursor, flags) :
                                          _textEdit->document()->find(toSearch, highlightCursor, flags);

            if (!highlightCursor.isNull()) {
                result = true;
                highlightCursor.movePosition(QTextCursor::WordRight,
                                             QTextCursor::KeepAnchor);
                highlightCursor.mergeCharFormat(colorFormat);
            }
        }

        textCursor.endEditBlock();

        if (result) {
            _findall = true;
            showError("");
        } else {
            showError(tr("no match found for '%1'").arg(textFind->text()));
            highlightCursor.setPosition(0); // move to the beginning of the document for the next find
            _textEdit->setTextCursor(highlightCursor);
        }
    }

    _textEdit->blockSignals(wasBlocked);
}

void FindReplaceCtrls::find(int direction) {
    if (!_textEdit) {
        lpub->messageSig(LOG_ERROR, tr("Document is null."));
        return;
    }

    const QString &toSearch = textFind->text();

    // undo previous change (if any)
    if (_findall){
        _textEdit->document()->undo();
        _findall = false;
    }

    bool result = false;

    QTextDocument::FindFlags flags;

    if (direction == PREVIOUS)
        flags |= QTextDocument::FindBackward;
    if (checkboxCase->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (checkboxWord->isChecked())
        flags |= QTextDocument::FindWholeWords;

    if (toSearch.isEmpty()) {
        showError(tr("The search field is empty. Enter a word and click Find."));
    } else {
        QTextCursor textCursor(_textEdit->document());

        textCursor.beginEditBlock();

        if (direction == NEXT && buttonFindPrevious->isEnabled() == false)
        {
             _textEdit->moveCursor(QTextCursor::Start);
        }

        if (checkboxRegExp->isChecked()) {
            QRegExp reg(toSearch,
                        (checkboxCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive));
            textCursor = _textEdit->document()->find(reg, textCursor, flags);
            _textEdit->setTextCursor(textCursor);
            result = (!textCursor.isNull());
        } else {
            result = _textEdit->find(toSearch, flags);
        }

        textCursor.endEditBlock();

        if (result) {
            if (direction == NEXT && buttonFindPrevious->isEnabled() == false) {
                buttonFindPrevious->setEnabled(true);
            }
            showError("");
        } else {
            if (direction == DEFAULT) {
                showError(tr("no match found for '%1'").arg(textFind->text()));
            } else {
                showError(tr("no more items found for '%1'").arg(textFind->text()));
            }
            textCursor.setPosition(0); // move to the beginning of the document for the next find
            _textEdit->setTextCursor(textCursor);
        }
    }
}

void FindReplaceCtrls::replaceInText() {
    if (!_textEdit->textCursor().hasSelection()) {
        find();
    } else {
        _textEdit->textCursor().insertText(textReplace->text());
    }
}

void FindReplaceCtrls::replaceInTextFind() {
    if (!_textEdit->textCursor().hasSelection()) {
        find();
    } else {
        _textEdit->textCursor().insertText(textReplace->text());
        find();
    }
}

void FindReplaceCtrls::replaceInTextAll() {
    if (!_textEdit) {
        lpub->messageSig(LOG_ERROR, tr("Document is null."));
        return;
    }

    const QString &toSearch = textFind->text();
    const QString &toReplace = textReplace->text();

    if (toReplace.isEmpty()) {
        showError(tr("The replace field is empty. Enter a word and click Replace."));
    } else if (toSearch.isEmpty()) {
        showError(tr("The search field is empty. Enter a word and click Replace."));
    } else {
        QTextCursor textCursor = _textEdit->textCursor();

        textCursor.beginEditBlock();

        textCursor.movePosition(QTextCursor::Start);
        QTextCursor newCursor = textCursor;
        quint64 count = 0;

        QTextDocument::FindFlags flags;

        if (checkboxCase->isChecked())
            flags |= QTextDocument::FindCaseSensitively;
        if (checkboxWord->isChecked())
            flags |= QTextDocument::FindWholeWords;

        while (true)
        {
            if (checkboxRegExp->isChecked()) {
                QRegExp reg(toSearch,
                            (checkboxCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive));
                newCursor = _textEdit->document()->find(reg, newCursor, flags);
            } else {
                newCursor = _textEdit->document()->find(toSearch, newCursor, flags);
            }

            if (!newCursor.isNull())
            {
                if (newCursor.hasSelection())
                {
                    newCursor.insertText(toReplace);
                    count++;
                }
            }
            else
            {
                break;
            }
        }

        textCursor.endEditBlock();
        showMessage( tr("%1 occurrence(s) were replaced.").arg(count));
    }
}

void FindReplaceCtrls::regexpSelected(bool sel) {
    if (sel)
        validateRegExp(textFind->text());
    else
        validateRegExp("");
}

void FindReplaceCtrls::validateRegExp(const QString &text) {
    if (!checkboxRegExp->isChecked() || text.size() == 0) {
        labelMessage->clear();
        return; // nothing to validate
    }

    QRegExp reg(text,
                (checkboxCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive));

    if (reg.isValid()) {
        showError("");
    } else {
        showError(reg.errorString());
    }
}

void FindReplaceCtrls::showError(const QString &error) {
    if (error == "") {
        labelMessage->clear();
    } else {
        labelMessage->setText("<span style=\" font-weight:200; color:#ff0000;\">" + error + "</span>");
    }
}

void FindReplaceCtrls::showMessage(const QString &message) {
    if (message == "") {
        labelMessage->clear();
    } else {
        labelMessage->setText("<span style=\" font-weight:200; color:green;\">" + message + "</span>");
    }
}

void FindReplaceCtrls::textReplaceChanged() {
    bool enable = textReplace->text().size() > 0;
    buttonReplace->setEnabled(enable);
    buttonReplaceAndFind->setEnabled(enable);
    buttonReplaceAll->setEnabled(enable);
    buttonReplaceClear->setEnabled(enable);
}

void FindReplaceCtrls::textFindChanged() {
    bool enable = textFind->text().size() > 0;
    buttonFind->setEnabled(enable);
    buttonFindNext->setEnabled(enable);
    buttonFindAll->setEnabled(enable);
    buttonFindClear->setEnabled(enable);
}

void  FindReplaceCtrls::disableButtons()
{
    bool enable = false;

    buttonFind->setEnabled(enable);
    buttonFindNext->setEnabled(enable);
    buttonFindPrevious->setEnabled(enable);
    buttonFindAll->setEnabled(enable);
    buttonFindClear->setEnabled(enable);

    buttonReplace->setEnabled(enable);
    buttonReplaceAndFind->setEnabled(enable);
    buttonReplaceAll->setEnabled(enable);
    buttonReplaceClear->setEnabled(enable);
}

void FindReplaceCtrls::findClear() {
    _textEdit->document()->undo();
    _textEdit->moveCursor(QTextCursor::Start);
    textFind->clear();
    labelMessage->clear();
}

void FindReplaceCtrls::replaceClear() {
    _textEdit->document()->undo();
    _textEdit->moveCursor(QTextCursor::Start);
    textReplace->clear();
    labelMessage->clear();
}
