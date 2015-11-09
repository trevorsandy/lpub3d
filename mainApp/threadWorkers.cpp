/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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

#include <QFileInfo>
#include <QString>

#include <stdio.h>      //can del
#include <string.h>     //can del
#include <sys/stat.h>   //can del

#include "threadworkers.h"
#include "LDrawIni.h"
#include "lpub_preferences.h"
#include "step.h"
#include "paths.h"
#include "lpub.h"

#ifdef WIN32
#include <clocale>
#endif // WIN32

PartWorker::PartWorker(QObject *parent) : QObject(parent)
{}

/*
 * Process LDraw search directories part files.
 */

void PartWorker::processLDSearchDirParts(){

  if (ldPartsDirs.loadLDrawSearchDirs("")){

//      if(!_ldSearchPartsDirs.size() == 0){
//         _ldSearchPartsDirs.empty();
//        }

      QStringList ldSearchPartsDirs;

      StringList ldrawSearchDirs = ldPartsDirs.getLDrawSearchDirs();

      for (StringList::const_iterator it = ldrawSearchDirs.begin();
           it != ldrawSearchDirs.end(); it++)
        {
          const char *dir = it->c_str();

          QString partsDir = QString("%1").arg(dir);

          if (partsDir != ".")  {  //REMOVE AFTER TEST

              ldSearchPartsDirs << QDir::toNativeSeparators(QString("%1/").arg(partsDir));

              qDebug() << "LDRAW SEARCH DIR:  " << QString(dir) <<
                          "\nLDRAW PARTS DIR: " << partsDir;
            }

        }

      processLDSearchPartsArchive(ldSearchPartsDirs, "search directory", true);

    } else {
        qDebug() << QString(tr("Failed to load LDraw search directores"));
//      QMessageBox::critical(NULL, tr("LPub3D"), tr("Failed to load LDraw search directores"));
    }

//  emit ldSearchDirFinishedSig();

 }

/*
 * Create fade version of static colour part files.
 */
void PartWorker::processFadeColourParts()
{
/*  bool doFadeStep = (gui->page.meta.LPub.fadeStep.fadeStep.value() || Preferences::enableFadeStep);

  if (doFadeStep) {

    QStringList contents;

        emit progressBarInitSig();
        emit progressMessageSig("Parse Model and Parts Library");
        Paths::mkfadedirs();

        ldrawFile = gui->getLDrawFile();
        // porcess top-level submodels
        emit progressRangeSig(1, ldrawFile._subFileOrder.size());
        for (int i = 0; i < ldrawFile._subFileOrder.size(); i++) {
            QString subfileNameStr = ldrawFile._subFileOrder[i].toLower();
            contents = ldrawFile.contents(subfileNameStr);
            emit progressSetValueSig(i);
            //logDebug() << "00 PROCESSING SUBFILE: " << subfileNameStr;
            for (int i = 0; i < contents.size(); i++) {
                QString line = contents[i];
                QStringList tokens;
                split(line,tokens);
                if (tokens.size() == 15 && tokens[0] == "1") {
                    // check if colored part and create fade version if yes
                    QString fileNameStr  = tokens[tokens.size()-1];
                    if (FadeStepColorParts::getStaticColorPartPath(fileNameStr)){
                        //logInfo() << "01 SUBMIT COLOUR PART: " << QString("%1**%2").arg(tokens[tokens.size()-1]).arg(fileNameStr) << " Line: " << i;
                        createFadePartContent(QString("%1**%2").arg(tokens[tokens.size()-1]).arg(fileNameStr), i);
                    }
                }
            }
        }
        emit progressSetValueSig(ldrawFile._subFileOrder.size());

        createFadePartFiles();

        // Append fade parts to unofficial library for LeoCAD's consumption
        QString fadePartsDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/parts/fade/"));
        QString fadePrimitivesDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/p/fade/"));

        if(!_fadePartsDirs.size() == 0){
           _fadePartsDirs.empty();
          }

        _fadePartsDirs << fadePartsDir
                       << fadePrimitivesDir;

        processFadeColourPartsArchive("colour fade");

        emit messageSig(true,QString("Colour parts created and parts library updated successfully."));
        emit removeProgressStatusSig();

        emit fadeColourFinishedSig();

        qDebug() << "\nfinished Process Fade Colour Parts.";
    } */
}

