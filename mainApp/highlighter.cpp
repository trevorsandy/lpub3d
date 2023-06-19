/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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
#include "declarations.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    QBrush br01,br02,br03,br04,br05,br06,br07,br08,br09,br10,br11,br12,br13,br14,br15,br16;
    QBrush br17,br18,br19,br20,br21,br22,br23,br24,br25,br26,br27,br28,br29,br30,br31,br32;
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
        br13 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPE_2]));
        br14 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPE_3]));
        br15 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPE_4]));
        br16 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPE_5]));
        br17 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_NUMBER]));
        br18 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_HEX_NUMBER]));
        br19 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_PAGE_SIZE]));
        br20 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LEOCAD]));
        br21 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LSYNTH]));
        br22 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDCAD]));
        br23 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_MLCAD]));
        br24 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_MLCAD_BODY]));
        br25 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_FALSE]));
        br26 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_TRUE]));
        br27 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D]));
        br28 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_BODY]));
        br29 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_HEADER_VALUE]));
        br30 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LPUB3D_QUOTED_TEXT]));
        br31 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDRAW_LINE_TYPE_0_FIRST_CHARACTER]));
        br32 = QBrush(QColor(Preferences::themeColors[THEME_DEFAULT_DECORATE_LDCAD_GROUP_DEFINE]));
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
        br13 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_LINE_TYPE_2]));
        br14 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_LINE_TYPE_3]));
        br15 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_LINE_TYPE_4]));
        br16 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_LINE_TYPE_5]));
        br17 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_NUMBER]));
        br18 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_HEX_NUMBER]));
        br19 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_PAGE_SIZE]));
        br20 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LEOCAD]));
        br21 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LSYNTH]));
        br22 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDCAD]));
        br23 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_MLCAD]));
        br24 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_MLCAD_BODY]));
        br25 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_FALSE]));
        br26 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_TRUE]));
        br27 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D]));
        br28 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_BODY]));
        br29 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_HEADER_VALUE]));
        br30 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_QUOTED_TEXT]));
        br31 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDRAW_LINE_TYPE_0_FIRST_CHARACTER]));
        br32 = QBrush(QColor(Preferences::themeColors[THEME_DARK_DECORATE_LDCAD_GROUP_DEFINE]));
    }

    // LPub3D Quoted Text Format
    LPubQuotedTextFormat.setForeground(br30);
    LPubQuotedTextFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression(QStringLiteral("[<\"].*[>\"]"));
    rule.format = LPubQuotedTextFormat;
    highlightingRules.append(rule);

    // LPub3D Number Format
    LPubNumberFormat.setForeground(br17);
    LPubNumberFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression(QStringLiteral("-?(?:0|[1-9]\\d*)(?:\\.\\d+)?"));
    rule.format = LPubNumberFormat;
    highlightingRules.append(rule);

    // LPub3D Font Number Format
    LPubFontNumberFormat.setForeground(br17);
    LPubFontNumberFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression(QStringLiteral("[,-](\\d+)")); // match digit if preceded by single character , or -
    rule.format = LPubFontNumberFormat;
    highlightingRules.append(rule);

    // LDraw Custom COLOUR Description Format
    LDrawColourDescFormat.setForeground(br29);
    LDrawColourDescFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("\\bLPub3D_[A-Za-z|_]+\\b"), QRegularExpression::CaseInsensitiveOption);
    rule.format = LDrawColourDescFormat;
    highlightingRules.append(rule);

    // LPub3D Substitute Color Format
    LPubSubColorFormat.setForeground(br07);
    LPubSubColorFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("\\bBEGIN SUB\\b\\s.*.[dat|mpd|ldr]\\s([0-9]+)")); // match color format if preceded by 'BEGIN SUB *.ldr|dat|mpd '
    rule.format = LPubSubColorFormat;
    highlightingRules.append(rule);

    // LPub3D Custom COLOUR Code Format
    LPubCustomColorFormat.setForeground(br07);
    LPubCustomColorFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("CODE\\s([0-9]+)\\sVALUE")); // match color format if preceded by 'CODE ' and followed by ' VALUE'
    rule.format = LPubCustomColorFormat;
    highlightingRules.append(rule);

    // LPub3D Substitute Part Format
    LPubSubPartFormat.setForeground(br12);
    LPubSubPartFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("\\bBEGIN SUB\\b\\s([A-Za-z0-9\\s_-]+.[dat|mpd|ldr]+)")); // match part format if preceded by 'BEGIN SUB'
    rule.format = LPubSubPartFormat;
    highlightingRules.append(rule);

    // LPub3D Font Number Comma and dash Format
    LPubFontCommaFormat.setForeground(br30);
    LPubFontCommaFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression(QStringLiteral("[,]"));
    rule.format = LPubFontCommaFormat;
    highlightingRules.append(rule);

    // LPub3D Page Size Format
    LPubPageSizeFormat.setForeground(br19);
    LPubPageSizeFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[AB][0-9]0?$\\b|\\bComm10E\\b$|\\bArch[1-3]\\b$"), QRegularExpression::CaseInsensitiveOption); // Qt::CaseInsensitive
    rule.format = LPubPageSizeFormat;
    highlightingRules.append(rule);

    // LDraw Custom COLOUR Meta Format
    LDrawColourMetaFormat.setForeground(br05);
    LDrawColourMetaFormat.setFontWeight(QFont::Bold);

    const QString LDrawColourPatterns[] =
    {
        QStringLiteral("\\bCODE\\b"),
        QStringLiteral("\\bVALUE\\b"),
        QStringLiteral("\\bEDGE\\b"),
        QStringLiteral("\\bALPHA\\b")
    };

    for (const QString &pattern : LDrawColourPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = LDrawColourMetaFormat;
        highlightingRules.append(rule);
    }

    // LPub3D Custom COLOUR, FADE, SILHOUETTE Meta Format
    LPubCustomColorFormat.setForeground(br23);
    LPubCustomColorFormat.setFontWeight(QFont::Bold);

    const QString LPubCustomColorPatterns[] =
    {
        QStringLiteral("!?\\bCOLOUR\\b"),
        QStringLiteral("!?\\bFADE\\b"),
        QStringLiteral("!?\\bSILHOUETTE\\b")
    };

    for (const QString &pattern : LPubCustomColorPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = LPubCustomColorFormat;
        highlightingRules.append(rule);
    }

    // LPub3D Local Context Format
    LPubLocalMetaFormat.setForeground(br04);
    LPubLocalMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("\\bLOCAL\\b"));
    rule.format = LPubLocalMetaFormat;
    highlightingRules.append(rule);

    // LPub3D Global Context Format
    LPubGlobalMetaFormat.setForeground(br05);
    LPubGlobalMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("\\bGLOBAL\\b"));
    rule.format = LPubGlobalMetaFormat;
    highlightingRules.append(rule);

    // LPub3D Boolean False Format
    LPubFalseMetaFormat.setForeground(br25);
    LPubFalseMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("\\bFALSE\\b"));
    rule.format = LPubFalseMetaFormat;
    highlightingRules.append(rule);

    // LPub3D Boolean True Format
    LPubTrueMetaFormat.setForeground(br26);
    LPubTrueMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("\\bTRUE\\b"));
    rule.format = LPubTrueMetaFormat;
    highlightingRules.append(rule);

    // LPub3D Body Meta Format
    LPubBodyMetaFormat.setForeground(br28);
    LPubBodyMetaFormat.setFontWeight(QFont::Bold);

    const QString LPubBodyMetaPatterns[] =
    {
        QStringLiteral("\\bADJUST_ON_ITEM_OFFSET\\b"),
        QStringLiteral("\\bALLOC\\b"),
        QStringLiteral("\\bANGLE\\b"),
        QStringLiteral("\\bANNOTATE\\b"),
        QStringLiteral("\\bANNOTATION\\b"),
        QStringLiteral("\\bAPPLY\\b"),
        QStringLiteral("\\bAPP_PLUG\\b"),
        QStringLiteral("\\bAPP_PLUG_IMAGE\\b"),
        QStringLiteral("\\bAREA\\b"),
        QStringLiteral("\\bARROW\\b"),
        QStringLiteral("\\bASPECT\\b"),
        QStringLiteral("\\bASSEM\\b"),
        QStringLiteral("\\bASSEMBLED\\b"),
        QStringLiteral("\\bATTRIBUTE_PIXMAP\\b"),
        QStringLiteral("\\bATTRIBUTE_TEXT\\b"),
        QStringLiteral("\\bAT_MODEL\\b"),
        QStringLiteral("\\bAT_STEP\\b"),
        QStringLiteral("\\bAT_TOP\\b"),
        QStringLiteral("\\bAUTOMATE_EDGE_COLOR\\b"),
        QStringLiteral("\\bAXLE\\b"),
        QStringLiteral("\\bBACK\\b"),
        QStringLiteral("\\bBACKGROUND\\b"),
        QStringLiteral("\\bBASE_BOTTOM\\b"),
        QStringLiteral("\\bBASE_LEFT\\b"),
        QStringLiteral("\\bBASE_RIGHT\\b"),
        QStringLiteral("\\bBASE_TOP\\b"),
        QStringLiteral("\\bBEAM\\b"),
        QStringLiteral("\\bBEGIN\\b"),
        QStringLiteral("\\bBLACK_EDGE_COLOR\\b"),
        QStringLiteral("\\bBOM\\b"),
        QStringLiteral("\\bBORDER\\b"),
        QStringLiteral("\\bBOTTOM\\b"),
        QStringLiteral("\\bBOTTOM_LEFT\\b"),
        QStringLiteral("\\bBOTTOM_RIGHT\\b"),
        QStringLiteral("\\bBRICKLINK\\b"),
        QStringLiteral("\\bBRING_TO_FRONT\\b"),
        QStringLiteral("\\bBUILD_MOD\\b"),
        QStringLiteral("\\bBUILD_MOD_ENABLED\\b"),
        QStringLiteral("\\bCABLE\\b"),
        QStringLiteral("\\bCALLOUT\\b"),
        QStringLiteral("\\bCALLOUT_INSTANCE\\b"),
        QStringLiteral("\\bCALLOUT_POINTER\\b"),
        QStringLiteral("\\bCALLOUT_UNDERPINNING\\b"),
        QStringLiteral("\\bCAMERA\\b"),
        QStringLiteral("\\bCAMERA_ANGLES\\b"),
        QStringLiteral("\\bCAMERA_DISTANCE\\b"),
        QStringLiteral("\\bCAMERA_DEFAULT_DISTANCE_FACTOR\\b"),
        QStringLiteral("\\bCAMERA_DISTANCE_NATIVE\\b"),
        QStringLiteral("\\bCAMERA_FOV\\b"),
        QStringLiteral("\\bCAMERA_NAME\\b"),
        QStringLiteral("\\bCAMERA_ORTHOGRAPHIC\\b"),
        QStringLiteral("\\bCAMERA_POSITION\\b"),
        QStringLiteral("\\bCAMERA_TARGET\\b"),
        QStringLiteral("\\bCAMERA_UPVECTOR\\b"),
        QStringLiteral("\\bCAMERA_ZFAR\\b"),
        QStringLiteral("\\bCAMERA_ZNEAR\\b"),
        QStringLiteral("\\bCENTER\\b"),
        QStringLiteral("\\bCIRCLE_STYLE\\b"),
        QStringLiteral("\\bCLEAR\\b"),
        QStringLiteral("\\bCOLOR\\b"),
        QStringLiteral("\\bCOLOR_LIGHT_DARK_INDEX\\b"),
        QStringLiteral("\\bCOLOR_RGB\\b"),
        QStringLiteral("\\bCOLS\\b"),
        QStringLiteral("\\bCONNECTOR\\b"),
        QStringLiteral("\\bCONSOLIDATE_INSTANCE_COUNT\\b"),
        QStringLiteral("\\bCONSOLIDATE_INSTANCE_COUNT_BY_COLOR\\b"),
        QStringLiteral("\\bCONSTRAIN\\b"),
        QStringLiteral("\\bCONTENT\\b"),
        QStringLiteral("\\bCONTINUOUS_STEP_NUMBERS\\b"),
        QStringLiteral("\\bCONTRAST\\b"),
        QStringLiteral("\\bCOUNT_GROUP_STEPS\\b"),
        QStringLiteral("\\bCOVER_PAGE\\b"),
        QStringLiteral("\\bCOVER_PAGE_MODEL_VIEW_ENABLED\\b"),
        QStringLiteral("\\bCROSS\\b"),
        QStringLiteral("\\bCSI_ANNOTATION\\b"),
        QStringLiteral("\\bCSI_ANNOTATION_PART\\b"),
        QStringLiteral("\\bCUSTOM_LENGTH\\b"),
        QStringLiteral("\\bCUTOFF_DISTANCE\\b"),
        QStringLiteral("\\bDARK_EDGE_COLOR\\b"),
        QStringLiteral("\\bDASH\\b"),
        QStringLiteral("\\bDASH_DOT\\b"),
        QStringLiteral("\\bDASH_DOT_DOT\\b"),
        QStringLiteral("\\bDISPLAY\\b"),
        QStringLiteral("\\bDISPLAY_MODEL\\b"),
        QStringLiteral("\\bDISPLAY_PAGE_NUMBER\\b"),
        QStringLiteral("\\bDIVIDER\\b"),
        QStringLiteral("\\bDIVIDER_ITEM\\b"),
        QStringLiteral("\\bDIVIDER_LINE\\b"),
        QStringLiteral("\\bDIVIDER_POINTER\\b"),
        QStringLiteral("\\bDIVIDER_POINTER_ATTRIBUTE\\b"),
        QStringLiteral("\\bDOCUMENT_AUTHOR\\b"),
        QStringLiteral("\\bDOCUMENT_AUTHOR_BACK\\b"),
        QStringLiteral("\\bDOCUMENT_AUTHOR_FRONT\\b"),
        QStringLiteral("\\bDOCUMENT_COVER_IMAGE\\b"),
        QStringLiteral("\\bDOCUMENT_LOGO\\b"),
        QStringLiteral("\\bDOCUMENT_LOGO_BACK\\b"),
        QStringLiteral("\\bDOCUMENT_LOGO_FRONT\\b"),
        QStringLiteral("\\bDOCUMENT_TITLE\\b"),
        QStringLiteral("\\bDOCUMENT_TITLE_BACK\\b"),
        QStringLiteral("\\bDOCUMENT_TITLE_FRONT\\b"),
        QStringLiteral("\\bDOT\\b"),
        QStringLiteral("\\bDPCM\\b"),
        QStringLiteral("\\bDPI\\b"),
        QStringLiteral("\\bEDGE_COLOR\\b"),
        QStringLiteral("\\bELEMENT\\b"),
        QStringLiteral("\\bELEMENT_STYLE\\b"),
        QStringLiteral("\\bENABLE\\b"),
        QStringLiteral("\\bENABLED\\b"),
        QStringLiteral("\\bENABLE_SETTING\\b"),
        QStringLiteral("\\bENABLE_STYLE\\b"),
        QStringLiteral("\\bENABLE_TEXT_PLACEMENT\\b"),
        QStringLiteral("\\bEND\\b"),
        QStringLiteral("\\bEND_MOD\\b"),
        QStringLiteral("\\bEXTENDED\\b"),
        QStringLiteral("\\bFADE_STEPS\\b"),
        QStringLiteral("\\bFILE\\b"),
        QStringLiteral("\\bFILL\\b"),
        QStringLiteral("\\bFINAL_MODEL_ENABLED\\b"),
        QStringLiteral("\\bFIXED_ANNOTATIONS\\b"),
        QStringLiteral("\\bFLATTENED_LOGO\\b"),
        QStringLiteral("\\bFONT\\b"),
        QStringLiteral("\\bFONT_COLOR\\b"),
        QStringLiteral("\\bFOR_SUBMODEL\\b"),
        QStringLiteral("\\bFRONT\\b"),
        QStringLiteral("\\bFREEFORM\\b"),
        QStringLiteral("\\bGRADIENT\\b"),
        QStringLiteral("\\bGROUP\\b"),
        QStringLiteral("\\bHEIGHT\\b"),
        QStringLiteral("\\bHIDDEN\\b"),
        QStringLiteral("\\bHIDE_TIP\\b"),
        QStringLiteral("\\bHIGHLIGHT_STEP\\b"),
        QStringLiteral("\\bHIGH_CONTRAST\\b"),
        QStringLiteral("\\bHIGH_CONTRAST_WITH_LOGO\\b"),
        QStringLiteral("\\bHOME\\b"),
        QStringLiteral("\\bHORIZONTAL\\b"),
        QStringLiteral("\\bHOSE\\b"),
        QStringLiteral("\\bHTML_TEXT\\b"),
        QStringLiteral("\\bICON\\b"),
        QStringLiteral("\\bID\\b"),
        QStringLiteral("\\bIGN\\b"),
        QStringLiteral("\\bIMAGE_SIZE\\b"),
        QStringLiteral("\\bINCLUDE\\b"),
        QStringLiteral("\\bINCLUDE_SUBMODELS\\b"),
        QStringLiteral("\\bINSERT\\b"),
        QStringLiteral("\\bINSIDE\\b"),
        QStringLiteral("\\bINSTANCE_COUNT\\b"),
        QStringLiteral("\\bJUSTIFY_CENTER\\b"),
        QStringLiteral("\\bJUSTIFY_CENTER_HORIZONTAL\\b"),
        QStringLiteral("\\bJUSTIFY_CENTER_VERTICAL\\b"),
        QStringLiteral("\\bJUSTIFY_LEFT\\b"),
        QStringLiteral("\\bLANDSCAPE\\b"),
        QStringLiteral("\\bLDGLITE\\b"),
        QStringLiteral("\\bLDGLITE_PARMS\\b"),
        QStringLiteral("\\bLDVIEW\\b"),
        QStringLiteral("\\bLDVIEW_PARMS\\b"),
        QStringLiteral("\\bLDVIEW_POV_GENERATOR\\b"),
        QStringLiteral("\\bLEFT\\b"),
        QStringLiteral("\\bLEGO\\b"),
        QStringLiteral("\\bLEGO_DISCLAIMER\\b"),
        QStringLiteral("\\bLIGHT\\b"),
        QStringLiteral("\\bLINE\\b"),
        QStringLiteral("\\bLINE_WIDTH\\b"),
        QStringLiteral("\\bLOAD_UNOFFICIAL_PARTS_IN_EDITOR\\b"),
        QStringLiteral("\\bLOCAL_LEGO_ELEMENTS_FILE\\b"),
        QStringLiteral("\\bLPUB_FADE\\b"),
        QStringLiteral("\\bLPUB_HIGHLIGHT\\b"),
        QStringLiteral("\\bMARGINS\\b"),
        QStringLiteral("\\bMODEL\\b"),
        QStringLiteral("\\bMODEL_CATEGORY\\b"),
        QStringLiteral("\\bMODEL_DESCRIPTION\\b"),
        QStringLiteral("\\bMODEL_ID\\b"),
        QStringLiteral("\\bMODEL_PARTS\\b"),
        QStringLiteral("\\bMODEL_SCALE\\b"),
        QStringLiteral("\\bMODEL_STEP_NUMBER\\b"),
        QStringLiteral("\\bMULTI_STEP\\b"),
        QStringLiteral("\\bMULTI_STEPS\\b"),
        QStringLiteral("\\bNAME\\b"),
        QStringLiteral("\\bNATIVE\\b"),
        QStringLiteral("\\bNONE\\b"),
        QStringLiteral("\\bNOSTEP\\b"),
        QStringLiteral("\\bNUMBER\\b"),
        QStringLiteral("\\bOFFSET\\b"),
        QStringLiteral("\\bOPACITY\\b"),
        QStringLiteral("\\bORIENTATION\\b"),
        QStringLiteral("\\bOUTLINE_LOGO\\b"),
        QStringLiteral("\\bOUTSIDE\\b"),
        QStringLiteral("\\bPAGE\\b"),
        QStringLiteral("\\bPAGE_FOOTER\\b"),
        QStringLiteral("\\bPAGE_HEADER\\b"),
        QStringLiteral("\\bPAGE_LENGTH\\b"),
        QStringLiteral("\\bPAGE_NUMBER\\b"),
        QStringLiteral("\\bPAGE_POINTER\\b"),
        QStringLiteral("\\bPANEL\\b"),
        QStringLiteral("\\bPARSE_NOSTEP\\b"),
        QStringLiteral("\\bPART\\b"),
        QStringLiteral("\\bPART_ELEMENTS\\b"),
        QStringLiteral("\\bPART_GROUP\\b"),
        QStringLiteral("\\bPART_GROUP_ENABLE\\b"),
        QStringLiteral("\\bPART_ROTATION\\b"),
        QStringLiteral("\\bPER_STEP\\b"),
        QStringLiteral("\\bPICTURE\\b"),
        QStringLiteral("\\bPIECE\\b"),
        QStringLiteral("\\bPLACEMENT\\b"),
        QStringLiteral("\\bPLAIN\\b"),
        QStringLiteral("\\bPLI\\b"),
        QStringLiteral("\\bPLI_ANNOTATION\\b"),
        QStringLiteral("\\bPLI_GRABBER\\b"),
        QStringLiteral("\\bPLI_INSTANCE\\b"),
        QStringLiteral("\\bPLI_PART\\b"),
        QStringLiteral("\\bPLI_PART_GROUP\\b"),
        QStringLiteral("\\bPOINTER\\b"),
        QStringLiteral("\\bPOINTER_ATTRIBUTE\\b"),
        QStringLiteral("\\bPOINTER_BASE\\b"),
        QStringLiteral("\\bPOINTER_GRABBER\\b"),
        QStringLiteral("\\bPOINTER_HEAD\\b"),
        QStringLiteral("\\bPOINTER_SEG_FIRST\\b"),
        QStringLiteral("\\bPOINTER_SEG_SECOND\\b"),
        QStringLiteral("\\bPOINTER_SEG_THIRD\\b"),
        QStringLiteral("\\bPORTRAIT\\b"),
        QStringLiteral("\\bPOSITION\\b"),
        QStringLiteral("\\bPOVRAY\\b"),
        QStringLiteral("\\bPOVRAY_PARMS\\b"),
        QStringLiteral("\\bPOWER\\b"),
        QStringLiteral("\\bPREFERRED_RENDERER\\b"),
        QStringLiteral("\\bPRIMARY\\b"),
        QStringLiteral("\\bPRIMARY_DIRECTION\\b"),
        QStringLiteral("\\bPUBLISH_COPYRIGHT\\b"),
        QStringLiteral("\\bPUBLISH_COPYRIGHT_BACK\\b"),
        QStringLiteral("\\bPUBLISH_DESCRIPTION\\b"),
        QStringLiteral("\\bPUBLISH_EMAIL\\b"),
        QStringLiteral("\\bPUBLISH_EMAIL_BACK\\b"),
        QStringLiteral("\\bPUBLISH_URL\\b"),
        QStringLiteral("\\bPUBLISH_URL_BACK\\b"),
        QStringLiteral("\\bRADIUS\\b"),
        QStringLiteral("\\bRANGE\\b"),
        QStringLiteral("\\bRECTANGLE_STYLE\\b"),
        QStringLiteral("\\bREMOVE\\b"),
        QStringLiteral("\\bRESERVE\\b"),
        QStringLiteral("\\bRESOLUTION\\b"),
        QStringLiteral("\\bRICH_TEXT\\b"),
        QStringLiteral("\\bRIGHT\\b"),
        QStringLiteral("\\bROTATED\\b"),
        QStringLiteral("\\bROTATE_ICON\\b"),
        QStringLiteral("\\bROUND\\b"),
        QStringLiteral("\\bROUNDED_TOP_LOGO\\b"),
        QStringLiteral("\\bSATURATION\\b"),
        QStringLiteral("\\bSCALE\\b"),
        QStringLiteral("\\bSCENE\\b"),
        QStringLiteral("\\bSECONDARY\\b"),
        QStringLiteral("\\bSECONDARY_DIRECTION\\b"),
        QStringLiteral("\\bSEND_TO_BACK\\b"),
        QStringLiteral("\\bSEPARATOR\\b"),
        QStringLiteral("\\bSETUP\\b"),
        QStringLiteral("\\bSHAPE\\b"),
        QStringLiteral("\\bSHARP_TOP_LOGO\\b"),
        QStringLiteral("\\bSHOW\\b"),
        QStringLiteral("\\bSHOW_GROUP_STEP_NUMBER\\b"),
        QStringLiteral("\\bSHOW_INSTANCE_COUNT\\b"),
        QStringLiteral("\\bSHOW_STEP_NUM\\b"),
        QStringLiteral("\\bSHOW_STEP_NUMBER\\b"),
        QStringLiteral("\\bSHOW_SUBMODEL_IN_CALLOUT\\b"),
        QStringLiteral("\\bSHOW_TOP_MODEL\\b"),
        QStringLiteral("\\bSINGLE_CALL\\b"),
        QStringLiteral("\\bSINGLE_CALL_EXPORT_LIST\\b"),
        QStringLiteral("\\bSINGLE_STEP\\b"),
        QStringLiteral("\\bSIZE\\b"),
        QStringLiteral("\\bSKIP_BEGIN\\b"),
        QStringLiteral("\\bSKIP_END\\b"),
        QStringLiteral("\\bSOLID\\b"),
        QStringLiteral("\\bSORT\\b"),
        QStringLiteral("\\bSORT_BY\\b"),
        QStringLiteral("\\bSORT_OPTION\\b"),
        QStringLiteral("\\bSORT_ORDER\\b"),
        QStringLiteral("\\bSPECULAR\\b"),
        QStringLiteral("\\bSPOT_BLEND\\b"),
        QStringLiteral("\\bSPOT_SIZE\\b"),
        QStringLiteral("\\bSQUARE\\b"),
        QStringLiteral("\\bSQUARE_STYLE\\b"),
        QStringLiteral("\\bSTART_PAGE_NUMBER\\b"),
        QStringLiteral("\\bSTART_STEP_NUMBER\\b"),
        QStringLiteral("\\bSTEP\\b"),
        QStringLiteral("\\bSTEPS\\b"),
        QStringLiteral("\\bSTEP_GROUP\\b"),
        QStringLiteral("\\bSTEP_NUMBER\\b"),
        QStringLiteral("\\bSTEP_PLI\\b"),
        QStringLiteral("\\bSTEP_RECTANGLE\\b"),
        QStringLiteral("\\bSTEP_SIZE\\b"),
        QStringLiteral("\\bSTRENGTH\\b"),
        QStringLiteral("\\bSTRETCH\\b"),
        QStringLiteral("\\bSTUD_CYLINDER_COLOR\\b"),
        QStringLiteral("\\bSTUD_STYLE\\b"),
        QStringLiteral("\\bSTYLE\\b"),
        QStringLiteral("\\bSUB\\b"),
        QStringLiteral("\\bLDRAW_TYPE\\b"),
        QStringLiteral("\\bSUBMODEL_BACKGROUND_COLOR\\b"),
        QStringLiteral("\\bSUBMODEL_DISPLAY\\b"),
        QStringLiteral("\\bSUBMODEL_FONT\\b"),
        QStringLiteral("\\bSUBMODEL_FONT_COLOR\\b"),
        QStringLiteral("\\bSUBMODEL_GRABBER\\b"),
        QStringLiteral("\\bSUBMODEL_INSTANCE\\b"),
        QStringLiteral("\\bSUBMODEL_INSTANCE_COUNT\\b"),
        QStringLiteral("\\bSUBMODEL_INSTANCE_COUNT_OVERRIDE\\b"),
        QStringLiteral("\\bSUBMODEL_INST_COUNT\\b"),
        QStringLiteral("\\bSUBMODEL_ROTATION\\b"),
        QStringLiteral("\\bSYNTHESIZED\\b"),
        QStringLiteral("\\bTARGET_POSITION\\b"),
        QStringLiteral("\\bTERTIARY\\b"),
        QStringLiteral("\\bTERTIARY_DIRECTION\\b"),
        QStringLiteral("\\bTEXT\\b"),
        QStringLiteral("\\bTEXT_PLACEMENT\\b"),
        QStringLiteral("\\bTHICKNESS\\b"),
        QStringLiteral("\\bTHIN_LINE_LOGO\\b"),
        QStringLiteral("\\bTILE\\b"),
        QStringLiteral("\\bTIP\\b"),
        QStringLiteral("\\bTOGGLE_PAGE_NUMBER_PLACEMENT\\b"),
        QStringLiteral("\\bTOP\\b"),
        QStringLiteral("\\bTOP_LEFT\\b"),
        QStringLiteral("\\bTOP_RIGHT\\b"),
        QStringLiteral("\\bTRANSPARENT\\b"),
        QStringLiteral("\\bTYPE\\b"),
        QStringLiteral("\\bUSE\\b"),
        QStringLiteral("\\bUSE_FREE_FORM\\b"),
        QStringLiteral("\\bUSE_TITLE\\b"),
        QStringLiteral("\\bUSE_TITLE_AND_FREE_FORM\\b"),
        QStringLiteral("\\bVERTICAL\\b"),
        QStringLiteral("\\bVIEW_ANGLE\\b"),
        QStringLiteral("\\bWHOLE\\b"),
        QStringLiteral("\\bWIDTH\\b")
    };

    for (const QString &pattern : LPubBodyMetaPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = LPubBodyMetaFormat;
        highlightingRules.append(rule);
    }

    // LPub3D Meta Format
    LPubMetaFormat.setForeground(br27);
    LPubMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("!?\\bLPUB\\b"));
    rule.format = LPubMetaFormat;
    highlightingRules.append(rule);

    // LDraw Header Value Format
    LDrawHeaderValueFormat.setForeground(br29);
    LDrawHeaderValueFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression(QStringLiteral("^(?!0 !LPUB|0 !LEOCAD|0 !LDCAD|0 MLCAD|0 GROUP|0 SYNTH|0 !COLOUR|0 !FADE|0 !SILHOUETTE|0 ROTSTEP|0 BUFEXCHG|0 PLIST|[1-5]).*$"));
    rule.format = LDrawHeaderValueFormat;
    highlightingRules.append(rule);

    // LDraw Header Format
    LDrawHeaderFormat.setForeground(br02);
    LDrawHeaderFormat.setFontWeight(QFont::Bold);

    const QString LDrawHeaderPatterns[] =
    {
        QStringLiteral("\\bAUTHOR\\b:"),
        QStringLiteral("\\bBFC\\b"),
        QStringLiteral("!?\\bCATEGORY\\b(?!\")"),  // (?!\") match CATEGORY not followed by "
        QStringLiteral("\\bCERTIFY\\b"),
        QStringLiteral("\\bCCW\\b"),
        QStringLiteral("\\bCLEAR\\b"),
        QStringLiteral("!?\\bCMDLINE\\b"),
        QStringLiteral("\\bCYLINDRICAL\\b"),
        QStringLiteral("!?\\bDATA\\b"),
        QStringLiteral("!?\\bHELP\\b"),
        QStringLiteral("!?\\bHISTORY\\b"),
        QStringLiteral("!?\\bKEYWORDS\\b"),
        QStringLiteral("!?\\bLDRAW_ORG\\b"),
        QStringLiteral("!?\\bLICENSE\\b"),
        QStringLiteral("\\bNAME\\b:"),
        QStringLiteral("^(?!0 !LPUB|1).*\\bFILE\\b"),
        QStringLiteral("\\bFALLBACK\\b"),
        QStringLiteral("\\bNEXT\\b"),
        QStringLiteral("\\bNOFILE\\b"),
        QStringLiteral("\\bOFFICIAL\\b"),
        QStringLiteral("\\bORIGINAL LDRAW\\b"),
        QStringLiteral("\\bPLANAR\\b"),
        QStringLiteral("\\bSPHERICAL\\b"),
        QStringLiteral("\\bSTART\\b"),
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
        QStringLiteral("!?\\bTEXMAP\\b"),
        QStringLiteral("!?\\bTEXMAP END\\b"),
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
        QStringLiteral("\\bPAUSE\\b"),
        QStringLiteral("\\bPRINT\\b"),
        QStringLiteral("\\bSAVE\\b"),
        QStringLiteral("\\bNOSTEP\\b"),
        QStringLiteral("\\bSTEP\\b"),
        QStringLiteral("\\bWRITE\\b")
    };

    for (const QString &pattern : LDrawBodyPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = LDrawBodyFormat;
        highlightingRules.append(rule);
    }

    // MLCad Body Meta Format
    MLCadBodyMetaFormat.setForeground(br20);
    MLCadBodyMetaFormat.setFontWeight(QFont::Bold);

    const QString MLCadBodyMetaPatterns[] =
    {
        QStringLiteral("\\bABS\\b"),
        QStringLiteral("\\bADD\\b"),
        QStringLiteral("\\bMLCAD ARROW\\b"),
        QStringLiteral("^(?!0 !LPUB|0 !LEOCAD).*\\bBACKGROUND\\b"),
        QStringLiteral("\\bBTG\\b"),
        QStringLiteral("\\bBUFEXCHG\\b"),
        QStringLiteral("\\bFLEXHOSE\\b"),
        QStringLiteral("\\bGHOST\\b"),
        QStringLiteral("^(?!0 !LPUB|0 !LEOCAD).*\\bGROUP\\b"),
        QStringLiteral("\\bHIDE\\b"),
        QStringLiteral("\\bREL\\b"),
        QStringLiteral("\\bRETRIEVE\\b"),
        QStringLiteral("\\bROTATION AXLE\\b"),
        QStringLiteral("\\bROTATION CENTER\\b"),
        QStringLiteral("\\bROTATION CONFIG\\b"),
        QStringLiteral("\\bROTATION\\b"),
        QStringLiteral("\\bROTSTEP END\\b"),
        QStringLiteral("\\bROTSTEP\\b"),
        QStringLiteral("\\bSKIP_BEGIN\\b"),
        QStringLiteral("\\bSKIP_END\\b"),
        QStringLiteral("\\bSTORE\\b")
    };

    for (const QString &pattern : MLCadBodyMetaPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = MLCadBodyMetaFormat;
        highlightingRules.append(rule);
    }

    // MLCad Meta Format
    MLCadMetaFormat.setForeground(br24);
    MLCadMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("!?\\bMLCAD\\b"));
    rule.format = MLCadMetaFormat;
    highlightingRules.append(rule);

    // LSynth Format
    LSynthMetaFormat.setForeground(br21);
    LSynthMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("!?\\bSYNTH\\b[^\n]*"));
    rule.format = LSynthMetaFormat;
    highlightingRules.append(rule);

    // LDCad Body Meta Format
    LDCadBodyMetaFormat.setForeground(br22);
    LDCadBodyMetaFormat.setFontWeight(QFont::Bold);

    const QString LDCadBodyMetaPatterns[] =
    {
        QStringLiteral("\\bLDCAD CONTENT\\b"),
        QStringLiteral("\\bGENERATED\\b"),
        QStringLiteral("\\bMARKER\\b"),
        QStringLiteral("\\bPATH_POINT\\b"),
        QStringLiteral("\\bPATH_CAP\\b"),
        QStringLiteral("\\bPATH_ANCHOR\\b"),
        QStringLiteral("\\bPATH_SKIN\\b"),
        QStringLiteral("\\bPATH_LENGTH\\b"),
        QStringLiteral("\\bSCRIPT\\b"),
        QStringLiteral("\\bSNAP_CLEAR\\b"),
        QStringLiteral("\\bSNAP_INCL\\b"),
        QStringLiteral("\\bSNAP_CYL\\b"),
        QStringLiteral("\\bSNAP_CLP\\b"),
        QStringLiteral("\\bSNAP_FGR\\b"),
        QStringLiteral("\\bSNAP_GEN\\b"),
        QStringLiteral("\\bSNAP_SPH\\b"),
        QStringLiteral("\\bSPRING_POINT\\b"),
        QStringLiteral("\\bSPRING_CAP\\b"),
        QStringLiteral("\\bSPRING_ANCHOR\\b"),
        QStringLiteral("\\bSPRING_SECTION\\b")
    };

    for (const QString &pattern : LDCadBodyMetaPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = LDCadBodyMetaFormat;
        highlightingRules.append(rule);
    }

    // LDCad Meta Value Format
    LDCadMetaValueFormat.setForeground(br08);
    LDCadMetaValueFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression(QStringLiteral("[=]([a-zA-Z\\0-9%.\\s]+)"),QRegularExpression::CaseInsensitiveOption);
    rule.format = LDCadMetaValueFormat;
    highlightingRules.append(rule);

    // LDCad Meta Group Format
    LDCadMetaGroupFormat.setForeground(br32);
    LDCadMetaGroupFormat.setFontWeight(QFont::Bold);

    const QString LDCadMetaGroupPatterns[] =
    {
        QStringLiteral("\\bGROUP_DEF\\b"),
        QStringLiteral("\\bGROUP_NXT\\b"),
        QStringLiteral("\\bGROUP_OBJ\\b")
    };

    for (const QString &pattern : LDCadMetaGroupPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = LDCadMetaGroupFormat;
        highlightingRules.append(rule);
    }

    // LDCad Value Bracket Format
    LDCadBracketFormat.setForeground(br20);
    LDCadBracketFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("[\\[|=|\\]]"));
    rule.format = LDCadBracketFormat;
    highlightingRules.append(rule);

    // LDCad Meta Key Format
    LDCadMetaKeyFormat.setForeground(br11);
    LDCadMetaKeyFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("!?\\bLDCAD\\b"));
    rule.format = LDCadMetaKeyFormat;
    highlightingRules.append(rule);

    // LeoCAD Body Meta Format
    LeoCADBodyMetaFormat.setForeground(br20);
    LeoCADBodyMetaFormat.setFontWeight(QFont::Bold);

    const QString LeoCADBodyMetaPatterns[] =
    {
        // (?<=LEOCAD\\s) match KEYWORD preceded by 'LEOCAD '
        QStringLiteral("\\bLEOCAD MODEL\\b"),
        QStringLiteral("\\bLEOCAD MODEL NAME\\b"),
        QStringLiteral("\\bMODEL AUTHOR\\b"),
        QStringLiteral("\\bMODEL DESCRIPTION\\b"),
        QStringLiteral("\\bMODEL COMMENT\\b"),
        QStringLiteral("\\bMODEL BACKGROUND COLOR\\b"),
        QStringLiteral("\\bMODEL BACKGROUND GRADIENT\\b"),
        QStringLiteral("\\bMODEL BACKGROUND IMAGE\\b"),
        QStringLiteral("\\bPIECE\\b"),
        QStringLiteral("\\bPIECE STEP_HIDE\\b"),
        QStringLiteral("\\bPIECE HIDDEN\\b"),
        QStringLiteral("\\bPIECE POSITION_KEY\\b"),
        QStringLiteral("\\bPIECE ROTATION_KEY\\b"),
        QStringLiteral("\\bPIECE PIVOT\\b"),
        QStringLiteral("\\bCAMERA\\b"),
        QStringLiteral("\\bCAMERA HIDDEN\\b"),
        QStringLiteral("\\bCAMERA ORTHOGRAPHIC\\b"),
        QStringLiteral("\\bCAMERA FOV\\b"),
        QStringLiteral("\\bCAMERA ZNEAR\\b"),
        QStringLiteral("\\bCAMERA ZFAR\\b"),
        QStringLiteral("\\bCAMERA POSITION\\b"),
        QStringLiteral("\\bCAMERA TARGET_POSITION\\b"),
        QStringLiteral("\\bCAMERA UP_VECTOR\\b"),
        QStringLiteral("\\bCAMERA POSITION_KEY\\b"),
        QStringLiteral("\\bCAMERA TARGET_POSITION_KEY\\b"),
        QStringLiteral("\\bCAMERA UP_VECTOR_KEY\\b"),
        QStringLiteral("\\bCAMERA NAME\\b"),
        QStringLiteral("\\bLIGHT\\b"),
        QStringLiteral("\\bLIGHT ANGLE\\b"),
        QStringLiteral("\\bLIGHT ANGLE_KEY\\b"),
        QStringLiteral("\\bLIGHT COLOR_RGB\\b"),
        QStringLiteral("\\bLIGHT COLOR_RGB_KEY\\b"),
        QStringLiteral("\\bLIGHT CUTOFF_DISTANCE\\b"),
        QStringLiteral("\\bLIGHT CUTOFF_DISTANCE_KEY\\b"),
        QStringLiteral("\\bLIGHT HEIGHT\\b"),
        QStringLiteral("\\bLIGHT NAME\\b"),
        QStringLiteral("\\bLIGHT POSITION\\b"),
        QStringLiteral("\\bLIGHT POSITION_KEY\\b"),
        QStringLiteral("\\bLIGHT POWER\\b"),
        QStringLiteral("\\bLIGHT POWER_KEY\\b"),
        QStringLiteral("\\bLIGHT RADIUS\\b"),
        QStringLiteral("\\bLIGHT RADIUS_AND_SPOT_BLEND_KEY\\b"),
        QStringLiteral("\\bLIGHT RADIUS_KEY\\b"),
        QStringLiteral("\\bLIGHT SHAPE\\b"),
        QStringLiteral("\\bLIGHT SHAPE_KEY\\b"),
        QStringLiteral("\\bLIGHT SIZE\\b"),
        QStringLiteral("\\bLIGHT SIZE_KEY\\b"),
        QStringLiteral("\\bLIGHT SPECULAR\\b"),
        QStringLiteral("\\bLIGHT SPECULAR_KEY\\b"),
        QStringLiteral("\\bLIGHT SPOT_BLEND\\b"),
        QStringLiteral("\\bLIGHT SPOT_SIZE\\b"),
        QStringLiteral("\\bLIGHT SPOT_SIZE_KEY\\b"),
        QStringLiteral("\\bLIGHT STRENGTH\\b"),
        QStringLiteral("\\bLIGHT STRENGTH_KEY\\b"),
        QStringLiteral("\\bLIGHT TARGET_POSITION\\b"),
        QStringLiteral("\\bLIGHT TARGET_POSITION_KEY\\b"),
        QStringLiteral("\\bLIGHT TYPE\\b"),
        QStringLiteral("\\bLIGHT TYPE_KEY\\b"),
        QStringLiteral("\\bLIGHT WIDTH\\b"),
        QStringLiteral("\\bLEOCAD GROUP BEGIN\\b"),
        QStringLiteral("\\bLEOCAD GROUP END\\b"),
        QStringLiteral("\\bLEOCAD SYNTH\\b")              // match SYNTH preceded by 'LEOCAD '
    };

    for (const QString &pattern : LeoCADBodyMetaPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = LeoCADBodyMetaFormat;
        highlightingRules.append(rule);
    }

    // LeoCAD Format
    LeoCADMetaFormat.setForeground(br23);
    LeoCADMetaFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("!?\\bLEOCAD\\b"));
    rule.format = LeoCADMetaFormat;
    highlightingRules.append(rule);

    // LDraw Meta Command Line Format
    LDrawLineType0Format.setForeground(br31);
    LDrawLineType0Format.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression(QStringLiteral("^0"));
    rule.format = LDrawLineType0Format;
    highlightingRules.append(rule);

    // LDraw Line Type 2 Format
    LDrawLineType2Format.setForeground(br13);
    LDrawLineType2Format.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("^2"));
    rule.format = LDrawLineType2Format;
    highlightingRules.append(rule);

    // LDraw Line Type 3 Format
    LDrawLineType3Format.setForeground(br14);
    LDrawLineType3Format.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("^3"));
    rule.format = LDrawLineType3Format;
    highlightingRules.append(rule);

    // LDraw Lines 4 Format
    LDrawLineType4Format.setForeground(br15);
    LDrawLineType4Format.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("^4"));
    rule.format = LDrawLineType4Format;
    highlightingRules.append(rule);

    // LDraw Lines 5 Format
    LDrawLineType5Format.setForeground(br16);
    LDrawLineType5Format.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("^5"));
    rule.format = LDrawLineType5Format;
    highlightingRules.append(rule);

    // LDraw Texmap Line Format
    LDrawTexmapLineFormat.setForeground(br07);
    LDrawTexmapLineFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("\\s+!:\\s+"));
    rule.format = LDrawTexmapLineFormat;
    highlightingRules.append(rule);

    // LDraw Data Line Format
    LDrawDataLineFormat.setForeground(br30);
    LDrawDataLineFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression(QStringLiteral("(?<=^0\\s!:\\s)[^\n]*")); // match content preceded by '0 !: '
    rule.format = LDrawDataLineFormat;
    highlightingRules.append(rule);

    // LDraw Comment Format
    LDrawCommentFormat.setForeground(br01);
    LDrawCommentFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegularExpression(QStringLiteral("0\\s{1}//[^\n]*"));
    rule.format = LDrawCommentFormat;
    highlightingRules.append(rule);

    // LDrww Multi-line Command Format
    LDrawMultiLineCommentFormat.setForeground(br01);

    LDrawMultiLineCommentStartExpression = QRegularExpression(QStringLiteral("0\\s{1}\\/\\*[^\n]*"));
    LDrawMultiLineCommentEndExpression = QRegularExpression(QStringLiteral("0\\s{1}\\*\\/[^\n]*"));

    // LPub3D Hex Number Format
    LPubHexNumberFormat.setForeground(br18);
    LPubHexNumberFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("(0x|#)([\\dA-F]+)"), QRegularExpression::CaseInsensitiveOption);
    rule.format = LPubHexNumberFormat;
    highlightingRules.append(rule);

    /* Line format only - no rules */

    // LDraw Line Type 1 Format (0)
    LDrawLineType1Format.setForeground(br06);
    LDrawLineType1Format.setFontWeight(QFont::Bold);
    lineTypeFormats.append(LDrawLineType1Format);

    // LDraw Color Format (1)
    LDrawColorFormat.setForeground(br07);
    LDrawColorFormat.setFontWeight(QFont::Bold);
    lineTypeFormats.append(LDrawColorFormat);

    // LDraw Position Format (2)
    LDrawPositionFormat.setForeground(br08);
    LDrawPositionFormat.setFontWeight(QFont::Normal);
    lineTypeFormats.append(LDrawPositionFormat);

    // LDraw Transform1 Format (3)
    LDrawTransform1Format.setForeground(br09);
    LDrawTransform1Format.setFontWeight(QFont::Normal);
    lineTypeFormats.append(LDrawTransform1Format);

    // LDraw Transform2 Format (4)
    LDrawTransform2Format.setForeground(br10);
    LDrawTransform2Format.setFontWeight(QFont::Normal);
    lineTypeFormats.append(LDrawTransform2Format);

    // LDraw Transform3 Format (5)
    LDrawTransform3Format.setForeground(br11);
    LDrawTransform3Format.setFontWeight(QFont::Normal);
    lineTypeFormats.append(LDrawTransform3Format);

    // LDraw Part File Format (6)
    LDrawFileFormat.setForeground(br12);
    LDrawFileFormat.setFontWeight(QFont::Bold);
    lineTypeFormats.append(LDrawFileFormat);
}

