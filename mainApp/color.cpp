 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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

#include <iostream>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include "lpub_preferences.h"

#include "lc_global.h"
#include "lc_colors.h"

#include "QsLog.h"

QHash<QString, int> LDrawColor::color2alpha;
QHash<QString, QColor>  LDrawColor::name2color;
QHash<QString, QString> LDrawColor::color2value;
QHash<QString, QString> LDrawColor::color2edge;
QHash<QString, QString> LDrawColor::color2name;
QHash<QString, QString> LDrawColor::ldname2ldcolor;

/*
 * This function extracts colours loaded by the 3DViewer and,
 * alternatively, reads in the alternate LDraw ldconfig.ldr file to
 * extract the color codes, color names, and color values and puts
 * them in the xlate (name to color translate) hash table.
 */
void LDrawColor::LDrawColorInit()
{
    name2color.clear();
    color2name.clear();
//  color2alpha.clear();
//  color2edge.clear();
//  color2value.clear();
//  ldname2ldcolor.clear();

    // fist, check if there is an alternative LDConfig defined
    if (!Preferences::altLDConfigPath.isEmpty())
    {
        QString ldrawFileName = Preferences::altLDConfigPath;
        QFile file(ldrawFileName);
        if (! file.open(QFile::ReadOnly | QFile::Text)) {
            logError() << QString("Failed to open altLDConfig file: %1, Error: [%2]")
                                  .arg(file.fileName()).arg(file.errorString());
            QString extrasFileName(Preferences::ldrawLibPath + "/LDConfig.ldr");
            file.setFileName(extrasFileName);
            // try extras location
            if (! file.open(QFile::ReadOnly | QFile::Text)){
                file.setFileName(":/resources/ldconfig.ldr");
                // try resource location
                if (! file.open(QFile::ReadOnly | QFile::Text)){
                    QMessageBox::warning(nullptr,QMessageBox::tr("LDrawColor"),
                                         QMessageBox::tr("LDConfig load: Cannot read disc file %1,\ndisc file %2"
                                                         "\nor resource file %3.")
                                         .arg(ldrawFileName)
                                         .arg(extrasFileName)
                                         .arg(file.fileName()));
                    return;
                } else
                    logTrace() << "WARNING - LDConfig loaded from resource cache.";
            } else
                logTrace() << QString("WARNING - LDConfig loaded from LDraw directory.").arg(Preferences::ldrawLibPath);
        }

        QRegExp rx("^\\s*0\\s+!COLOUR\\s+(\\w+)\\s+CODE\\s+(\\d+)\\s+VALUE\\s+"
                   "#([\\da-fA-F]+)\\s+EDGE\\s+#([\\da-fA-F]+)(?:.*ALPHA\\s+(\\d+))?");

        QTextStream in(&file);
        while ( ! in.atEnd()) {
            QString line = in.readLine(0);
            if (line.contains(rx)) {
                bool ok;
                QString name;
                QRgb hex = rx.cap(3).toLong(&ok,16);
                QColor color(hex);
                QString code = rx.cap(2);               // color code - e.g. 219
                int alpha = rx.cap(5).toInt(&ok);       //0(0) = colourless, 255(0xff) = fully opaque

                if (ok && alpha >= 0 && alpha <= 256 ) {
                    color2alpha.insert(code,alpha);
                    color.setAlpha(alpha);
                } else {
                    color.setAlpha(255);
                }
                QString value = rx.cap(3);              // color value
                color2value.insert(code,value);

                QString edge = rx.cap(4);               // color edge
                color2edge.insert(code,edge);

                name = rx.cap(1).toLower();             // color name (lower) - e.g. dark_nougat
                name2color.insert(name,color);
                name2color.insert(code,color);

                ldname2ldcolor.insert(name,code);       // using name (lower) and code

                name = rx.cap(1);                       // color name (normal) - e.g. Dark_Nougat
                color2name.insert(code,name);
                color2name.insert(color.name(),name);
            }
        }

    }
    // Load the LDConfig from the loaded parts archive
    else
    {
        for (int ColorIdx = 0; ColorIdx < gColorList.GetSize(); ColorIdx++)
        {
            bool ok;
            lcColor* Color = &gColorList[ColorIdx];
            QString name  = Color->SafeName;
            QString code  = QString::number(Color->Code);
            QString value = QString("%1").arg(Color->CValue, 2, 16, QChar('0')).toUpper();
            QString edge  = QString("%1").arg(Color->EValue, 2, 16, QChar('0')).toUpper();
            int alpha     = Color->Alpha;
            QRgb hex      = value.toLong(&ok,16);
            QColor color(hex);

//            logDebug() << QString("0 !COLOUR %1 CODE %2 VALUE #%3 EDGE #%4 ALPHA %5")
//                          .arg(name).arg(code).arg(value).arg(edge).arg(alpha);

            color.setAlpha(alpha);
            color2alpha.insert(code,alpha);             // color alpha
            color2value.insert(code,value);             // color value
            color2edge.insert(code,edge);               // color edge
            name2color.insert(code,color);
            name2color.insert(name.toLower(),color);    // color name (lower) - e.g. dark_nougat
            ldname2ldcolor.insert(name.toLower(),code); // using name (lower) and code
            color2name.insert(code,name);               // color name (normal) - e.g. Dark_Nougat
            color2name.insert(color.name(),name);       // color name (normal)
        }
    }
}