void PartWorker::processLDSearchPartsArchive(const QStringList &ldSearchPartsDirs, const QString &comment = QString(""), bool silent){

  // Append fade parts to unofficial library for LeoCAD's consumption
  QFileInfo libFileInfo(Preferences::viewerLibFile);
  QString archiveFile = QDir::toNativeSeparators(QString("%1/%2").arg(libFileInfo.dir().path()).arg("ldrawunf.zip"));

  if (!silent)
    emit progressMessageSig(QString("Archiving %1 parts.").arg(comment));

  if(!ldSearchPartsDirs.size() == 0){

      if (!silent)
        emit progressRangeSig(1, ldSearchPartsDirs.size());

      for (int i = 0; i < ldSearchPartsDirs.size(); i++){

          if (!silent)
            emit progressSetValueSig(i);

          QDir foo = ldSearchPartsDirs[i];
          qDebug() << QString(tr("ARCHIVING %1").arg(foo.absolutePath()));

          if (! archiveLDSearchParts.Archive(archiveFile, foo.absolutePath(), QString("Append %1 parts").arg(comment))){
            qDebug() << QString(tr("Failed to archive %1 parts to \n %2")
                        .arg(comment)
                        .arg(ldSearchPartsDirs[i]));

//            QMessageBox::warning(NULL, tr("LPub3D"), tr("Failed to archive %1 parts to \n %2")
//                                                        .arg(comment)
//                                                        .arg(_ldSearchPartsDirs[i]));
            }
        }

      // Reload unofficial library parts into memory
      if (!g_App->mLibrary->ReloadUnoffLib()){
          qDebug() << QString(tr("Failed to reload unofficial parts library into memory."));
//          QMessageBox::warning(NULL, tr("LPub3D"), tr("Failed to reload unofficial parts library into memory."));
      }

      if (!silent) {
          emit progressSetValueSig(ldSearchPartsDirs.size());
          emit progressMessageSig(QString("Finished archiving %1 parts.").arg(comment));
        }

    } else {
      qDebug() << QString(tr("Failed to retrieve %1 parts directory.").arg(comment));
//     QMessageBox::warning(NULL, tr("LPub3D"), tr("Failed to retrieve %1 parts directory.").arg(comment));
    }
}


void PartWorker::processFadeColourPartsArchive(const QString &comment = QString(""), bool silent){
/*
  // Append fade parts to unofficial library for LeoCAD's consumption
  QFileInfo libFileInfo(Preferences::viewerLibFile);
  QString archiveFile = QDir::toNativeSeparators(QString("%1/%2").arg(libFileInfo.dir().path()).arg("ldrawunf.zip"));

  if (!silent)
    emit progressMessageSig(QString("Archiving %1 parts.").arg(comment));

  if(!_fadePartsDirs.size() == 0){

      if (!silent)
        emit progressRangeSig(1, _fadePartsDirs.size());

      for (int i = 0; i < _fadePartsDirs.size(); i++){

          if (!silent)
            emit progressSetValueSig(i);

          if (! archiveFadeColourParts.Archive(archiveFile, _fadePartsDirs[i], QString("Append %1 parts").arg(comment))){
            qDebug() << QString(tr("Failed to archive %1 parts to \n %2")
                        .arg(comment)
                        .arg(_fadePartsDirs[i]));

            }
        }

      // Reload unofficial library parts into memory
      if (!g_App->mLibrary->ReloadUnoffLib()){
          qDebug() << QString(tr("Failed to reload unofficial parts library into memory."));
      }

      if (!silent) {
          emit progressSetValueSig(_fadePartsDirs.size());
          emit progressMessageSig(QString("Finished archiving %1 parts.").arg(comment));
        }

    } else {
      qDebug() << QString(tr("Failed to retrieve %1 parts directory.").arg(comment));
    } */
}

void PartWorker::createFadePartContent(const QString &fileNameComboStr, const int &lineNum){

    QString fileNameParts = fileNameComboStr;
    QString fileNameStr = fileNameParts.section("**",0,-2);
    QString fileAbsPath = fileNameParts.section("**",-1,-1);

    QStringList inputContents;
    retrieveContent(inputContents, fileAbsPath, fileNameStr, lineNum);

    for (int i = 0; i < inputContents.size(); i++){
        QString line = inputContents[i];
        QStringList tokens;

        split(line,tokens);
        if (tokens.size() == 15 && tokens[0] == "1") {
            QString childFileNameStr  = tokens[tokens.size()-1];
            if (FadeStepColorParts::getStaticColorPartPath(childFileNameStr)){
                //logInfo() << "03 SUBMIT CHILD COLOUR PART: " << childFileNameStr << " Colour: " << tokens[1];
                createFadePartContent(QString("%1**%2").arg(tokens[tokens.size()-1]).arg(childFileNameStr), i);
            }
        }
    }
}

