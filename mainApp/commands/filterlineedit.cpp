/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** Copyright (C) 2022 - 2023 Trevor SANDY. All rights reserved.
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "filterlineedit.h"

#include <QIcon>
#include <QPixmap>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QToolButton>
#include <QWidgetAction>
#include <QStringListModel>
#include <QCompleter>
#include <QSettings>

#include "commandcollection.h"
#include "snippets/snippetcollection.h"

FilterLineEdit::FilterLineEdit(CommandCollection *collection, QWidget *parent)
    : QLineEdit(parent)
    , m_commandsCollection(collection)
    , m_patternGroup(new QActionGroup(this))
{
    m_useCompleterAction = addAction(QIcon(":/resources/filter.png"), QLineEdit::TrailingPosition);
    m_useCompleterAction->setData(QString("command").toLatin1());
    setPlaceholderText(tr("Filter commands"));
    initialize();
}

FilterLineEdit::FilterLineEdit(SnippetCollection *collection, QWidget *parent)
    : QLineEdit(parent)
    , m_snippetsCollection(collection)
    , m_patternGroup(new QActionGroup(this))
{
    m_useCompleterAction = addAction(QIcon(":/resources/filter.png"), QLineEdit::TrailingPosition);
    m_useCompleterAction->setData(QString("snippet").toLatin1());
    setPlaceholderText(tr("Filter snippets"));
    initialize();
}

void FilterLineEdit::initialize()
{
    connect(this, &QLineEdit::textChanged, this, &::FilterLineEdit::filterTextChanged);

    m_useCompleterAction->setCheckable(true);
    m_useCompleterAction->setChecked(false);
    connect(m_useCompleterAction, &QAction::toggled, this, &FilterLineEdit::completerChanged);
    connect(m_useCompleterAction, &QAction::triggered, this, &FilterLineEdit::filterTriggered);
    completerChanged();

    QMenu *menu = new QMenu(this);

    m_caseSensitivityAction = menu->addAction(tr("Match Case"));
    m_caseSensitivityAction->setCheckable(true);
    connect(m_caseSensitivityAction, &QAction::toggled, this, &FilterLineEdit::filterChanged);

    menu->addSeparator();
    m_patternGroup->setExclusive(true);
    QAction *patternAction = menu->addAction("Fixed String");
    patternAction->setData(QVariant(int(QRegExp::FixedString)));
    patternAction->setCheckable(true);
    patternAction->setChecked(true);
    m_patternGroup->addAction(patternAction);
    patternAction = menu->addAction("Regular Expression");
    patternAction->setCheckable(true);
    patternAction->setData(QVariant(int(QRegExp::RegExp2)));
    m_patternGroup->addAction(patternAction);
    patternAction = menu->addAction("Wildcard");
    patternAction->setCheckable(true);
    patternAction->setData(QVariant(int(QRegExp::Wildcard)));
    m_patternGroup->addAction(patternAction);
    connect(m_patternGroup, &QActionGroup::triggered, this, &FilterLineEdit::filterChanged);

    QToolButton *optionsButton = new QToolButton;
#ifndef QT_NO_CURSOR
    optionsButton->setCursor(Qt::ArrowCursor);
#endif
    optionsButton->setFocusPolicy(Qt::NoFocus);
    optionsButton->setStyleSheet("* { border: none; }");
    optionsButton->setIcon(QIcon(":/resources/gear_in.png"));
    optionsButton->setToolTip(tr("Filter Options..."));
    optionsButton->setMenu(menu);
    optionsButton->setPopupMode(QToolButton::InstantPopup);

    QWidgetAction *optionsAction = new QWidgetAction(this);
    optionsAction->setDefaultWidget(optionsButton);
    addAction(optionsAction, QLineEdit::LeadingPosition);
}

void FilterLineEdit::completerChanged()
{
    static const QLatin1String COMMAND("command");
    static const QLatin1String SETTINGS("Settings");
    static const QLatin1String COMMANDSKEY("CommandsDialogCompleter");
    static const QLatin1String SNIPPETSKEY("SnippetsDialogCompleter");

    bool commands = true;
    if (m_useCompleterAction)
        commands = m_useCompleterAction->data().toString() == COMMAND;
    const QString KEY = commands ? COMMANDSKEY : SNIPPETSKEY;

    QSettings Settings;
    if (sender() == m_useCompleterAction || !Settings.contains(QString("%1/%2").arg(SETTINGS,KEY)))
        Settings.setValue(QString("%1/%2").arg(SETTINGS,KEY),m_useCompleterAction->isChecked());
    else
        m_useCompleterAction->setChecked(Settings.value(QString("%1/%2").arg(SETTINGS,KEY)).toBool());
    if (!m_useCompleterAction->isChecked()) {
        setCompleter(nullptr);
        return;
    }

    QCompleter *completer = new QCompleter(this);
    if (commands)
        completer->setModel(new QStringListModel(m_commandsCollection->preambleList(), completer));
    else
        completer->setModel(new QStringListModel(m_snippetsCollection->triggerList(), completer));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);

    setCompleter(completer);
}

void FilterLineEdit::filterTextChanged(const QString& Text)
{
    if (m_useCompleterAction) {
        if (Text.isEmpty()) {
            m_useCompleterAction->setIcon(QIcon(":/resources/filter.png"));
            m_useCompleterAction->setToolTip("");
        } else {
            m_useCompleterAction->setIcon(QIcon(":/resources/resetaction.png"));
            m_useCompleterAction->setToolTip(tr("Reset"));
        }
    }
    emit filterChanged();
}

void FilterLineEdit::filterTriggered()
{
    clear();
}

Qt::CaseSensitivity FilterLineEdit::caseSensitivity() const
{
    return m_caseSensitivityAction->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
}

void FilterLineEdit::setCaseSensitivity(Qt::CaseSensitivity cs)
{
    m_caseSensitivityAction->setChecked(cs == Qt::CaseSensitive);
}

static inline QRegExp::PatternSyntax patternSyntaxFromAction(const QAction *a)
{
    return static_cast<QRegExp::PatternSyntax>(a->data().toInt());
}

QRegExp::PatternSyntax FilterLineEdit::patternSyntax() const
{
    return patternSyntaxFromAction(m_patternGroup->checkedAction());
}

void FilterLineEdit::setPatternSyntax(QRegExp::PatternSyntax s)
{
    const QList<QAction*> actions = m_patternGroup->actions();
    for (QAction *a : actions) {
        if (patternSyntaxFromAction(a) == s) {
            a->setChecked(true);
            break;
        }
    }
}
