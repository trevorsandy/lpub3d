/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2020 - 2024 Trevor SANDY. All rights reserved.
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
#include "highlightersimple.h"
#include "application.h"
#include "lpub_preferences.h"
#include "declarations.h"

HighlighterSimple::HighlighterSimple(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    
    QBrush br01,br03,br17,br22,br25;
    if (Application::instance()->getTheme() == THEME_DEFAULT) {
        br01 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_COMMENTS]));
        br03 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_BODY]));
        br17 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LEOCAD]));
        br22 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_FALSE]));
        br25 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_BODY]));
    }
    else
    if (Application::instance()->getTheme() == THEME_DARK) {
        br01 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_COMMENTS]));
        br03 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_BODY]));
        br17 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LEOCAD]));
        br22 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_FALSE]));
        br25 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_BODY]));
    }
    
    // LDraw Header Format
    LDrawHeaderFormat.setForeground(br03);
    LDrawHeaderFormat.setFontWeight(QFont::Bold);

    const QString LDrawHeaderPatterns[] =
    {
        QStringLiteral("\\bAUTHOR\\b"),
        QStringLiteral("\\bBFC\\b"),
        QStringLiteral("!?\\bCATEGORY\\b"),
        QStringLiteral("\\bCERTIFY\\b"),
        QStringLiteral("\\bCCW\\b"),
        QStringLiteral("\\bCLEAR\\b"),
        QStringLiteral("!?\\bCMDLINE\\b"),
        QStringLiteral("!?\\bHELP\\b"),
        QStringLiteral("!?\\bHISTORY\\b"),
        QStringLiteral("!?\\bKEYWORDS\\b"),
        QStringLiteral("!?\\bLDRAW_ORG\\b"),
        QStringLiteral("!?\\bLICENSE\\b"),
        QStringLiteral("\\bNAME\\b"),
        QStringLiteral("^(?!0 !LPUB|1).*\\bFILE\\b"),
        QStringLiteral("\\bNOFILE\\b"),
        QStringLiteral("!?\\bHELP\\b"),
        QStringLiteral("\\bOFFICIAL\\b"),
        QStringLiteral("\\bORIGINAL LDRAW\\b"),
        QStringLiteral("!?\\bTHEME\\b"),
        QStringLiteral("\\bUNOFFICIAL MODEL\\b"),
        QStringLiteral("\\bUN-OFFICIAL\\b"),
        QStringLiteral("\\bUNOFFICIAL\\b"),
        QStringLiteral("\\bUNOFFICIAL_PART\\b"),
        QStringLiteral("\\bUNOFFICIAL_SUBPART\\b"),
        QStringLiteral("\\bUNOFFICIAL_SHORTCUT\\b"),
        QStringLiteral("\\bUNOFFICIAL_PRIMITIVE\\b"),
        QStringLiteral("\\bUNOFFICIAL_8_PRIMITIVE\\b"),
        QStringLiteral("\\bUNOFFICIAL_48_PRIMITIVE\\b"),
        QStringLiteral("\\bUNOFFICIAL_PART ALIAS\\b"),
        QStringLiteral("\\bUNOFFICIAL_SHORTCUT ALIAS\\b"),
        QStringLiteral("\\bUNOFFICIAL_PART PHYSICAL_COLOUR\\b"),
        QStringLiteral("\\bUNOFFICIAL_SHORTCUT PHYSICAL_COLOUR\\b"),
        QStringLiteral("\\bUNOFFICIAL PART\\b"),
        QStringLiteral("\\bUNOFFICIAL SUBPART\\b"),
        QStringLiteral("\\bUNOFFICIAL SHORTCUT\\b"),
        QStringLiteral("\\bUNOFFICIAL PRIMITIVE\\b"),
        QStringLiteral("\\bUNOFFICIAL 8_PRIMITIVE\\b"),
        QStringLiteral("\\bUNOFFICIAL 48_PRIMITIVE\\b"),
        QStringLiteral("\\bUNOFFICIAL PART ALIAS\\b"),
        QStringLiteral("\\bUNOFFICIAL SHORTCUT ALIAS\\b"),
        QStringLiteral("\\bUNOFFICIAL PART PHYSICAL_COLOUR\\b"),
        QStringLiteral("\\bUNOFFICIAL SHORTCUT PHYSICAL_COLOUR\\b"),
        QStringLiteral("\\b~MOVED TO\\b")
    };

    for (const QString &pattern : LDrawHeaderPatterns) {
        rule.pattern = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        rule.format = LDrawHeaderFormat;
        highlightingRules.append(rule);
    }

    // LDraw Body Format
    LDrawBodyFormat.setForeground(br03);
    LDrawBodyFormat.setFontWeight(QFont::Bold);

    const QString LDrawBodyPatterns[] =
    {
        QStringLiteral("\\bPAUSE\\b[^\n]*"),
        QStringLiteral("\\bPRINT\\b[^\n]*"),
        QStringLiteral("\\bSAVE\\b[^\n]*"),
        QStringLiteral("\\bNOSTEP\\b[^\n]*"),
        QStringLiteral("\\bSTEP\\b[^\n]*"),
        QStringLiteral("\\bWRITE\\b[^\n]*")
    };

    for (const QString &pattern : LDrawBodyPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = LDrawBodyFormat;
        highlightingRules.append(rule);
    }
    
    // Module (MLCAD, LeoCAD, LDCAD Meta Format
    ModuleMetaFormat.setForeground(br17);   // Qt::darkBlue
    ModuleMetaFormat.setFontWeight(QFont::Bold);

    const QString ModuleMetaPatterns[] =
    {
        QStringLiteral("\\bABS\\b[^\n]*"),
        QStringLiteral("\\bADD\\b[^\n]*"),
        QStringLiteral("\\bMLCAD ARROW\\b[^\n]*"),
        QStringLiteral("\\bBACKGROUND\\b[^\n]*"),
        QStringLiteral("\\bBTG\\b[^\n]*"),
        QStringLiteral("\\bBUFEXCHG\\b[^\n]*"),
        QStringLiteral("\\bFLEXHOSE\\b[^\n]*"),
        QStringLiteral("\\bGHOST\\b[^\n]*"),
        QStringLiteral("\\bGROUP\\b[^\n]*"),
        QStringLiteral("\\bHIDE\\b[^\n]*"),
        QStringLiteral("\\bREL\\b[^\n]*"),
        QStringLiteral("\\bRETRIEVE\\b[^\n]*"),
        QStringLiteral("\\bROTATION AXLE\\b[^\n]*"),
        QStringLiteral("\\bROTATION CENTER\\b[^\n]*"),
        QStringLiteral("\\bROTATION CONFIG\\b[^\n]*"),
        QStringLiteral("\\bROTATION\\b[^\n]*"),
        QStringLiteral("\\bROTSTEP END\\b[^\n]*"),
        QStringLiteral("\\bROTSTEP\\b[^\n]*"),
        QStringLiteral("\\bSKIP_BEGIN\\b[^\n]*"),
        QStringLiteral("\\bSKIP_END\\b[^\n]*"),
        QStringLiteral("\\bSTORE\\b[^\n]*"),

        QStringLiteral("!?\\bMLCAD\\b[^\n]*"),
        QStringLiteral("!?\\bLDCAD\\b[^\n]*"),
        QStringLiteral("!?\\bLEOCAD\\b[^\n]*"),
    };

    for (const QString &pattern : ModuleMetaPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = ModuleMetaFormat;

        highlightingRules.append(rule);
    }

    // LPub Meta Format
    LPubMetaFormat.setForeground(br25);  // Qt::darkRed
    LPubMetaFormat.setFontWeight(QFont::Bold);

    const QString LPubMetaPatterns[] =
    {
        QStringLiteral("!?\\bLPUB\\b[^\n]*"),
        QStringLiteral("\\bPLIST\\b[^\n]*")
    };

    for (const QString &pattern : LPubMetaPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = LPubMetaFormat;

        highlightingRules.append(rule);
    }

    // LSynth Meta Format
    LSynthMetaFormat.setForeground(br22);     // Qt::red
    LSynthMetaFormat.setFontWeight(QFont::Bold);

    const QString LSynthMetaPatterns[] =
    {
        QStringLiteral("\\bSYNTH\\b[^\n]*"),
        QStringLiteral("\\b!SYNTH\\b[^\n]*")
    };

    for (const QString &pattern : LSynthMetaPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = LSynthMetaFormat;

        highlightingRules.append(rule);
    }

    // LDraw Comment Format
    LDrawCommentFormat.setForeground(br01);   // Qt::darkGreen
    LDrawCommentFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression(QStringLiteral("0\\s{1}//[^\n]*"));
    rule.format = LDrawCommentFormat;
    highlightingRules.append(rule);

    // LDrww Multi-line Command Format
    LDrawMultiLineCommentFormat.setForeground(br01);

    LDrawMultiLineCommentStartExpression = QRegularExpression(QStringLiteral("0\\s{1}\\/\\*[^\n]*"));
    LDrawMultiLineCommentEndExpression = QRegularExpression(QStringLiteral("0\\s{1}\\*\\/[^\n]*"));
}

void HighlighterSimple::highlightBlock(const QString &text)
{
    // apply the predefined rules
    const QVector<HighlightingRule> rules = highlightingRules;
    for (const HighlightingRule &rule : rules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Multi-line comments 0 /*  0 */
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(LDrawMultiLineCommentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = LDrawMultiLineCommentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, LDrawMultiLineCommentFormat);
        startIndex = text.indexOf(LDrawMultiLineCommentStartExpression, startIndex + commentLength);
    }
}