/*
 * Create fade version of static colour part files.
 */
void PartWorker::retrieveContent(
        QStringList         &inputContents,
        const QString       &fileAbsPathStr,
        const QString       &fileNameStr,
        const int           &lineNum) {

    // Initialize file to be found
    QFileInfo fileInfo(fileAbsPathStr);

    // identify part type
    int partType = -1;
    QRegExp rxPart("\\S*(\\/ldraw\\/parts\\/)[^\\n]*");                 // partType=0
    QRegExp rxUPart("\\S*(\\/unofficial\\/parts\\/)[^\\n]*");           // partType=1
    QRegExp rxSPart("\\S*(\\/parts\\/s\\/)[^\\n]*");                    // partType=2
    QRegExp rxUSPart("\\S*(\\/unofficial\\/parts\\/s\\/)[^\\n]*");      // partType=3
    QRegExp rxPPart("\\S*(\\/p\\/)[^\\n]*/i");                          // partType=4
    QRegExp rxUPPart("\\S*(\\/unofficial\\/p\\/)[^\\n]*");              // partType=5
    QRegExp rxP8Part("\\S*(\\/p\\/8\\/)[^\\n]*/i");                     // partType=6
    QRegExp rxUP8Part("\\S*(\\/unofficial\\/p\\/8\\/)[^\\n]*");         // partType=7
    QRegExp rxP48Part("\\S*(\\/p\\/48\\/)[^\\n]*/i");                   // partType=8
    QRegExp rxUP48Part("\\S*(\\/unofficial\\/p\\/48\\/)[^\\n]*");       // partType=9
    if (fileAbsPathStr.contains(rxP48Part)){
        partType=8;
    } else if (fileAbsPathStr.contains(rxP8Part)){
        partType=6;
    } else if (fileAbsPathStr.contains(rxPPart)){
        partType=4;
    } else if (fileAbsPathStr.contains(rxSPart)){
        partType=2;
        //logTrace()  << " SUB PARTS Type: " << partType << "Path: " << fileAbsPathStr;
    } else if (fileAbsPathStr.contains(rxPart)){
        partType=0;
        //logTrace()  << " PARTS Type: " << partType << "Path: " << fileAbsPathStr;
    } else if (fileAbsPathStr.contains(rxUP48Part)){
        partType=9;
    } else if (fileAbsPathStr.contains(rxUP8Part)){
        partType=7;
    } else if (fileAbsPathStr.contains(rxUPPart)){
        partType=5;
    } else if (fileAbsPathStr.contains(rxUSPart)){
        partType=3;
        //logTrace()  << " UNOFFICIAL SUB PARTS Type: " << partType << "Path: " << fileAbsPathStr;
    }else if (fileAbsPathStr.contains(rxUPart)){
        partType=1;
        //logTrace()  << " UNOFFICIAL PARTS Type: " << partType << "Path: " << fileAbsPathStr;
    }

    if(fileInfo.exists()){
        //logNotice() << "Part FOUND: " << fileInfo.absoluteFilePath();
        QFile file(fileInfo.absoluteFilePath());
        QString fileErrorString;
        fileErrorString += file.errorString();
        if (file.open(QFile::ReadOnly | QFile::Text)){
            // read content
            QTextStream in(&file);
            while ( ! in.atEnd()) {
                QString line = in.readLine(0);
                _partFileContents << line;
            }
            // populate PartWorker
            //logNotice() << "02 INSERT PART CONTENTS " << fileNameStr << ", LineNum: " << lineNum << ", partType: " << partType ;
            insert(_partFileContents, fileNameStr, lineNum, partType);
            inputContents = _partFileContents;
            _partFileContents.clear();

        } else {
            //logError() << QString("%1%2%3").arg("Failed to OPEN Part file: ").arg(fileInfo.fileName()).arg(fileErrorString);
            QMessageBox::warning(NULL,QMessageBox::tr("LPub3D"),
                                 QMessageBox::tr("Failed to OPEN Part file :%1\n%2")
                                 .arg(fileInfo.fileName())
                                 .arg(fileErrorString));
            return;
        }
    }    
}

