 
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

#ifndef COLOR_H
#define COLOR_H

#include <QHash>
#include <QString>
#include <QColor>

/*
 * This class encapsulates LDraw color codes, color names and Qt's Qcolor
 * into a workable translation system.
 *
 * So far, the translate from color code, color name and hexidecimal expressions
 * of color works.  We want to have a reverse translate mechanism that
 * translates QColor values back into LDraw names.
 *
 * We also need a color dialog that is tailored to LDraw names first, and then
 * arbitrary colors second.
 */

class LDrawColor {
  private:
    static int index;
    static QStringList         nonnative;
    static QHash<QString, int> color2alpha;
    static QMultiHash<QString, int> value2code;
    static QHash<QString, QColor>  name2QColor;
    static QHash<QString, QString> color2value;
    static QHash<QString, QString> color2edge;
    static QHash<QString, QString> color2name;
    static QHash<QString, QString> ldname2ldcolor;
    /*
     * This function adds a QColor object to LDrawColor
     */
    static void AddColor(const QColor& color, const QString& name, const QString& code, const QString& edge, bool native = true);
  public:

    /*
     * This constructor does nothing.
     */
    LDrawColor()
    {}

    /*
     * This function extracts colours loaded by the Visual Editor  to
     * extract the color codes, color names, and color values and puts
     * them in the xlate (name to color translate) hash table.
     */
    static void LDrawColorInit();

    /*
     * This function removes user defined QColor objects and attributes
     * from LDrawColor
     */
    static void removeUserDefinedColors();

    /*
     * This function provides the translate from LDraw color name, LDraw code or
     * LDraw color value (hex RGB) to LDraw QColor object.
     */
    static QColor color(const QString& argument);

    /*
     * This function provides the translate from QColor name (hex RGB) or LDraw code
     * to LDraw color name and returns the LDraw color name value if it exist.
     * If there is no LDraw color name but the QColor is valid, the QColor name (hex RGB)
     * is returned.
     * If there is no translation, an empty string is returned.
     */
    static QString name(const QString& codeorvalue);

    /*
     * This function provides all the color names.
     */
    static QStringList names();

    /*
     * This function provides the translate from LDraw color code to
     * alpha value and returns the color alpha value if it exist.
     * If there is no color alpha value, 255 (fully opaque) is returned.
     */
    static int alpha(const QString& code);

    /*
     * This function provides the translate from LDraw color code to
     * color value and returns the color value if it exist.
     * If there is no color value, #FFFF80 (material main_colour) - is returned.
     */
    static QString value(const QString& code);

    /*
     * This function provides the translate from LDraw name or
     * LDraw color value to LDraw color code.
     * If there is no translation, -1 is returned.
     */
    static QString code(const QString& value);

    /*
     * This function provides the translate from LDraw color code to
     * color edge value and returns the color edge value if it exist.
     * If there is no color edge value, 333333 (default edge color) is returned.
     */
    static QString edge(const QString& code);

    /*
     * This function performs a lookup of the provided LDraw color code
     * and returns true if found or false if not found
     */
    static bool exist(const QString& code);
};

#endif