void Highlighter::highlightBlock(const QString &text)
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

    // Geometry Line Types
    // 1 <colour> x y z a b c d e f g h i <file>
    // 2 <colour> x1 y1 z1 x2 y2 z2
    // 3 <colour> x1 y1 z1 x2 y2 z2 x3 y3 z3
    // 4 <colour> x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4
    // 5 <colour> x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4
    QRegularExpression typeRx("^([1-5])\\s+");
    QRegularExpression texmapRx("^0\\s+!?TEXMAP\\s+(?:START|NEXT)");
    QRegularExpressionMatch typeMatch = typeRx.match(text);
    int index = -1;
    bool texmap = false, type1_5 = false, type1 = false, type2_5 = false;
    if ((type1_5 = typeMatch.hasMatch()) || (texmap = text.contains(texmapRx)))
        index = 0;
    else if (text.startsWith("0 GHOST "))
        index = 8;
    else if (text.startsWith("0 MLCAD HIDE "))
        index = 13;
    else if (text.startsWith("0 !: ") && text.split(" ").size() > 3)
        index = 1;
    else
        return;

    if (type1_5) {
        type1 = typeMatch.captured(1) == "1";
        type2_5 = !type1;
    }

    QStringList tt = text.mid(index).trimmed().split(" ", SkipEmptyParts);
    if (tt.size()) {
        QString part;
        for (int t = 14; t < tt.size(); t++)
            part += (tt[t]+" ");
        QStringList tokens;
        if (tt.size() > 14 && !texmap) {
            tokens  << tt[ 0]                        // (0)  1 - part type
                    << tt[ 1]                        // (1)  2 - color
                    << tt[ 2]+" "+tt[ 3]+" "+tt[ 4]  // (2)  5 - position
                    << tt[ 5]+" "+tt[ 6]+" "+tt[ 7]  // (3)  8 - transform
                    << tt[ 8]+" "+tt[ 9]+" "+tt[10]  // (4) 11 - transform
                    << tt[11]+" "+tt[12]+" "+tt[13]  // (5) 14 - transform
                    << part.trimmed();               // (6) 15 - part
        } else {
            for (int i = 0; i < tt.size(); i++) {
                if (texmap) {
                    if (i == 3)  { // i = 1, insert type (0), texmap (1) START|NEXT (2), method (2)
                        tokens << tt[0];
                        tokens << tt[1];
                        tokens << tt[2]+" "+tt[3];
                    } else
                    if (i == 6)  { // append x1 y1 z1 point (3)
                        tokens << tt[4]+" "+tt[5]+" "+tt[6];
                    } else
                    if (i == 9)  { // append x2 y2 z2 point (4)
                        tokens << tt[7]+" "+tt[8]+" "+tt[9];
                    } else
                    if (i == 12) { // append x3 y3 z3 point (5)
                        tokens << tt[10]+" "+tt[11]+" "+tt[12];
                    } else
                    if (i == 13) { // append angle1/.png (6) - planar
                        tokens << tt[13];
                    } else
                    if (i == 14) { // append angle2/.png (7) - cylindrical
                        tokens << tt[14];
                    } else
                    if (i == 15) { // append .png (8) - spherical
                        tokens << tt[15];
                    }
                } else {
                    if (i == 1)  { // i = 1, insert type (0) and colour (1)
                        tokens << tt[0];
                        tokens << tt[1];
                    } else
                    if (i == 4)  { // append x y z position (2)
                        tokens << tt[2]+" "+tt[3]+" "+tt[4];
                    } else
                    if (i == 7)  { // append x1 y1 z1 transform (3)
                        tokens << tt[5]+" "+tt[6]+" "+tt[7];
                    } else
                    if (i == 10) { // append x2 y2 z2 transform (4)
                        tokens << tt[8]+" "+tt[9]+" "+tt[10];
                    } else
                    if (i == 13) { // append x3 y3 z3 transform (5)
                        tokens << tt[11]+" "+tt[12]+" "+tt[13];
                    } else
                    if (i == 14) { // append part (6)
                        tokens << tt[14];
                    }
                }
            }
        }
        for (int i = 0; i < tokens.size(); i++) {
            if (index >= 0 && index < text.length()) {
                int idx = (texmap && i > 5) ? 6 : i;              // set texmap tokens after (6) to last (type) format
                if (type1 || (type2_5 && i) || (texmap && i > 2)) // skip type format for type2_5 and position format for texmap
                    setFormat(index, tokens[i].length(), lineTypeFormats[idx]);
                index += tokens[i].length() + 1;                  // add 1 position for the space
            }
        }
    }
}