/*
 * This function provides the translate from LDraw color names and codes
 * to QColor.
 */
QColor LDrawColor::color(QString nickname)
{
  QString lower(nickname.toLower());
  QRegExp rx("\\s*(0x|#)([\\da-fA-F]+)\\s*$");
  if (name2color.contains(lower)) {
      return name2color[lower];
    } else
      if (nickname.contains(rx)) {
          QString prefix("0xf"+rx.cap(2));
          bool ok;
          QRgb rgb = prefix.toLong(&ok,16);
          QColor color(rgb);
          color.setAlpha(255);
          return color;
    }
  return Qt::black;
}

/*
 * This function provides the translate from LDraw color code to
 * alpha value and returns the color alpha value if it exist.
 * If there is no color alpha value, 255 (fully opaque) is returned.
 */
int LDrawColor::alpha(QString code)
{
  if (color2alpha.contains(code))
    return color2alpha[code];
  return 255;
}

/*
 * This function provides the translate from LDraw color code to
 * color value and returns the color value if it exist.
 * If there is no color value, FFFF80 (material main_colour) - is returned.
 */
QString LDrawColor::value(QString code)
{
  if (color2value.contains(code))
    return color2value[code];
  return "FFFF80";
}

/*
 * This function provides the translate from LDraw color code to
 * color edge value and returns the color edge value if it exist.
 * If there is no color edge value, 333333 (default edge color) is returned.
 */
QString LDrawColor::edge(QString code)
{
  if (color2edge.contains(code))
    return color2edge[code];
  return "333333";
}

/*
 * This function provides the translate from QColor or LDraw code to
 * LDraw color name and returns the LDraw color name value if it exist.
 * If there is no translation, an empty string is returned.
 */
QString LDrawColor::name(QString code)
{
  if (color2name.contains(code))
    return color2name[code];
  return "";
}

/* This function provides all the color names */
QStringList LDrawColor::names()
{
    QString key;
    QStringList colorNames;
    QRegExp rx("\\s*(0x|#)([\\da-fA-F]+)\\s*$");

    foreach(key,color2name.keys()) {

        if (! key.contains(rx)) {

            colorNames << color2name[key];
        }
    }
    colorNames.sort();
    return colorNames;
}

/* This function provides the translate from LDraw name to LDraw color code
 * If there is no translation, -1 is returned.
 */
QString LDrawColor::ldColorCode(QString name)
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
bool LDrawColor::colorExist(QString code)
{
  if (name2color.contains(code))
    return true;
  return false;
}
