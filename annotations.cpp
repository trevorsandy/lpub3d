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
        QString annotations = Preferences::lpubPath+"/extras/titleAnnotations.lst";
        QFile file(annotations);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(NULL,QMessageBox::tr("LPub"),
                                 QMessageBox::tr("failed to open %1:\n%2")
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
        qDebug() << "TITLE ANNO OUT COUNT: " << titleAnnotations.size();
    }

    if (freeformAnnotations.size() == 0) {
        QString annotations = Preferences::lpubPath+"/extras/freeformAnnotations.lst";
        QFile file(annotations);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(NULL,QMessageBox::tr("LPub"),
                                 QMessageBox::tr("failed to open %1:\n%2")
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
        qDebug() << "CUSTOM ANNO OUT COUNT: " << freeformAnnotations.size();
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
