/****************************************************************************
**
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
    QBrush br03; // Custom  blue
    QBrush br04; // Custom  orange/violet

    QBrush br05; //
    QBrush br06; //
    QBrush br07; //

    if (Preferences::displayTheme == THEME_DEFAULT) {
        br01 = QBrush(QColor(THEME_HIGHLIGHT_A_DEFAULT));
        br02 = QBrush(QColor(THEME_HIGHLIGHT_B_DEFAULT));
        br03 = QBrush(QColor(THEME_HIGHLIGHT_C_DEFAULT));
        br04 = QBrush(QColor(THEME_HIGHLIGHT_D_DEFAULT));

        br05 = QBrush(QColor(THEME_HIGHLIGHT_E_DEFAULT));
        br06 = QBrush(QColor(THEME_HIGHLIGHT_F_DEFAULT));
        br07 = QBrush(QColor(THEME_HIGHLIGHT_G_DEFAULT));
      }
    else
    if (Preferences::displayTheme == THEME_DARK)  {
        br01 = QBrush(QColor(THEME_HIGHLIGHT_A_DARK));
        br02 = QBrush(QColor(THEME_HIGHLIGHT_B_DARK));
        br03 = QBrush(QColor(THEME_HIGHLIGHT_C_DARK));
        br04 = QBrush(QColor(THEME_HIGHLIGHT_D_DARK));

        br05 = QBrush(QColor(THEME_HIGHLIGHT_E_DARK));
        br06 = QBrush(QColor(THEME_HIGHLIGHT_F_DARK));
        br07 = QBrush(QColor(THEME_HIGHLIGHT_G_DARK));
      }

    /* INI file formats */

    // INI Header
    LPubParmsHdrFormat.setForeground(br02);
    LPubParmsHdrFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("^\\[.*[^\n]\\]$");
    rule.format = LPubParmsHdrFormat;
    highlightingRules.append(rule);

    // Right side value
    LPubParmsValueFormat.setForeground(br04);
    LPubParmsValueFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("\\=(.*)");
    rule.format = LPubParmsValueFormat;
    highlightingRules.append(rule);

    // Equal sign
    LPubParmsEqualFormat.setForeground(br03);
    LPubParmsEqualFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("=");
    rule.format = LPubParmsEqualFormat;
    highlightingRules.append(rule);

    // Comment
    LPubParmsCommentFormat.setForeground(br01);
    LPubParmsCommentFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("^[#|;][^\n]*");
    rule.format = LPubParmsCommentFormat;
    highlightingRules.append(rule);

    /* List file formats */

    // br05 - Part ID
    LPubVal1Format.setForeground(br05);
    LPubVal1Format.setFontWeight(QFont::Bold);
    lineFormats.append(LPubVal1Format);

    // br06 - Part Control
    LPubVal2Format.setForeground(br06);
    LPubVal2Format.setFontWeight(QFont::Bold);


    // br07 - Part Description
    LPubVal3Format.setForeground(br07);
    LPubVal3Format.setFontWeight(QFont::Normal);

    option = 0;

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

    if (text.contains(QString::fromLatin1(VER_PLI_SUBSTITUTE_PARTS_FILE),Qt::CaseInsensitive))
        option = 1;
    else if (text.contains(QString::fromLatin1(VER_TITLE_ANNOTATIONS_FILE),Qt::CaseInsensitive))
        option = 2;
    else if (text.contains(QString::fromLatin1(VER_FREEFOM_ANNOTATIONS_FILE),Qt::CaseInsensitive))
        option = 3;
    else if (text.contains(QString::fromLatin1(VER_EXCLUDED_PARTS_FILE),Qt::CaseInsensitive))
        option = 4;
    else if (text.contains(QString::fromLatin1(VER_LPUB3D_LEGO_COLOR_PARTS),Qt::CaseInsensitive))
        option = 5;
    else if (!option)
        return;

    int index  = 0;

    QStringList tokens;

    switch (option)
    {
    case 1:
    {
        // VER_PLI_SUBSTITUTE_PARTS_FILE
        QRegExp rx1("^(\\b.+\\b)\\s+\"(.*)\"\\s+(.*)$");
        if (text.contains(rx1)) {
            tokens
            << rx1.cap(1).trimmed()
            << "\""+rx1.cap(2).trimmed()+"\""
            << rx1.cap(3).trimmed();
            lineFormats.append(LPubVal2Format);
            lineFormats.append(LPubVal3Format);
        }
    }
        break;
    case 2:
    {
        // VER_TITLE_ANNOTATIONS_FILE
        QRegExp rx2("^(\\b.*[^\\s]\\b:)\\s+([\\(|\\^].*)$");
        if (text.contains(rx2)) {
            tokens
            << rx2.cap(1).trimmed()
            << rx2.cap(2).trimmed();
            lineFormats.append(LPubVal3Format);
        }
    }
        break;
    case 3:
    {
        // VER_FREEFOM_ANNOTATIONS_FILE
        QRegExp rx3("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        if (text.contains(rx3)) {
            tokens
            << rx3.cap(1).trimmed()
            << rx3.cap(2).trimmed();
            lineFormats.append(LPubVal3Format);
        }
    }
        break;
    case 4:
    {
        // VER_EXCLUDED_PARTS_FILE
        QRegExp rx4("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        if (text.contains(rx4)) {
            tokens
            << rx4.cap(1).trimmed()
            << rx4.cap(2).trimmed();
            lineFormats.append(LPubVal3Format);
        }
    }
        break;
    case 5:
    {
        // VER_LPUB3D_LEGO_COLOR_PARTS
        QRegExp rx5("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(u|o)\\s+(.*)$");
        if (text.contains(rx5)) {
            tokens
            << rx5.cap(1).trimmed()
            << rx5.cap(2).trimmed()
            << rx5.cap(3).trimmed();
            lineFormats.append(LPubVal2Format);
            lineFormats.append(LPubVal3Format);
        }
    }
        break;
    default:
        return;
    }

    for (int i = 0; i < tokens.size(); i++) {
        if (index >= 0 && index < text.length()) {
            setFormat(index, tokens[i].length(), lineFormats[i]);
            index += tokens[i].length();
            for ( ; index < text.length(); index++) {  // move past blank spaces
                if (text[index] != ' ') {
                    break;
                }
            }
        }
    }
}

