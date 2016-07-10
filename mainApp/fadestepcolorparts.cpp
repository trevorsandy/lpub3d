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

#include "fadestepcolorparts.h"

#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "lpub_preferences.h"

bool                    FadeStepColorParts::result;
QString                 FadeStepColorParts::empty;
QString                 FadeStepColorParts::path;
QMap<QString, QString>  FadeStepColorParts::fadeStepStaticColorParts;

FadeStepColorParts::FadeStepColorParts()
{
    if (fadeStepStaticColorParts.size() == 0) {
        QString colorPartsFile = Preferences::fadeStepColorPartsFile;
        QFile file(colorPartsFile);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(NULL,QMessageBox::tr("LPub3D"),
                                 QMessageBox::tr("Failed to open fadeStepColorParts.lst file: %1:\n%2")
                                 .arg(colorPartsFile)
                                 .arg(file.errorString()));
            return;
        }
        QTextStream in(&file);

        //QRegExp rx("^\\b([\\d\\w\\-\\_\\+\\\\.]+)\\b\\s*\\b([\\d\\w\\:\\/\\-\\_\\+\\\\.]+)\\b\\s*(.*)\\s*$"); // 3 groups (number, path, desc)
        QRegExp rx("^\\b([\\d\\w\\-\\_\\+\\\\.]+)\\b\\s*(.*)\\s*$");    // 2 groups (number, desc)
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rx)) {
                QString colorPartID = rx.cap(1);
                //QString colorPartPath = rx.cap(2);
                //fadeStepStaticColorParts.insert(colorPartID.toLower().trimmed(),colorPartPath.toLower().trimmed());
                //qDebug() << "** Color Parts Loaded: " << colorPartID.toLower() << " Path: " << colorPartPath.toLower(); // OLD TEST
                fadeStepStaticColorParts.insert(colorPartID.toLower().trimmed(),colorPartID.toLower().trimmed());
                //qDebug() << "** Color Parts Loaded: " << colorPartID.toLower() << " Path: " << colorPartID.toLower(); //TEST
            }
        }
    }
}

const bool &FadeStepColorParts::isStaticColorPart(QString part)
{
    if (fadeStepStaticColorParts.contains(part.toLower().trimmed())) {
        result = true;
        return result;
    } else {
        result = false;
        return result;
    }
}

const bool &FadeStepColorParts::getStaticColorPartPath(QString &part){
    if (fadeStepStaticColorParts.contains(part.toLower().toLower().trimmed())) {
        part = fadeStepStaticColorParts.value(part.toLower());
        result = true;
        return result;
    } else {
        part = empty;
        result = false;
        return result;
    }
}

const QString &FadeStepColorParts::staticColorPartPath(QString part){
    if (fadeStepStaticColorParts.contains(part.toLower().toLower().trimmed())) {
        path = fadeStepStaticColorParts.value(part.toLower());
      return path;
    } else {
      return empty;
    }
}
