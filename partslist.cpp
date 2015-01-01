 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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
 * This class reads and internalizes the part titles described in PARTS.LST
 * distributed with the LDraw parts library.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "partslist.h"

#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "lpub_preferences.h"

QHash<QString, QString> PartsList::list;
QString                 PartsList::empty;
QStringList             PartsList::partialPaths;

PartsList::PartsList()
{
  if (list.size() == 0) {
    QString partsname = Preferences::ldrawPath+"/parts.lst";
    QFile file(partsname);
    if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(NULL,QMessageBox::tr("LPub"),
                                QMessageBox::tr("failed to open %1:\n%2")
                                .arg(partsname)
                                .arg(file.errorString()));
      return;
    }
    QTextStream in(&file);
    QString line;
    QRegExp rx("^([\\d\\w\\.]+)\\s+~*\\b(.*)\\b\\s+$");
    while ( ! in.atEnd()) {
      line = in.readLine(0);
      if (line.contains(rx)) {
        QString parttype = rx.cap(1);
        QString title = rx.cap(2);
        if (title[0] == '~' || title[0] == '_') {
          title.remove(0,1);
        }
        list[parttype.toLower()] = title;
      }
    }
  }
  if (partialPaths.size() == 0) {
    partialPaths << "/parts/" << "/parts/s/" << "/p/" << "/p/48/" <<
                    "/Unofficial/parts/" << "/Unofficial/p/" <<
                    "/Unofficial/LSynth/" <<
                    "/Helpers/" << "/Custom/" << "/Development/";
  }
}
bool PartsList::isKnownPart(QString &part)
{
  if (list.contains(part.toLower())) {
    return true;
  } else {
    QString testName;
    QFileInfo info;
    for (int i = 0; i < partialPaths.size(); i++) {
      testName = Preferences::ldrawPath + partialPaths[i] + part;
      info.setFile(testName);
	  
      if (info.exists()) {
        QFile file(testName);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
          return false;
        }
        QTextStream in(&file);
        QString line = in.readLine(0).trimmed();
        if (line[0] == '0') {
          while (line[0] == '0' || line[0] == ' ' || line[0] == '~' || line[0] == '_') {
            line.remove(0,1);
          }
          list[part.toLower()] = line;  
        }
        file.close();
        return true;
      }
    }
    return false;
  }
}

const QString &PartsList::title(QString part)
{
  if (list.contains(part.toLower())) {
    return list[part.toLower()];
  } else {
    return empty;
  }
}





