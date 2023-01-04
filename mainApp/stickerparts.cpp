/****************************************************************************
**
** Copyright (C) 2020 - 2023 Trevor SANDY. All rights reserved.
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

#include "stickerparts.h"

#include <QMessageBox>
#include <QIcon>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "lpub_preferences.h"
#include "lpub_qtcompat.h"
#include "declarations.h"
#include "version.h"
#include "QsLog.h"


bool            StickerParts::result;
QString         StickerParts::empty;
QList<QString>  StickerParts::stickerParts;

StickerParts::StickerParts()
{
    if (stickerParts.size() == 0) {
        bool rxFound = false;
        QString stickerPartsFile = Preferences::stickerPartsFile;
        QRegExp rx("^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$");
        if (!stickerPartsFile.isEmpty()) {
            QFile file(stickerPartsFile);
            if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
                QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Sticker Parts"),
                                     QMessageBox::tr("Failed to open sticker parts file: %1:\n%2")
                                     .arg(stickerPartsFile)
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
                   //logDebug() << "StickerParts RegExp Pattern: " << rxin.cap(1);
                   break;
                }
            }

            if (rxFound) {
                in.seek(0);

                // Load input values
                while ( ! in.atEnd()) {
                    QString sLine = in.readLine(0);
                    if (sLine.contains(rx)) {
                        QString stickerPartID = rx.cap(1);
                        stickerParts.append(stickerPartID.toLower().trimmed());
                        //logDebug() << "** StickerPartID: " << stickerPartID.toLower();
                    }
                }
            } else {
                QString message = QString("Regular expression pattern was not found in %1.<br>"
                                          "Be sure the following lines exist in the file header:<br>"
                                          "# File: %1<br>"
                                          "# The Regular Expression used is: ^(\\b.*[^\\s]\\b:)\\s+([\\(|\\^].*)$")
                                  .arg(QFileInfo(stickerPartsFile).fileName());
                if (Preferences::modeGUI){
                    QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Sticker Parts"),message);
                } else {
                    logError() << message.replace("<br>"," ");
                }
            }
        } else {
            stickerParts.clear();
            QByteArray Buffer;
            loadStickerParts(Buffer);
            QTextStream instream(Buffer);
            for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
            {
                int Equals = sLine.indexOf('=');
                if (Equals == -1)
                    continue;
                if (sLine.contains(rx)) {
                    QString stickerPartID = rx.cap(1);
                    stickerParts.append(stickerPartID.toLower().trimmed());
                }
            }
        }
    }
}

const bool &StickerParts::hasStickerPart(QString part)
{
    if (stickerParts.contains(part.toLower().trimmed())) {
        result = true;
        return result;
    } else {
        result = false;
        return result;
    }
}

const bool &StickerParts::lineHasStickerPart(const QString &line)
{
    QString part;
    QStringList tt = line.split(" ", SkipEmptyParts);
    for (int t = 14; t < tt.size(); t++) // treat spaces
        part += (tt[t]+" ");

    return hasStickerPart(part);
}

void StickerParts::loadStickerParts(QByteArray &Buffer)
{
/*
# File: stickerParts.lst

# This space-delimited list captures sticker part name and, optionally, part descripton

# This list captures sticker parts to support accurate part count.
# Sticker parts must be defined using the file name.
# Parts on this list are sticker from the part count and PLI.

# This file can be edited from LPub3D from:
#    Configuration=>Edit Parameter Files=>Edit Part Count Sticker Parts List

# LPub3D will attempt to load the regular expression below first, if the
# load fails, LPub3D will load the hard-coded (default) regular expression.
# If you wish to modify the file import, you can edit this regular expression.
# It would be wise to backup the default entry before performing and update - copy
# and paste to a new line with starting phrase other than 'The Regular Expression...'

# The Regular Expression used is: ^(\b.*[^\s]\b)(?:\s)\s+(.*)$

# 1. Part ID:          LDraw Part Name                               (Required)
# 2. Part Description: LDraw Part Description - for reference only   (Optional)
*/
    const char LEGOStickerPartsList[] = {
        "# Official Stickers\n"
        "4640189c.dat     Sticker  0.49 x  3.77 with Black 'Shinkai 6500' on White Background\n"
        "821472e.dat      Sticker  0.5 x  1.8 with Red and Black 'GT 500' on Transparent Background\n"
        "168335r.dat      Sticker  0.5 x  1.9 with  3 White Stars\n"
        "164575g.dat      Sticker  0.6 x  1.5 with White Train Logo\n"
        "4613857d.dat     Sticker  0.6 x  1.6 Left with Spirit of Luis & Eagle on DkRed\n"
        "4613857e.dat     Sticker  0.6 x  1.6 Right with Spirit of Luis & Eagle on DkRed\n"
        "59718b.dat       Sticker  0.6 x  2 with 'JM 7991'\n"
        "168335s.dat      Sticker  0.6 x  2.9 with  3 White Stars with Black Outline\n"
        "59718a.dat       Sticker  0.6 x  4 with Yellow/Black Chevrons\n"
        "190085c.dat      Sticker  0.7 x  0.7 Round Mirror\n"
        "168265b.dat      Sticker  0.7 x  1.5 with Black Train Logo\n"
        "6058327d.dat     Sticker  0.7 x  1.7 with Buttons and Circuitry\n"
        "4544258m.dat     Sticker  0.7 x  1.8 with White '10194'\n"
        "169685b.dat      Sticker  0.7 x  1.9 Red/White Diagonal Stripes Right Up\n"
        "6153754p.dat     Sticker  0.7 x  1.9 with Black Surfer in Orange Sunset\n"
        "169685a.dat      Sticker  0.7 x  1.9 with Red/White Diagonal Stripes Left Up\n"
        "4622303q.dat     Sticker  0.75 x  1.75 Badge 'BLUETEC 5' on Orange\n"
        "4622303p.dat     Sticker  0.75 x  1.75 Black Vehicle Door Handle Left on Orange\n"
        "4622303o.dat     Sticker  0.75 x  1.75 Black Vehicle Door Handle Right on Orange\n"
        "4622303m.dat     Sticker  0.75 x  1.75 Turn Indicator Left on Orange\n"
        "4622303n.dat     Sticker  0.75 x  1.75 Turn Indicator Right on Orange\n"
        "4622303g.dat     Sticker  0.75 x  1.75 with Mercedes-Benz Logo\n"
        "4631649b.dat     Sticker  0.75 x  3.75 DkBluishGray Control Panel on Black Dashboard\n"
        "4622303h.dat     Sticker  0.75 x  3.75 Silver 'Mercedes-Benz' on Black\n"
        "4622303k.dat     Sticker  0.75 x  3.75 Silver 'U400' on Orange\n"
        "4622303i.dat     Sticker  0.75 x  3.75 Silver 'UNIMOG' on Black\n"
        "4622303j.dat     Sticker  0.75 x  3.75 White 'LEGO System A/S' on Black\n"
        "4622303r.dat     Sticker  0.75 x  5 with Pneumatic Cylinder / Half Trucks Diagram\n"
        "4622303ac01.dat  Sticker  0.75 x  5.5 Red/White Diag. Stripes Right Up (Formed)\n"
        "4622303b.dat     Sticker  0.75 x  5.5 Red/White Diagonal Stripes Left Up\n"
        "4622303bc01.dat  Sticker  0.75 x  5.5 Red/White Diagonal Stripes Left Up (Formed)\n"
        "4622303a.dat     Sticker  0.75 x  5.5 Red/White Diagonal Stripes Right Up\n"
        "4622303t.dat     Sticker  0.75 x  7.5 with Pneumatic Cylinder and Truck Diagram\n"
        "4622303s.dat     Sticker  0.75 x  7.5 with Two Trucks Diagram\n"
        "4640189b.dat     Sticker  0.75 x  7.65 with White 'Shinkai 6500' on Black Background\n"
        "003238i.dat      Sticker Flag with Crown on Dark Pink Background\n"
        "003238r.dat      Sticker Flag with Crown on Violet Background\n"
        "13710ac01.dat    Sticker Mindstorms EV3 #1 for Panel Smooth Medium (Formed) (Needs Work)\n"
        "13710a.dat       Sticker Mindstorms EV3 #1 for Panel Smooth Medium (Needs Work)\n"
        "13710jc01.dat    Sticker Mindstorms EV3 #10 for Panel Smooth Long (Formed) (Needs Work)\n"
        "13710j.dat       Sticker Mindstorms EV3 #10 for Panel Smooth Long (Needs Work)\n"
        "13710l.dat       Sticker Mindstorms EV3 #12 for Beam 3 / Fairing Left (Flat) (Needs Work)\n"
        "13710lc01.dat    Sticker Mindstorms EV3 #12 for Beam 3 / Fairing Left (Formed) (Needs Work)\n"
        "13710k.dat       Sticker Mindstorms EV3 #12 for Beam 3 / Fairing Right (Flat) (Needs Work)\n"
        "13710kc01.dat    Sticker Mindstorms EV3 #12 for Beam 3 / Fairing Right (Formed) (Needs Work)\n"
        "13710mc01.dat    Sticker Mindstorms EV3 #13 for Panel Smooth Long (Formed) (Needs Work)\n"
        "13710m.dat       Sticker Mindstorms EV3 #13 for Panel Smooth Long (Needs Work)\n"
        "13710nc01.dat    Sticker Mindstorms EV3 #14 for Panel Smooth Long (Formed) (Needs Work)\n"
        "13710n.dat       Sticker Mindstorms EV3 #14 for Panel Smooth Long (Needs Work)\n"
        "13710bc01.dat    Sticker Mindstorms EV3 #2 for Panel Smooth Medium (Formed) (Needs Work)\n"
        "13710b.dat       Sticker Mindstorms EV3 #2 for Panel Smooth Medium (Needs Work)\n"
        "13710cc01.dat    Sticker Mindstorms EV3 #3 for Panel Smooth Medium (Formed) (Needs Work)\n"
        "13710c.dat       Sticker Mindstorms EV3 #3 for Panel Smooth Medium (Needs Work)\n"
        "13710dc01.dat    Sticker Mindstorms EV3 #4 for Panel Smooth Medium (Formed) (Needs Work)\n"
        "13710d.dat       Sticker Mindstorms EV3 #4 for Panel Smooth Medium (Needs Work)\n"
        "13710ec01.dat    Sticker Mindstorms EV3 #5 for Panel Smooth Medium (Formed) (Needs Work)\n"
        "13710e.dat       Sticker Mindstorms EV3 #5 for Panel Smooth Medium (Needs Work)\n"
        "13710fc01.dat    Sticker Mindstorms EV3 #6 for Panel Smooth Medium (Formed) (Needs Work)\n"
        "13710f.dat       Sticker Mindstorms EV3 #6 for Panel Smooth Medium (Needs Work)\n"
        "13710gc01.dat    Sticker Mindstorms EV3 #7 for Panel Smooth Long (Formed) (Needs Work)\n"
        "13710g.dat       Sticker Mindstorms EV3 #7 for Panel Smooth Long (Needs Work)\n"
        "13710hc01.dat    Sticker Mindstorms EV3 #8 for Panel Smooth Long (Formed) (Needs Work)\n"
        "13710h.dat       Sticker Mindstorms EV3 #8 for Panel Smooth Long (Needs Work)\n"
        "13710ic01.dat    Sticker Mindstorms EV3 #9 for Panel Smooth Long (Formed) (Needs Work)\n"
        "13710i.dat       Sticker Mindstorms EV3 #9 for Panel Smooth Long (Needs Work)\n"
        "003238a.dat      Sticker Minifig Shield Triangular with Crown on Dark Pink Background\n"
        "003238k.dat      Sticker Minifig Shield Triangular with Crown on Violet Background\n"
        "003238b.dat      Sticker Minifig Shield Triangular with Green and Yellow Chevrons on Dark Grey Background\n"
        "003238l.dat      Sticker Minifig Shield Triangular with Green and Yellow Chevrons on Light Grey Background\n"
        "003238c.dat      Sticker Minifig Shield Triangular with White Maltese Cross on Red Background\n"
        "003238d.dat      Sticker Minifig Shield Triangular with Yellow Trefoils on Blue Background\n"
        "003238m.dat      Sticker Minifig Shield Triangular with Yellow Trefoils on Dark Blue Background\n"
        "003497g.dat      Sticker Minifig Torso with Black Suit with Two Buttons and Red Tie\n"
        "003497h.dat      Sticker Minifig Torso with Red Shirt with Four Buttons and Patch Pockets\n"
        "004695c.dat      Sticker Minifig Torso with Shell Logo\n"
        "004585a.dat      Sticker Minifig Torso with White Buttons and Police Badge Plain\n"
        "003238e.dat      Sticker Minifig Vest with Crown on Dark Pink Background\n"
        "003238n.dat      Sticker Minifig Vest with Crown on Violet Background\n"
        "003238f.dat      Sticker Minifig Vest with Shield with Green and Yellow Chevrons on Dark Grey Background\n"
        "003238o.dat      Sticker Minifig Vest with Shield with Green Chevrons on Light Grey Background\n"
        "003238g.dat      Sticker Minifig Vest with Shield with White Maltese Cross on Dark Grey Background\n"
        "003238p.dat      Sticker Minifig Vest with Shield with White Maltese Cross on Light Grey Background\n"
        "003238h.dat      Sticker Minifig Vest with Yellow Trefoils on Blue Background\n"
        "003238q.dat      Sticker Minifig Vest with Yellow Trefoils on Dark Blue Background\n"
        "199025a.dat      Sticker Minifig Vest with Yellow/Black Chevrons\n"
        "4542422b.dat     Sticker Roadsign Triangle with Traffic Light\n"
        "4297014c.dat     Sticker Tail Left with Red 'J' on White\n"
        "4297014d.dat     Sticker Tail Right with Red 'J' on White\n"
        "86037c.dat       Sticker Technic Front Loader LEGO Technic Banner Side A\n"
        "86037d.dat       Sticker Technic Front Loader LEGO Technic Banner Side B\n"
        "821435f.dat      Sticker with Black/Red Triangles and Racing Number '3', Left\n"
        "821435e.dat      Sticker with Black/Red Triangles and Racing Number '3', Right\n"
        "821435b.dat      Sticker with Red/Green Triangles and Racing Number '1', Left\n"
        "821435a.dat      Sticker with Red/Green Triangles and Racing Number '1', Right\n"
        "821435d.dat      Sticker with White/Black Triangles and Racing Number '2', Left\n"
        "821435c.dat      Sticker with White/Black Triangles and Racing Number '2', Right\n"
        "821435h.dat      Sticker with White/Red Triangles and Racing Number '4', Left\n"
        "821435g.dat      Sticker with White/Red Triangles and Racing Number '4', Right\n"
        "190267.dat       Sticker Yellow 'Police' for Door  1 x  5 x  3.75"
    };
    const char TENTEStickerPartsList[] = {
         "no sticker part defined\n"
    };

    const char VEXIQStickerPartsList[] = {
         "no sticker part defined\n"
    };

    const char StickerPartsList[] = {
        "no sticker part defined\n"
    };

    if (Preferences::validLDrawLibrary == LEGO_LIBRARY)
        Buffer.append(LEGOStickerPartsList, sizeof(LEGOStickerPartsList));
    else
    if (Preferences::validLDrawLibrary == TENTE_LIBRARY)
        Buffer.append(TENTEStickerPartsList, sizeof(TENTEStickerPartsList));
    else
    if (Preferences::validLDrawLibrary == VEXIQ_LIBRARY)
        Buffer.append(VEXIQStickerPartsList, sizeof(VEXIQStickerPartsList));
    else
        Buffer.append(StickerPartsList, sizeof(StickerPartsList));
}


