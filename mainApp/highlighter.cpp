/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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
    QBrush br15,br16,br17,br18,br19,br20,br21,br22,br23,br24,br25,br26,br27,br28,br29;
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
        br29 = QBrush(QColor(THEME_HIGHLIGHT_29_DEFAULT));
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
        br29 = QBrush(QColor(THEME_HIGHLIGHT_29_DARK));
      }

    // LPub3D Number Format
    LPubNumberFormat.setForeground(br14);
    LPubNumberFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("-?(?:0|[1-9]\\d*)(?:\\.\\d+)?");
    rule.format = LPubNumberFormat;
    highlightingRules.append(rule);

    // LDraw Custom COLOUR Description Format
    LDrawColourDescFormat.setForeground(br26);
    LDrawColourDescFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\bLPub3D_[A-Z|a-z|_]+\\b");
    rule.format = LDrawColourDescFormat;
    highlightingRules.append(rule);

    // LPub3D Quoted Text Format
    LPubQuotedTextFormat.setForeground(br27);
    LPubQuotedTextFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("\".*\"");
    rule.format = LPubQuotedTextFormat;
    highlightingRules.append(rule);

    // LPub3D Hex Number Format
    LPubHexNumberFormat.setForeground(br15);
    LPubHexNumberFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("#(?:[A-Fa-f0-9]{6}|[A-Fa-f0-9]{3})",Qt::CaseInsensitive);
    rule.format = LPubHexNumberFormat;
    highlightingRules.append(rule);

    // LPub3D Font Number Format
    LPubFontNumberFormat.setForeground(br14);
    LPubFontNumberFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("[,|-](\\d+)"); // match digit if preceded by , or -
    rule.format = LPubFontNumberFormat;
    highlightingRules.append(rule);

    // LPub3D Substitute Color Format
    LPubSubColorFormat.setForeground(br07);
    LPubSubColorFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("BEGIN\\sSUB\\s.*.[dat|mpd|ldr]\\s([0-9]+)",Qt::CaseInsensitive); // match color format if preceded by 'BEGIN SUB *.ldr|dat|mpd '
    rule.format = LPubSubColorFormat;
    highlightingRules.append(rule);

    // LPub3D Custom COLOUR Code Format
    LPubCustomColorFormat.setForeground(br07);
    LPubCustomColorFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("CODE\\s([0-9]+)\\sVALUE",Qt::CaseInsensitive); // match color format if preceded by 'CODE ' and followed by ' VALUE'
    rule.format = LPubCustomColorFormat;
    highlightingRules.append(rule);

    // LPub3D Substitute Part Format
    LPubSubPartFormat.setForeground(br12);
    LPubSubPartFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("BEGIN\\sSUB\\s([A-Za-z0-9\\s_-]+.[dat|mpd|ldr]+)",Qt::CaseInsensitive); // match part format if preceded by 'BEGIN SUB '
    rule.format = LPubSubPartFormat;
    highlightingRules.append(rule);

    // LPub3D Font Number Comma Format
    LPubFontCommaFormat.setForeground(br27);
    LPubFontCommaFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("[,]");
    rule.format = LPubFontCommaFormat;
    highlightingRules.append(rule);

    // LPub3D Page Size Format
    LPubPageSizeFormat.setForeground(br16);
    LPubPageSizeFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\b[A|B][0-9]0?$\\b|\\bComm10E\\b$|\\bArch[1-3]\\b$",Qt::CaseInsensitive);
    rule.format = LPubPageSizeFormat;
    highlightingRules.append(rule);

    // LDraw Custom COLOUR Meta Format
    LDrawColourMetaFormat.setForeground(br05);
    LDrawColourMetaFormat.setFontWeight(QFont::Bold);

    QStringList LDrawColourPatterns;
    LDrawColourPatterns
    << "\\bCOLOUR\\b"
    << "\\bCODE\\b"
    << "\\bVALUE\\b"
    << "\\bEDGE\\b"
    << "\\bALPHA\\b"
    ;

    foreach (QString pattern, LDrawColourPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LDrawColourMetaFormat;
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
    << "\\bWRITE\\b"
       ;

    foreach (QString pattern, LDrawBodyPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LDrawBodyFormat;
        highlightingRules.append(rule);
    }

    // LPub3D Meta Format
    LPubMetaFormat.setForeground(br24);
    LPubMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("!?\\bLPUB\\b");
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

    // LPub3D Body Meta Format
    LPubBodyMetaFormat.setForeground(br25);
    LPubBodyMetaFormat.setFontWeight(QFont::Bold);

    QStringList LPubBodyMetaPatterns;
    LPubBodyMetaPatterns
    << "\\bADJUST_ON_ITEM_OFFSET\\b"
    << "\\bASSEM_PART\\b"
    << "\\bAFTER\\b"
    << "\\bALLOC\\b"
    << "\\bANNOTATE\\b"
    << "\\bANNOTATION\\b"
    << "\\bANNOTATION_SHOW\\b"
    << "\\bAPP_PLUG\\b"
    << "\\bAPP_PLUG_IMAGE\\b"
    << "\\bAREA\\b"
    << "\\bASPECT\\b"
    << "\\bASSEM\\b"
    << "\\bASSEM_PART\\b"
    << "\\bAT_MODEL\\b"
    << "\\bAT_STEP\\b"
    << "\\bAT_TOP\\b"
    << "\\bATTRIBUTE_PIXMAP\\b"
    << "\\bATTRIBUTE_TEXT\\b"
    << "\\bAXLE\\b"
    << "\\bBACK\\b"
    << "\\bBACKGROUND\\b"
    << "\\bBASE_BOTTOM\\b"
    << "\\bBASE_BOTTOM_LEFT\\b"
    << "\\bBASE_BOTTOM_RIGHT\\b"
    << "\\bBASE_CENTER\\b"
    << "\\bBASE_LEFT\\b"
    << "\\bBASE_RIGHT\\b"
    << "\\bBASE_TOP\\b"
    << "\\bBASE_TOP_LEFT\\b"
    << "\\bBASE_TOP_RIGHT\\b"
    << "\\bBEAM\\b"
    << "\\bBEFORE\\b"
    << "\\bBEGIN\\b"
    << "\\bBOM\\b"
    << "\\bBORDER\\b"
    << "\\bBOTTOM\\b"
    << "\\bBOTTOM_LEFT\\b"
    << "\\bBOTTOM_RIGHT\\b"
    << "\\bBRICKLINK\\b"
    << "\\bBRING_TO_FRONT\\b"
    << "\\bCABLE\\b"
    << "\\bCALLOUT\\b"
    << "\\bCALLOUT_ASSEM\\b"
    << "\\bCALLOUT_INSTANCE\\b"
    << "\\bCALLOUT_POINTER\\b"
    << "\\bCALLOUT_UNDERPINNING\\b"
    << "\\bCAMERA_ANGLES\\b"
    << "\\bCAMERA_DISTANCE\\b"
    << "\\bCAMERA_DISTANCE_NATIVE\\b"
    << "\\bCAMERA_FOV\\b"
    << "\\bCAMERA_ORTHOGRAPHIC\\b"
    << "\\bCAMERA_TARGET\\b"
    << "\\bCAMERA_ZFAR\\b"
    << "\\bCAMERA_ZNEAR\\b"
    << "\\bCENTER\\b"
    << "\\bCIRCLE_STYLE\\b"
    << "\\bCLEAR\\b"
    << "\\bCOLOR\\b"
    << "\\bCOLS\\b"
    << "\\bCONNECTOR\\b"
    << "\\bCONSOLIDATE_INSTANCE_COUNT\\b"
    << "\\bCONSTRAIN\\b"
    << "\\bCONTENT\\b"
    << "\\bCONTINUOUS_STEP_NUMBERS\\b"
    << "\\bCOVER_PAGE\\b"
    << "\\bCROSS\\b"
    << "\\bCSI_ANNOTATION\\b"
    << "\\bCSI_ANNOTATION_PART\\b"
    << "\\bCUSTOM_LENGTH\\b"
    << "\\bDISPLAY\\b"
    << "\\bDISPLAY_PAGE_NUMBER\\b"
    << "\\bDIVIDER\\b"
    << "\\bDIVIDER_ITEM\\b"
    << "\\bDIVIDER_LINE\\b"
    << "\\bDIVIDER_POINTER\\b"
    << "\\bDIVIDER_POINTER_ATTRIBUTE\\b"
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
    << "\\bELEMENT_STYLE\\b"
    << "\\bENABLE_STYLE\\b"
    << "\\bENABLE_TEXT_PLACEMENT\\b"
    << "\\bEND\\b"
    << "\\bEND_SUB\\b"
    << "\\bEXTENDED\\b"
    << "\\bFACTOR\\b"
    << "\\bFADE\\b"
    << "\\bFADE_COLOR\\b"
    << "\\bFADE_OPACITY\\b"
    << "\\bFADE_STEP\\b"
    << "\\bFILE\\b"
    << "\\bFILL\\b"
    << "\\bFIXED_ANNOTATIONS\\b"
    << "\\bFONT\\b"
    << "\\bFONT_COLOR\\b"
    << "\\bFREEFORM\\b"
    << "\\bFRONT\\b"
    << "\\bGRABBER\\b"
    << "\\bGRADIENT\\b"
    << "\\bGROUP\\b"
    << "\\bHEIGHT\\b"
    << "\\bHIDDEN\\b"
    << "\\bHIDE\\b"
    << "\\bHIGHLIGHT\\b"
    << "\\bHIGHLIGHT_COLOR\\b"
    << "\\bHIGHLIGHT_LINE_WIDTH\\b"
    << "\\bHIGHLIGHT_STEP\\b"
    << "\\bHORIZONTAL\\b"
    << "\\bHOSE\\b"
    << "\\bRICH_TEXT\\b"
    << "\\bICON\\b"
    << "\\bIGN\\b"
    << "\\bIMAGE_SIZE\\b"
    << "\\bINCLUDE\\b"
    << "\\bINCLUDE_SUBMODELS\\b"
    << "\\bINSERT MODEL\\b"
    << "\\bINSERT\\b"
    << "\\bINSIDE\\b"
    << "\\bINSTANCE_COUNT\\b"
    << "\\bITEM\\b"
    << "\\bJUSTIFY_CENTER\\b"
    << "\\bJUSTIFY_CENTER_HORIZONTAL\\b"
    << "\\bJUSTIFY_CENTER_VERTICAL\\b"
    << "\\bJUSTIFY_LEFT\\b"
    << "\\bLANDSCAPE\\b"
    << "\\bLDGLITE_PARMS\\b"
    << "\\bLDVIEW_PARMS\\b"
    << "\\bLEFT\\b"
    << "\\bLEGO\\b"
    << "\\bLEGO_DISCLAIMER\\b"
    << "\\bLINE\\b"
    << "\\bLOCAL_LEGO_ELEMENTS_FILE\\b"
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
    << "\\bPAGE_LENGTH\\b"
    << "\\bPAGE_POINTER\\b"
    << "\\bPANEL\\b"
    << "\\bPART\\b"
    << "\\bPART_CATEGORY\\b"
    << "\\bPART_COLOR\\b"
    << "\\bPART_ELEMENT\\b"
    << "\\bPART_ELEMENTS\\b"
    << "\\bPART_GROUP\\b"
    << "\\bPART_GROUP_ENABLE\\b"
    << "\\bPART_SIZE\\b"
    << "\\bPER_STEP\\b"
    << "\\bPICTURE\\b"
    << "\\bPLACEMENT\\b"
    << "\\bPLI\\b"
    << "\\bPLI_ANNOTATION\\b"
    << "\\bPLI_INSTANCE\\b"
    << "\\bPLI_GRABBER\\b"
    << "\\bPLI_PART\\b"
    << "\\bPLI_PART_GROUP\\b"
    << "\\bPOINTER\\b"
    << "\\bPOINTER_ATTRIBUTE\\b"
    << "\\bPOINTER_HEAD\\b"
    << "\\bPOINTER_GRABBER\\b"
    << "\\bPOINTER_SEG_FIRST\\b"
    << "\\bPOINTER_SEG_SECOND\\b"
    << "\\bPOINTER_SEG_THIRD\\b"
    << "\\bPORTRAIT\\b"
    << "\\bPOVRAY_PARMS\\b"
    << "\\bPRIMARY\\b"
    << "\\bPRIMARY_DIRECTION\\b"
    << "\\bPUBLISH_COPYRIGHT\\b"
    << "\\bPUBLISH_COPYRIGHT_BACK\\b"
    << "\\bPUBLISH_DESCRIPTION\\b"
    << "\\bPUBLISH_EMAIL\\b"
    << "\\bPUBLISH_EMAIL_BACK\\b"
    << "\\bPUBLISH_URL\\b"
    << "\\bPUBLISH_URL_BACK\\b"
    << "\\bRANGE\\b"
    << "\\bRECTANGLE_STYLE\\b"
    << "\\bREMOVE\\b"
    << "\\bRESERVE\\b"
    << "\\bRESOLUTION\\b"
    << "\\bRIGHT\\b"
    << "\\bROTATE_ICON\\b"
    << "\\bROTATED\\b"
    << "\\bROUND\\b"
    << "\\bSCALE\\b"
    << "\\bSCENE\\b"
    << "\\bSCENE_OBJECT\\b"
    << "\\bSECONDARY\\b"
    << "\\bSECONDARY_DIRECTION\\b"
    << "\\bSEND_TO_BACK\\b"
    << "\\bSEPARATOR\\b"
    << "\\bSHOW\\b"
    << "\\bSHOW_INSTANCE_COUNT\\b"
    << "\\bSHOW_STEP_NUMBER\\b"
    << "\\bSHOW_TOP_MODEL\\b"
    << "\\bSIZE\\b"
    << "\\bSORT\\b"
    << "\\bSORT_BY\\b"
    << "\\bSORT_OPTION\\b"
    << "\\bSORT_ORDER\\b"
    << "\\bSQUARE\\b"
    << "\\bSQUARE_STYLE\\b"
    << "\\bSTEP_NUMBER\\b"
    << "\\bSTEP_PLI\\b"
    << "\\bSTEP_RECTANGLE\\b"
    << "\\bSTEP_SIZE\\b"
    << "\\bSTEPS\\b"
    << "\\bSTRETCH\\b"
    << "\\bSTYLE\\b"
    << "\\bSTUD_LOGO\\b"
    << "\\bSUB\\b"
    << "\\bSUBMODEL_BACKGROUND_COLOR\\b"
    << "\\bSUBMODEL_DISPLAY\\b"
    << "\\bSUBMODEL_FONT\\b"
    << "\\bSUBMODEL_FONT_COLOR\\b"
    << "\\bSUBMODEL_GRABBER\\b"
    << "\\bSUBMODEL_INSTANCE\\b"
    << "\\bSUBMODEL_INSTANCE_COUNT\\b"
    << "\\bSUBMODEL_INSTANCE_COUNT_OVERRIDE\\b"
    << "\\bSUBMODEL_ROTATION\\b"
    << "\\bSYNTHESIZED\\b"
    << "\\bTERTIARY\\b"
    << "\\bTERTIARY_DIRECTION\\b"
    << "\\bTEXT\\b"
    << "\\bTEXT_PLACEMENT\\b"
    << "\\bTHICKNESS\\b"
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
    << "\\bWIDTH\\b"
    << "\\bZ_VALUE\\b"
       ;

    foreach (QString pattern, LPubBodyMetaPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LPubBodyMetaFormat;
        highlightingRules.append(rule);
    }

    // LDraw Header Value Format
    LDrawHeaderValueFormat.setForeground(br26);
    LDrawHeaderValueFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("^(?!0 !LPUB|1).*\\b(?:AUTHOR|CATEGORY|CMDLINE|HELP|HISTORY|KEYWORDS|LDRAW_ORG|LICENSE|NAME|FILE|THEME|~MOVED TO)\\b.*$",Qt::CaseInsensitive);
    rule.format = LDrawHeaderValueFormat;
    highlightingRules.append(rule);

    // LDraw Header Format
    LDrawHeaderFormat.setForeground(br02);
    LDrawHeaderFormat.setFontWeight(QFont::Bold);

    QStringList LDrawHeaderPatterns;
    LDrawHeaderPatterns
    << "\\bAUTHOR\\b:?"
    << "\\bBFC\\b"
    << "!?\\bCATEGORY\\b"
    << "\\bCERTIFY\\b"
    << "\\bCCW\\b"
    << "\\bCLEAR\\b"
    << "!?\\bCMDLINE\\b"
    << "!?\\bHELP\\b"
    << "!?\\bHISTORY\\b"
    << "!?\\bKEYWORDS\\b"
    << "!?\\bLDRAW_ORG\\b"
    << "!?\\bLICENSE\\b"
    << "\\bNAME\\b:?"
    << "^(?!0 !LPUB|1).*\\bFILE\\b"
    << "\\bNOFILE\\b"
    << "!?\\bHELP\\b"
    << "\\bOFFICIAL\\b"
    << "\\bORIGINAL LDRAW\\b"
    << "!?\\bTHEME\\b"
    << "\\bUNOFFICIAL MODEL\\b"
    << "\\bUN-OFFICIAL\\b"
    << "\\bUNOFFICIAL\\b"
    << "\\UNOFFICIAL PART\\b"
    << "\\UNOFFICIAL_PART\\b"
    << "\\UNOFFICIAL_SUBPART\\b"
    << "\\UNOFFICIAL_SHORTCUT\\b"
    << "\\UNOFFICIAL_PRIMITIVE\\b"
    << "\\UNOFFICIAL_8_PRIMITIVE\\b"
    << "\\UNOFFICIAL_48_PRIMITIVE\\b"
    << "\\UNOFFICIAL_PART ALIAS\\b"
    << "\\UNOFFICIAL_SHORTCUT ALIAS\\b"
    << "\\UNOFFICIAL_PART PHYSICAL_COLOUR\\b"
    << "\\UNOFFICIAL_SHORTCUT PHYSICAL_COLOUR\\b"
    << "\\UNOFFICIAL PART\\b"
    << "\\UNOFFICIAL SUBPART\\b"
    << "\\UNOFFICIAL SHORTCUT\\b"
    << "\\UNOFFICIAL PRIMITIVE\\b"
    << "\\UNOFFICIAL 8_PRIMITIVE\\b"
    << "\\UNOFFICIAL 48_PRIMITIVE\\b"
    << "\\UNOFFICIAL PART ALIAS\\b"
    << "\\UNOFFICIAL SHORTCUT ALIAS\\b"
    << "\\UNOFFICIAL PART PHYSICAL_COLOUR\\b"
    << "\\UNOFFICIAL SHORTCUT PHYSICAL_COLOUR\\b"
    << "\\b~MOVED TO\\b"
       ;

    foreach (QString pattern, LDrawHeaderPatterns) {
        rule.pattern = QRegExp(pattern,Qt::CaseInsensitive);
        rule.format = LDrawHeaderFormat;
        highlightingRules.append(rule);
    }

    // LDraw Meta Line Format
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
    rule.pattern = QRegExp("!?\\bMLCAD\\b");
    rule.format = MLCadMetaFormat;
    highlightingRules.append(rule);

    // MLCad Body Meta Format
    MLCadBodyMetaFormat.setForeground(br17);
    MLCadBodyMetaFormat.setFontWeight(QFont::Bold);

    QStringList MLCadBodyMetaPatterns;
    MLCadBodyMetaPatterns
    << "\\bARROW\\b"
    << "\\bBTG\\b"
    << "\\bBUFEXCHG\\b"
    << "\\bGHOST\\b"
    << "\\bGROUP\\b"
    << "\\bRETRIEVE\\b"
    << "\\bROTATION CENTER\\b"
    << "\\bROTATION CONFIG\\b"
    << "\\bROTATION\\b"
    << "\\bROTSTEP END\\b"
    << "\\bROTSTEP\\b"
    << "\\bSKIP_BEGIN\\b"
    << "\\bSKIP_END\\b"
    << "\\bSTORE\\b"
    << "\\d\\.?\\d*\\s\\d\\.?\\d*\\s\\d\\.?\\d*\\s[ABS|ADD|REL]+"
       ;

    foreach (QString pattern, MLCadBodyMetaPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = MLCadBodyMetaFormat;
        highlightingRules.append(rule);
    }

    // LSynth Format
    LSynthMetaFormat.setForeground(br18);
    LSynthMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("!?\\bSYNTH\\b[^\n]*");
    rule.format = LSynthMetaFormat;
    highlightingRules.append(rule);

    // LDCad Meta Key Format
    LDCadMetaKeyFormat.setForeground(br11);
    LDCadMetaKeyFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("!?\\bLDCAD\\b[^\n]*");
    rule.format = LDCadMetaKeyFormat;
    highlightingRules.append(rule);

    // LDCad Body Meta Format
    LDCadBodyMetaFormat.setForeground(br19);
    LDCadBodyMetaFormat.setFontWeight(QFont::Bold);

    QStringList LDCadBodyMetaPatterns;
    LDCadBodyMetaPatterns
    << "!?\\bLDCAD\\b:?"
    << "!?\\bLDCAD CONTENT\\b"
    << "!?\\bLDCAD PATH_CAP\\b"
    << "!?\\bLDCAD PATH_POINT\\b"
    << "!?\\bLDCAD PATH_SKIN\\b"
    << "!?\\bLDCAD GENERATED\\b"
    << "!?\\bLDCAD SCRIPT\\b"
    << "!?\\bLDCAD GROUP_NXT\\b"
    ;

    foreach (QString pattern, LDCadBodyMetaPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LDCadBodyMetaFormat;
        highlightingRules.append(rule);
    }

    // LDCad Meta Value Format
    LDCadMetaValueFormat.setForeground(br08);
    LDCadMetaValueFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("[=]([a-zA-Z\\0-9%.\\s]+)");
    rule.format = LDCadMetaValueFormat;
    highlightingRules.append(rule);

    // LDCad Meta Group Def Format
    LDCadMetaGrpDefFormat.setForeground(br29);
    LDCadMetaGrpDefFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("!?\\bLDCAD GROUP_DEF\\b");
    rule.format = LDCadMetaGrpDefFormat;
    highlightingRules.append(rule);

    // LDCad Value Bracket Format
    LDCadBracketFormat.setForeground(br17);
    LDCadBracketFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("[\\[|=|\\]]");
    rule.format = LDCadBracketFormat;
    highlightingRules.append(rule);

    // LeoCAD Format
    LeoCADMetaFormat.setForeground(br20);
    LeoCADMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("!?\\bLEOCAD\\b[^\n]*");
    rule.format = LeoCADMetaFormat;
    highlightingRules.append(rule);

    // LDraw Comment Format
    LDrawCommentFormat.setForeground(br01);
    LDrawCommentFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("0\\s+\\/\\/[^\n]*",Qt::CaseInsensitive);
    rule.format = LDrawCommentFormat;
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

    int index = -1;
    if (text.startsWith("1 "))
        index = 0;
    else if (text.startsWith("0 GHOST "))
        index = 8;
    else if (text.startsWith("0 MLCAD HIDE "))
        index = 13;
    else
        return;

    QStringList tt = text.mid(index).trimmed().split(" ",QString::SkipEmptyParts);
    QString part;
    for (int t = 14; t < tt.size(); t++)
        part += (tt[t]+" ");
    QStringList tokens;
    tokens  << tt[0]                             // - part type
            << tt[1]                             // - color
            << tt[2]+" "+tt[3]+" "+tt[4]         // - position
            << tt[5]+" "+tt[6]+" "+tt[7]         // - transform
            << tt[8]+" "+tt[9]+" "+tt[10]        // - transform
            << tt[11]+" "+tt[12]+" "+tt[13]      // - transform
            << part.trimmed();                   // - part

    for (int i = 0; i < tokens.size(); i++) {
        if (index >= 0 && index < text.length()) {
            setFormat(index, tokens[i].length(), lineType1Formats[i]);
            index += tokens[i].length() + 1;     // add 1 position for the space
        }
    }
}
