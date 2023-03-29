 
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

#include "color.h"
#include "lc_global.h"
#include "lc_colors.h"
#include "lpub_object.h"

QStringList        LDrawColor::userdefinedcolors;
QHash<QString,int> LDrawColor::color2alpha;
QMultiHash<QString,int> LDrawColor::value2code;
QHash<QString, QColor>  LDrawColor::name2QColor;
QHash<QString, QString> LDrawColor::color2value;
QHash<QString, QString> LDrawColor::color2edge;
QHash<QString, QString> LDrawColor::color2name;
QHash<QString, QString> LDrawColor::ldname2ldcolor;

/*
 * This function extracts colours loaded by the Visual Editor  to
 * extract the color codes, color names, and color values and puts
 * them in the xlate (name to color translate) hash table.
 */
void LDrawColor::LDrawColorInit()
{
  //qDebug() << qUtf8Printable(QString("LOAD LDraw Colors in LDrawColor::LDrawColorInit"));
  value2code.clear();
  color2alpha.clear();
  color2value.clear();
  color2edge.clear();
  color2name.clear();
  name2QColor.clear();
  ldname2ldcolor.clear();
  userdefinedcolors.clear();

  for (lcColor& gColor : gColorList)
  {
    lcColor* nativeColor = &gColor;
    if (nativeColor->Code == quint32(LC_COLOR_NOCOLOR))
      continue;
    QColor color(nativeColor->CValue);
    color.setAlpha(nativeColor->Alpha);
    QString const edge = QColor(nativeColor->EValue).name(QColor::HexRgb);
    QString const name = nativeColor->SafeName;
    QString const code = QString::number(nativeColor->Code);
    AddColor(color, name, code, edge);
  }
}

/*
 * This function adds a QColor object to LDrawColor
 */
void LDrawColor::AddColor(const QColor& color, const QString& name, const QString& code, const QString& edge, bool native)
{
  ldname2ldcolor.insert(name.toLower(),code);                           // color  code   from name (lower)
  value2code. insert(color.name(QColor::HexRgb).toLower(),code.toInt());// color  code   from value (lower)
  color2value.insert(code,color.name(QColor::HexRgb).toUpper());        // color  value  from code
  color2edge. insert(code,edge.toUpper());                              // color  edge   from code
  color2alpha.insert(code,color.alpha());                               // color  alpha  from code
  color2name. insert(code,name);                                        // color  name   from code  (safe name e.g. Dark_Nougat)
  color2name. insert(color.name(QColor::HexRgb).toLower(),name);        // color  name   from value (lower)
  name2QColor.insert(code,color);                                       // QColor object from code
  name2QColor.insert(name.toLower(),color);                             // QColor object from name (lower) - e.g. dark_nougat
  QString const message = QObject::tr("%1. ADD 0 !COLOUR %2 CODE %3 VALUE %4 EDGE %5 ALPHA %6 QCOLOR_NAME %7")
                                      .arg(ldname2ldcolor.size(), 3, 10, QChar(' ')).arg(name).arg(code).arg(color.name().toUpper())
                                      .arg(edge.toUpper()).arg(color.alpha()).arg(color.name());
  if (!native) {
    userdefinedcolors.append(name.toLower());
    emit lpub->messageSig(LOG_INFO, message);
#ifdef QT_DEBUG_MODE
  } else {
    ;
    //qDebug() << qUtf8Printable(message);
#endif
  }

}

/*
 * This function removes user defined QColor objects and attributes
 * from LDrawColor
 */
