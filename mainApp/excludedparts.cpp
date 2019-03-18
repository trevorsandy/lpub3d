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

#include "excludedparts.h"

#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "lpub_preferences.h"

#include "QsLog.h"


bool            ExcludedParts::result;
QString         ExcludedParts::empty;
QList<QString>  ExcludedParts::excludedParts;

ExcludedParts::ExcludedParts()
{
    if (excludedParts.size() == 0) {
        QString excludedPartsFile = Preferences::excludedPartsFile;
        QFile file(excludedPartsFile);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),
                                 QMessageBox::tr("Failed to open excludedParts.lst file: %1:\n%2")
                                 .arg(excludedPartsFile)
                                 .arg(file.errorString()));
            return;
        }
        QTextStream in(&file);

        // Load RegExp from file;
        QRegExp rx("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rxin)) {
               rx.setPattern(rxin.cap(1));
               //logDebug() << "ExcludedParts RegExp Pattern: " << rxin.cap(1);
               break;
            }
        }

        // Load input values
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rx)) {
                QString excludedPartID = rx.cap(1);
                excludedParts.append(excludedPartID.toLower().trimmed());
                //logDebug() << "** ExcludedPartID: " << excludedPartID.toLower();
            }
        }
    }
}

const bool &ExcludedParts::hasExcludedPart(QString part)
{
    if (excludedParts.contains(part.toLower().trimmed())) {
        result = true;
        return result;
    } else {
        result = false;
        return result;
    }
}

const bool &ExcludedParts::lineHasExcludedPart(const QString &line)
{
    QString part;
    QStringList tt = line.split(" ",QString::SkipEmptyParts);
    for (int t = 14; t < tt.size(); t++) // treat spaces
        part += (tt[t]+" ");

    return hasExcludedPart(part);
}