bool StickerParts::exportStickerParts(){
    QFile file(QString("%1/extras/%2").arg(Preferences::lpubDataPath,Preferences::validStickerPliParts));

    if (!overwriteFile(file.fileName()))
        return true;

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QTextStream outstream(&file);
        outstream << "# File: " << VER_STICKER_PARTS_FILE << lpub_endl;
        outstream << "# " << lpub_endl;
        outstream << "# This list captures sticker parts to support accurate part count." << lpub_endl;
        outstream << "# Sticker parts must be defined using the file name." << lpub_endl;
        outstream << "# " << lpub_endl;
        outstream << "# This file can be edited from LPub3D from:" << lpub_endl;
        outstream << "#    Configuration=>Edit Parameter Files=>Edit Part Count Sticker Parts List" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# LPub3D will attempt to load the regular expression below first, if the" << lpub_endl;
        outstream << "# load fails, LPub3D will load the hard-coded (default) regular expression." << lpub_endl;
        outstream << "# If you wish to modify the file import, you can edit this regular expression." << lpub_endl;
        outstream << "# It would be wise to backup the default entry before performing and update - copy" << lpub_endl;
        outstream << "# and paste to a new line with starting phrase other than 'The Regular Expression...'" << lpub_endl;
        outstream << "# " << lpub_endl;
        outstream << "# The Regular Expression used is: ^(\\b.*[^\\s]\\b)(?:\\s)\\s+(.*)$" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# 1. Part ID:          LDraw Part Name                               (Required)" << lpub_endl;
        outstream << "# 2. Part Description: LDraw Part Description - for reference only   (Optional)" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# When adding a Part Description, be sure to replace double quotes \" with '." << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# ----------------------Do not delete above this line----------------------------------" << lpub_endl;
        outstream << "#" << lpub_endl;
        outstream << "# The generated list is a truncated set from the official and unofficial LDraw parts archive." << lpub_endl;
        outstream << "#" << lpub_endl;

        QByteArray Buffer;
        loadStickerParts(Buffer);
        QTextStream instream(Buffer);
        for (QString sLine = instream.readLine(); !sLine.isNull(); sLine = instream.readLine())
        {
            outstream << sLine << lpub_endl;
            counter++;
        }

        file.close();
        QString message = QString("Finished Writing Sticker Part Entries, Processed %1 lines in file [%2]")
                                   .arg(counter)
                                   .arg(file.fileName());
        if (Preferences::modeGUI){
            QMessageBox::information(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Sticker Parts"),message);
        } else {
            logNotice() << message;
        }
    }
    else
    {
        QString message = QString("Failed to open sticker parts file: %1:\n%2")
                                  .arg(file.fileName())
                                  .arg(file.errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR " - Sticker Parts"),message);
        } else {
            logError() << message;
        }
       return false;
    }
    return true;
}

bool StickerParts::overwriteFile(const QString &file)
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