void LDrawColor::removeUserDefinedColors()
{
  for (const QString &name : userdefinedcolors) {

    QColor color(name2QColor[name]);
    QString const hexname = color.name(QColor::HexRgb).toLower();
    QString const code = QString::number(value2code.value(hexname));
    QString const edge = color2edge[code];
    QString const colorname = color2name[code];
    bool ok[10];

    ok[0] = false;
    QHash<QString, int>::iterator a = value2code.find(hexname);
    while (a != value2code.end()) {
      if (a.key() == hexname) {
        a = value2code.erase(a);
        ok[0] = true;
      } else
        a++;
    }
    ok[1] = false;
    QHash<QString, int>::iterator b = color2alpha.find(code);
    while (b != color2alpha.end()) {
      if (b.key() == code) {
        b = color2alpha.erase(b);
        ok[1] = true;
      } else
        b++;
    }
    ok[2] = false;
    QHash<QString, QString>::iterator c = color2value.find(code);
    while (c != color2value.end()) {
      if (c.key() == code) {
        c = color2value.erase(c);
        ok[2] = true;
      } else
        c++;
    }
    ok[3] = false;
    QHash<QString, QString>::iterator d = color2edge.find(code);
    while (d != color2edge.end()) {
      if (d.key() == code) {
        d = color2edge.erase(d);
        ok[3] = true;
      } else
        d++;
    }
    ok[4] = false;
    QHash<QString, QString>::iterator e = color2name.find(code);
    while (e != color2name.end()) {
      if (e.key() == code) {
        e = color2name.erase(e);
        ok[4] = true;
      } else
        e++;
    }
    ok[5] = false;
    QHash<QString, QString>::iterator f = color2name.find(hexname);
    while (f != color2name.end()) {
      if (f.key() == hexname) {
        f = color2name.erase(f);
        ok[5] = true;
      } else
        f++;
    }
    ok[6] = false;
    QHash<QString, QColor>::iterator g = name2QColor.find(code);
    while (g != name2QColor.end()) {
      if (g.key() == code) {
        g = name2QColor.erase(g);
        ok[6] = true;
      } else
        g++;
    }
    ok[7] = false;
    QHash<QString, QColor>::iterator h = name2QColor.find(colorname);
    while (h != name2QColor.end()) {
      if (h.key() == colorname) {
        h = name2QColor.erase(h);
        ok[7] = true;
      } else
        h++;
    }
    ok[8] = false;
    QHash<QString, QString>::iterator i = ldname2ldcolor.find(colorname);
    while (i != ldname2ldcolor.end()) {
      if (i.key() == colorname) {
        i = ldname2ldcolor.erase(i);
        ok[8] = true;
      } else
        i++;
    }
    ok[9] = userdefinedcolors.removeAll(name) > 0;
    if (ok[9] && ok[8] && ok[7] && ok[6] && ok[5] && ok[4] && ok[3] && ok[2] && ok[1] && ok[0]) {
      QString const message = QObject::tr("REMOVED 0 !COLOUR %1 CODE %2 VALUE %3 EDGE %4 ALPHA %5 QCOLOR_NAME %6")
                                          .arg(name.toUpper()).arg(code).arg(color.name().toUpper()).arg(edge.toUpper()).arg(color.alpha()).arg(color.name());
      emit lpub->messageSig(LOG_INFO, message);
    }
  }
}

/*
 * This function provides the translate from LDraw color name, LDraw code or
 * LDraw color value (hex RGB) to LDraw QColor object.
 */
QColor LDrawColor::color(const QString& argument)
{
  //qDebug() << qUtf8Printable(QString("RECEIVED Color ARGUMENT [%1] for LDrawColor::color").arg(argument));
  QString key(argument.toLower());
  QRegExp hexRx("\\s*(0x|#)([\\dA-F]+)\\s*$",Qt::CaseInsensitive);
  bool isHexRGB = key.contains(hexRx);
  bool isCode = false;
  int code = -1;
  QColor color;

  if (name2QColor.contains(key)) {
    color = name2QColor[key];
    code = key.toInt(&isCode);
    Q_UNUSED(code)
  } else if (isHexRGB) {
    color = QColor(QString("#%1").arg(hexRx.cap(2)));
  }

  if (color.isValid()) {
    //qDebug() << qUtf8Printable(QString("RETURNED Color OBJECT   [%1] ALPHA [%2] for %3 [%4]").arg(color.name(QColor::HexRgb).toUpper()).arg(color.alpha()).arg(isHexRGB ? QString("HEX(%1) VALUE").arg(hexRx.cap(1)) : isCode ? "CODE" : "NAME").arg(argument));
    return color;
  }

  emit lpub->messageSig(LOG_WARNING, QObject::tr("Could not resolve Color OBJECT for %1 [%2] ").arg(isHexRGB ? "VALUE" : isCode ? "CODE" : "NAME").arg(argument));

  color = QColor(Qt::black);

  return color;
}

