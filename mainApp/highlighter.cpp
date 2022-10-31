/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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
#include "lpub_qtcompat.h"
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

    // LPub3D Quoted Text Format
    LPubQuotedTextFormat.setForeground(br27);
    LPubQuotedTextFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("[<\"].*[>\"]");
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

    // LPub3D Font Number Format
    LPubFontNumberFormat.setForeground(br14);
    LPubFontNumberFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("[,-](\\d+)"); // match digit if preceded by single character , or -
    rule.format = LPubFontNumberFormat;
    highlightingRules.append(rule);

    // LDraw Custom COLOUR Description Format
    LDrawColourDescFormat.setForeground(br26);
    LDrawColourDescFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\bLPub3D_[A-Za-z|_]+\\b");
    rule.format = LDrawColourDescFormat;
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

    // LPub3D Font Number Comma and dash Format
    LPubFontCommaFormat.setForeground(br27);
    LPubFontCommaFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("[,]");
    rule.format = LPubFontCommaFormat;
    highlightingRules.append(rule);

    // LPub3D Page Size Format
    LPubPageSizeFormat.setForeground(br16);
    LPubPageSizeFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\b[AB][0-9]0?$\\b|\\bComm10E\\b$|\\bArch[1-3]\\b$",Qt::CaseInsensitive);
    rule.format = LPubPageSizeFormat;
    highlightingRules.append(rule);

    // LDraw Custom COLOUR Meta Format
    LDrawColourMetaFormat.setForeground(br05);
    LDrawColourMetaFormat.setFontWeight(QFont::Bold);

    QStringList LDrawColourPatterns;
    LDrawColourPatterns
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

    // LPub3D Custom COLOUR, FADE, SILHOUETTE Meta Format
    LPubCustomColorFormat.setForeground(br20);
    LPubCustomColorFormat.setFontWeight(QFont::Bold);

    QStringList LPubCustomColorPatterns;
    LPubCustomColorPatterns
            << "!?\\bCOLOUR\\b"
            << "!?\\bFADE\\b"
            << "!?\\bSILHOUETTE\\b"
            ;

    Q_FOREACH (QString pattern, LPubCustomColorPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LPubCustomColorFormat;
        highlightingRules.append(rule);
    }

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
    << "\\bALLOC\\b"
    << "\\bANGLE\\b"
    << "\\bANNOTATE\\b"
    << "\\bANNOTATION\\b"
    << "\\bAPPLY\\b"
    << "\\bAPP_PLUG\\b"
    << "\\bAPP_PLUG_IMAGE\\b"
    << "\\bAREA\\b"
    << "\\bARROW\\b"
    << "\\bASPECT\\b"
    << "\\bASSEM\\b"
    << "\\bASSEMBLED\\b"
    << "\\bATTRIBUTE_PIXMAP\\b"
    << "\\bATTRIBUTE_TEXT\\b"
    << "\\bAT_MODEL\\b"
    << "\\bAT_STEP\\b"
    << "\\bAT_TOP\\b"
    << "\\bAUTOMATE_EDGE_COLOR\\b"
    << "\\bAXLE\\b"
    << "\\bBACKGROUND\\b"
    << "\\bBASE_BOTTOM\\b"
    << "\\bBASE_LEFT\\b"
    << "\\bBASE_RIGHT\\b"
    << "\\bBASE_TOP\\b"
    << "\\bBEAM\\b"
    << "\\bBEGIN\\b"
    << "\\bBLACK_EDGE_COLOR\\b"
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
    << "\\bCAMERA\\b"
    << "\\bCAMERA_ANGLES\\b"
    << "\\bCAMERA_DISTANCE\\b"
    << "\\bCAMERA_DISTANCE_NATIVE\\b"
    << "\\bCAMERA_FOV\\b"
    << "\\bCAMERA_NAME\\b"
    << "\\bCAMERA_ORTHOGRAPHIC\\b"
    << "\\bCAMERA_POSITION\\b"
    << "\\bCAMERA_TARGET\\b"
    << "\\bCAMERA_UPVECTOR\\b"
    << "\\bCAMERA_ZFAR\\b"
    << "\\bCAMERA_ZNEAR\\b"
    << "\\bCENTER\\b"
    << "\\bCIRCLE_STYLE\\b"
    << "\\bCLEAR\\b"
    << "\\bCOLOR\\b"
    << "\\bCOLOR_LIGHT_DARK_INDEX\\b"
    << "\\bCOLOR_RGB\\b"
    << "\\bCOLS\\b"
    << "\\bCONNECTOR\\b"
    << "\\bCONSOLIDATE_INSTANCE_COUNT\\b"
    << "\\bCONSTRAIN\\b"
    << "\\bCONTENT\\b"
    << "\\bCONTINUOUS_STEP_NUMBERS\\b"
    << "\\bCONTRAST\\b"
    << "\\bCOUNT_GROUP_STEPS\\b"
    << "\\bCOVER_PAGE\\b"
    << "\\bCOVER_PAGE_MODEL_VIEW_ENABLED\\b"
    << "\\bCROSS\\b"
    << "\\bCSI_ANNOTATION\\b"
    << "\\bCSI_ANNOTATION_PART\\b"
    << "\\bCUSTOM_LENGTH\\b"
    << "\\bCUTOFF_DISTANCE\\b"
    << "\\bDARK_EDGE_COLOR\\b"
    << "\\bDASH\\b"
    << "\\bDASH_DOT\\b"
    << "\\bDASH_DOT_DOT\\b"
    << "\\bDISPLAY\\b"
    << "\\bDISPLAY_MODEL\\b"
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
    << "\\bDOCUMENT_LOGO\\b"
    << "\\bDOCUMENT_LOGO_BACK\\b"
    << "\\bDOCUMENT_LOGO_FRONT\\b"
    << "\\bDOCUMENT_TITLE\\b"
    << "\\bDOCUMENT_TITLE_BACK\\b"
    << "\\bDOCUMENT_TITLE_FRONT\\b"
    << "\\bDOT\\b"
    << "\\bDPCM\\b"
    << "\\bDPI\\b"
    << "\\bEDGE_COLOR\\b"
    << "\\bELEMENT\\b"
    << "\\bELEMENT_STYLE\\b"
    << "\\bENABLE\\b"
    << "\\bENABLED\\b"
    << "\\bENABLE_SETTING\\b"
    << "\\bENABLE_STYLE\\b"
    << "\\bENABLE_TEXT_PLACEMENT\\b"
    << "\\bEND\\b"
    << "\\bEND_MOD\\b"
    << "\\bEXTENDED\\b"
    << "\\bFADE_STEP\\b"
    << "\\bFILE\\b"
    << "\\bFILL\\b"
    << "\\bFINAL_MODEL_ENABLED\\b"
    << "\\bFIXED_ANNOTATIONS\\b"
    << "\\bFLATTENED_LOGO\\b"
    << "\\bFONT\\b"
    << "\\bFONT_COLOR\\b"
    << "\\bFOR_SUBMODEL\\b"
    << "\\bFREEFORM\\b"
    << "\\bGRADIENT\\b"
    << "\\bGROUP\\b"
    << "\\bHEIGHT\\b"
    << "\\bHIDDEN\\b"
    << "\\bHIDE_TIP\\b"
    << "\\bHIGHLIGHT_STEP\\b"
    << "\\bHIGH_CONTRAST\\b"
    << "\\bHIGH_CONTRAST_WITH_LOGO\\b"
    << "\\bHORIZONTAL\\b"
    << "\\bHOSE\\b"
    << "\\bHTML_TEXT\\b"
    << "\\bICON\\b"
    << "\\bID\\b"
    << "\\bIGN\\b"
    << "\\bIMAGE_SIZE\\b"
    << "\\bINCLUDE\\b"
    << "\\bINCLUDE_SUBMODELS\\b"
    << "\\bINSERT\\b"
    << "\\bINSIDE\\b"
    << "\\bINSTANCE_COUNT\\b"
    << "\\bJUSTIFY_CENTER\\b"
    << "\\bJUSTIFY_CENTER_HORIZONTAL\\b"
    << "\\bJUSTIFY_CENTER_VERTICAL\\b"
    << "\\bJUSTIFY_LEFT\\b"
    << "\\bLANDSCAPE\\b"
    << "\\bLDGLITE\\b"
    << "\\bLDGLITE_PARMS\\b"
    << "\\bLDVIEW\\b"
    << "\\bLDVIEW_PARMS\\b"
    << "\\bLDVIEW_POV_GENERATOR\\b"
    << "\\bLEFT\\b"
    << "\\bLEGO\\b"
    << "\\bLEGO_DISCLAIMER\\b"
    << "\\bLIGHT\\b"
    << "\\bLINE\\b"
    << "\\bLINE_WIDTH\\b"
    << "\\bLOAD_UNOFFICIAL_PARTS_IN_EDITOR\\b"
    << "\\bLOCAL_LEGO_ELEMENTS_FILE\\b"
    << "\\bMARGINS\\b"
    << "\\bMODEL\\b"
    << "\\bMODEL_CATEGORY\\b"
    << "\\bMODEL_DESCRIPTION\\b"
    << "\\bMODEL_ID\\b"
    << "\\bMODEL_PARTS\\b"
    << "\\bMODEL_SCALE\\b"
    << "\\bMODEL_STEP_NUMBER\\b"
    << "\\bMULTI_STEP\\b"
    << "\\bMULTI_STEPS\\b"
    << "\\bNAME\\b"
    << "\\bNATIVE\\b"
    << "\\bNONE\\b"
    << "\\bNOSTEP\\b"
    << "\\bNUMBER\\b"
    << "\\bOFFSET\\b"
    << "\\bOPACITY\\b"
    << "\\bORIENTATION\\b"
    << "\\bOUTLINE_LOGO\\b"
    << "\\bOUTSIDE\\b"
    << "\\bPAGE\\b"
    << "\\bPAGE_FOOTER\\b"
    << "\\bPAGE_HEADER\\b"
    << "\\bPAGE_LENGTH\\b"
    << "\\bPAGE_NUMBER\\b"
    << "\\bPAGE_POINTER\\b"
    << "\\bPANEL\\b"
    << "\\bPARSE_NOSTEP\\b"
    << "\\bPART\\b"
    << "\\bPART_ELEMENTS\\b"
    << "\\bPART_GROUP\\b"
    << "\\bPART_GROUP_ENABLE\\b"
    << "\\bPART_ROTATION\\b"
    << "\\bPER_STEP\\b"
    << "\\bPICTURE\\b"
    << "\\bPIECE\\b"
    << "\\bPLACEMENT\\b"
    << "\\bPLI\\b"
    << "\\bPLI_ANNOTATION\\b"
    << "\\bPLI_GRABBER\\b"
    << "\\bPLI_INSTANCE\\b"
    << "\\bPLI_PART\\b"
    << "\\bPLI_PART_GROUP\\b"
    << "\\bPOINTER\\b"
    << "\\bPOINTER_ATTRIBUTE\\b"
    << "\\bPOINTER_BASE\\b"
    << "\\bPOINTER_GRABBER\\b"
    << "\\bPOINTER_HEAD\\b"
    << "\\bPOINTER_SEG_FIRST\\b"
    << "\\bPOINTER_SEG_SECOND\\b"
    << "\\bPOINTER_SEG_THIRD\\b"
    << "\\bPORTRAIT\\b"
    << "\\bPOSITION\\b"
    << "\\bPOVRAY\\b"
    << "\\bPOVRAY_PARMS\\b"
    << "\\bPOWER\\b"
    << "\\bPREFERRED_RENDERER\\b"
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
    << "\\bRANGE\\b"
    << "\\bRECTANGLE_STYLE\\b"
    << "\\bREMOVE\\b"
    << "\\bRESERVE\\b"
    << "\\bRESOLUTION\\b"
    << "\\bRICH_TEXT\\b"
    << "\\bRIGHT\\b"
    << "\\bROTATED\\b"
    << "\\bROTATE_ICON\\b"
    << "\\bROUND\\b"
    << "\\bROUNDED_TOP_LOGO\\b"
    << "\\bSATURATION\\b"
    << "\\bSCALE\\b"
    << "\\bSCENE\\b"
    << "\\bSECONDARY\\b"
    << "\\bSECONDARY_DIRECTION\\b"
    << "\\bSEND_TO_BACK\\b"
    << "\\bSEPARATOR\\b"
    << "\\bSETUP\\b"
    << "\\bSHAPE\\b"
    << "\\bSHARP_TOP_LOGO\\b"
    << "\\bSHOW\\b"
    << "\\bSHOW_GROUP_STEP_NUMBER\\b"
    << "\\bSHOW_INSTANCE_COUNT\\b"
    << "\\bSHOW_STEP_NUM\\b"
    << "\\bSHOW_STEP_NUMBER\\b"
    << "\\bSHOW_TOP_MODEL\\b"
    << "\\bSINGLE_CALL\\b"
    << "\\bSINGLE_CALL_EXPORT_LIST\\b"
    << "\\bSINGLE_STEP\\b"
    << "\\bSIZE\\b"
    << "\\bSKIP_BEGIN\\b"
    << "\\bSKIP_END\\b"
    << "\\bSOLID\\b"
    << "\\bSORT\\b"
    << "\\bSORT_BY\\b"
    << "\\bSORT_OPTION\\b"
    << "\\bSORT_ORDER\\b"
    << "\\bSPECULAR\\b"
    << "\\bSPOT_BLEND\\b"
    << "\\bSPOT_SIZE\\b"
    << "\\bSQUARE\\b"
    << "\\bSQUARE_STYLE\\b"
    << "\\bSTART_PAGE_NUMBER\\b"
    << "\\bSTART_STEP_NUMBER\\b"
    << "\\bSTEP\\b"
    << "\\bSTEPS\\b"
    << "\\bSTEP_GROUP\\b"
    << "\\bSTEP_NUMBER\\b"
    << "\\bSTEP_PLI\\b"
    << "\\bSTEP_RECTANGLE\\b"
    << "\\bSTEP_SIZE\\b"
    << "\\bSTRENGTH\\b"
    << "\\bSTRETCH\\b"
    << "\\bSTUD_CYLINDER_COLOR\\b"
    << "\\bSTUD_STYLE\\b"
    << "\\bSTYLE\\b"
    << "\\bSUB\\b"
    << "\\bLDRAW_TYPE\\b"
    << "\\bSUBMODEL_BACKGROUND_COLOR\\b"
    << "\\bSUBMODEL_DISPLAY\\b"
    << "\\bSUBMODEL_FONT\\b"
    << "\\bSUBMODEL_FONT_COLOR\\b"
    << "\\bSUBMODEL_GRABBER\\b"
    << "\\bSUBMODEL_INSTANCE\\b"
    << "\\bSUBMODEL_INSTANCE_COUNT\\b"
    << "\\bSUBMODEL_INSTANCE_COUNT_OVERRIDE\\b"
    << "\\bSUBMODEL_INST_COUNT\\b"
    << "\\bSUBMODEL_ROTATION\\b"
    << "\\bSYNTHESIZED\\b"
    << "\\bTARGET_POSITION\\b"
    << "\\bTERTIARY\\b"
    << "\\bTERTIARY_DIRECTION\\b"
    << "\\bTEXT\\b"
    << "\\bTEXT_PLACEMENT\\b"
    << "\\bTHICKNESS\\b"
    << "\\bTHIN_LINE_LOGO\\b"
    << "\\bTILE\\b"
    << "\\bTIP\\b"
    << "\\bTOGGLE_PAGE_NUMBER_PLACEMENT\\b"
    << "\\bTOP\\b"
    << "\\bTOP_LEFT\\b"
    << "\\bTOP_RIGHT\\b"
    << "\\bTRANSPARENT\\b"
    << "\\bTYPE\\b"
    << "\\bUSE\\b"
    << "\\bUSE_FREE_FORM\\b"
    << "\\bUSE_TITLE\\b"
    << "\\bUSE_TITLE_AND_FREE_FORM\\b"
    << "\\bVERTICAL\\b"
    << "\\bVIEW_ANGLE\\b"
    << "\\bWHOLE\\b"
    << "\\bWIDTH\\b"
    ;

    Q_FOREACH (QString pattern, LPubBodyMetaPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LPubBodyMetaFormat;
        highlightingRules.append(rule);
    }

    // LPub3D Meta Format
    LPubMetaFormat.setForeground(br24);
    LPubMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("!?\\bLPUB\\b");
    rule.format = LPubMetaFormat;
    highlightingRules.append(rule);

    // LDraw Header Value Format
    LDrawHeaderValueFormat.setForeground(br26);
    LDrawHeaderValueFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("^(?!0 !LPUB |0 !LEOCAD |0 !LDCAD |0 MLCAD |0 SYNTH |0 !COLOUR |0 !FADE|0 !SILHOUETTE|0 ROTSTEP |0 BUFEXCHG |0 PLIST |1 )\\b(?:AUTHOR|)\\b.*$");
    rule.format = LDrawHeaderValueFormat;
    highlightingRules.append(rule);

    // LDraw Header Format
    LDrawHeaderFormat.setForeground(br02);
    LDrawHeaderFormat.setFontWeight(QFont::Bold);

    QStringList LDrawHeaderPatterns;
    LDrawHeaderPatterns
    << "\\bAUTHOR\\b:"
    << "\\bBFC\\b"
    << "!?\\bCATEGORY\\b(?!\")"             // (?!\") match CATEGORY not followed by "
    << "\\bCERTIFY\\b"
    << "\\bCCW\\b"
    << "\\bCLEAR\\b"
    << "!?\\bCMDLINE\\b"
    << "!?\\bHELP\\b"
    << "!?\\bHISTORY\\b"
    << "!?\\bKEYWORDS\\b"
    << "!?\\bLDRAW_ORG\\b"
    << "!?\\bLICENSE\\b"
    << "\\bNAME\\b:"
    << "^(?!0 !LPUB|1).*\\bFILE\\b"
    << "\\bNOFILE\\b"
    << "!?\\bHELP\\b"
    << "\\bOFFICIAL\\b"
    << "\\bORIGINAL LDRAW\\b"
    << "!?\\bTHEME\\b"
    << "\\bUNOFFICIAL MODEL\\b"
    << "\\bUN-OFFICIAL\\b"
    << "\\bUNOFFICIAL\\b"
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

    // MLCad Body Meta Format
    MLCadBodyMetaFormat.setForeground(br17);
    MLCadBodyMetaFormat.setFontWeight(QFont::Bold);

    QStringList MLCadBodyMetaPatterns;
    MLCadBodyMetaPatterns
    << "\\bABS\\b"
    << "\\bADD\\b"
    << "\\bMLCAD ARROW\\b"
    << "\\bMLCAD BACKGROUND\\b"
    << "\\bBTG\\b"
    << "\\bBUFEXCHG\\b"
    << "\\bFLEXHOSE\\b"
    << "\\bGHOST\\b"
    << "\\bMLCAD GROUP\\b"
    << "\\bHIDE\\b"
    << "\\bREL\\b"
    << "\\bRETRIEVE\\b"
    << "\\bROTATION AXLE\\b"
    << "\\bROTATION CENTER\\b"
    << "\\bROTATION CONFIG\\b"
    << "\\bROTATION\\b"
    << "\\bROTSTEP END\\b"
    << "\\bROTSTEP\\b"
    << "\\bSKIP_BEGIN\\b"
    << "\\bSKIP_END\\b"
    << "\\bSTORE\\b"
       ;

    Q_FOREACH (QString pattern, MLCadBodyMetaPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = MLCadBodyMetaFormat;
        highlightingRules.append(rule);
    }

    // MLCad Meta Format
    MLCadMetaFormat.setForeground(br21);
    MLCadMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("!?\\bMLCAD\\b");
    rule.format = MLCadMetaFormat;
    highlightingRules.append(rule);

    // LSynth Format
    LSynthMetaFormat.setForeground(br18);
    LSynthMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("!?\\bSYNTH\\b[^\n]*");
    rule.format = LSynthMetaFormat;
    highlightingRules.append(rule);

    // LDCad Body Meta Format
    LDCadBodyMetaFormat.setForeground(br19);
    LDCadBodyMetaFormat.setFontWeight(QFont::Bold);

    QStringList LDCadBodyMetaPatterns;
    LDCadBodyMetaPatterns
    << "\\bLDCAD CONTENT\\b"
    << "\\bGENERATED\\b"
    << "\\bMARKER\\b"
    << "\\bPATH_POINT\\b"
    << "\\bPATH_CAP\\b"
    << "\\bPATH_ANCHOR\\b"
    << "\\bPATH_SKIN\\b"
    << "\\bPATH_LENGTH\\b"
    << "\\bSCRIPT\\b"
    << "\\bSNAP_CLEAR\\b"
    << "\\bSNAP_INCL\\b"
    << "\\bSNAP_CYL\\b"
    << "\\bSNAP_CLP\\b"
    << "\\bSNAP_FGR\\b"
    << "\\bSNAP_GEN\\b"
    << "\\bSNAP_SPH\\b"
    << "\\bSPRING_POINT\\b"
    << "\\bSPRING_CAP\\b"
    << "\\bSPRING_ANCHOR\\b"
    << "\\bSPRING_SECTION\\b"
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

    // LDCad Meta Group Format
    LDCadMetaGroupFormat.setForeground(br29);
    LDCadMetaGroupFormat.setFontWeight(QFont::Bold);

    QStringList LDCadMetaGroupPatterns;
    LDCadMetaGroupPatterns
            << "\\bGROUP_DEF\\b"
            << "\\bGROUP_NXT\\b"
            << "\\bGROUP_OBJ\\b"
               ;

    Q_FOREACH (QString pattern, LDCadMetaGroupPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LDCadMetaGroupFormat;
        highlightingRules.append(rule);
    }

    // LDCad Value Bracket Format
    LDCadBracketFormat.setForeground(br17);
    LDCadBracketFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("[\\[|=|\\]]");
    rule.format = LDCadBracketFormat;
    highlightingRules.append(rule);

    // LDCad Meta Key Format
    LDCadMetaKeyFormat.setForeground(br11);
    LDCadMetaKeyFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("!?\\bLDCAD\\b");
    rule.format = LDCadMetaKeyFormat;
    highlightingRules.append(rule);

    // LeoCAD Body Meta Format
    LeoCADBodyMetaFormat.setForeground(br17);
    LeoCADBodyMetaFormat.setFontWeight(QFont::Bold);

    QStringList LeoCADBodyMetaPatterns;    // (?<=LEOCAD\\s) match KEYWORD preceded by 'LEOCAD '
    LeoCADBodyMetaPatterns
    << "\\bLEOCAD MODEL\\b"
    << "\\bLEOCAD MODEL NAME\\b"
    << "\\bMODEL AUTHOR\\b"
    << "\\bMODEL DESCRIPTION\\b"
    << "\\bMODEL COMMENT\\b"
    << "\\bMODEL BACKGROUND COLOR\\b"
    << "\\bMODEL BACKGROUND GRADIENT\\b"
    << "\\bMODEL BACKGROUND IMAGE\\b"
    << "\\bPIECE\\b"
    << "\\bPIECE STEP_HIDE\\b"
    << "\\bPIECE HIDDEN\\b"
    << "\\bPIECE POSITION_KEY\\b"
    << "\\bPIECE ROTATION_KEY\\b"
    << "\\bPIECE PIVOT\\b"
    << "\\bCAMERA\\b"
    << "\\bCAMERA HIDDEN\\b"
    << "\\bCAMERA ORTHOGRAPHIC\\b"
    << "\\bCAMERA FOV\\b"
    << "\\bCAMERA ZNEAR\\b"
    << "\\bCAMERA ZFAR\\b"
    << "\\bCAMERA POSITION\\b"
    << "\\bCAMERA TARGET_POSITION\\b"
    << "\\bCAMERA UP_VECTOR\\b"
    << "\\bCAMERA POSITION_KEY\\b"
    << "\\bCAMERA TARGET_POSITION_KEY\\b"
    << "\\bCAMERA UP_VECTOR_KEY\\b"
    << "\\bCAMERA NAME\\b"
    << "\\bLIGHT\\b"
    << "\\bLIGHT ANGLE\\b"
    << "\\bLIGHT ANGLE_KEY\\b"
    << "\\bLIGHT COLOR_RGB\\b"
    << "\\bLIGHT COLOR_RGB_KEY\\b"
    << "\\bLIGHT CUTOFF_DISTANCE\\b"
    << "\\bLIGHT CUTOFF_DISTANCE_KEY\\b"
    << "\\bLIGHT HEIGHT\\b"
    << "\\bLIGHT NAME\\b"
    << "\\bLIGHT POSITION\\b"
    << "\\bLIGHT POSITION_KEY\\b"
    << "\\bLIGHT POWER\\b"
    << "\\bLIGHT POWER_KEY\\b"
    << "\\bLIGHT RADIUS\\b"
    << "\\bLIGHT RADIUS_AND_SPOT_BLEND_KEY\\b"
    << "\\bLIGHT RADIUS_KEY\\b"
    << "\\bLIGHT SHAPE\\b"
    << "\\bLIGHT SHAPE_KEY\\b"
    << "\\bLIGHT SIZE\\b"
    << "\\bLIGHT SIZE_KEY\\b"
    << "\\bLIGHT SPECULAR\\b"
    << "\\bLIGHT SPECULAR_KEY\\b"
    << "\\bLIGHT SPOT_BLEND\\b"
    << "\\bLIGHT SPOT_SIZE\\b"
    << "\\bLIGHT SPOT_SIZE_KEY\\b"
    << "\\bLIGHT STRENGTH\\b"
    << "\\bLIGHT STRENGTH_KEY\\b"
    << "\\bLIGHT TARGET_POSITION\\b"
    << "\\bLIGHT TARGET_POSITION_KEY\\b"
    << "\\bLIGHT TYPE\\b"
    << "\\bLIGHT TYPE_KEY\\b"
    << "\\bLIGHT WIDTH\\b"
    << "\\bLEOCAD GROUP BEGIN\\b"
    << "\\bLEOCAD GROUP END\\b"
    << "\\bLEOCAD SYNTH\\b"              // match SYNTH preceded by 'LEOCAD '
    ;

    Q_FOREACH (QString pattern, LeoCADBodyMetaPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = LeoCADBodyMetaFormat;
        highlightingRules.append(rule);
    }

    // LeoCAD Format
    LeoCADMetaFormat.setForeground(br20);
    LeoCADMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("!?\\bLEOCAD\\b");
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

    QStringList tt = text.mid(index).trimmed().split(" ", SkipEmptyParts);
    if (tt.size()) {
        QString part;
        for (int t = 14; t < tt.size(); t++)
            part += (tt[t]+" ");
        QStringList tokens;
        if (tt.size() > 14) {
            tokens  << tt[0]                         //  1 - part type
                    << tt[1]                         //  2 - color
                    << tt[ 2]+" "+tt[ 3]+" "+tt[ 4]  //  5 - position
                    << tt[ 5]+" "+tt[ 6]+" "+tt[ 7]  //  8 - transform
                    << tt[ 8]+" "+tt[ 9]+" "+tt[10]  // 11 - transform
                    << tt[11]+" "+tt[12]+" "+tt[13]  // 14 - transform
                    << part.trimmed();               // 15 - part
        } else {
            for (int i = 0; i < tt.size(); i++) {
                if (i < 2)
                    tokens << tt[i];
                if (tt.size() < 2)
                    break;
                if (i == 4)
                    tokens << tt[2] +" "+tt[3] +" "+tt[4];
                if (tt.size() < 5)
                    break;
                if (i == 7)
                    tokens << tt[5] +" "+tt[6] +" "+tt[7];
                if (tt.size() < 8)
                    break;
                if (i == 10)
                    tokens << tt[8] +" "+tt[9] +" "+tt[10];
                if (tt.size() < 11)
                    break;
                if (i == 13)
                    tokens << tt[11]+" "+tt[12]+" "+tt[13];
                if (tt.size() < 14)
                    break;
            }
        }
        for (int i = 0; i < tokens.size(); i++) {
            if (index >= 0 && index < text.length()) {
                setFormat(index, tokens[i].length(), lineType1Formats[i]);
                index += tokens[i].length() + 1;     // add 1 position for the space
            }
        }
    }
}