void PartWorker::createFadePartFiles(){

    int maxValue            = _partList.size();
    emit progressResetSig();
    emit progressMessageSig("Creating Fade Colour Parts");
    emit progressRangeSig(1, maxValue);

    QString materialColor  ="16";  // Internal Common Material Colour (main)
    QString edgeColor      ="24";  // Internal Common Material Color (edge)

    QStringList fadePartContent;
    QString fadePartFile;

    for(int part = 0; part < _partList.size(); part++){

        emit progressSetValueSig(part);
        QMap<QString, ColourPart>::iterator cp = _colourParts.find(_partList[part]);

        if(cp != _colourParts.end()){

            // prepare absoluteFilePath
            QDir fadePartDir;
            switch (cp.value()._partType)
            {
            case OFFICIAL_PARTS:
                fadePartDir = QDir::toNativeSeparators(Paths::fadeDir);
                break;
            case UNOFFICIAL_PARTS:
                fadePartDir = QDir::toNativeSeparators(Paths::fadeDir);
                break;
            case OFFICIAL_SUB_PARTS:
                fadePartDir = QDir::toNativeSeparators(Paths::fadeSDir);
                break;
            case UNOFFICIAL_SUP_PARTS:
                fadePartDir = QDir::toNativeSeparators(Paths::fadeSDir);
                break;
            case OFFICIAL_PRIMITIVES:
                fadePartDir = QDir::toNativeSeparators(Paths::fadePDir);
                break;
            case UNOFFICIAL_PRIMITIVES:
                fadePartDir = QDir::toNativeSeparators(Paths::fadePDir);
                break;
            case OFFICIAL_PRIMITIVES_8:
                fadePartDir = QDir::toNativeSeparators(Paths::fadeP8Dir);
                break;
            case UNOFFICIAL_PRIMITIVES_8:
                fadePartDir = QDir::toNativeSeparators(Paths::fadeP8Dir);
                break;
            case OFFICIAL_PRIMITIVES_48:
                fadePartDir = QDir::toNativeSeparators(Paths::fadeP48Dir);
                break;
            case UNOFFICIAL_PRIMITIVES_48:
                fadePartDir = QDir::toNativeSeparators(Paths::fadeP48Dir);
                break;
            }
            QFileInfo fadeFileInfo = cp.value()._fileNameStr;
            //logTrace() << "A PART CONTENT FILENAME: " << fadeFileInfo.absoluteFilePath();

            QFileInfo fadeStepColorPartFileInfo(fadePartDir, fadeFileInfo.fileName().replace(".dat","-fade.dat"));
            // check if part exist and if yes return
            if (fadeStepColorPartFileInfo.exists()){
                //logError() << "PART EXISTS: " << fadeStepColorPartFileInfo.absoluteFilePath();
                continue;
            }

            fadePartFile = fadeStepColorPartFileInfo.absoluteFilePath();
            // process part contents
            for (int i = 0; i < cp.value()._contents.size(); i++) {
                QString line =  cp.value()._contents[i];
                QStringList tokens;
                QString fileNameStr;

                split(line,tokens);
                if (tokens.size() == 15 && tokens[0] == "1") {
                    fileNameStr = tokens[tokens.size()-1];
                    // check PartWorker contents if line part is an identified colour part and rename accordingly
                    QString uniqueID = QString("%1_%2").arg(QString::number(i)).arg(fileNameStr.toLower());
                    QMap<QString, ColourPart>::iterator cpc = _colourParts.find(uniqueID);                    
                    //logTrace() << "B. COMPARE CHILD - LINE NUM: "<< i << " PART ID: " << fileNameStr << " AND CONTENT UID:" << uniqueID;

                    if (cpc != _colourParts.end()){
                        if (cpc.value()._fileNameStr == fileNameStr){

                            fileNameStr = "fade\\" + fileNameStr.replace(".dat","-fade.dat");
                        }
                    }
                    tokens[tokens.size()-1] = fileNameStr;
                }
                // check if coloured line and set to 16 if yes
                if((((tokens.size() == 15 && tokens[0] == "1")  ||
                     (tokens.size() == 8  && tokens[0] == "2")  ||
                     (tokens.size() == 11 && tokens[0] == "3")  ||
                     (tokens.size() == 14 && tokens[0] == "4")  ||
                     (tokens.size() == 14 && tokens[0] == "5")) &&
                     (tokens[1] != materialColor) 				&&
                     (tokens[1] != edgeColor))){

                    //QString oldColour(tokens[1]);       //logging only
                    tokens[1] = materialColor;
                    //logTrace() << "D. CHANGE CHILD PART COLOUR: " << fileNameStr << " NewColour: " << tokens[1] << " OldColour: " << oldColour;
                }
                line = tokens.join(" ");
                fadePartContent << line;
            }
        }
        //logTrace() << "04 SAVE COLOUR PART: " << fadePartFile;
        saveFadeFile(fadePartFile, fadePartContent);
        fadePartContent.clear();
    }
    emit progressSetValueSig(maxValue);
}