/*
 * This function provides the translate from QColor name (hex RGB) or LDraw code
 * to LDraw color name and returns the LDraw color name value if it exist.
 * If there is no LDraw color name but the QColor is valid, the QColor name (hex RGB)
 * is returned.
 * If there is no translation, an empty string is returned.
 */
QString LDrawColor::name(const QString &codeorvalue)
{
  QString key(codeorvalue.toLower());
  QRegExp hexRx("\\s*(0x|#)([\\dA-F]+)\\s*$",Qt::CaseInsensitive);
  bool isHexRGB = key.contains(hexRx);
  //qDebug() << qUtf8Printable(QString("RECEIVED Color %1 [%2] for LDrawColor::name").arg(isHexRGB ? "VALUE" : "CODE").arg(codeorvalue));

  if (isHexRGB) {                                // RECEIVED VALUE
    QString name;
    key = hexRx.cap(2).size() == 8 ? hexRx.cap(2).left(6).toLower() : hexRx.cap(2).toLower();
    if (value2code.contains(key)) {
      QList<int> codes = value2code.values(key);
      if (codes.size() > 1) {
        QStringList names;
        std::sort(codes.begin(), codes.end());
        for(int i = 0; i < codes.size(); i++)
          names << color2name[QString::number(codes.at(i))];
        name = names.first();
        emit lpub->messageSig(LOG_WARNING, QObject::tr("RETRIEVED Mulitple Color NAMES [%1] for VALUE [%2] - RETURNED [%3]").arg(names.join(",")).arg(key).arg(name));
        return name;
      } else {
        name = color2name[QString::number(value2code.value(key))];
        //qDebug() << qUtf8Printable(QString("RETURNED Color NAME [%1] for VALUE [%2]").arg(name).arg(key));
        return name;
      }
    } else {
      QColor color(key);
      if (color.isValid()) {
        name = color.name(QColor::HexRgb).toUpper();
        //qDebug() << qUtf8Printable(QString("RETURNED Color NAME [%1] for valid HEX VALUE [%2]").arg(name).arg(key));
        return name;
      }
    }
  } else if (color2name.contains(codeorvalue)) { // RECEIVED CODE
    //qDebug() << qUtf8Printable(QString("RETURNED Color NAME [%1] for CODE [%2]").arg(color2name[codeorvalue]).arg(codeorvalue));
    return color2name[codeorvalue];
  }

  emit lpub->messageSig(LOG_WARNING, QObject::tr("Could not resolve NAME for %1 [%3]").arg(isHexRGB ? "VALUE" : "CODE").arg(codeorvalue));
  return QString();
}

/* This function provides all the color names */
QStringList LDrawColor::names()
{
  QStringList colorNames;
  QRegExp hexRx("\\s*(0x|#)([\\dA-F]+)\\s*$",Qt::CaseInsensitive);
  for (QString &key : color2name.keys()) {
    if (! key.contains(hexRx))
      colorNames << color2name[key];
  }

  colorNames.sort();

  return colorNames;
}

/*
 * This function provides the translate from LDraw color code to
 * alpha value and returns the color alpha value if it exist.
 * If there is no color alpha value, 255 (fully opaque) is returned.
 */
int LDrawColor::alpha(const QString& code)
{
  //qDebug() << qUtf8Printable(QString("RECEIVED Color CODE [%1] for LDrawColor::alpha").arg(code));
  if (color2alpha.contains(code))
    return color2alpha[code];

  emit lpub->messageSig(LOG_WARNING, QObject::tr("Could not resolve ALPHA for CODE [%1] - RETURNED [255]").arg(code));
  return 255;
}

/*
 * This function provides the translate from LDraw color code to
 * color hex value and returns the color value if it exist.
 * If there is no color value, #FFFF80 (material main_colour) - is returned.
 */
QString LDrawColor::value(const QString& code)
{
  //qDebug() << qUtf8Printable(QString("RECEIVED Color CODE [%1] for LDrawColor::value").arg(code));
  if (color2value.contains(code)) {
    //qDebug() << qUtf8Printable(QString("RETURNED Color VALUE [%1] for CODE [%2]").arg(color2value[code]).arg(code));
    return color2value[code];
  }

  emit lpub->messageSig(LOG_WARNING, QObject::tr("Could not resolve VALUE for CODE [%1] - RETURNED [#FFFF80]").arg(code));
  return QLatin1String("#FFFF80");
}

