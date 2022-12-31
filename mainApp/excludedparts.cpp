/****************************************************************************
**
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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
#include "declarations.h"
#include "version.h"
#include "lpub_qtcompat.h"
#include "QsLog.h"

bool ExcludedParts::result;
QList<ExcludedParts::Part> ExcludedParts::excludedParts;

ExcludedParts::ExcludedParts()
{
    if (excludedParts.size() == 0) {
        bool rxFound = false;
        QString excludedPartsFile = Preferences::excludedPartsFile;
        QRegExp rx("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        if (!excludedPartsFile.isEmpty()) {
            QFile file(excludedPartsFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Excluded Parts"),
                                     QMessageBox::tr("Failed to open excluded parts file: %1:\n%2")
                                     .arg(excludedPartsFile),file.errorString());
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
                        Part excludedPart(rx.cap(1), rx.cap(2).toLower().startsWith("helper") ? EP_HELPER : EP_STANDARD);
                        excludedParts.append(excludedPart);
                        //logDebug() << "** ExcludedPartName: " << excludedPartID.id;
                    }
                }
            } else {
                QString message = QMessageBox::tr("Regular expression pattern was not found in %1.<br>"
                                  "Be sure the following lines exist in the file header:<br>"
                                  "# File: %1<br>"
                                  "# The Regular Expression used is: ^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$")
                                  .arg(QFileInfo(excludedPartsFile).fileName());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Excluded Parts"),message);
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
                QChar comment = sLine.at(0);
                if (comment == '#' || comment == ' ')
                    continue;
                if (sLine.contains(rx)) {
                    Part excludedPart(rx.cap(1), rx.cap(2).trimmed().toLower().startsWith("helper") ? EP_HELPER : EP_STANDARD);
                    excludedParts.append(excludedPart);
                }
            }
        }
    }
}

const bool &ExcludedParts::isExcludedPart(QString part)
{
    result = false;
    for (Part &excludedPart : excludedParts) {
        if (excludedPart.id.toLower() == part.toLower().trimmed()) {
            result = true;
            return result;
        }
    }
    return result;
}

const bool &ExcludedParts::isExcludedHelperPart(QString part)
{
    result = false;
    for (Part &excludedPart : excludedParts) {
        if (excludedPart.id.toLower() == part.toLower().trimmed()) {
            result = excludedPart.type == EP_HELPER;
            return result;
        }
    }
    return result;
}

const bool &ExcludedParts::lineHasExcludedPart(const QString &line)
{
    result = false;
    QString part;
    QStringList tt = line.split(" ", SkipEmptyParts);

    if (tt.size() < 15)
        return result;

    for (int t = 14; t < tt.size(); t++) // treat parts with spaces in the name
        part += (tt[t]+" ");

    result = isExcludedPart(part.replace(QRegExp("[\"']"),""));
    return result;
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

# 1. Part ID:          LDraw Part Name          (Required)
# 2. Part Description: LDraw Part Description - (Required)
*/
    const char ExcludedPartsList[] = {
        "arrow80.dat       Helper arrow 08\n"
        "arrow88.dat       Helper arrow 88\n"
        "arrow108.dat      Helper arrow 108\n"
        "Har025.dat        Helper Har025\n"
        "Har050.dat        Helper Har050\n"
        "Har075.dat        Helper Har075\n"
        "HashL2.dat        Helper HashL2\n"
        "HashL3.dat        Helper HashL3\n"
        "HashL4.dat        Helper HashL4\n"
        "HashL5.dat        Helper HashL5\n"
        "HashL6.dat        Helper HashL6\n"
        "HasvL1.dat        Helper HasvL1\n"
        "HasvL2.dat        Helper HasvL2\n"
        "HasvL3.dat        Helper HasvL3\n"
        "HasvL4.dat        Helper HasvL4\n"
        "HasvL5.dat        Helper HasvL5\n"
        "HasvL6.dat        Helper HasvL6\n"
        "HdshL2.dat        Helper HdshL2\n"
        "HdshL3.dat        Helper HdshL3\n"
        "HdshL4.dat        Helper HdshL4\n"
        "HdshL5.dat        Helper HdshL5\n"
        "HdshL6.dat        Helper HdshL6\n"
        "HdsvL1.dat        Helper HdsvL1\n"
        "HdsvL2.dat        Helper HdsvL2\n"
        "HdsvL3.dat        Helper HdsvL3\n"
        "HdsvL4.dat        Helper HdsvL4\n"
        "HdsvL5.dat        Helper HdsvL5\n"
        "HdsvL6.dat        Helper HdsvL6\n"
        "Her.dat           Helper Her\n"
        "Hn0.dat           Helper Hn0\n"
        "Hn1.dat           Helper Hn1\n"
        "Hn2.dat           Helper Hn2\n"
        "Hn3.dat           Helper Hn3\n"
        "Hn4.dat           Helper Hn4\n"
        "Hn5.dat           Helper Hn5\n"
        "Hn6.dat           Helper Hn6\n"
        "Hn7.dat           Helper Hn7\n"
        "Hn8.dat           Helper Hn8\n"
        "Hn9.dat           Helper Hn9\n"
        "Mark-Cross.dat    Helper Mark-Cross\n"
        "Mark-Tick.dat     Helper Mark-Tick\n"
    };

    Buffer.append(ExcludedPartsList, sizeof(ExcludedPartsList));
}


