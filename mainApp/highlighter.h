/**************************************************************************** 
**
** Copyright (C) 2005-2009 Trolltech ASA. All rights reserved.
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
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
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QTextCharFormat>
#include <QSyntaxHighlighter>
#include <QRegularExpression>

class QTextDocument;

class Highlighter : public QSyntaxHighlighter
{

    Q_OBJECT

public:
    Highlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text);

private:

    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QRegularExpression LDrawMultiLineCommentStartExpression;
    QRegularExpression LDrawMultiLineCommentEndExpression;

    QVector<HighlightingRule> highlightingRules;
    QVector<QTextCharFormat> lineTypeFormats;

    QTextCharFormat LDrawCommentFormat;          // br01 - Comments
    QTextCharFormat LDrawMultiLineCommentFormat; // br01 - Comments

    QTextCharFormat LPubInvalidFormat;           // br25 - LPub3D False
    QTextCharFormat LPubLocalMetaFormat;         // br04 - LPub3D Local
    QTextCharFormat LPubGlobalMetaFormat;        // br05 - LPub3D Global
    QTextCharFormat LPubFalseMetaFormat;         // br25 - LPub3D False
    QTextCharFormat LPubTrueMetaFormat;          // br28 - LPub3D True
    QTextCharFormat LPubMetaFormat;              // br27 - LPub3D
    QTextCharFormat LPubBodyMetaFormat;          // br28 - LPub3D Body
    QTextCharFormat LPubQuotedTextFormat;        // br30 - LPub3D Quoted Text
    QTextCharFormat LPubFontCommaFormat;         // br30 - LPub3D Quoted Text
    QTextCharFormat LPubFontNumberFormat;        // br17 - LPub3D Number
    QTextCharFormat LPubNumberFormat;            // br17 - LPub3D Number
    QTextCharFormat LPubHexNumberFormat;         // br18 - LPub3D Hex Number
    QTextCharFormat LPubPageSizeFormat;          // br19 - LPub3D Page Size
    QTextCharFormat LPubSubPartFormat;           // br12 - LPub3D Part File
    QTextCharFormat LPubSubColorFormat;          // br07 - LDraw Part Colour Code
    QTextCharFormat LPubCustomColorFormat;       // br27 - LeoCAD
    QTextCharFormat LPubCustomColorCodeFormat;   // br07 - LDraw Part Colour Code

    QTextCharFormat LDrawHeaderValueFormat;      // br29 - LDraw Header Value
    QTextCharFormat LDrawHeaderFormat;           // br02 - LDraw Header
    QTextCharFormat LDrawBodyFormat;             // br03 - LDraw Body
    QTextCharFormat LDrawColourMetaFormat;       // br05 - LPub3D Global
    QTextCharFormat LDrawColourDescFormat;       // br29 - LDraw Header Value
    QTextCharFormat LDrawLineType0Format;        // br31 - LDraw Line Type 0 First Character

    // position 0
    QTextCharFormat LDrawLineType1Format;        // br06 - LDraw Line Type 1
    // position 1
    QTextCharFormat LDrawColorFormat;            // br07 - LDraw Part Colour Code
    // positions 2-4
    QTextCharFormat LDrawPositionFormat;         // br08 - LDraw Part Position [x y z]
    // transform1 5-7
    QTextCharFormat LDrawTransform1Format;       // br09 - LDraw Part Transform1 [a b c]
    // transform2 8-10
    QTextCharFormat LDrawTransform2Format;       // br10 - LDraw Part Transform2 [d e f]
    // transform3 11-13
    QTextCharFormat LDrawTransform3Format;       // br11 - LDraw Part Transform3 [g h i]
    // ldraw file 14
    QTextCharFormat LDrawFileFormat;             // br12 - LDraw Part File

    QTextCharFormat LDrawLineType2Format;        // br13 - LDraw Line Types 2-5
    QTextCharFormat LDrawLineType3Format;        // br14 - LDraw Triangle Line
    QTextCharFormat LDrawLineType4Format;        // br15 - LDraw Quadrilateral Line
    QTextCharFormat LDrawLineType5Format;        // br16 - LDraw Optional Line Line

    QTextCharFormat LDrawTexmapLineFormat;       // br16 - LDraw Optional Line Line

    QTextCharFormat LDrawDataLineFormat;         // br16 - LDraw Optional Line Line

    QTextCharFormat LeoCADMetaFormat;            // br23 - LeoCAD
    QTextCharFormat LeoCADBodyMetaFormat;        // br20 - LeoCAD

    QTextCharFormat LSynthMetaFormat;            // br21 - LSynth

    QTextCharFormat LDCadMetaKeyFormat;          // br11 - LDCad Key
    QTextCharFormat LDCadMetaValueFormat;        // br21 - LDCad Value
    QTextCharFormat LDCadBodyMetaFormat;         // br22 - LDCad
    QTextCharFormat LDCadBracketFormat;          // br20 - LDCad Value Bracket
    QTextCharFormat LDCadMetaGroupFormat;        // br32 - LDCad Group Define

    QTextCharFormat MLCadMetaFormat;             // br23 - MLCad
    QTextCharFormat MLCadBodyMetaFormat;         // br24 - MLCad Body

};

#endif

