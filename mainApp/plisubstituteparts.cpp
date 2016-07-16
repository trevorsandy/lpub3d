/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

#include "plisubstituteparts.h"

#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "lpub_preferences.h"

bool                    PliSubstituteParts::result;
QString                 PliSubstituteParts::empty;
QMap<QString, QString>  PliSubstituteParts::substituteParts;

PliSubstituteParts::PliSubstituteParts()
{
    if (substituteParts.size() == 0) {
        QString substitutePartsFile = Preferences::pliSubstitutePartsFile;
        QFile file(substitutePartsFile);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(NULL,QMessageBox::tr("LPub3D"),
                                 QMessageBox::tr("Failed to open pliSubstituteParts.lst file: %1:\n%2")
                                 .arg(substitutePartsFile)
                                 .arg(file.errorString()));
            return;
        }
        QTextStream in(&file);

        QRegExp rx("^\\b([\\d\\w\\-\\_\\+\\\\.]+)\\b\\s*\\b([\\d\\w\\:\\/\\-\\_\\+\\\\.]+)\\b\\s*(.*)\\s*$");
                while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rx)) {
                QString modeledPartID = rx.cap(1);
                QString substitutePartID = rx.cap(2);
                substituteParts.insert(modeledPartID.toLower().trimmed(),substitutePartID.toLower().trimmed());
//                qDebug() << "** ModeledPartID Loaded: " << modeledPartID.toLower() << " SubstitutePartID: " << substitutePartID.toLower(); //TEST
            }
        }
    }
}

const bool &PliSubstituteParts::hasSubstitutePart(QString part)
{
    if (substituteParts.contains(part.toLower().trimmed())) {
        result = true;
        return result;
    } else {
        result = false;
        return result;
    }
}

const bool &PliSubstituteParts::getSubstitutePart(QString &part){
    if (substituteParts.contains(part.toLower().toLower().trimmed())) {
        part = substituteParts.value(part.toLower());
        qDebug() << "Substitute Part: " << part;
        result = true;
        return result;
    } else {
        part = empty;
        result = false;
        return result;
    }
}
