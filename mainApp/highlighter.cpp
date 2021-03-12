/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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
#include "highlighter.h"
#include "application.h"
#include "lpub_preferences.h"
#include "name.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    QBrush br01,br02,br03,br04,br05,br06,br07,br08,br09,br10,br11,br12,br13,br14;
    QBrush br15,br16,br17,br18,br19,br20,br21,br22,br23,br24,br25,br26,br27,br28,br29;
    if (Application::instance()->getTheme() == THEME_DEFAULT) {
        br01 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_COMMENTS]));
        br02 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_HEADER]));
        br03 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_BODY]));
        br04 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_LOCAL]));
        br05 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_GLOBAL]));
        br06 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPE_1]));
        br07 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_COLOUR_CODE]));
        br08 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_PART_POSITION]));
        br09 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_PART_TRANSFORM_1]));
        br10 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_PART_TRANSFORM_2]));
        br11 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_PART_TRANSFORM_3]));
        br12 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_PART_FILE]));
        br13 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPES_2_TO_5]));
        br14 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_NUMBER]));
        br15 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_HEX_NUMBER]));
        br16 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_PAGE_SIZE]));
        br17 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LEOCAD]));
        br18 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LSYNTH]));
        br19 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDCAD]));
        br20 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_MLCAD]));
        br21 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_MLCAD_BODY]));
        br22 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_FALSE]));
        br23 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_TRUE]));
        br24 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D]));
        br25 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_BODY]));
        br26 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_HEADER_VALUE]));
        br27 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_QUOTED_TEXT]));
        br28 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPE_0_FIRST_CHARACTER]));
        br29 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDCAD_GROUP_DEFINE]));
      }
    else
    if (Application::instance()->getTheme() == THEME_DARK) {
        br01 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_COMMENTS]));
        br02 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_HEADER]));
        br03 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_BODY]));
        br04 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_LOCAL]));
        br05 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_GLOBAL]));
        br06 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_LINE_TYPE_1]));
        br07 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_COLOUR_CODE]));
        br08 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_PART_POSITION]));
        br09 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_PART_TRANSFORM_1]));
        br10 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_PART_TRANSFORM_2]));
        br11 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_PART_TRANSFORM_3]));
        br12 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_PART_FILE]));
        br13 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_LINE_TYPES_2_TO_5]));
        br14 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_NUMBER]));
        br15 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_HEX_NUMBER]));
        br16 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_PAGE_SIZE]));
        br17 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LEOCAD]));
        br18 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LSYNTH]));
        br19 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDCAD]));
        br20 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_MLCAD]));
        br21 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_MLCAD_BODY]));
        br22 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_FALSE]));
        br23 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_TRUE]));
        br24 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D]));
        br25 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_BODY]));
        br26 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_HEADER_VALUE]));
        br27 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_QUOTED_TEXT]));
        br28 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_LINE_TYPE_0_FIRST_CHARACTER]));
        br29 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDCAD_GROUP_DEFINE]));
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
    rule.pattern = QRegExp("[,|-](\\d+)"); // match digit if preceded by single character , or -
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
    rule.pattern = QRegExp("BEGIN\\sSUB\\s([A-Za-z0-9\\s_-]+.[dat|mpd|ldr]+)",Qt::CaseInsensitive); // match part format if preceded by 'BEGIN SUB'
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

    Q_FOREACH (QString pattern, LDrawColourPatterns) {
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

    Q_FOREACH (QString pattern, LDrawBodyPatterns) {
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
    << "\\bACTION\\b"
    << "\\bADJUST_ON_ITEM_OFFSET\\b"
    << "\\bAFTER\\b"
    << "\\bALLOC\\b"
    << "\\bANGLE\\b"
    << "\\bANGLE_KEY\\b"
    << "\\bANNOTATE\\b"
    << "\\bANNOTATION\\b"
    << "\\bANNOTATION_SHOW\\b"
    << "\\bAPP_PLUG\\b"
    << "\\bAPP_PLUG_IMAGE\\b"
    << "\\bAPPLY\\b"
    << "\\bAREA\\b"
    << "\\bASPECT\\b"
    << "\\bASSEM\\b"
    << "\\bASSEM_PART\\b"
    << "\\bAT_MODEL\\b"
    << "\\bAT_STEP\\b"
    << "\\bAT_TOP\\b"
    << "\\bATTRIBUTE_PIXMAP\\b"
    << "\\bATTRIBUTE_TEXT\\b"
    << "\\bAUTHOR\\b"
    << "\\bAXLE\\b"
    << "\\bBACK\\b"
    << "\\bBACKGROUND COLOR\\b"
    << "\\bBACKGROUND GRADIENT\\b"
    << "\\bBACKGROUND IMAGE\\b"
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
    << "\\bBUILD_MOD\\b"
    << "\\bBUILD_MOD_ENABLED\\b"
    << "\\bCABLE\\b"
    << "\\bCALLOUT\\b"
    << "\\bCALLOUT_INSTANCE\\b"
    << "\\bCALLOUT_POINTER\\b"
    << "\\bCALLOUT_UNDERPINNING\\b"
    << "\\bCAMERA FOV\\b"
    << "\\bCAMERA HIDDEN\\b"
    << "\\bCAMERA NAME\\b"
    << "\\bCAMERA ORTHOGRAPHIC\\b"
    << "\\bCAMERA POSITION\\b"
    << "\\bCAMERA POSITION_KEY\\b"
    << "\\bCAMERA TARGET_POSITION\\b"
    << "\\bCAMERA TARGET_POSITION_KEY\\b"
    << "\\bCAMERA UP_VECTOR\\b"
    << "\\bCAMERA UP_VECTOR_KEY\\b"
    << "\\bCAMERA ZFAR\\b"
    << "\\bCAMERA ZNEAR\\b"
    << "\\bCAMERA\\b"
    << "\\bCAMERA_ANGLES\\b"
    << "\\bCAMERA_DISTANCE\\b"
    << "\\bCAMERA_DISTANCE_NATIVE\\b"
    << "\\bCAMERA_FOV\\b"
    << "\\bCAMERA_NAME\\b"
    << "\\bCAMERA_ORTHOGRAPHIC\\b"
    << "\\bCAMERA_TARGET\\b"
    << "\\bCENTER\\b"
    << "\\bCIRCLE_STYLE\\b"
    << "\\bCLEAR\\b"
    << "\\bCOLOR\\b"
    << "\\bCOLOR_RGB\\b"
    << "\\bCOLOR_RGB_KEY\\b"
    << "\\bCOLOUR\\b"
    << "\\bCOLS\\b"
    << "\\bCOMMENT\\b"
    << "\\bCONNECTOR\\b"
    << "\\bCONSOLIDATE_INSTANCE_COUNT\\b"
    << "\\bCONSTRAIN\\b"
    << "\\bCONTENT\\b"
    << "\\bCONTINUOUS_STEP_NUMBERS\\b"
    << "\\bCONTROL_POINT\\b"
    << "\\bCOVER_PAGE\\b"
    << "\\bCROSS\\b"
    << "\\bCOUNT_GROUP_STEPS\\b"
    << "\\bCSI_ANNOTATION\\b"
    << "\\bCSI_ANNOTATION_PART\\b"
    << "\\bCUSTOM_LENGTH\\b"
    << "\\bCUTOFF_DISTANCE\\b"
    << "\\bCUTOFF_DISTANCE_KEY\\b"
    << "\\bDESCRIPTION\\b"
    << "\\bDISPLAY\\b"
    << "\\bDISPLAY_MODEL\\b"
    << "\\bDISPLAY_PAGE_NUMBER\\b"
    << "\\bDISTANCE\\b"
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
    << "\\bEND_MOD\\b"
    << "\\bEND_SUB\\b"
    << "\\bEXTENDED\\b"
    << "\\bFACTOR\\b"
    << "\\bFADE\\b"
    << "\\bFADE_COLOR\\b"
    << "\\bFADE_OPACITY\\b"
    << "\\bFADE_STEP\\b"
    << "\\bFILE\\b"
    << "\\bFILL\\b"
    << "\\bFINAL_MODEL_ENABLED\\b"
    << "\\bFIXED_ANNOTATIONS\\b"
    << "\\bFONT\\b"
    << "\\bFONT_COLOR\\b"
    << "\\bFOV\\b"
    << "\\bFREEFORM\\b"
    << "\\bFRONT\\b"
    << "\\bGRABBER\\b"
    << "\\bGRADIENT\\b"
    << "\\bGROUP BEGIN\\b"
    << "\\bGROUP END\\b"
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
    << "\\bLATITUDE\\b"
    << "\\bLDGLITE_PARMS\\b"
    << "\\bLDRAW_TYPE\\b"
    << "\\bLDVIEW_PARMS\\b"
    << "\\bLEFT\\b"
    << "\\bLEGO\\b"
    << "\\bLEGO_DISCLAIMER\\b"
    << "\\bLIGHT\\b"
    << "\\bLINE\\b"
    << "\\bLOCAL_LEGO_ELEMENTS_FILE\\b"
    << "\\bLONGITUDE\\b"
    << "\\bMARGINS\\b"
    << "\\bMODEL AUTHOR\\b"
    << "\\bMODEL BACKGROUND COLOR\\b"
    << "\\bMODEL BACKGROUND GRADIENT\\b"
    << "\\bMODEL BACKGROUND IMAGE\\b"
    << "\\bMODEL COMMENT\\b"
    << "\\bMODEL DESCRIPTION\\b"
    << "\\bMODEL NAME\\b"
    << "\\bMODEL\\b"
    << "\\bMODEL_CATEGORY\\b"
    << "\\bMODEL_DESCRIPTION\\b"
    << "\\bMODEL_ID\\b"
    << "\\bMODEL_PARTS\\b"
    << "\\bMODEL_SCALE\\b"
    << "\\bMULTI_STEP\\b"
    << "\\bNAME\\b"
    << "\\bNONE\\b"
    << "\\bNOSTEP\\b"
    << "\\bNUMBER\\b"
    << "\\bOFFSET\\b"
    << "\\bORIENTATION\\b"
    << "\\bORTHOGRAPHIC\\b"
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
    << "\\bPART_ROTATION\\b"
    << "\\bPART_SIZE\\b"
    << "\\bPER_STEP\\b"
    << "\\bPICTURE\\b"
    << "\\bPIECE HIDDEN\\b"
    << "\\bPIECE PIVOT\\b"
    << "\\bPIECE POSITION_KEY\\b"
    << "\\bPIECE ROTATION_KEY\\b"
    << "\\bPIECE STEP_HIDE\\b"
    << "\\bPIECE\\b"
    << "\\bPIVOT\\b"
    << "\\bPLACEMENT\\b"
    << "\\bPLI\\b"
    << "\\bPLI_ANNOTATION\\b"
    << "\\bPLI_GRABBER\\b"
    << "\\bPLI_INSTANCE\\b"
    << "\\bPLI_PART\\b"
    << "\\bPLI_PART_GROUP\\b"
    << "\\bPOINTER\\b"
    << "\\bPOINTER_ATTRIBUTE\\b"
    << "\\bPOINTER_GRABBER\\b"
    << "\\bPOINTER_HEAD\\b"
    << "\\bPOINTER_SEG_FIRST\\b"
    << "\\bPOINTER_SEG_SECOND\\b"
    << "\\bPOINTER_SEG_THIRD\\b"
    << "\\bPORTRAIT\\b"
    << "\\bPOSITION\\b"
    << "\\bPOSITION_KEY\\b"
    << "\\bPOSITION_KEY\\b"
    << "\\bPOVRAY_PARMS\\b"
    << "\\bPOWER\\b"
    << "\\bPOWER_KEY\\b"
    << "\\bPRIMARY\\b"
    << "\\bPRIMARY_DIRECTION\\b"
    << "\\bPUBLISH_COPYRIGHT\\b"
    << "\\bPUBLISH_COPYRIGHT_BACK\\b"
    << "\\bPUBLISH_DESCRIPTION\\b"
    << "\\bPUBLISH_EMAIL\\b"
    << "\\bPUBLISH_EMAIL_BACK\\b"
    << "\\bPUBLISH_URL\\b"
    << "\\bPUBLISH_URL_BACK\\b"
    << "\\bRADIUS\\b"
    << "\\bRADIUS_AND_SPOT_BLEND_KEY\\b"
    << "\\bRADIUS_KEY\\b"
    << "\\bRANGE\\b"
    << "\\bRECTANGLE_STYLE\\b"
    << "\\bREMOVE\\b"
    << "\\bRESERVE\\b"
    << "\\bRESOLUTION\\b"
    << "\\bRICH_TEXT\\b"
    << "\\bRIGHT\\b"
    << "\\bROTATE_ICON\\b"
    << "\\bROTATED\\b"
    << "\\bROTATION_KEY\\b"
    << "\\bROUND\\b"
    << "\\bSCALE\\b"
    << "\\bSCENE\\b"
    << "\\bSCENE_OBJECT\\b"
    << "\\bSECONDARY\\b"
    << "\\bSECONDARY_DIRECTION\\b"
    << "\\bSEND_TO_BACK\\b"
    << "\\bSEPARATOR\\b"
    << "\\bSHAPE\\b"
    << "\\bSHAPE_KEY\\b"
    << "\\bSHOW\\b"
    << "\\bSHOW_GROUP_STEP_NUMBER\\b"
    << "\\bSHOW_INSTANCE_COUNT\\b"
    << "\\bSHOW_STEP_NUMBER\\b"
    << "\\bSHOW_TOP_MODEL\\b"
    << "\\bSILHOUETTE\\b"
    << "\\bSIZE\\b"
    << "\\bSIZE_KEY\\b"
    << "\\bSPACING\\b"
    << "\\bSORT\\b"
    << "\\bSORT_BY\\b"
    << "\\bSORT_OPTION\\b"
    << "\\bSORT_ORDER\\b"
    << "\\bSPECULAR\\b"
    << "\\bSPECULAR_KEY\\b"
    << "\\bSPOT_BLEND\\b"
    << "\\bSPOT_SIZE\\b"
    << "\\bSPOT_SIZE_KEY\\b"
    << "\\bSQUARE\\b"
    << "\\bSQUARE_STYLE\\b"
    << "\\bSTART_PAGE_NUMBER\\b"
    << "\\bSTART_STEP_NUMBER\\b"
    << "\\bSTEP_HIDE\\b"
    << "\\bSTEP_NUMBER\\b"
    << "\\bSTEP_PLI\\b"
    << "\\bSTEP_RECTANGLE\\b"
    << "\\bSTEP_SIZE\\b"
    << "\\bSTEPS\\b"
    << "\\bSTRENGTH\\b"
    << "\\bSTRENGTH_KEY\\b"
    << "\\bSTRETCH\\b"
    << "\\bSTUD_LOGO\\b"
    << "\\bSTYLE\\b"
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
    << "\\bSYNTH BEGIN\\b"
    << "\\bSYNTH CONTROL_POINT\\b"
    << "\\bSYNTH END\\b"
    << "\\bSYNTH\\b"
    << "\\bSYNTHESIZED\\b"
    << "\\bTARGET_POSITION\\b"
    << "\\bTARGET_POSITION_KEY\\b"
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
    << "\\bTYPE\\b"
    << "\\bTYPE_KEY\\b"
    << "\\bUP_VECTOR\\b"
    << "\\bUP_VECTOR_KEY\\b"
    << "\\bUSE_FADE_COLOR\\b"
    << "\\bUSE_FREE_FORM\\b"
    << "\\bUSE_TITLE\\b"
    << "\\bUSE_TITLE_AND_FREE_FORM\\b"
    << "\\bVERTICAL\\b"
    << "\\bVIEW_ANGLE\\b"
    << "\\bWHOLE\\b"
    << "\\bWIDTH\\b"
    << "\\bZ_VALUE\\b"
    << "\\bZFAR\\b"
    << "\\bZNEAR\\b"
       ;

    Q_FOREACH (QString pattern, LPubBodyMetaPatterns) {
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
    << "!?\\bCATEGORY\\b(?!\")"             // (?!\") match Category not followed by "
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
    << "\\bUNOFFICIAL PART\\b"
    << "\\bUNOFFICIAL_PART\\b"
    << "\\bUNOFFICIAL_SUBPART\\b"
    << "\\bUNOFFICIAL_SHORTCUT\\b"
    << "\\bUNOFFICIAL_PRIMITIVE\\b"
    << "\\bUNOFFICIAL_8_PRIMITIVE\\b"
    << "\\bUNOFFICIAL_48_PRIMITIVE\\b"
    << "\\bUNOFFICIAL_PART ALIAS\\b"
    << "\\bUNOFFICIAL_SHORTCUT ALIAS\\b"
    << "\\bUNOFFICIAL_PART PHYSICAL_COLOUR\\b"
    << "\\bUNOFFICIAL_SHORTCUT PHYSICAL_COLOUR\\b"
    << "\\bUNOFFICIAL PART\\b"
    << "\\bUNOFFICIAL SUBPART\\b"
    << "\\bUNOFFICIAL SHORTCUT\\b"
    << "\\bUNOFFICIAL PRIMITIVE\\b"
    << "\\bUNOFFICIAL 8_PRIMITIVE\\b"
    << "\\bUNOFFICIAL 48_PRIMITIVE\\b"
    << "\\bUNOFFICIAL PART ALIAS\\b"
    << "\\bUNOFFICIAL SHORTCUT ALIAS\\b"
    << "\\bUNOFFICIAL PART PHYSICAL_COLOUR\\b"
    << "\\bUNOFFICIAL SHORTCUT PHYSICAL_COLOUR\\b"
    << "\\b~MOVED TO\\b"
       ;

    Q_FOREACH (QString pattern, LDrawHeaderPatterns) {
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
    << "\\b(ABS|ADD|REL)$"
    << "\\bSKIP_BEGIN\\b"
    << "\\bSKIP_END\\b"
    << "\\bSTORE\\b"
       ;

    Q_FOREACH (QString pattern, MLCadBodyMetaPatterns) {
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
    << "(?<=LDCAD )\\bCONTENT\\b"
    << "(?<=LDCAD )\\bPATH_CAP\\b"
    << "(?<=LDCAD )\\bPATH_POINT\\b"
    << "(?<=LDCAD )\\bPATH_SKIN\\b"
    << "(?<=LDCAD )\\bSCRIPT\\b"
    << "(?<=LDCAD )\\bGROUP_NXT\\b"
    ;

    Q_FOREACH (QString pattern, LDCadBodyMetaPatterns) {
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

    // LeoCAD Body Meta Format
    LeoCADBodyMetaFormat.setForeground(br17);
    LeoCADBodyMetaFormat.setFontWeight(QFont::Bold);

    QStringList LeoCADBodyMetaPatterns;
    LeoCADBodyMetaPatterns
    << "(?<=LEOCAD )\\bMODEL NAME\\b"
    << "(?<=LEOCAD )\\bMODEL AUTHOR\\b"
    << "(?<=LEOCAD )\\bMODEL DESCRIPTION\\b"
    << "(?<=LEOCAD )\\bMODEL COMMENT\\b"
    << "(?<=LEOCAD )\\bMODEL BACKGROUND COLOR\\b"
    << "(?<=LEOCAD )\\bMODEL BACKGROUND GRADIENT\\b"
    << "(?<=LEOCAD )\\bMODEL BACKGROUND IMAGE\\b"
    << "(?<=LEOCAD )\\bNAME\\b"
    << "(?<=LEOCAD )\\bPIECE STEP_HIDE\\b"
    << "(?<=LEOCAD )\\bPIECE HIDDEN\\b"
    << "(?<=LEOCAD )\\bPIECE POSITION_KEY\\b"
    << "(?<=LEOCAD )\\bPIECE ROTATION_KEY\\b"
    << "(?<=LEOCAD )\\bPIECE PIVOT\\b"
    << "(?<=LEOCAD )\\bCAMERA HIDDEN\\b"
    << "(?<=LEOCAD )\\bCAMERA ORTHOGRAPHIC\\b"
    << "(?<=LEOCAD )\\bCAMERA FOV\\b"
    << "(?<=LEOCAD )\\bCAMERA ZNEAR\\b"
    << "(?<=LEOCAD )\\bCAMERA ZFAR\\b"
    << "(?<=LEOCAD )\\bCAMERA POSITION\\b"
    << "(?<=LEOCAD )\\bCAMERA TARGET_POSITION\\b"
    << "(?<=LEOCAD )\\bCAMERA UP_VECTOR\\b"
    << "(?<=LEOCAD )\\bCAMERA POSITION_KEY\\b"
    << "(?<=LEOCAD )\\bCAMERA TARGET_POSITION_KEY\\b"
    << "(?<=LEOCAD )\\bCAMERA UP_VECTOR_KEY\\b"
    << "(?<=LEOCAD )\\bCAMERA NAME\\b"
    << "(?<=LEOCAD )\\bGROUP BEGIN\\b"
    << "(?<=LEOCAD )\\bGROUP END\\b"
    ;

    Q_FOREACH (QString pattern, LeoCADBodyMetaPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LeoCADBodyMetaFormat;
        highlightingRules.append(rule);
    }

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
    Q_FOREACH (HighlightingRule rule, highlightingRules) {
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
