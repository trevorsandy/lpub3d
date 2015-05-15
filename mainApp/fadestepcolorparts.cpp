#include "fadestepcolorparts.h"

#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "lpub_preferences.h"

bool                    FadeStepColorParts::result;
QList<QString>          FadeStepColorParts::fadeStepStaticColorParts;

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
        QRegExp rx("^\\b([\\d\\w\\-\\_\\+\\\\.]+)\\b\\s*~*\\b(.*)\\b\\s*$");
                while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if (sLine.contains(rx)) {
                QString colorParts = rx.cap(1);
                fadeStepStaticColorParts << colorParts.toLower().trimmed();
                //qDebug() << "** Color Parts Loaded: " << colorParts.toLower(); //TEST
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