bool ExcludedParts::exportExcludedParts(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,Preferences::validExcludedPliParts));

    if (!overwriteFile(file.fileName()))
        return true;

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << QMessageBox::tr("# File: ") << VER_EXCLUDED_PARTS_FILE << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << QMessageBox::tr("# This list captures excluded parts to support accurate part count.") << lpub_endl;
        outstream << QMessageBox::tr("# Excluded parts must be defined using the file name.") << lpub_endl;
        outstream << QMessageBox::tr("# Parts on this list are excluded from the part count and PLI.") << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << QMessageBox::tr("# This file can be edited from LPub3D from:") << lpub_endl;
        outstream << QMessageBox::tr("#    Configuration=>Edit Parameter Files=>Edit Part Count Excluded Parts List") << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << QMessageBox::tr("# LPub3D will attempt to load the regular expression below first, if the") << lpub_endl;
        outstream << QMessageBox::tr("# load fails, LPub3D will load the hard-coded (default) regular expression.") << lpub_endl;
        outstream << QMessageBox::tr("# If you wish to modify the file import, you can edit this regular expression.") << lpub_endl;
        outstream << QMessageBox::tr("# It would be wise to backup the default entry before performing and update - copy") << lpub_endl;
        outstream << QMessageBox::tr("# and paste to a new line with starting phrase other than 'The Regular Expression...'") << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << QMessageBox::tr("# The Regular Expression used is: ^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$") << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << QMessageBox::tr("# 1. Part ID:          LDraw Part Name        (Required)") << lpub_endl;
        outstream << QMessageBox::tr("# 2. Part Description: LDraw Part Description (Required)") << lpub_endl;
        outstream << QMessageBox::tr("#") << lpub_endl;
        outstream << QMessageBox::tr("# For Helper Parts, be sure the start the description with the keyword Helper.") << lpub_endl;
        outstream << QMessageBox::tr("# When adding a Part Description, be sure to replace double quotes \" with '.") << lpub_endl;
        outstream << QMessageBox::tr("#") << lpub_endl;
        outstream << QMessageBox::tr("# ----------------------Do not delete above this line----------------------------------") << lpub_endl;
        outstream << "#" << lpub_endl;

        QByteArray Buffer;
        loadExcludedParts(Buffer);
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << lpub_endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing Excluded Part Entries, Processed %1 lines in file [%2]")
                                   .arg(counter)
                                   .arg(file.fileName());
        if (Preferences::modeGUI){
            QMessageBox::information(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Excluded Parts"),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open excluded parts file: %1:\n%2")
                                  .arg(file.fileName(),file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Excluded Parts"),message);
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
