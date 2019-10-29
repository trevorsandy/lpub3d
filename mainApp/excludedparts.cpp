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
#include <QIcon>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "lpub_preferences.h"
#include "name.h"
#include "QsLog.h"


bool            ExcludedParts::result;
QString         ExcludedParts::empty;
QList<QString>  ExcludedParts::excludedParts;

ExcludedParts::ExcludedParts()
{
    if (excludedParts.size() == 0) {
        bool rxFound = false;
        QString excludedPartsFile = Preferences::excludedPartsFile;
        QRegExp rx("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        if (!excludedPartsFile.isEmpty()) {
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
            QRegExp rxin("^#\\sThe\\sRegular\\sExpression\\sused\\sis\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if ((rxFound = sLine.contains(rxin))) {
                   rx.setPattern(rxin.cap(1));
                   //logDebug() << "ExcludedParts RegExp Pattern: " << rxin.cap(1);
                   break;
                }
            }

            if (rxFound) {
                in.seek(0);

                // Load input values
                while ( ! in.atEnd()) {
                    QString sLine = in.readLine(0);
                    if (sLine.contains(rx)) {
                        QString excludedPartID = rx.cap(1);
                        excludedParts.append(excludedPartID.toLower().trimmed());
                        //logDebug() << "** ExcludedPartID: " << excludedPartID.toLower();
                    }
                }
            } else {
                QString message = QString("Regular expression pattern was not found in %1.<br>"
                                  "Be sure the following lines exist in the file header:<br>"
                                  "# File: %1<br>"
                                  "# The Regular Expression used is: ^(\\b.*[^\\s]\\b:)\\s+([\\(|\\^].*)$")
                                  .arg(QFileInfo(excludedPartsFile).fileName());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
            }
        } else {
            excludedParts.clear();
            QByteArray Buffer;
            loadExcludedParts(Buffer);
            QTextStream instream(Buffer);
            for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
            {
                int Equals = sLine.indexOf('=');
                if (Equals == -1)
                    continue;
                if (sLine.contains(rx)) {
                    QString excludedPartID = rx.cap(1);
                    excludedParts.append(excludedPartID.toLower().trimmed());
                }
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

void ExcludedParts::loadExcludedParts(QByteArray &Buffer)
{
/*
# File: excludedParts.lst

# This space-delimited list captures excluded part name and, optionally, part descripton

# This list captures excluded parts to support accurate part count.
# Excluded parts must be defined using the file name.
# Parts on this list are excluded from the part count and PLI.

# This file can be edited from LPub3D from:
#    Configuration=>Edit Parameter Files=>Edit Part Count Excluded Parts List

# LPub3D will attempt to load the regular expression below first, if the
# load fails, LPub3D will load the hard-coded (default) regular expression.
# If you wish to modify the file import, you can edit this regular expression.
# It would be wise to backup the default entry before performing and update - copy
# and paste to a new line with starting phrase other than 'The Regular Expression...'

# The Regular Expression used is: ^(\b.*[^\s]\b)(?:\s)\s+(.*)$

# 1. Part ID:          LDraw Part Name                               (Required)
# 2. Part Description: LDraw Part Description - for reference only   (Optional)
*/
    const char LEGOExcludedPartsList[] = {
        "arrow80.dat       helper arrow 08\n"
        "arrow88.dat       helper arrow 88\n"
        "arrow108.dat      helper arrow 108\n"
        "Har025.dat        helper Har025\n"
        "Har050.dat        helper Har050\n"
        "Har075.dat        helper Har075\n"
        "HashL2.dat        helper HashL2\n"
        "HashL3.dat        helper HashL3\n"
        "HashL4.dat        helper HashL4\n"
        "HashL5.dat        helper HashL5\n"
        "HashL6.dat        helper HashL6\n"
        "HasvL1.dat        helper HasvL1\n"
        "HasvL2.dat        helper HasvL2\n"
        "HasvL3.dat        helper HasvL3\n"
        "HasvL4.dat        helper HasvL4\n"
        "HasvL5.dat        helper HasvL5\n"
        "HasvL6.dat        helper HasvL6\n"
        "HdshL2.dat        helper HdshL2\n"
        "HdshL3.dat        helper HdshL3\n"
        "HdshL4.dat        helper HdshL4\n"
        "HdshL5.dat        helper HdshL5\n"
        "HdshL6.dat        helper HdshL6\n"
        "HdsvL1.dat        helper HdsvL1\n"
        "HdsvL2.dat        helper HdsvL2\n"
        "HdsvL3.dat        helper HdsvL3\n"
        "HdsvL4.dat        helper HdsvL4\n"
        "HdsvL5.dat        helper HdsvL5\n"
        "HdsvL6.dat        helper HdsvL6\n"
        "Her.dat           helper Her\n"
        "Hn0.dat           helper Hn0\n"
        "Hn1.dat           helper Hn1\n"
        "Hn2.dat           helper Hn2\n"
        "Hn3.dat           helper Hn3\n"
        "Hn4.dat           helper Hn4\n"
        "Hn5.dat           helper Hn5\n"
        "Hn6.dat           helper Hn6\n"
        "Hn7.dat           helper Hn7\n"
        "Hn8.dat           helper Hn8\n"
        "Hn9.dat           helper Hn9\n"
        "Mark-Cross.dat    helper Mark-Cross\n"
        "Mark-Tick.dat     helper Mark-Tick\n"
    };
    const char TENTEExcludedPartsList[] = {
         "no excluded part defined\n"
    };

    const char VEXIQExcludedPartsList[] = {
         "no excluded part defined\n"
    };

    const char ExcludedPartsList[] = {
        "no excluded part defined\n"
    };

    if (Preferences::validLDrawLibrary == LEGO_LIBRARY)
        Buffer.append(LEGOExcludedPartsList, sizeof(LEGOExcludedPartsList));
    else
    if (Preferences::validLDrawLibrary == TENTE_LIBRARY)
        Buffer.append(TENTEExcludedPartsList, sizeof(TENTEExcludedPartsList));
    else
    if (Preferences::validLDrawLibrary == VEXIQ_LIBRARY)
        Buffer.append(VEXIQExcludedPartsList, sizeof(VEXIQExcludedPartsList));
    else
        Buffer.append(ExcludedPartsList, sizeof(ExcludedPartsList));
}


bool ExcludedParts::exportExcludedParts(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,Preferences::excludedPartsFile));

    if (!overwriteFile(file.fileName()))
        return true;

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File:" << Preferences::validAnnotationStyleFile << endl;
        outstream << "# " << endl;
        outstream << "# This list captures excluded parts to support accurate part count." << endl;
        outstream << "# Excluded parts must be defined using the file name." << endl;
        outstream << "# Parts on this list are excluded from the part count and PLI." << endl;
        outstream << "# " << endl;
        outstream << "# This file can be edited from LPub3D from:" << endl;
        outstream << "#    Configuration=>Edit Parameter Files=>Edit Part Count Excluded Parts List" << endl;
        outstream << "# " << endl;
        outstream << "# LPub3D will attempt to load the regular expression below first, if the" << endl;
        outstream << "# load fails, LPub3D will load the hard-coded (default) regular expression." << endl;
        outstream << "# If you wish to modify the file import, you can edit this regular expression." << endl;
        outstream << "# It would be wise to backup the default entry before performing and update - copy" << endl;
        outstream << "# and paste to a new line with starting phrase other than 'The Regular Expression...'" << endl;
        outstream << "# " << endl;
        outstream << "# The Regular Expression used is: ^(\\b.*[^\\s]\b)(?:\\s)\\s+(.*)$" << endl;
        outstream << "# " << endl;
        outstream << "# 1. Part ID:          LDraw Part Name                               (Required)" << endl;
        outstream << "# 2. Part Description: LDraw Part Description - for reference only   (Optional)" << endl;
        outstream << "#" << endl;
        outstream << "#" << endl;
        outstream << "# ----------------------Do not delete above this line----------------------------------" << endl;
        outstream << "#" << endl;

        QByteArray Buffer;
        loadExcludedParts(Buffer);
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing Excluded Part Entries, Processed %1 lines in file [%2]")
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
        QString message = QString("Failed to open Excluded Parts file: %1:\n%2")
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

bool ExcludedParts::overwriteFile(const QString &file)
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