/*
     * Write faded part files to fade directory.
     */
bool PartWorker::saveFadeFile(
        const QString     &fileName,
        const QStringList &fadePartContent) {

    QFile file(fileName);
    if ( ! file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(NULL,QMessageBox::tr("LPub3D"),
                             QMessageBox::tr("Failed to open %1 for writing: %2")
                             .arg(fileName) .arg(file.errorString()));
                return false;
    } else {
        QTextStream out(&file);
        for (int i = 0; i < fadePartContent.size(); i++) {
            out << fadePartContent[i] << endl;
        }
        file.close();
        //logWarn() << "05 WRITE PART TO DISC: " << fileName << "\n";
        return true;
    }
}


void PartWorker::insert(
        const QStringList   &contents,
        const QString       &fileNameStr,
        const int           &lineNum,
        const int           &partType){

    QString uniqueID = QString("%1_%2").arg(QString::number(lineNum)).arg(fileNameStr.toLower());
    QMap<QString, ColourPart>::iterator i = _colourParts.find(uniqueID);

    if (i != _colourParts.end()){
        _colourParts.erase(i);
    }
    ColourPart colourPartEntry(contents, fileNameStr, lineNum, partType);
    _colourParts.insert(uniqueID, colourPartEntry);
    _partList << uniqueID;
    //logNotice() << "02 INSERT PART CONTENTS - UID: " << uniqueID  <<  " partType: " << partType;
}

int PartWorker::size(const QString &fileNameStr, const int &lineNum){

  QString uniqueID = QString("%1_%2").arg(QString::number(lineNum)).arg(fileNameStr.toLower());
  QMap<QString, ColourPart>::iterator i = _colourParts.find(uniqueID);

  int mySize;
  if (i ==  _colourParts.end()) {
    mySize = 0;
  } else {
    mySize = i.value()._contents.size();
  }
  return mySize;
}

QStringList PartWorker::contents(const QString &fileNameStr, const int &lineNum){

    QString uniqueID = QString("%1_%2").arg(QString::number(lineNum)).arg(fileNameStr.toLower());
    QMap<QString, ColourPart>::iterator i = _colourParts.find(uniqueID);

  if (i != _colourParts.end()) {
    return i.value()._contents;
  } else {
    return _emptyList;
  }
}

void PartWorker::remove(const QString &fileNameStr, const int &lineNum)
{
    QString uniqueID = QString("%1_%2").arg(QString::number(lineNum)).arg(fileNameStr.toLower());
    QMap<QString, ColourPart>::iterator i = _colourParts.find(uniqueID);

  if (i != _colourParts.end()) {
    i.value()._contents.removeAll(uniqueID);
    _partList.removeAll(uniqueID);
    //logWarn() << "REMOVE COLOUR PART: " << uniqueID << " from contents and _partList";
  }
}

void PartWorker::empty()
{
   _colourParts.clear();
}

bool PartWorker::endThreadEventLoopNow(){

    //logTrace() << "endThreadEventLoopNow: " << _endThreadNowRequested;
    return _endThreadNowRequested;
}

void PartWorker::requestEndThreadNow(){

    //logTrace() << "requestEndThreadNow: " << _endThreadNowRequested;
    _endThreadNowRequested = true;
    emit requestFinishSig();
}

ColourPart::ColourPart(
        const QStringList   &contents,
        const QString       &fileNameStr,
        const int           &lineNum,
        const int           &partType)
{
    _contents         = contents,
    _fileNameStr      = fileNameStr;
    _lineNum          = lineNum;
    _partType         = partType;
}


ColourPartListWorker::ColourPartListWorker(QObject *parent) : QObject(parent)
{_endThreadNowRequested = false;}

/*
 * build colour part directory
 *
 */

