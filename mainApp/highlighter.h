/**************************************************************************** 
**
** Copyright (C) 2005-2009 Trolltech ASA. All rights reserved.
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QTextCharFormat>
#include <QSyntaxHighlighter>
#include <QHash>

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
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;
    QList<QTextCharFormat> lineType1Formats;

    QTextCharFormat LDrawCommentFormat;    // b01 - Comments

    QTextCharFormat LPubLocalMetaFormat;   // b04 - LPub3D Local
    QTextCharFormat LPubGlobalMetaFormat;  // b05 - LPub3D Global
    QTextCharFormat LPubFalseMetaFormat;   // b22 - LPub3D False
    QTextCharFormat LPubTrueMetaFormat;    // b23 - LPub3D True
    QTextCharFormat LPubMetaFormat;        // b24 - LPub3D
    QTextCharFormat LPubBodyMetaFormat;    // b25 - LPub3D Body
    QTextCharFormat LPubQuotedTextFormat;  // b27 - LPub3D Quoted Text
    QTextCharFormat LPubFontCommaFormat;   // b27 - LPub3D Quoted Text
    QTextCharFormat LPubFontNumberFormat;  // b14 - LPub3D Number
    QTextCharFormat LPubNumberFormat;      // b14 - LPub3D Number
    QTextCharFormat LPubHexNumberFormat;   // b15 - LPub3D Hex Number
    QTextCharFormat LPubPageSizeFormat;    // b16 - LPub3D Page Size
    QTextCharFormat LPubSubPartFormat;     // b12 - LPub3D Part File
    QTextCharFormat LPubSubColorFormat;    // b07 - LDraw Part Colour Code
    QTextCharFormat LPubCustomColorFormat; // b07 - LDraw Part Colour Code

    QTextCharFormat LDrawHeaderValueFormat;// b26 - LDraw Header Value
    QTextCharFormat LDrawHeaderFormat;     // b02 - LDraw Header
    QTextCharFormat LDrawBodyFormat;       // b03 - LDraw Body
    QTextCharFormat LDrawColourMetaFormat; // b05 - LPub3D Global
    QTextCharFormat LDrawColourDescFormat; // b26 - LDraw Header Value
    QTextCharFormat LDrawLineType0Format;  // b28 - LDraw Line Type 0 First Character

    // position 0
    QTextCharFormat LDrawLineType1Format;  // b06 - LDraw Line Type 1
    // position 1
    QTextCharFormat LDrawColorFormat;      // b07 - LDraw Part Colour Code
    // positions 2-4
    QTextCharFormat LDrawPositionFormat;   // b08 - LDraw Part Position [x y z]
    // transform1 5-7
    QTextCharFormat LDrawTransform1Format; // b09 - LDraw Part Transform1 [a b c]
    // transform2 8-10
    QTextCharFormat LDrawTransform2Format; // b10 - LDraw Part Transform2 [d e f]
    // transform3 11-13
    QTextCharFormat LDrawTransform3Format; // b11 - LDraw Part Transform3 [g h i]
    // ldraw file 14
    QTextCharFormat LDrawFileFormat;       // b12 - LDraw Part File

    QTextCharFormat LDrawLineType2_5Format;// b13 - LDraw Line Types 2-5
    QTextCharFormat LDrawLineType3Format;  // bXX - LDraw Triangle Line       [not used]
    QTextCharFormat LDrawLineType4Format;  // bXX - LDraw Quadrilateral Line  [not used]
    QTextCharFormat LDrawLineType5Format;  // bXX - LDraw Optional Line Line  [not used]

    QTextCharFormat LeoCADMetaFormat;      // b17 - LeoCAD
    QTextCharFormat LSynthMetaFormat;      // b18 - LSynth

    QTextCharFormat LDCadMetaKeyFormat;    // b11 - LDCad Key
    QTextCharFormat LDCadMetaValueFormat;  // b18 - LDCad Value
    QTextCharFormat LDCadBodyMetaFormat;   // b19 - LDCad
    QTextCharFormat LDCadBracketFormat;    // b17 - LDCad Value Bracket
    QTextCharFormat LDCadMetaGrpDefFormat; // b29 - LDCad Group Define

    QTextCharFormat MLCadMetaFormat;       // b20 - MLCad
    QTextCharFormat MLCadBodyMetaFormat;   // b21 - MLCad Body

};

#endif

