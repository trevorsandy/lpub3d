 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2021 Trevor SANDY. All rights reserved.
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

#include "color.h"
#include "lc_global.h"
#include "lc_colors.h"
#include "QsLog.h"

QHash<QString, int> LDrawColor::color2alpha;
QHash<QString, int> LDrawColor::value2code;
QHash<QString, QColor>  LDrawColor::name2QColor;
QHash<QString, QString> LDrawColor::color2value;
QHash<QString, QString> LDrawColor::color2edge;
QHash<QString, QString> LDrawColor::color2name;
QHash<QString, QString> LDrawColor::ldname2ldcolor;

/*
 * This function extracts colours loaded by the 3DViewer  to
 * extract the color codes, color names, and color values and puts
 * them in the xlate (name to color translate) hash table.
 */
void LDrawColor::LDrawColorInit()
{
  name2QColor.clear();
  color2name.clear();

  for (lcColor& gColor : gColorList)
  {
    lcColor* nativeColor = &gColor;

    QColor color(nativeColor->CValue);
    QColor edge(nativeColor->EValue);
    color.setAlpha(nativeColor->Alpha);
    QString name   = nativeColor->SafeName;
    QString code   = QString::number(nativeColor->Code);

//    logDebug() << QString("0 !COLOUR %1 CODE %2 VALUE %3 EDGE %4 ALPHA %5 QCOLOR_NAME %6")
//                  .arg(name).arg(code).arg(color.name().toUpper()).arg(edge.name().toUpper())
//                  .arg(nativeColor->Alpha).arg(color.name());

    value2code.insert(color.name(),code.toInt()); // color code  from value
    color2alpha.insert(code,nativeColor->Alpha);  // color alpha from code
    color2value.insert(code,color.name());        // color value from code
    color2edge.insert(code,edge.name());          // color edge from code
    name2QColor.insert(code,color);               // QColor from code
    name2QColor.insert(name.toLower(),color);     // QColor from name (lower) - e.g. dark_nougat
    ldname2ldcolor.insert(name.toLower(),code);   // color code from name
    color2name.insert(code,name);                 // color name from code (normal) - e.g. Dark_Nougat
    color2name.insert(color.name(),name);         // color name from value (normal)
  }
}

/*
 * This function provides the translate from LDraw color names or codes
 * to QColor.
 */
QColor LDrawColor::color(const QString& nickname)
{
  bool isHex = false;
  QRegExp hexRx("\\s*(0x|#)([\\da-fA-F]+)\\s*$",Qt::CaseInsensitive);
  QString name(nickname.toLower());
  if (nickname.contains(name)){
    name = nickname.toUpper();
    isHex= nickname.contains(hexRx);
  }
//  logDebug() << QString("RECEIVED Color NICKNAME (formatted)  [%1] for QCOLOR").arg(name);
  QColor color(Qt::black);

  if (name2QColor.contains(name)) {
    color = name2QColor[name];
  } else if (isHex) {
    if (hexRx.cap(1) == "0x") {
      QString prefix("0xf"+hexRx.cap(2));
      bool ok;
      QRgb rgb = QRgb(prefix.toLong(&ok,16));
      if (ok)
        color.setRgb(rgb);
    } else {
      color.setNamedColor(name);
    }
    color.setAlpha(255);
  }
//  logNotice() << QString("RETURNED [%1], ALPHA %2 from HEX for QCOLOR").arg(color.name().arg(color.alpha()));
  return color;
}

/*
 * This function provides the translate from LDraw color code to
 * alpha value and returns the color alpha value if it exist.
 * If there is no color alpha value, 255 (fully opaque) is returned.
 */
int LDrawColor::alpha(const QString& code)
{
  if (color2alpha.contains(code))
    return color2alpha[code];
  return 255;
}

/*
 * This function provides the translate from LDraw color code to
 * color hex value and returns the color value if it exist.
 * If there is no color value, #FFFF80 (material main_colour) - is returned.
 */
QString LDrawColor::value(const QString& code)
{
//  logTrace() << QString("RECEIVED Color CODE [%1] for VALUE").arg(code);
  if (color2value.contains(code)) {
//      logTrace() << QString("RETURNED Color VALUE [%1] for CODE %2").arg(QString(color2value[code])).arg(code);
    return color2value[code];
  }
  return "#FFFF80";
}

/*
 * This function provides the translate from LDraw color hex value to
 * color code and returns the code if it exist.
 * If there is no color code, 0 (black) is returned.
 */
int LDrawColor::code(const QString &value){
//    logTrace() << QString("RECEIVED Color VALUE [%1] for CODE").arg(value);
    if (value2code.contains(value)) {
//      logTrace() << QString("RETURNED Color CODE %1 for Color VALUE [%2]").arg(QString(value2code[value])).arg(value);
      return value2code[value];
    }
    return 0;
}

/*
 * This function provides the translate from LDraw color code to
 * edge color hex value and returns the value if it exist.
 * If there is no color edge value, #333333 (default edge color) is returned.
 */
QString LDrawColor::edge(const QString& code)
{
  if (color2edge.contains(code))
    return color2edge[code];
  return "#333333";
}

/*
 * This function provides the translate from QColor hex string or LDraw code to
 * LDraw color name and returns the LDraw color name value if it exist.
 * If there is no translation, an empty string is returned.
 */
QString LDrawColor::name(const QString &code)
{
//  logTrace() << QString("RECEIVED Color CODE  [%1] for NAME").arg(code);
  if (color2name.contains(code))
    return color2name[code];
  return QString();
}

/* This function provides all the color names */
QStringList LDrawColor::names()
{
    QString key;
    QStringList colorNames;
    QRegExp hexRx("\\s*(0x|#)([\\da-fA-F]+)\\s*$");
    Q_FOREACH (key,color2name.keys()) {
      if (! key.contains(hexRx))
        colorNames << color2name[key];
    }
    colorNames.sort();
    return colorNames;
}

/* This function provides the translate from LDraw name to LDraw color code
 * If there is no translation, -1 is returned.
 */
QString LDrawColor::ldColorCode(const QString& name)
{
    QString key(name.toLower());
    if (ldname2ldcolor.contains(key))
      return ldname2ldcolor[key];
    return "-1";
}
/*
 * This function performs a lookup of the provided LDraw color code
 * and returns true if found or false if not found
 */
bool LDrawColor::colorExist(const QString &code)
{
  if (name2QColor.contains(code))
    return true;
  return false;
}
