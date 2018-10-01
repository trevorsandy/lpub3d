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

#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "lpub_preferences.h"
#include "version.h"

QHash<QString, QString>  LDrawColourParts::ldrawColourParts;

LDrawColourParts::LDrawColourParts()
{
    ldrawColourParts.clear();
    QString colorPartsFile = Preferences::ldrawColourPartsFile;
    QFile file(colorPartsFile);
    if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR),
                             QMessageBox::tr("Failed to open LDraw color parts file %1: %2:\n%3")
                             .arg(VER_LDRAW_COLOR_PARTS_FILE)
                             .arg(colorPartsFile)
                             .arg(file.errorString()));
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
