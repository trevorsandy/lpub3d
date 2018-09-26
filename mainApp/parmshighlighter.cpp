/****************************************************************************
**
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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

#include <QtWidgets>
#include "parmshighlighter.h"
#include "version.h"
#include "name.h"
#include "lpub_preferences.h"

ParmsHighlighter::ParmsHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    QBrush br01; // Qt Dark green
    QBrush br02; // Qt Dark blue
    QBrush br06; // Custom  blue
    QBrush br07; // Custom  orange/voilet
    if (Preferences::displayTheme == THEME_DEFAULT) {
        br01 = QBrush(QColor(THEME_HIGHLIGHT_01_DEFAULT));
        br02 = QBrush(QColor(THEME_HIGHLIGHT_02_DEFAULT));
        br06 = QBrush(QColor(THEME_HIGHLIGHT_06_DEFAULT));
        br07 = QBrush(QColor(THEME_HIGHLIGHT_07_DEFAULT));
      }
    else
    if (Preferences::displayTheme == THEME_DARK)  {
        br01 = QBrush(QColor(THEME_HIGHLIGHT_01_DARK));
        br02 = QBrush(QColor(THEME_HIGHLIGHT_02_DARK));
        br06 = QBrush(QColor(THEME_HIGHLIGHT_06_DARK));
        br07 = QBrush(QColor(THEME_HIGHLIGHT_07_DARK));
      }

    LPubParmsFormat.setForeground(br01);
    LPubParmsFormat.setFontWeight(QFont::Bold);

    rule.pattern = QRegExp("[#|;][^\n]*");
    rule.format = LPubParmsFormat;
    highlightingRules.append(rule);

    LPubParmsHdrFormat.setForeground(br02);
    LPubParmsHdrFormat.setFontWeight(QFont::Bold);

    rule.pattern = QRegExp("^\\[.*[^\n]\\]$");
    rule.format = LPubParmsHdrFormat;
    highlightingRules.append(rule);

    LPubParmsValueFormat.setForeground(br07);
    LPubParmsValueFormat.setFontWeight(QFont::Normal);

    rule.pattern = QRegExp("\\=(.*)");
    rule.format = LPubParmsValueFormat;
    highlightingRules.append(rule);

    LPubParmsEqualFormat.setForeground(br06);
    LPubParmsEqualFormat.setFontWeight(QFont::Bold);

    rule.pattern = QRegExp("=");
    rule.format = LPubParmsEqualFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(br01);
    commentStartExpression = QRegExp("\\b#\\b+[^\n]*");
    commentEndExpression   = QRegExp("\\#!\\b+[^\n]*");
}

void ParmsHighlighter::highlightBlock(const QString &text)
{
    foreach (HighlightingRule rule, highlightingRules) {
        QRegExp expression(rule.pattern);

        int index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = text.indexOf(expression, index + length);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        int endIndex = text.indexOf(commentEndExpression, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }

        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression,
                                                startIndex + commentLength);
    }
}

