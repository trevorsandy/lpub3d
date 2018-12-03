/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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
#include "highlighter.h"
#include "version.h"

#include "name.h"
#include "lpub_preferences.h"

#ifdef QT_DEBUG_MODE
#include "lpubalert.h"
#endif

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    QBrush br01,br02,br03,br04,br05,br06,br07,br08,br09,br10,br11,br12,br13,br14;
    QBrush br15,br16,br17,br18,br19,br20,br21,br22,br23,br24,br25,br26,br27,br28;
    if (Preferences::displayTheme == THEME_DEFAULT) {
        br01 = QBrush(QColor(THEME_HIGHLIGHT_01_DEFAULT));
        br02 = QBrush(QColor(THEME_HIGHLIGHT_02_DEFAULT));
        br03 = QBrush(QColor(THEME_HIGHLIGHT_03_DEFAULT));
        br04 = QBrush(QColor(THEME_HIGHLIGHT_04_DEFAULT));
        br05 = QBrush(QColor(THEME_HIGHLIGHT_05_DEFAULT));
        br06 = QBrush(QColor(THEME_HIGHLIGHT_06_DEFAULT));
        br07 = QBrush(QColor(THEME_HIGHLIGHT_07_DEFAULT));
        br08 = QBrush(QColor(THEME_HIGHLIGHT_08_DEFAULT));
        br09 = QBrush(QColor(THEME_HIGHLIGHT_09_DEFAULT));
        br10 = QBrush(QColor(THEME_HIGHLIGHT_10_DEFAULT));
        br11 = QBrush(QColor(THEME_HIGHLIGHT_11_DEFAULT));
        br12 = QBrush(QColor(THEME_HIGHLIGHT_12_DEFAULT));
        br13 = QBrush(QColor(THEME_HIGHLIGHT_13_DEFAULT));
        br14 = QBrush(QColor(THEME_HIGHLIGHT_14_DEFAULT));
        br15 = QBrush(QColor(THEME_HIGHLIGHT_15_DEFAULT));
        br16 = QBrush(QColor(THEME_HIGHLIGHT_16_DEFAULT));
        br17 = QBrush(QColor(THEME_HIGHLIGHT_17_DEFAULT));
        br18 = QBrush(QColor(THEME_HIGHLIGHT_18_DEFAULT));
        br19 = QBrush(QColor(THEME_HIGHLIGHT_19_DEFAULT));
        br20 = QBrush(QColor(THEME_HIGHLIGHT_20_DEFAULT));
        br21 = QBrush(QColor(THEME_HIGHLIGHT_21_DEFAULT));
        br22 = QBrush(QColor(THEME_HIGHLIGHT_22_DEFAULT));
        br23 = QBrush(QColor(THEME_HIGHLIGHT_23_DEFAULT));
        br24 = QBrush(QColor(THEME_HIGHLIGHT_24_DEFAULT));
        br25 = QBrush(QColor(THEME_HIGHLIGHT_25_DEFAULT));
        br26 = QBrush(QColor(THEME_HIGHLIGHT_26_DEFAULT));
        br27 = QBrush(QColor(THEME_HIGHLIGHT_27_DEFAULT));
        br28 = QBrush(QColor(THEME_HIGHLIGHT_28_DEFAULT));
      }
    else
    if (Preferences::displayTheme == THEME_DARK) {
        br01 = QBrush(QColor(THEME_HIGHLIGHT_01_DARK));
        br02 = QBrush(QColor(THEME_HIGHLIGHT_02_DARK));
        br03 = QBrush(QColor(THEME_HIGHLIGHT_03_DARK));
        br04 = QBrush(QColor(THEME_HIGHLIGHT_04_DARK));
        br05 = QBrush(QColor(THEME_HIGHLIGHT_05_DARK));
        br06 = QBrush(QColor(THEME_HIGHLIGHT_06_DARK));
        br07 = QBrush(QColor(THEME_HIGHLIGHT_07_DARK));
        br08 = QBrush(QColor(THEME_HIGHLIGHT_08_DARK));
        br09 = QBrush(QColor(THEME_HIGHLIGHT_09_DARK));
        br10 = QBrush(QColor(THEME_HIGHLIGHT_10_DARK));
        br11 = QBrush(QColor(THEME_HIGHLIGHT_11_DARK));
        br12 = QBrush(QColor(THEME_HIGHLIGHT_12_DARK));
        br13 = QBrush(QColor(THEME_HIGHLIGHT_13_DARK));
        br14 = QBrush(QColor(THEME_HIGHLIGHT_14_DARK));
        br15 = QBrush(QColor(THEME_HIGHLIGHT_15_DARK));
        br16 = QBrush(QColor(THEME_HIGHLIGHT_16_DARK));
        br17 = QBrush(QColor(THEME_HIGHLIGHT_17_DARK));
        br18 = QBrush(QColor(THEME_HIGHLIGHT_18_DARK));
        br19 = QBrush(QColor(THEME_HIGHLIGHT_19_DARK));
        br20 = QBrush(QColor(THEME_HIGHLIGHT_20_DARK));
        br21 = QBrush(QColor(THEME_HIGHLIGHT_21_DARK));
        br22 = QBrush(QColor(THEME_HIGHLIGHT_22_DARK));
        br23 = QBrush(QColor(THEME_HIGHLIGHT_23_DARK));
        br24 = QBrush(QColor(THEME_HIGHLIGHT_24_DARK));
        br25 = QBrush(QColor(THEME_HIGHLIGHT_25_DARK));
        br26 = QBrush(QColor(THEME_HIGHLIGHT_26_DARK));
        br27 = QBrush(QColor(THEME_HIGHLIGHT_27_DARK));
        br28 = QBrush(QColor(THEME_HIGHLIGHT_28_DARK));
      }

    // LDraw Header Value Format
    LDrawHeaderValueFormat.setForeground(br26);
    LDrawHeaderValueFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("^.*\\b(?:AUTHOR|BFC|CATEGORY|CMDLINE|COLOUR|HELP|HISTORY|KEYWORDS|LDRAW_ORG|LICENSE|NAME|FILE|THEME|~MOVED TO)\\b.*$",Qt::CaseInsensitive);
    rule.format = LDrawHeaderValueFormat;
    highlightingRules.append(rule);

    // LDraw Header Format
    LDrawHeaderFormat.setForeground(br02);
    LDrawHeaderFormat.setFontWeight(QFont::Bold);

    QStringList LDrawHeaderPatterns;
    LDrawHeaderPatterns
    << "\\bAUTHOR\\b\\W"
    << "\\bBFC\\b"
    << "\\W\\bCATEGORY\\b"
    << "\\bCLEAR\\b"
    << "\\W\\bCMDLINE\\b"
    << "\\W\\bCOLOUR\\b"
    << "\\W\\bHELP\\b"
    << "\\W\\bHISTORY\\b"
    << "\\W\\bKEYWORDS\\b"
    << "\\W\\bLDRAW_ORG\\b"
    << "\\W\\bLICENSE\\b"
    << "\\bNAME\\b\\W"
    << "\\bFILE\\b"
    << "\\bNOFILE\\b"
    << "\\W\\bHELP\\b"
    << "\\bOFFICIAL\\b"
    << "\\bORIGINAL LDRAW\\b"
    << "\\W\\bTHEME\\b"
    << "\\bUNOFFICIAL MODEL\\b"
    << "\\bUN-OFFICIAL\\b"
    << "\\bUNOFFICIAL\\b"
    << "\\b~MOVED TO\\b";

    foreach (QString pattern, LDrawHeaderPatterns) {
        rule.pattern = QRegExp(pattern,Qt::CaseInsensitive);
        rule.format = LDrawHeaderFormat;
        highlightingRules.append(rule);
    }

    // LDraw Body Format
    LDrawBodyFormat.setForeground(br03);
    LDrawBodyFormat.setFontWeight(QFont::Bold);

    QStringList LDrawBodyPatterns;
    LDrawBodyPatterns
    << "\\bPAUSE\\b"
    << "\\bPRINT\\b"
    << "\\bSAVE\\b"
    << "\\bNOSTEP\\b"
    << "\\bSTEP\\b"
    << "\\bWRITE\\b";

    foreach (QString pattern, LDrawBodyPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LDrawBodyFormat;
        highlightingRules.append(rule);
    }

    // LPub3D Quoted Text Format
    LPubQuotedTextFormat.setForeground(br27);
    LPubQuotedTextFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("\".*\"");
    rule.format = LPubQuotedTextFormat;
    highlightingRules.append(rule);

    // LPub3D Number Format
    LPubNumberFormat.setForeground(br14);
    LPubNumberFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("-?(?:0|[1-9]\\d*)(?:\\.\\d+)?");
    rule.format = LPubNumberFormat;
    highlightingRules.append(rule);

    // LPub3D Hex Number Format
    LPubHexNumberFormat.setForeground(br15);
    LPubHexNumberFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("#(?:[A-Fa-f0-9]{6}|[A-Fa-f0-9]{3})",Qt::CaseInsensitive);
    rule.format = LPubHexNumberFormat;
    highlightingRules.append(rule);

    // LPub3D Meta Format
    LPubMetaFormat.setForeground(br24);
    LPubMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\W\\bLPUB\\b");
    rule.format = LPubMetaFormat;
    highlightingRules.append(rule);

    // LPub3D Local Context Format
    LPubLocalMetaFormat.setForeground(br04);
    LPubLocalMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\bLOCAL\\b");
    rule.format = LPubLocalMetaFormat;
    highlightingRules.append(rule);

    // LPub3D Global Context Format
    LPubGlobalMetaFormat.setForeground(br05);
    LPubGlobalMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\bGLOBAL\\b");
    rule.format = LPubGlobalMetaFormat;
    highlightingRules.append(rule);

    // LPub3D Boolean False Format
    LPubFalseMetaFormat.setForeground(br22);
    LPubFalseMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\bFALSE\\b");
    rule.format = LPubFalseMetaFormat;
    highlightingRules.append(rule);

    // LPub3D Boolean True Format
    LPubTrueMetaFormat.setForeground(br23);
    LPubTrueMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\bTRUE\\b");
    rule.format = LPubTrueMetaFormat;
    highlightingRules.append(rule);

    // LPub3D Page Size Format
    LPubPageSizeFormat.setForeground(br16);
    LPubPageSizeFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\b[A|B][0-9]0?$\\b|\\bComm10E\\b$|\\bArch[1-3]\\b$",Qt::CaseInsensitive);
    rule.format = LPubPageSizeFormat;
    highlightingRules.append(rule);

    // LPub3D Body Meta Format
    LPubBodyMetaFormat.setForeground(br25);
    LPubBodyMetaFormat.setFontWeight(QFont::Bold);

    QStringList LPubBodyMetaPatterns;
    LPubBodyMetaPatterns
    << "\\bALLOC\\b"
    << "\\bANNOTATE\\b"
    << "\\bANNOTATION\\b"
    << "\\bAPP_PLUG\\b"
    << "\\bAPP_PLUG_IMAGE\\b"
    << "\\bAREA\\b"
    << "\\bASSEM\\b"
    << "\\bBACK\\b"
    << "\\bBACKGROUND\\b"
    << "\\bBEGIN\\b"
    << "\\bBOM\\b"
    << "\\bBORDER\\b"
    << "\\bBOTTOM\\b"
    << "\\bBOTTOM_LEFT\\b"
    << "\\bBOTTOM_RIGHT\\b"
    << "\\bBTG\\b"
    << "\\bCALLOUT\\b"
    << "\\bCAMERA_DISTANCE_NATIVE\\b"
    << "\\bCENTER\\b"
    << "\\bCLEAR\\b"
    << "\\bCOLOR\\b"
    << "\\bCOLS\\b"
    << "\\bCONSOLIDATE_INSTANCE_COUNT\\b"
    << "\\bCONSTRAIN\\b"
    << "\\bCONTENT\\b"
    << "\\bCROSS\\b"
    << "\\bCOVER_PAGE\\b"
    << "\\bDISPLAY\\b"
    << "\\bDISPLAY_PAGE_NUMBER\\b"
    << "\\bDIVIDER\\b"
    << "\\bDOCUMENT_AUTHOR\\b"
    << "\\bDOCUMENT_AUTHOR_BACK\\b"
    << "\\bDOCUMENT_AUTHOR_FRONT\\b"
    << "\\bDOCUMENT_COVER_IMAGE\\b"
    << "\\bDOCUMENT_LOGO_BACK\\b"
    << "\\bDOCUMENT_LOGO_FRONT\\b"
    << "\\bDOCUMENT_TITLE_BACK\\b"
    << "\\bDOCUMENT_TITLE_FRONT\\b"
    << "\\bDPCM\\b"
    << "\\bDPI\\b"
    << "\\bEND\\b"
    << "\\bFACTOR\\b"
    << "\\bFADE\\b"
    << "\\bFADE_COLOR\\b"
    << "\\bFADE_OPACITY\\b"
    << "\\bFADE_STEP\\b"
    << "\\bFONT\\b"
    << "\\bFONT_COLOR\\b"
    << "\\bFREEFORM\\b"
    << "\\bFRONT\\b"
    << "\\bGRADIENT\\b"
    << "\\bGROUP\\b"
    << "\\bHEIGHT\\b"
    << "\\bHIDE\\b"
    << "\\bHIGHLIGHT\\b"
    << "\\bHIGHLIGHT_COLOR\\b"
    << "\\bHIGHLIGHT_LINE_WIDTH\\b"
    << "\\bHIGHLIGHT_STEP\\b"
    << "\\bHORIZONTAL\\b"
    << "\\bIGN\\b"
    << "\\bINCLUDE\\b"
    << "\\bINCLUDE_SUBMODELS\\b"
    << "\\bINSERT\\b"
    << "\\bINSERT MODEL\\b"
    << "\\bINSIDE\\b"
    << "\\bINSTANCE_COUNT\\b"
    << "\\bLANDSCAPE\\b"
    << "\\bLDGLITE_PARMS\\b"
    << "\\bLDVIEW_PARMS\\b"
    << "\\bLEFT\\b"
    << "\\bLEGO_DISCLAIMER\\b"
    << "\\bMARGINS\\b"
    << "\\bMODEL_CATEGORY\\b"
    << "\\bMODEL_DESCRIPTION\\b"
    << "\\bMODEL_ID\\b"
    << "\\bMODEL_PARTS\\b"
    << "\\bMODEL_SCALE\\b"
    << "\\bMULTI_STEP\\b"
    << "\\bNONE\\b"
    << "\\bNOSTEP\\b"
    << "\\bNUMBER\\b"
    << "\\bOFFSET\\b"
    << "\\bORIENTATION\\b"
    << "\\bOUTIDE\\b"
    << "\\bOUTSIDE\\b"
    << "\\bPAGE\\b"
    << "\\bPAGE_FOOTER\\b"
    << "\\bPAGE_HEADER\\b"
    << "\\bPART\\b"
    << "\\bPER_STEP\\b"
    << "\\bPICTURE\\b"
    << "\\bPLACEMENT\\b"
    << "\\bPLI\\b"
    << "\\bPOINTER\\b"
    << "\\bPORTRAIT\\b"
    << "\\bPOVRAY_PARMS\\b"
    << "\\bPUBLISH_COPYRIGHT\\b"
    << "\\bPUBLISH_COPYRIGHT_BACK\\b"
    << "\\bPUBLISH_DESCRIPTION\\b"
    << "\\bPUBLISH_EMAIL\\b"
    << "\\bPUBLISH_EMAIL_BACK\\b"
    << "\\bPUBLISH_URL\\b"
    << "\\bPUBLISH_URL_BACK\\b"
    << "\\bREMOVE\\b"
    << "\\bRESERVE\\b"
    << "\\bRESOLUTION\\b"
    << "\\bRIGHT\\b"
    << "\\bROTATED\\b"
    << "\\bROTATE_ICON\\b"
    << "\\bROUND\\b"
    << "\\bSCALE\\b"
    << "\\bSEPARATOR\\b"
    << "\\bSHOW\\b"
    << "\\bSHOW_STEP_NUMBER\\b"
    << "\\bSHOW_TOP_MODEL\\b"
    << "\\bSIZE\\b"
    << "\\bSORT\\b"
    << "\\bSORT_BY\\b"
    << "\\bSORT_OPTION\\b"
    << "\\bSQUARE\\b"
    << "\\bSTEP_NUMBER\\b"
    << "\\bSTEP_PLI\\b"
    << "\\bSTRETCH\\b"
    << "\\bSUB\\b"
    << "\\bSUBMODEL_DISPLAY\\b"
    << "\\bSUBMODEL_ROTATION\\b"
    << "\\bSUBMODEL_BACKGROUND_COLOR\\b"
    << "\\bSUBMODEL_FONT\\b"
    << "\\bSUBMODEL_FONT_COLOR\\b"
    << "\\bSUBMODEL_INSTANCE_COUNT\\b"
    << "\\bSYNTHESIZED\\b"
    << "\\bTEXT\\b"
    << "\\bTILE\\b"
    << "\\bTOGGLE_PAGE_NUMBER_PLACEMENT\\b"
    << "\\bTOP\\b"
    << "\\bTOP_LEFT\\b"
    << "\\bTOP_RIGHT\\b"
    << "\\bTRANSPARENT\\b"
    << "\\bUSE_FADE_COLOR\\b"
    << "\\bUSE_FREE_FORM\\b"
    << "\\bUSE_TITLE\\b"
    << "\\bUSE_TITLE_AND_FREE_FORM\\b"
    << "\\bVERTICAL\\b"
    << "\\bVIEW_ANGLE\\b"
    << "\\bWHOLE\\b"
    << "\\bWIDTH\\b";

    foreach (QString pattern, LPubBodyMetaPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LPubBodyMetaFormat;
        highlightingRules.append(rule);
    }

    // LDraw Meta Line Format - This should come at the end
    // to overwrite any formats that impact the first 0
    LDrawLineType0Format.setForeground(br28);
    LDrawLineType0Format.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("^0");
    rule.format = LDrawLineType0Format;
    highlightingRules.append(rule);

    // LDraw Lines 2-5 Format
    LDrawLineType2_5Format.setForeground(br13);
    LDrawLineType2_5Format.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("^[2-5][^\n]*");
    rule.format = LDrawLineType2_5Format;
    highlightingRules.append(rule);

    // MLCad Meta Format
    MLCadMetaFormat.setForeground(br21);
    MLCadMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\W\\bMLCAD\\b");
    rule.format = MLCadMetaFormat;
    highlightingRules.append(rule);

    // MLCad Body Meta Format
    MLCadBodyMetaFormat.setForeground(br17);
    MLCadBodyMetaFormat.setFontWeight(QFont::Bold);

    QStringList MLCadBodyMetaPatterns;
    MLCadBodyMetaPatterns
    << "\\bROTSTEP\\b"
    << "\\bROTATION\\b"
    << "\\bROTSTEP END\\b"
    << "\\bROTATION CENTER\\b"
    << "\\bROTATION CONFIG\\b"
    << "\\bBUFEXCHG\\b"
    << "\\bGHOST\\b"
    << "\\bGROUP\\b"
    << "\\bREL\\b"
    << "\\bABS\\b"
    << "\\bADD\\b"
    << "\\bSTORE\\b"
    << "\\bRETRIEVE\\b"
    << "\\bARROW\\b"
    << "\\bSKIP_BEGIN\\b"
    << "\\bSKIP_END\\b";

    foreach (QString pattern, MLCadBodyMetaPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = MLCadBodyMetaFormat;
        highlightingRules.append(rule);
    }

    // LSynth Format
    LSynthMetaFormat.setForeground(br18);
    LSynthMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\W\\bSYNTH\\b[^\n]*");
    rule.format = LSynthMetaFormat;
    highlightingRules.append(rule);

    // LDCad Format
    LDCadMetaFormat.setForeground(br19);
    LDCadMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\W\\bLDCAD\\b[^\n]*");
    rule.format = LDCadMetaFormat;
    highlightingRules.append(rule);

    // LeoCAD Format
    LeoCADMetaFormat.setForeground(br20);
    LeoCADMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\W\\bLEOCAD\\b[^\n]*");
    rule.format = LeoCADMetaFormat;
    highlightingRules.append(rule);

    /* Line format only - no rules */

    // LDraw Line Type Format
    LDrawLineType1Format.setForeground(br06);
    LDrawLineType1Format.setFontWeight(QFont::Bold);
    lineType1Formats.append(LDrawLineType1Format);

    // LDraw Color Format
    LDrawColorFormat.setForeground(br07);
    LDrawColorFormat.setFontWeight(QFont::Bold);
    lineType1Formats.append(LDrawColorFormat);

    // LDraw Position Format
    LDrawPositionFormat.setForeground(br08);
    LDrawPositionFormat.setFontWeight(QFont::Normal);
    lineType1Formats.append(LDrawPositionFormat);

    // LDraw Transform1 Format
    LDrawTransform1Format.setForeground(br09);
    LDrawTransform1Format.setFontWeight(QFont::Normal);
    lineType1Formats.append(LDrawTransform1Format);
    // LDraw Transform2 Format
    LDrawTransform2Format.setForeground(br10);
    LDrawTransform2Format.setFontWeight(QFont::Normal);
    lineType1Formats.append(LDrawTransform2Format);
    // LDraw Transform3 Format
    LDrawTransform3Format.setForeground(br11);
    LDrawTransform3Format.setFontWeight(QFont::Normal);
    lineType1Formats.append(LDrawTransform3Format);

    // LDraw Part File Format
    LDrawFileFormat.setForeground(br12);
    LDrawFileFormat.setFontWeight(QFont::Bold);
    lineType1Formats.append(LDrawFileFormat);

    // LDraw Comment Format
    LDrawCommentFormat.setForeground(br01);
    LDrawCommentFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("0\\s+\\/\\/[^\n]*",Qt::CaseInsensitive);
    rule.format = LDrawCommentFormat;
    highlightingRules.append(rule);
}

void Highlighter::highlightBlock(const QString &text)
{
    // apply the predefined rules
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

    if (text.contains(QRegExp("^1"))) {
        QStringList tt = text.split(" ");
        QString part;
        for (int t = 14; t < tt.size(); t++)
            part += (tt[t]+" ");
        QStringList tokens;
        tokens  << tt[0]                        // - part type
                << tt[1]                        // - color
                << tt[2]+" "+tt[3]+" "+tt[4]    // - position
                << tt[5]+" "+tt[6]+" "+tt[7]    // - transform
                << tt[8]+" "+tt[9]+" "+tt[10]   // - transform
                << tt[11]+" "+tt[12]+" "+tt[13] // - transform
                << part.trimmed();              // - part
        int index = 0;
        for (int i = 0; i < tokens.size(); i++) {
            if (index >= 0 && index < text.length()) {
                setFormat(index, tokens[i].length(), lineType1Formats[i]);
                index += tokens[i].length() + 1;// add 1 position for the space
            }
        }
    }
}
