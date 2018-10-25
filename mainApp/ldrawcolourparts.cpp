/****************************************************************************
**
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "ldrawcolourparts.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include "lpub_preferences.h"
#include "QsLog.h"
#include "version.h"

QHash<QString, QString>  LDrawColourParts::ldrawColourParts;

LDrawColourParts::LDrawColourParts()
{
    ldrawColourParts.clear();
    QString colorPartsFile = Preferences::ldrawColourPartsFile;
    QFile file(colorPartsFile);
    if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
        QString message = QString("Failed to open %1 LDraw color parts file [%2], Error: %3")
                          .arg(Preferences::ldrawLibrary).arg(colorPartsFile).arg(file.errorString());
        logError() << message;
        return;
    }

    QTextStream in(&file);

    QRegExp rx("^\\b([\\d\\w\\-\\_\\+\\\\.]+)\\b\\s*(u|o)\\s*(.*)\\s*$");    // 4 groups (file, libtype, path, desc)
    while ( ! in.atEnd()) {
        QString sLine = in.readLine(0);
        if (sLine.contains(rx)) {
            QString partFile = rx.cap(1).toLower().trimmed();
            QString partLibType = rx.cap(2).toLower().trimmed();
            ldrawColourParts.insert(partFile, QString("%1:::%2").arg(partLibType).arg(partFile));
            //qDebug() << "** Color part loaded: " << partFile << " Lib: " << QString("%1:::%2").arg(partLibType).arg(partFile);
        }
    }
}

bool LDrawColourParts::isLDrawColourPart(QString part)
{
    if (ldrawColourParts.contains(part.toLower())) {
        return true;
    } else {
        return false;
    }
}

QString LDrawColourParts::getLDrawColourPartInfo(QString part){
    if (ldrawColourParts.contains(part.toLower())) {
        return ldrawColourParts[part.toLower()];
    } else {
        return QString();
    }
}
