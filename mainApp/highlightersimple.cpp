/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2020 Trevor SANDY. All rights reserved.
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
#include "name.h"

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
    LDrawHeaderFormat.setForeground(br03);  // Qt::blue
    LDrawHeaderFormat.setFontWeight(QFont::Bold);

    QStringList LDrawHeaderPatterns;
    LDrawHeaderPatterns
    << "\\bAUTHOR[^\n]*"
    << "\\bBFC[^\n]*"
    << "!?\\bCATEGORY[^\n]*"
    << "\\bCERTIFY[^\n]*"
    << "\\bCCW[^\n]*"
    << "\\bCLEAR[^\n]*"
    << "!?\\bCMDLINE[^\n]*"
    << "!?\\bHELP[^\n]*"
    << "!?\\bHISTORY[^\n]*"
    << "!?\\bKEYWORDS[^\n]*"
    << "!?\\bLDRAW_ORG[^\n]*"
    << "!?\\bLICENSE[^\n]*"
    << "\\bName[^\n]*"
    << "\\bPAUSE[^\n]*"
    << "\\bPRINT[^\n]*"
    << "\\bSAVE[^\n]*"
    << "\\bSTEP[^\n]*\\b"
    << "\\bWRITE[^\n]*\\b"
    << "\\bFILE[^\n]*"
    << "\\bNOFILE[^\n]*"
    << "!?\\bHELP[^\n]*"
    << "\\bOFFICIAL[^\n]*"
    << "\\bORIGINAL LDRAW[^\n]*"
    << "!?\\bTHEME[^\n]*"
    << "\\bUNOFFICIAL MODEL[^\n]*"
    << "\\bUN-OFFICIAL[^\n]*"
    << "\\bUNOFFICIAL[^\n]*"
    << "\\b~MOVED TO[^\n]*"
       ;

    Q_FOREACH (QString pattern, LDrawHeaderPatterns) {
        rule.pattern = QRegExp(pattern,Qt::CaseInsensitive);
        rule.format = LDrawHeaderFormat;
        highlightingRules.append(rule);
    }
    
    // Module (MLCAD, LeoCAD, LDCAD Meta Format
    ModuleMetaFormat.setForeground(br17);   // Qt::darkBlue
    ModuleMetaFormat.setFontWeight(QFont::Bold);

    QStringList ModuleMetaPatterns;
    ModuleMetaPatterns 
    << "\\bARROW[^\n]*"
    << "\\bBTG[^\n]*"
    << "\\bBUFEXCHG[^\n]*"
    << "\\bGHOST[^\n]*"
    << "\\bGROUP[^\n]*"
    << "\\bRETRIEVE[^\n]*"
    << "\\bROTATION CENTER[^\n]*"
    << "\\bROTATION CONFIG[^\n]*"
    << "\\bROTATION[^\n]*"
    << "\\bROTSTEP END[^\n]*"
    << "\\bROTSTEP[^\n]*"
    << "\\b(ABS|ADD|REL)$"
    << "\\bSKIP_BEGIN[^\n]*"
    << "\\bSKIP_END[^\n]*"
    << "\\bSTORE[^\n]*"
    
    << "\\bLDCAD\\b[^\n]*"
    << "\\b!LDCAD\\b[^\n]*"
    
    << "\\bLEOCAD\\b[^\n]*"
    << "\\b!LEOCAD\\b[^\n]*"
      ;

    Q_FOREACH (QString pattern, ModuleMetaPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = ModuleMetaFormat;

        highlightingRules.append(rule);
    }

    // LPub Meta Format
    LPubMetaFormat.setForeground(br25);  // Qt::darkRed
    LPubMetaFormat.setFontWeight(QFont::Bold);

    QStringList LPubMetaPatterns;
    LPubMetaPatterns
    << "\\bLPUB\\b[^\n]*"
    << "\\b!LPUB\\b[^\n]*"
    << "\\bPLIST\\b[^\n]*"
       ;

    Q_FOREACH (QString pattern, LPubMetaPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LPubMetaFormat;

        highlightingRules.append(rule);
    }

    // LSynth Meta Format
    LSynthMetaFormat.setForeground(br22);     // Qt::red
    LSynthMetaFormat.setFontWeight(QFont::Bold);

    QStringList LSynthMetaPatterns;
    LSynthMetaPatterns
    << "\\bSYNTH\\b[^\n]*"
    << "\\b!SYNTH\\b[^\n]*"
       ;

    Q_FOREACH (QString pattern, LSynthMetaPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LSynthMetaFormat;

        highlightingRules.append(rule);
    }

    // LDraw Comment Format
    LDrawCommentFormat.setForeground(br01);           // Qt::darkGreen
    LDrawCommentFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("0\\s+\\/\\/[^\n]*",Qt::CaseInsensitive);
    rule.format  = LDrawCommentFormat;
    highlightingRules.append(rule);
}

void HighlighterSimple::highlightBlock(const QString &text)
{
    Q_FOREACH (HighlightingRule rule, highlightingRules) {
        QRegExp expression(rule.pattern);

        int index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = text.indexOf(expression, index + length);
        }
    }
}

