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
#include "annotations.h"

#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "lpub_preferences.h"

QString                 Annotations::empty;
QList<QString>          Annotations::titleAnnotations;
QHash<QString, QString> Annotations::freeformAnnotations;

Annotations::Annotations()
{
    if (titleAnnotations.size() == 0) {
        QString annotations = Preferences::titleAnnotationsFile;
        QFile file(annotations);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(NULL,QMessageBox::tr("LPub3D"),
                                 QMessageBox::tr("Failed to open Title Annotations file: %1:\n%2")
                                 .arg(annotations)
                                 .arg(file.errorString()));
            return;
        }
        QTextStream in(&file);
        QRegExp rx("^([\\w\\:]+)\\s+(\\^+\\b.*)\\B\\s*$");
                while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rx)) {
                QString annotation = rx.cap(2);
                titleAnnotations << annotation;
            }
        }
    }

    if (freeformAnnotations.size() == 0) {
        QString annotations = Preferences::freeformAnnotationsFile;
        QFile file(annotations);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(NULL,QMessageBox::tr("LPub3D"),
                                 QMessageBox::tr("Failed to open Freeform Annotations file: %1:\n%2")
                                 .arg(annotations)
                                 .arg(file.errorString()));
            return;
        }
        QTextStream in(&file);
        QRegExp rx("^([\\d\\w\\.]+)\\s+~*\\b(.*)\\b\\s*$");
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rx)) {
                QString parttype = rx.cap(1);
                QString annotation = rx.cap(2);
                freeformAnnotations[parttype.toLower()] = annotation;
            }
        }
    }
}

const QString &Annotations::freeformAnnotation(QString part)
{
  if (freeformAnnotations.contains(part.toLower())) {
    return freeformAnnotations[part.toLower()];
  } else {
    return empty;
  }
}