void ColourPartListWorker::scanDir()
{
    QString filePath = Preferences::ldrawPath;

//    QStringList partTypeDir;
//    partTypeDir << QDir::toNativeSeparators(filePath + "/Unofficial/parts/");

    QStringList partTypeDir;
    partTypeDir << QDir::toNativeSeparators(filePath + "/parts/");
    partTypeDir << QDir::toNativeSeparators(filePath + "/Unofficial/parts/");
    partTypeDir << QDir::toNativeSeparators(filePath + "/p/");
    partTypeDir << QDir::toNativeSeparators(filePath + "/Unofficial/p/");

    //logDebug() << "Launch Direcory Listing.";
    _timer.start();
    fileHeader(FADESTEP_FILE_HEADER);
    emit progressBarInitSig();
    for (int d = 0; d < partTypeDir.size() && !_endThreadNowRequested; d++) {
        QDir dir(partTypeDir[d]);
        fileHeader(d);
        scanDir(dir);
    }
    fileHeader(FADESTEP_COLOUR_CHILDREN_HEADER);
    colourChildren();
    writeFile();
    int secs = _timer.elapsed() / 1000;
    int mins = (secs / 60) % 60;
    int hours = (secs / 3600);
    secs = secs % 60;
    QString time = QString("%1:%2:%3")
    .arg(hours, 2, 10, QLatin1Char('0'))
    .arg(mins,  2, 10, QLatin1Char('0'))
    .arg(secs,  2, 10, QLatin1Char('0'));
    //logNotice()<< QString("COLOUR PARTS FILE WRITTEN WITH %1 LINES, ELAPSED TIME %2").arg(QString::number(_cpLines)).arg(time);
    emit messageSig(true,QString("Colour parts list successfully created with %1 entries, elapsed time %2.").arg(QString::number(_cpLines)).arg(time));
    emit removeProgressStatusSig();
    emit colourPartListFinishedSig();
}

void ColourPartListWorker::scanDir(QDir dir){

    emit progressMessageSig("Generating Colour Part List");
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList nameFilters;
    nameFilters << "*.dat" << "*.DAT";
    dir.setNameFilters(nameFilters);

    int dirFiles = 0;
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while(it.hasNext() && !_endThreadNowRequested) {
        it.next();
        dirFiles++;
    }

    emit progressRangeSig(1, dirFiles);
    QString dirFadeParts = QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/parts/fade").replace("\\","/").toLower();
    QString dirFadePrims = QString("%1/%2").arg(Preferences::ldrawPath).arg("p/fade").replace("\\","/").toLower();

    int i = 1;
    QDirIterator itDir(dir, QDirIterator::Subdirectories);
    while(itDir.hasNext() && !_endThreadNowRequested) {

        emit progressSetValueSig(i++);
        QString dirNameParts = itDir.next().toLower();
        QString dirName = dirNameParts.section("/",0,-2);

        if (dirName == dirFadeParts      ||
            dirName == dirFadeParts+"/s" ||
            dirName == dirFadePrims      ||
            dirName == dirFadePrims+"/8" ||
            dirName == dirFadePrims+"/48")
            continue;

        //logInfo() << "Sending FILE to getParts: " << itDir.fileInfo().absoluteFilePath();
        getParts(itDir.fileInfo());
        //logTrace() << "Sending FILE to buildList: " << itDir.fileInfo().absoluteFilePath();
        buildList(itDir.fileInfo());

    }
    emit progressSetValueSig(dirFiles);
}

void ColourPartListWorker::getParts(const QFileInfo &fileInfo)
{
    if(fileInfo.exists()){
        //logInfo() << "Part FOUND: " << fileInfo.absoluteFilePath();
        QFile file(fileInfo.absoluteFilePath());
        QString fileErrorString;
        fileErrorString += file.errorString();
        if (file.open(QFile::ReadOnly | QFile::Text)){
            // read content
            QTextStream in(&file);
            while ( ! in.atEnd() && !_endThreadNowRequested) {
                QString line = in.readLine(0);
                _partFileContents << line.toLower();
            }
            // add content to ColourParts map
            insert(_partFileContents, fileInfo.absoluteFilePath(), -1, -1);

        } else {
            //logError() << QString("Failed to OPEN Part file :%1\n%2").arg(fileInfo.fileName()).arg(fileErrorString);
            emit messageSig(false,QString("Failed to OPEN Part file :%1\n%2").arg(fileInfo.fileName()).arg(fileErrorString));
            return;
        }
    }
}


/*
 * parse colour part file to determine childre parts with static colour.
 */