/*
 * This function provides the translate from LDraw name or
 * LDraw color value to LDraw color code.
 * If there is no translation, -1 is returned.
 */
QString LDrawColor::code(const QString& name)
{
  QString key(name.toLower());
  QRegExp hexRx("\\s*(0x|#)([\\dA-F]+)\\s*$",Qt::CaseInsensitive);
  bool isHexRGB = key.contains(hexRx);
  //qDebug() << qUtf8Printable(QString("RECEIVED Color %1 [%2] for LDrawColor::code").arg(isHexRGB ? "VALUE" : "NAME").arg(name));
  bool isHexARGB = hexRx.cap(2).size() == 8;
  if (isHexARGB)
    key = QString("#%1").arg(hexRx.cap(2).right(6).toLower());

  if (ldname2ldcolor.contains(key)) {
    //qDebug() << qUtf8Printable(QString("RETURNED Color CODE [%1] for NAME [%2]").arg(ldname2ldcolor[key]).arg(name));
    return ldname2ldcolor[key];
  } else if (value2code.contains(key)) {
    QString code;
    QList<int> codes = value2code.values(key);
    if (codes.size() > 1) {
      QStringList list;
      std::sort(codes.begin(), codes.end());
      for(int i = 0; i < codes.size(); i++)
        list << QString::number(codes.at(i));
      code = list.first();
      emit lpub->messageSig(LOG_WARNING, QObject::tr("RETRIEVED Mulitple Color CODES [%1] for VALUE [%2] - RETURNED [%3 %4]").arg(list.join(",")).arg(name).arg(code).arg(color2name[code]));
      return code;
    } else {
      code = QString::number(value2code.value(key));
      //qDebug() << qUtf8Printable(QString("RETURNED Color CODE [%1] for VALUE [%2]").arg(code).arg(name));
      return code;
    }
  }

  if (isHexRGB) {
    QColor color = QColor(QString("#%1").arg(hexRx.cap(2)));
    if (color.isValid()) {
      //qDebug() << qUtf8Printable(QString("CREATED Color OBJECT [%1] ALPHA [%2] for valid HEX(%3) VALUE [%4]").arg(color.name(QColor::HexRgb).toUpper()).arg(color.alpha()).arg(hexRx.cap(1)).arg(name));
      QString const code = QString::number(*std::max_element(value2code.begin(), value2code.end()) + 1);
      QString const edge = QLatin1String("#333333");
      AddColor(color, name, code, edge, false /*native*/);
      //qDebug() << qUtf8Printable(QString("RETURNED Color CODE [%1] for valid HEX(%2) VALUE [%3]").arg(code).arg(hexRx.cap(1)).arg(name));
      return code;
    }
  }

  emit lpub->messageSig(LOG_WARNING, QObject::tr("Could not resolve CODE for %1 [%2] - RETURNED [-1]").arg(isHexRGB ? "VALUE" : "NAME").arg(name));
  return QLatin1String("-1");
}

/*
 * This function provides the translate from LDraw color code to
 * edge color hex value and returns the value if it exist.
 * If there is no color edge value, #333333 (default edge color) is returned.
 */
QString LDrawColor::edge(const QString& code)
{
  //qDebug() << qUtf8Printable(QString("RECEIVED Color CODE [%1] for LDrawColor::edge").arg(code));
  if (color2edge.contains(code))
    return color2edge[code];

  emit lpub->messageSig(LOG_WARNING, QObject::tr("Could not resolve EDGE for CODE [%1] - RETURNED [#333333]").arg(code));
  return QLatin1String("#333333");
}

/*
 * This function performs a lookup of the provided LDraw color code
 * and returns true if found or false if not found
 */
bool LDrawColor::exist(const QString &code)
{
  //qDebug() << qUtf8Printable(QString("RECEIVED Color CODE [%1] for LDrawColor::exist").arg(code));
  if (name2QColor.contains(code))
    return true;

  emit lpub->messageSig(LOG_WARNING, QObject::tr("Could not resolve CODE [%1]").arg(code));
  return false;
}
