/**************************************************************************** 
**
** Copyright (C) 2005-2009 Trolltech ASA. All rights reserved.
** Copyright (C) 2020 Trevor SANDY. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
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
 * This implements a syntax highlighter class that works with the editwindow
 * to display LDraw files with syntax highlighting.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef HIGHLIGHTERSIMPLE_H
#define HIGHLIGHTERSIMPLE_H

#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>

class QTextDocument;

class HighlighterSimple : public QSyntaxHighlighter
{

    Q_OBJECT

public:
    HighlighterSimple(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat LDrawCommentFormat; // b01  - Comments      Qt::darkGreen
    QTextCharFormat LDrawHeaderFormat;  // b02  - LDraw Header  Qt::blue
    QTextCharFormat ModuleMetaFormat;   // br17 - MLCad         Qt::darkBlue
    QTextCharFormat LPubMetaFormat;     // br25 - LPub Meta     Qt::darkRed
    QTextCharFormat LSynthMetaFormat;   // br22 - LSynth Meta   Qt::red
};

#endif