void ColourPartListWorker::buildList(const QFileInfo &fileInfo){

    QString materialColor  ="16";  // Internal Common Material Colour (main)
    QString edgeColor      ="24";  // Internal Common Material Color (edge)
    QString fileEntry;
    QString fileName;
    QString colour;
    bool hasColour = false;

    for (int i = 0; i < _partFileContents.size() && !_endThreadNowRequested; i++){
        QString line = _partFileContents[i];
        QStringList tokens;

        //logInfo() << " File contents CHECK: " << line;
        split(line,tokens);
        if (tokens.size() == 3 && line.contains("Name:", Qt::CaseInsensitive))
            fileName  = tokens[tokens.size()-1];

        //      if (tokens.size() == 15 && tokens[0] == "1") {
         if((tokens.size() == 15 && tokens[0] == "1") ||
            (tokens.size() == 8  && tokens[0] == "2") ||
            (tokens.size() == 11 && tokens[0] == "3") ||
            (tokens.size() == 14 && tokens[0] == "4") ||
            (tokens.size() == 14 && tokens[0] == "5")) {
            colour = tokens[1];
            if (colour == edgeColor || colour == materialColor){
                continue;

            } else {
                hasColour = true;
                //logInfo() << " File contents VERIFY: " << line << " COLOUR: " << colour;
                if (fileName.isEmpty()){
                    fileName = fileInfo.fileName();
                    emit messageSig(false,QString("Part: %1 \nhas no 'Name:' attribute. Using file name instead.\n"
                                                  "You may want to update the part content and fade colour parts list.")
                                    .arg(fileInfo.absoluteFilePath()));
                }
                fileEntry = QString("%1\t\t%2\t\t%3").arg(fileName).arg(fileInfo.absoluteFilePath()).arg(_partFileContents[0].remove(0,2));
                remove(fileInfo.absoluteFilePath(), -1);
                break;
            }
        } // token size
    }
    _partFileContents.clear();
    if(hasColour) {
        _fadeStepColourParts  << fileEntry.toLower();
        //logWarn() << "ADD COLOUR PART: " << fileEntry << " Colour: " << colour;
    }
}

void ColourPartListWorker::colourChildren(){

    emit progressResetSig();
    emit progressMessageSig("Generating Child Colour Part List");
    emit progressRangeSig(1, _partList.size());

    for(int part = 0; part < _partList.size() && !_endThreadNowRequested; part++){
        QString     parentFileNameStr;
        bool        gotoMainLoop = false;

        emit progressSetValueSig(part);
        QMap<QString, ColourPart>::iterator ap = _colourParts.find(_partList[part]);

        if(ap != _colourParts.end()){

            QFileInfo fadeFileInfo(ap.value()._fileNameStr);

            // process partWorker content
            for (int i = 0; i < ap.value()._contents.size() && !gotoMainLoop && !_endThreadNowRequested; i++) {
                QString line =  ap.value()._contents[i];
                QStringList tokens;

                split(line,tokens);
                if (tokens.size() == 3 && line.contains("Name:", Qt::CaseInsensitive))
                    parentFileNameStr = tokens[tokens.size()-1];                            // short name of parent part

                if (tokens.size() == 15 && tokens[0] == "1") {

                    QString childFileNameStr = tokens[tokens.size()-1];                     // child part name in parent part
                    // match partWorker file with fadeStepColourParts
                    for (int j = 0; j < _fadeStepColourParts.size() && !gotoMainLoop && !_endThreadNowRequested; ++j){

                        if (_fadeStepColourParts.at(j).contains(childFileNameStr) && _fadeStepColourParts.at(j).contains(QRegExp("\\b"+childFileNameStr.replace("\\","\\\\")+"[^\n]*"))){
                           //logTrace() << " CHILD FILE: " << childFileNameStr << " PARENT FILE: " << parentFileNameStr << " COLOUR FILE: "<<  _fadeStepColourParts.at(j);
                           QString fileEntry;
                           fileEntry = QString("%1\t\t%2\t\t%3").arg(parentFileNameStr).arg(fadeFileInfo.absoluteFilePath()).arg(ap.value()._contents[0].remove(0,2));
                           _fadeStepColourParts  << fileEntry.toLower();
                           //logInfo() << "ADD CHILD COLOUR PART: " << fileEntry;
                           gotoMainLoop = true;
                        }
                    }
                }
            }
        }
    }
    emit progressSetValueSig(_partList.size());
}


