/****************************************************************************
**
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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
#include <QFileInfo>
#include <QTextStream>
#include "lpub_preferences.h"
#include "QsLog.h"

QHash<QString, QString>  LDrawColourParts::ldrawColourParts;

bool LDrawColourParts::LDrawColorPartsLoad(QString &result)
{
    ldrawColourParts.clear();
    QString colorPartsFile = Preferences::ldrawColourPartsFile;
    QFile file(colorPartsFile);
    if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
        result = file.errorString();
        return false;
    }

    QTextStream in(&file);

    // Load RegExp from file;
    QRegExp rx("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(u|o)\\s+(.*)$"); // 3 groups (file, libtype, desc)
    QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
    while ( ! in.atEnd()) {
        QString sLine = in.readLine(0);
        if (sLine.contains(rxin)) {
           rx.setPattern(rxin.cap(1));
           //logDebug() << "LDrawColourParts RegExp Pattern: " << rxin.cap(1);
           break;
        }
    }

    // Load input values
    while ( ! in.atEnd()) {
        QString sLine = in.readLine(0);
        if (sLine.contains(rx)) {
            QString partFile = rx.cap(1).toLower().trimmed();
            QString partLibType = rx.cap(2).toLower().trimmed();
            ldrawColourParts.insert(partFile, QString("%1:::%2").arg(partLibType).arg(partFile));
            //qDebug() << "** Color part loaded: " << partFile << " Lib: " << QString("%1:::%2").arg(partLibType).arg(partFile);
        }
    }
    return true;
}

bool LDrawColourParts::ldrawColorPartsIsLoaded() {
    return ldrawColourParts.size() > 0;
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

void LDrawColourParts::addLDrawColorPart(QString part)
{
    if (ldrawColourParts.contains(part.toLower()))
        return;
    QString partFile = part.toLower().trimmed();
    QString partEntry = QString("g:::%1").arg(partFile);  // partLibType is 'g' Generated
    ldrawColourParts.insert(partFile, partEntry);
    logTrace() << "Add generated colour part: " << partEntry.replace(":::", " ");
}

void LDrawColourParts::clearGeneratedColorParts()
{
    QHashIterator<QString, QString> i(ldrawColourParts);
    while (i.hasNext()) {
        i.next();
        if (i.value().at(0) == QChar('g'))
            ldrawColourParts.remove(i.key());
    }
}
