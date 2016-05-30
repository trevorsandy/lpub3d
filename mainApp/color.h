 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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
    static QHash<QString, QColor>  name2color;
    static QHash<QString, QString> color2name;
    static QHash<QString, QString> ldname2ldcolor;
  public:

    /*
     * This constructor reads in the LDraw ldconfig.ldr file and extracts
     * the color codes, color names, and color values and puts them in
     * the xlate (name to color translate) hash table.
     */
    LDrawColor ();
    /*
     * This function provides the translate from LDraw color names and codes
     * to QColor.
     */
    static QColor color(QString nickname);
    /*
     * This function provides the translate from QColor back to LDraw color
     * names.  If there is no translation the hexadecimal value for the
     * color is returned as a string.
     */
    static QString name(QString code);
    /*
     * This function provides all the color names.
     */
    static QStringList names();
    /*
     * This function provides LDraw color codes.
     */
    static QString ldColorCode(QString name);
};

#endif