void ColourPartListWorker::writeFile(){

    emit progressResetSig();
    emit progressMessageSig("Writing Colour Part List");
    emit progressRangeSig(1, _fadeStepColourParts.size());
    // change to insttall path
    QFileInfo colourFileList(Preferences::fadeStepColorPartsFile);
    if (! _fadeStepColourParts.empty())
    {
        _cpLines = 0;
        QFile file(colourFileList.absoluteFilePath());
        if ( ! file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
            //logError() << QString("Failed to OPEN colourFileList %1 for writing:\n%2").arg(file.fileName()).arg(file.errorString());
            emit messageSig(false,QString("Failed to OPEN colourFileList %1 for writing:\n%2").arg(file.fileName()).arg(file.errorString()));
            return;
        }
        QTextStream out(&file);
        for (int i = 0; i < _fadeStepColourParts.size() && !_endThreadNowRequested; i++) {
            emit progressSetValueSig(i);
            QString cpLine = _fadeStepColourParts[i];
            out << cpLine << endl;
            _cpLines++;
        }
        file.close();
        emit progressSetValueSig(_fadeStepColourParts.size());
    }
}

void ColourPartListWorker::insert(
        const QStringList   &contents,
        const QString       &fileNameStr,
        const int           &lineNum,
        const int           &partType){

    QString uniqueID = QString("%1***%2").arg(QString::number(lineNum)).arg(fileNameStr.toLower());
    QMap<QString, ColourPart>::iterator i = _colourParts.find(uniqueID);

    if (i != _colourParts.end()){
        _colourParts.erase(i);
    }
    ColourPart colourPartEntry(contents, fileNameStr, lineNum, partType);
    _colourParts.insert(uniqueID, colourPartEntry);
    _partList << uniqueID;
    //logNotice() << "02 INSERT PART CONTENTS - UID: " << uniqueID  <<  " partType: " << partType;
}

void ColourPartListWorker::remove(const QString &fileNameStr, const int &lineNum)
{
    QString uniqueID = QString("%1***%2").arg(QString::number(lineNum)).arg(fileNameStr.toLower());
    QMap<QString, ColourPart>::iterator i = _colourParts.find(uniqueID);

  if (i != _colourParts.end()) {
    i.value()._contents.removeAll(uniqueID);
    _partList.removeAll(uniqueID);
    //logWarn() << "REMOVE COLOUR PART: " << uniqueID << " from contents and _partList";
  }
}

void ColourPartListWorker::fileHeader(const int &option){

    static const QString fmtDateTime("MM-dd-yyyy hh:mm:ss");

    switch(option)
    {
    case FADESTEP_FILE_HEADER:
        _fadeStepColourParts  << QString("# File: fadeStepColorParts.lst  Generated on: %1").arg(QDateTime::currentDateTime().toString(fmtDateTime));
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# This list captures the static coloured parts (or their components) to support part fade";
        _fadeStepColourParts  << "# Parts on this list are copied to their respective fade directory and modified.";
        _fadeStepColourParts  << "# Colour codes are replaced with colour code 16";
        _fadeStepColourParts  << "# Copied files are appended with '-fade', for example, ...\\Fade\\99499-fade.dat";
        _fadeStepColourParts  << "# Part identifiers with spaces will not be properly recoginzed.";
        _fadeStepColourParts  << "# This file is automatically generated from Configuration=>Generate Fade List";
        _fadeStepColourParts  << "# However, it can also be modified manually from Configuration=>Edit Colour Parts List";
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# Part ID (LDraw Name:)		Part Absolute File Path		Part Description (LDraw Description) - for reference only";
        _fadeStepColourParts  << "";
        break;
    case OFFICIAL_PARTS_HEADER:
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# Official Parts";
        _fadeStepColourParts  << "";
        break;
    case UNOFFICIAL_PARTS_HEADER:
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# Unofficial Parts";
        _fadeStepColourParts  << "";
        break;
    case OFFICIAL_PRIMITIVES_HEADER:
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# Official Primitives";
        _fadeStepColourParts  << "";
        break;
    case UNOFFICIAL_PRIMITIVES_HEADER:
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# Unofficial Primitives";
        _fadeStepColourParts  << "";
        break;
    case FADESTEP_COLOUR_CHILDREN_HEADER:
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# Parts with no colour but include parts that are coloured";
        _fadeStepColourParts  << "";
        break;
    }
}

bool ColourPartListWorker::endThreadEventLoopNow(){

    //logTrace() << "endThreadEventLoopNow: " << _endThreadNowRequested;
    return _endThreadNowRequested;
}

void ColourPartListWorker::requestEndThreadNow(){

    //logTrace() << "requestEndThreadNow: " << _endThreadNowRequested;
    _endThreadNowRequested = true;
    emit requestFinishSig();
}












