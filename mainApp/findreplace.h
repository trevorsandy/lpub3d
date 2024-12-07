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
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef FINDREPLACE_H
#define FINDREPLACE_H

#include <QDialog>

class QLabel;
class QCheckBox;
class QPlainTextEdit;
class QAbstractItemModel;
class QCompleter;

class FindReplaceCtrls;
class HistoryLineEdit;
class FindReplace : public QDialog
{
    Q_OBJECT

public:
    explicit FindReplace(QPlainTextEdit *textEdit, const QString &selectedText, QWidget *parent = nullptr);

protected slots:
    void popUpClose();

protected:
    FindReplaceCtrls *find;
    FindReplaceCtrls *findReplace;
    void readFindReplaceSettings(FindReplaceCtrls *fr);
    void writeFindReplaceSettings(FindReplaceCtrls *fr);
private:
    QAbstractItemModel *metaCommandModel(QObject *parent);
    QCompleter *completer;
};

class FindReplaceCtrls : public QWidget
{
    Q_OBJECT

public:
    explicit FindReplaceCtrls(QPlainTextEdit *textEdit, QWidget *parent = nullptr);
    QPlainTextEdit *_textEdit;
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
