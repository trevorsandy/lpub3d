/****************************************************************************
**
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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

QList<ExcludedParts::Part> ExcludedParts::excludedParts;

ExcludedParts::ExcludedParts()
{
    qRegisterMetaType<Part>("Part");
    qRegisterMetaType<ExcludedPartType>("ExcludedPartType");
    if (excludedParts.size() == 0) {
        QString excludedPartsFile = Preferences::excludedPartsFile;
        QRegExp rx("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        QRegExp helperRx("^Helper", Qt::CaseInsensitive);
        QRegExp lsynthRx("^~?LSynth",Qt::CaseInsensitive);
        if (!excludedPartsFile.isEmpty()) {
            QFile file(excludedPartsFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QString message(QObject::tr("Failed to open %1.<br>"
                                "Regenerate by renaming the existing file and select<br>"
                                "Part Count Excluded Parts List from<br>Configuration,<br>"
                                "Edit Parameter Files menu.<br>%2")
                                .arg(excludedPartsFile, file.errorString()));
                if (Preferences::modeGUI) { 
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Excluded Parts"),message); 
                } else { 
                    logWarning() << qPrintable(message.replace("<br>"," "));
                }
                return;
            }
            QTextStream in(&file);

            // Load RegExp from file;
            QRegExp rxin("^#[\\w\\s]+\\:[\\s](\\^.*)$");
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rxin)) {
                   rx.setPattern(rxin.cap(1));
                   //logDebug() << "ExcludedParts RegExp Pattern: " << rxin.cap(1);
                   break;
                }
            }

            // Load input values
            in.seek(0);
            while ( ! in.atEnd()) {
                QString sLine = in.readLine(0);
                if (sLine.contains(rx)) {
                    ExcludedPartType type = rx.cap(2).contains(helperRx)
                            ? EP_HELPER
                            : rx.cap(2).contains(lsynthRx)
                              ? EP_LSYNTH
                              : EP_STANDARD;
                    Part excludedPart(rx.cap(1), type);
                    excludedParts.append(excludedPart);
                    //logDebug() << "** ExcludedPartName: " << excludedPartID.id;
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
                    ExcludedPartType type = rx.cap(2).contains(helperRx)
                            ? EP_HELPER
                            : rx.cap(2).contains(lsynthRx)
                              ? EP_LSYNTH
                              : EP_STANDARD;
                    Part excludedPart(rx.cap(1), type);
                    excludedParts.append(excludedPart);
                }
            }
        }
    }
}

bool ExcludedParts::isExcludedPart(const QString &part, bool &helperPart)
{
    bool allowHelperPart = helperPart;
    helperPart = false;
    for (Part &excludedPart : excludedParts) {
        if (excludedPart.id.toLower() == part.toLower().trimmed()) {
            if (allowHelperPart)
                if ((helperPart = excludedPart.type == EP_HELPER))
                    return false;
            return true;
        }
    }
    return false;
}

bool ExcludedParts::isExcludedPart(const QString &part)
{
    for (Part &excludedPart : excludedParts) {
        if (excludedPart.id.toLower() == part.toLower().trimmed()) {
            return true;
        }
    }
    return false;
}

int ExcludedParts::isExcludedSupportPart(const QString &part)
{
    for (Part &excludedPart : excludedParts) {
        if (excludedPart.id.toLower() == part.toLower().trimmed()) {
            return excludedPart.type;
        }
    }
    return EP_STANDARD;
}

bool ExcludedParts::lineHasExcludedPart(const QString &line)
{
    QRegExp typeRx("^([1-5]) ");
    if (!line.contains(typeRx))
        return false;

    QString part;
    QString type = typeRx.cap(1);
    QStringList le = line.split(" ", SkipEmptyParts);

    // treat parts with spaces in the name
    int i = 0;
    if (le.size() >= 15 && type == "1")
        i = 14;
    else if (le.size() >= 14 && type.contains(QRegExp("4|5")))
        i = 13;
    else if (le.size() >= 11 && type == "3")
        i = 10;
    else if (le.size() >= 8 && type == "2")
        i = 7;
    else
        return false;

    for (; i < le.size(); i++)
        part += (le.at(i)+" ");

    return isExcludedPart(part.replace(QRegExp("[\"']"),""));
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

# The Regular Expression used to load this file is: ^(\b.*[^\s]\b)(?:\s)\s+(.*)$

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
        "LS00.dat          ~LSynth Moved to LS01\n"
        "LS01.dat          LSynth Constraint Part - Type 1 - \"Hose\"\n"
        "LS02.dat          LSynth Constraint Part - Type 2 - \"Hose\"\n"
        "LS03.dat          LSynth Constraint Part - Type 3 - \"Hose\"\n"
        "LS04.dat          LSynth Constraint Part - Type 4 - \"String\"\n"
        "LS05.dat          LSynth Constraint Part - Type 5 - \"NXT Cable\"\n"
        "LS06.dat          LSynth Constraint Part - Type 6 - \"Power Functions Cable\"\n"
        "LS07.dat          LSynth Constraint Part - Type 7 - \"RCX Cable\"\n"
        "LS08.dat          LSynth Constraint Part - Type 8 - \"Minifig Chain\"\n"
        "LS09.dat          LSynth Constraint Part - Type 9 - \"String Minifig Grip\"\n"
        "LS10.dat          ~LSynth Electric Cable Segment\n"
        "LS11.dat          ~LSynth Electric Cable Segment NXT\n"
        "LS12.dat          ~LSynth Electric Mindstorms NXT Cable Segment\n"
        "LS20.dat          ~LSynth Technic Pneumatic Hose - End Piece\n"
        "LS21.dat          ~LSynth Technic Pneumatic Hose - Cross Section\n"
        "LS22.dat          ~LSynth Technic Pneumatic Hose - Beveled End Piece\n"
        "LS23.dat          ~LSynth Technic Pneumatic Hose - Beveled Cross Section\n"
        "LS30.dat          ~LSynth Electric Technic Fiber Optics - End Piece\n"
        "LS40.dat          ~LSynth Technic Flexible Axle - End Piece\n"
        "LS41.dat          ~LSynth Technic Flexible Axle - Cross Section\n"
        "LS50.dat          ~LSynth Technic Flex-System Hose - End Piece\n"
        "LS51.dat          ~LSynth Technic Flex-System Hose - Cross Section\n"
        "LS60.dat          ~LSynth Short Straight String Segment\n"
        "LS61.dat          ~LSynth Straight String Segment for STRING_WITH_GRIPS_21L\n"
        "LS70.dat          ~LSynth Electric Power Functions Cable Segment\n"
        "LS71.dat          ~LSynth Electric Power Functions 2-Wires Cable Segment\n"
        "LS80.dat          ~LSynth Technic Tread Crawler Segment Straight\n"
        "LS90.dat          ~LSynth Hose Flexible 19M End Segment\n"
        "LS91.dat          ~LSynth Hose Flexible 19M Segment\n"
        "LS100.dat         LSynth Constraint Part - Type 10 - \"HOSE_FLEXIBLE_8.5L\"\n"
        "LS101.dat         LSynth Constraint Part - Type 11 - \"HOSE_FLEXIBLE_12L\"\n"
        "LS102.dat         LSynth Constraint Part - Type 12 - \"HOSE_FLEXIBLE_19L\"\n"
        "572a.dat          LSynth String End Stud\n"
        "757.dat           LSynth Hose Flexible 12L Top  1 x  1 x  2/3 with Stud\n"
    };

    Buffer.append(ExcludedPartsList, sizeof(ExcludedPartsList));
}


bool ExcludedParts::exportExcludedParts() {
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
        outstream << QMessageBox::tr("# The Regular Expression used to load this file is: ^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$") << lpub_endl;
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
        if (Preferences::modeGUI) {
            QMessageBox::information(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Excluded Parts"),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open excluded parts file: %1:\n%2")
                                  .arg(file.fileName(),file.errorString());
        if (Preferences::modeGUI) {
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

    QMessageBox box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
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
