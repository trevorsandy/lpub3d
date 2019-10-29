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

#include "plisubstituteparts.h"

#include <QMessageBox>
#include <QIcon>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include "lpub_preferences.h"
#include "QsLog.h"

bool                    PliSubstituteParts::result;
QString                 PliSubstituteParts::empty;
QMap<QString, QString>  PliSubstituteParts::substituteParts;

PliSubstituteParts::PliSubstituteParts()
{
    if (substituteParts.size() == 0) {
        bool rxFound = false;
        QString substitutePartsFile = Preferences::pliSubstitutePartsFile;
        QFile file(substitutePartsFile);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
           logError() << QMessageBox::tr("Failed to open pliSubstituteParts.lst file: %1:\n%2")
                                       .arg(substitutePartsFile)
                                       .arg(file.errorString());
            return;
        }
        QTextStream in(&file);

        // Load RegExp from file;
        QRegExp rx("^(\\b.+\\b)\\s+\"(.*)\"\\s+(.*)$");
        QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
        while ( ! in.atEnd()) {
            QString sLine = in.readLine(0);
            if ((rxFound = sLine.contains(rxin))) {
                rx.setPattern(rxin.cap(1));
                //logDebug() << "SubstituteParts RegExp Pattern: " << rxin.cap(1);
                break;
            }
        }

        if (rxFound) {
            in.seek(0);
            // Load input values
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rx)) {
                    QString modeledPartID = rx.cap(1);
                    QString substitutePartID = rx.cap(2);
                    substituteParts.insert(modeledPartID.toLower().trimmed(),substitutePartID.toLower().trimmed());
                    //logDebug() << "** ModeledPartID Loaded: " << modeledPartID.toLower() << " SubstitutePartID: " << substitutePartID.toLower(); //TEST
                }
            }
        } else {
            QString message = QString("Regular expression pattern was not found in %1.<br>"
                              "Be sure the following lines exist in the file header:<br>"
                              "# File: %1<br>"
                              "# The Regular Expression used is: ^(\\b.*[^\\s]\\b:)\\s+([\\(|\\^].*)$")
                              .arg(QFileInfo(substitutePartsFile).fileName());
            if (Preferences::modeGUI){
                QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
            } else {
                logError() << message.replace("<br>"," ");
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
#ifdef QT_DEBUG_MODE
        logError() <<  QString("Substitute Part: ").arg(part);
#endif
        result = true;
        return result;
    } else {
        part = empty;
        result = false;
        return result;
    }
}

bool PliSubstituteParts::exportSubstitutePartsHeader(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,Preferences::pliSubstitutePartsFile));

    if (!overwriteFile(file.fileName()))
        return true;

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File:" << Preferences::pliSubstitutePartsFile << endl;
        outstream << "#" << endl;
        outstream << "# This space-delimited list captures substitute part ID and its substitute part absolute path." << endl;
        outstream << "# This file is an alternative to the embedded file substitution used when defining a PLI/BOM." << endl;
        outstream << "# Parts on this list should have an accompanying substitute part." << endl;
        outstream << "# Substitue parts must be defined using their absolute file path." << endl;
        outstream << "# When building the PLI/BOM files on this list are replaced with their substitute." << endl;
        outstream << "#" << endl;
        outstream << "# The file path must be quoted even if there are no spaces in the path" << endl;
        outstream << "#" << endl;
        outstream << "# This file can be edited from LPub3D from:" << endl;
        outstream << "#    Configuration=>Edit Parameter Files=>Edit PLI/BOM Substitue Parts List" << endl;
        outstream << "#" << endl;
        outstream << "# LPub3D will attempt to load the regular expression below first, if the" << endl;
        outstream << "# load fails, LPub3D will load the hard-coded (default) regular expression." << endl;
        outstream << "# If you wish to modify the file import, you can edit this regular expression." << endl;
        outstream << "# It would be wise to backup the default entry before performing and update - copy" << endl;
        outstream << "# and paste to a new line with starting phrase other than 'The Regular Expression...'" << endl;
        outstream << "#" << endl;
        outstream << "# The Regular Expression used is: ^(\\b.+\\b)\\s+\"(.*)\"\\s+(.*)$" << endl;
        outstream << "#" << endl;
        outstream << "# ----------------------Do not delete above this line----------------------------------" << endl;
        outstream << "#" << endl;
        outstream << "# Part ID (LDraw Name:)     Substitute Part Absolute File Path      Part Description - for reference only" << endl;
        outstream << "#" << endl;
        outstream << "# Official Parts" << endl;
        outstream << "" << endl;
        outstream << "" << endl;
        outstream << "# Unofficial Parts" << endl;
        outstream << "" << endl;
        outstream << "" << endl;
        outstream << "# Custom Parts" << endl;

        QByteArray Buffer;
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing Substitute Part Entries, Processed %1 lines in file [%2]")
                                   .arg(counter)
                                   .arg(file.fileName());
        if (Preferences::modeGUI){
            QMessageBox::information(nullptr,QMessageBox::tr("LPub3D"),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open Substitute Parts file: %1:\n%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
        } else {
            logError() << message;
        }
       return false;
    }
    return true;
}

bool PliSubstituteParts::overwriteFile(const QString &file)
{
    QFileInfo fileInfo(file);

    if (!fileInfo.exists())
        return true;

    // Get the application icon as a pixmap
    QPixmap _icon = QPixmap(":/icons/lpub96.png");
    if (_icon.isNull())
        _icon = QPixmap (":/icons/update.png");

    QMessageBox box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (_icon);
    box.setTextFormat (Qt::RichText);
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    QString title = "<b>" + QMessageBox::tr ("Export %1").arg(fileInfo.fileName()) + "</b>";
    QString text = QMessageBox::tr("\"%1\"<br>This file already exists.<br>Replace existing file?").arg(fileInfo.fileName());
    box.setText (title);
    box.setInformativeText (text);
    box.setStandardButtons (QMessageBox::Cancel | QMessageBox::Yes);
    box.setDefaultButton   (QMessageBox::Yes);

    return (box.exec() == QMessageBox::Yes);
}
