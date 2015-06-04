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
        // ^^\\b([\\d\\w\\-\\_\\+\\\\.]+)\\b\\s*\\b([\\d\\w\\:\\/\\-\\_\\+\\\\.]+)\\b\\s*(.*)\\s*$   // QMap
        // ^\\b([\\d\\w\\-\\_\\+\\\\.]+)\\b\\s*~*\\b(.*)\\b\\s*$"                                    // QString

        QRegExp rx("^\\b([\\d\\w\\-\\_\\+\\\\.]+)\\b\\s*\\b([\\d\\w\\:\\/\\-\\_\\+\\\\.]+)\\b\\s*(.*)\\s*$");
                while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rx)) {
                QString colorPartID = rx.cap(1);
                QString colorPartPath = rx.cap(2);
                fadeStepStaticColorParts.insert(colorPartID.toLower().trimmed(),colorPartPath.toLower().trimmed());
                //qDebug() << "** Color Parts Loaded: " << colorPartID.toLower() << " Path: " << colorPartPath.toLower(); //TEST
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
