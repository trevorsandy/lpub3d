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
{
  _doFadeStep = false;
  _doReload   = false;
  _doLDSearch = false;
}

/*
 * Process LDraw search directories part files.
 */

void PartWorker::processLDSearchDirParts(){

  if (ldPartsDirs.loadLDrawSearchDirs("")){

      setDoLDSearch(true);

      QString offPartsDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("PARTS"));
      QString offPrimsDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("P"));
      QString unoffPartsDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/parts"));
      QString unoffPrimsDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/p"));

      QStringList ldSearchPartsDirs;

      StringList ldrawSearchDirs = ldPartsDirs.getLDrawSearchDirs();

      qDebug() << "";

      for (StringList::const_iterator it = ldrawSearchDirs.begin();
           it != ldrawSearchDirs.end(); it++)
        {
          const char *dir = it->c_str();

          QString partsDir = QString(dir);

          if (
              partsDir.toLower() == "." ||
              partsDir.toLower().toLower().contains(offPartsDir.toLower()) ||
              partsDir.toLower().toLower().contains(offPrimsDir.toLower()) ||
              partsDir.toLower().toLower().contains(unoffPartsDir.toLower()) ||
              partsDir.toLower().toLower().contains(unoffPrimsDir.toLower())
              ){

              qDebug() << "<-EXCLUDE LDRAW SEARCH DIR: " << partsDir;
              continue;

            } else {

              ldSearchPartsDirs << QDir::toNativeSeparators(partsDir);
              qDebug() << "->INSERT LDRAW SEARCH DIR: " << partsDir;

            }
        }

      processPartsArchive(ldSearchPartsDirs, "search directory");

    } else {

      qDebug() << QString(tr("Failed to load search directories - ldPartsDirs.loadLDrawSearchDirs() returned false."));
    }

//  emit ldSearchDirFinishedSig();

  qDebug() << "\nFinished Processing Search Directory Parts.";

 }

/*
 * Create fade version of static colour part files.
 */
void PartWorker::processFadeColourParts()
{
  setDoFadeStep((gui->page.meta.LPub.fadeStep.fadeStep.value() || Preferences::enableFadeStep));

  if (doFadeStep()) {

      QStringList fadePartsDirs;
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
                        logInfo() << "01 SUBMIT COLOUR PART: " << QString("%1**%2").arg(tokens[tokens.size()-1]).arg(fileNameStr) << " Line: " << i ;
                        //tokens[tokens.size()-1] = part name
                        //fileNameStr             = absolute path file name

                        // create incremented number id for each directory
                        createFadePartContent(QString("%1**%2").arg(tokens[tokens.size()-1]).arg(fileNameStr));
                    }
                }
            }
        }
        emit progressSetValueSig(ldrawFile._subFileOrder.size());

        createFadePartFiles();

        // Append fade parts to unofficial library for LeoCAD's consumption
        QString fadePartsDir = Paths::fadePartDir;
        QString fadePrimitivesDir = Paths::fadePrimDir;

        if(!fadePartsDirs.size() == 0){
           fadePartsDirs.empty();
          }

        fadePartsDirs << fadePartsDir
                      << fadePrimitivesDir;

        processPartsArchive(fadePartsDirs, "colour fade");

        emit messageSig(true,QString("Colour parts created and parts library updated successfully."));
        emit removeProgressStatusSig();
        emit fadeColourFinishedSig();

        qDebug() << "\nfinished Process Fade Colour Parts.";
    }
}


void PartWorker::createFadePartContent(const QString &fileNameComboStr){

    QString fileNameParts = fileNameComboStr;
    QString fileNameStr   = fileNameParts.section("**",0,-2);
    QString fileAbsPath   = fileNameParts.section("**",-1,-1);

    QStringList inputContents;
    retrieveContent(inputContents, fileAbsPath, fileNameStr.toLower());

    for (int i = 0; i < inputContents.size(); i++){
        QString line = inputContents[i];
        QStringList tokens;

        split(line,tokens);
        if (tokens.size() == 15 && tokens[0] == "1") {
            QString childFileNameStr  = tokens[tokens.size()-1];
            if (FadeStepColorParts::getStaticColorPartPath(childFileNameStr)){
                //logInfo() << "03 SUBMIT CHILD COLOUR PART: " << childFileNameStr << " Colour: " << tokens[1];
                createFadePartContent(QString("%1**%2").arg(tokens[tokens.size()-1]).arg(childFileNameStr));
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
        const QString       &fileNameStr) {

    // Initialize file to be found
    QFileInfo fileInfo(fileAbsPathStr);

    // identify part type
    int partType = -1;
    QRegExp rxPart("\\S*(\\/parts\\/)[^\\n]*");                    // partType=0
    QRegExp rxSubPart("\\S*(\\/parts\\/s\\/)[^\\n]*");             // partType=1
    QRegExp rxPrimPart("\\S*(\\/p\\/)[^\\n]*/i");                  // partType=2
    QRegExp rxPrim8Part("\\S*(\\/p\\/8\\/)[^\\n]*/i");             // partType=3
    QRegExp rxPrim48Part("\\S*(\\/p\\/48\\/)[^\\n]*/i");           // partType=4
    if (fileAbsPathStr.contains(rxPrim48Part)){
        partType=LD_PRIMITIVES_48;
    } else if (fileAbsPathStr.contains(rxPrim8Part)){
        partType=LD_PRIMITIVES_8;
    } else if (fileAbsPathStr.contains(rxPrimPart)){
        partType=LD_PRIMITIVES;
    } else if (fileAbsPathStr.contains(rxSubPart)){
        partType=LD_SUB_PARTS;
        //logTrace()  << " SUB PARTS Type: " << partType << "Path: " << fileAbsPathStr;
    } else if (fileAbsPathStr.contains(rxPart)){
        partType=LD_PARTS;
        //logTrace()  << " PARTS Type: " << partType << "Path: " << fileAbsPathStr;
    } else
        partType=LD_PARTS;

    if(fileInfo.exists() && ! partAlreadyInList(fileNameStr)){
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
            insert(_partFileContents, fileNameStr, partType);
            inputContents = _partFileContents;
            _partFileContents.clear();

        } else {

            emit messageSig(true,QString("Failed to OPEN Part file :%1\n%2").arg(fileInfo.fileName()).arg(fileErrorString));
//            logError() << QString("Failed to OPEN Part file :%1\n%2").arg(fileInfo.fileName()).arg(fileErrorString);

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

            // prepare absoluteFilePath for fade file
            QDir fadePartDirPath;
            switch (cp.value()._partType)
            {
            case LD_PARTS:
                fadePartDirPath = Paths::fadePartDir;
                break;
            case LD_SUB_PARTS:
                fadePartDirPath = Paths::fadeSubDir;
                break;
            case LD_PRIMITIVES:
                fadePartDirPath = Paths::fadePrimDir;
                break;
            case LD_PRIMITIVES_8:
                fadePartDirPath = Paths::fadePrim8Dir;
                break;
            case LD_PRIMITIVES_48:
                fadePartDirPath = Paths::fadePrim48Dir;
                break;
            }
            QFileInfo fadeFileInfo = cp.value()._fileNameStr;

            QFileInfo fadeStepColorPartFileInfo(fadePartDirPath,fadeFileInfo.fileName().replace(".dat","-fade.dat"));
            logTrace() << "A PART CONTENT ABSOLUTE FILEPATH: " << fadeStepColorPartFileInfo.absoluteFilePath();

            // check if part exist and if yes return
            if (fadeStepColorPartFileInfo.exists()){
                logInfo() << "PART ALREADY EXISTS: " << fadeStepColorPartFileInfo.absoluteFilePath();
                continue;
            }
;
            fadePartFile = fadeStepColorPartFileInfo.absoluteFilePath();
            // process fade part contents
            for (int i = 0; i < cp.value()._contents.size(); i++) {
                QString line =  cp.value()._contents[i];
                QStringList tokens;
                QString fileNameStr;

                split(line,tokens);
                if (tokens.size() == 15 && tokens[0] == "1") {
                    fileNameStr = tokens[tokens.size()-1];
                    // check PartWorker contents if line part is an identified colour part and rename accordingly
                    QMap<QString, ColourPart>::iterator cpc = _colourParts.find(fileNameStr.toLower());
//                    logTrace() << "B. COMPARE CHILD - LINE NUM: "<< i << " PART ID: " << fileNameStr << " AND CONTENT UID:" << fileNameStr;

                    if (cpc != _colourParts.end()){
                        if (cpc.value()._fileNameStr == fileNameStr.toLower()){

                            fileNameStr = fileNameStr.replace(".dat","-fade.dat");
//                            fileNameStr = "fade\\" + fileNameStr.replace(".dat","-fade.dat");
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
            //logTrace() << "04 SAVE COLOUR PART: " << fadePartFile;
            saveFadeFile(fadePartFile, fadePartContent);
            fadePartContent.clear();
        }

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
    if ( ! file.open(QFile::WriteOnly | QFile::Text)) {
        emit messageSig(true,QString("Failed to open %1 for writing: %2").arg(fileName).arg(file.errorString()));
//       logError() << QString("Failed to open %1 for writing: %2").arg(fileName).arg(file.errorString());

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
        const int           &partType){

    bool partErased = false;
    QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

    if (i != _colourParts.end()){
        _colourParts.erase(i);
        partErased = true;
        logError() << "PART ALREADY IN LIST - PART ERASED: " << fileNameStr;
    }

    ColourPart colourPartEntry(contents, fileNameStr, partType);
    _colourParts.insert(fileNameStr, colourPartEntry);

    if (! partErased)
      _partList << fileNameStr;

    logNotice() << "02 INSERT (COLOUR PART ENTRY) - UID: " << fileNameStr  <<  " fileNameStr: " << fileNameStr <<  " partType: " << partType;
}

int PartWorker::size(const QString &fileNameStr){

  QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

  int mySize;
  if (i ==  _colourParts.end()) {
    mySize = 0;
  } else {
    mySize = i.value()._contents.size();
  }
  return mySize;
}

QStringList PartWorker::contents(const QString &fileNameStr){

    QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

  if (i != _colourParts.end()) {
    return i.value()._contents;
  } else {
    return _emptyList;
  }
}

void PartWorker::remove(const QString &fileNameStr)
{
    QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

  if (i != _colourParts.end()) {

    _colourParts.erase(i);
    _partList.removeAll(fileNameStr.toLower());
    //logWarn() << "REMOVE COLOUR PART: " << fileNameStr.toLower() << " from contents and _partList";
  }
}

bool PartWorker::partAlreadyInList(const QString &fileNameStr)
{
  QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

  if (i != _colourParts.end()) {
      return true;
    }
  return false;
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

void PartWorker::processPartsArchive(const QStringList &ldPartsDirs, const QString &comment = QString("")){

  // Append fade parts to unofficial library for 3D Viewer's consumption
  QFileInfo libFileInfo(Preferences::viewerLibFile);
  QString archiveFile = QDir::toNativeSeparators(QString("%1/%2").arg(libFileInfo.dir().path()).arg("ldrawunf.zip"));

  if (okToEmit()) {
      emit progressResetSig();
      emit progressMessageSig(QString("Archiving %1 parts.").arg(comment));
    }

  if(!ldPartsDirs.size() == 0){

      if (okToEmit())
          emit progressRangeSig(0, 0);

      for (int i = 0; i < ldPartsDirs.size(); i++){

          QDir foo = ldPartsDirs[i];
          qDebug() << QString(tr("ARCHIVING %1 DIR %2").arg(comment.toUpper()).arg(foo.absolutePath()));

          if (!archiveParts.Archive(archiveFile,
                                     foo.absolutePath(),
                                     QString("Append %1 parts").arg(comment))){

            if (okToEmit())
              emit messageSig(false,QString(tr("Failed to archive %1 parts from \n %2")
                                            .arg(comment)
                                            .arg(ldPartsDirs[i])));
            else
              qDebug() << QString(tr("Failed to archive %1 parts from \n %2")
                                  .arg(comment)
                                  .arg(ldPartsDirs[i]));
            }

          emit progressSetValueSig(i);
        }

      // Reload unofficial library parts into memory
      if (okToEmit()) {//changed on 05/12/2015 move LDSearch Directories to lc_application
          if (!g_App->mLibrary->ReloadUnoffLib()){

              //if (!silent)  //changed on 05/12/2015 move LDSearch Directories to lc_application
              emit messageSig(false,QString(tr("Failed to reload unofficial parts library into memory.")));

            } else {
              qDebug() << QString(tr("Reloaded unofficial parts library into memory."));
            }
        }

      if (okToEmit()) {

          emit progressMessageSig(QString("Finished archiving %1 parts.").arg(comment));

        } else {

          qDebug() << "Finished archiving " + comment +  " parts.";
        }

    } else {

      if (okToEmit())
        emit messageSig(false,QString(tr("Failed to retrieve %1 parts directory.").arg(comment)));
      else
        qDebug() << QString(tr("Failed to retrieve %1 parts directory.").arg(comment));
    }
}

ColourPart::ColourPart(
        const QStringList   &contents,
        const QString       &fileNameStr,
        const int           &partType)
{
    _contents         = contents,
    _fileNameStr      = fileNameStr;
    _partType         = partType;
}

ColourPartListWorker::ColourPartListWorker(QObject *parent) : QObject(parent)
{_endThreadNowRequested = false; _cpLines = 0; _filePath = "";}


/*
 * build colour part directory
 *
 */

void ColourPartListWorker::scanDir()
{
    QStringList partTypeDirs;
    int dirCount = 0;

    if (ldPartsDirs.loadLDrawSearchDirs("")){

        StringList ldrawSearchDirs = ldPartsDirs.getLDrawSearchDirs();

        qDebug() << "";

        for (StringList::const_iterator it = ldrawSearchDirs.begin();
             it != ldrawSearchDirs.end(); it++)
          {
            const char *dir = it->c_str();

            partTypeDirs << QDir::toNativeSeparators(QString(dir));

            qDebug() << "LDRAW SEARCH DIR:  " << QString(dir);
          }

      } else {

        //qDebug() << QString(tr("Failed to load search directories - ldPartsDirs.loadLDrawSearchDirs() returned false."));
        emit messageSig(false, QString(tr("Failed to load search directories - ldPartsDirs.loadLDrawSearchDirs() returned false.")));
      }

    _timer.start();
    emit progressBarInitSig();

    fileSectionHeader(FADESTEP_INTRO_HEADER);
    foreach (QString partTypeDir, partTypeDirs)
      _fadeStepColourParts  << QString("# %1. %2").arg(dirCount++).arg(partTypeDir);
    fileSectionHeader(FADESTEP_FILE_HEADER);

    for (int d = 0; d < partTypeDirs.size() && !_endThreadNowRequested; d++) {
        QDir dir(partTypeDirs[d]);
        scanDir(dir);
    }

    colourChildren();

    writeFadeFile();
    int secs = _timer.elapsed() / 1000;
    int mins = (secs / 60) % 60;
    int hours = (secs / 3600);
    secs = secs % 60;
    QString time = QString("%1:%2:%3")
    .arg(hours, 2, 10, QLatin1Char('0'))
    .arg(mins,  2, 10, QLatin1Char('0'))
    .arg(secs,  2, 10, QLatin1Char('0'));

    //logNotice()<< QString("COLOUR PARTS FILE WRITTEN WITH %1 LINES, ELAPSED TIME %2").arg(QString::number(_cpLines)).arg(time);
    QString fileStatus = QString("Colour parts list successfully created with %1 entries, elapsed time %2.").arg(QString::number(_cpLines)).arg(time);
    fileSectionHeader(FADESTEP_FILE_STATUS, QString("# %1").arg(fileStatus));
    bool appendFile = true;
    writeFadeFile(appendFile);
    emit messageSig(true, fileStatus);
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

    int i = 1;
    QDirIterator itDir(dir, QDirIterator::Subdirectories);

    while(itDir.hasNext() && !_endThreadNowRequested) {

        itDir.next();

        //logInfo() << "Sending FILE to getParts: " << itDir.fileInfo().absoluteFilePath();
        getParts(itDir.fileInfo());
        //logTrace() << "Sending FILE to buildList: " << itDir.fileInfo().absoluteFilePath();
        buildList(itDir.fileInfo());

        emit progressSetValueSig(i++);

    }
    emit progressSetValueSig(dirFiles);
}

void ColourPartListWorker::getParts(const QFileInfo &fileInfo)
{
    if(fileInfo.exists() && ! partAlreadyInList(fileInfo.absoluteFilePath())){
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
            insert(_partFileContents, fileInfo.absoluteFilePath(),-1);

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
                remove(fileInfo.absoluteFilePath());
                break;
            }
        } // token size
    }
    _partFileContents.clear();
    if(hasColour) {
        if (fileInfo.absolutePath() != _filePath){
            fileSectionHeader(FADESTEP_COLOUR_PARTS_HEADER, QString("# in directory: %1").arg(fileInfo.absolutePath()));
            _filePath = fileInfo.absolutePath();
          }
        _cpLines++;
        _fadeStepColourParts  << fileEntry.toLower();
        //logWarn() << "ADD COLOUR PART: " << fileEntry << " Colour: " << colour;
    }
}

void ColourPartListWorker::colourChildren(){

    emit progressResetSig();
    emit progressMessageSig("Generating Child Colour Part List");
    emit progressRangeSig(1, _partList.size());

    QString     filePath = "";
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
                            if (fadeFileInfo.absolutePath() != filePath){
                                fileSectionHeader(FADESTEP_COLOUR_CHILDREN_PARTS_HEADER, QString("# in directory: %1").arg(fadeFileInfo.absolutePath()));
                                filePath = fadeFileInfo.absolutePath();
                              }
                            _cpLines++;
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


void ColourPartListWorker::writeFadeFile(bool append){
    // change to insttall path
    QFileInfo colourFileList(Preferences::fadeStepColorPartsFile);
    if (! _fadeStepColourParts.empty())
    {
        emit progressResetSig();
        emit progressMessageSig("Writing Colour Part List");
        emit progressRangeSig(1, _fadeStepColourParts.size());

        QFile file(colourFileList.absoluteFilePath());
     if ( ! file.open(append ? QFile::Append | QFile::Text : QFile::WriteOnly | QFile::Text)) {
            //logError() << QString("Failed to OPEN colourFileList %1 for writing:\n%2").arg(file.fileName()).arg(file.errorString());
            emit messageSig(false,QString("Failed to OPEN colourFileList %1 for writing:\n%2").arg(file.fileName()).arg(file.errorString()));
            return;
        }
        QTextStream out(&file);
        for (int i = 0; i < _fadeStepColourParts.size() && !_endThreadNowRequested; i++) {
            emit progressSetValueSig(i);
            QString cpLine = _fadeStepColourParts[i];
            out << cpLine << endl;
        }
        _fadeStepColourParts.clear();
        file.close();
        emit progressSetValueSig(_fadeStepColourParts.size());
    }
}

void ColourPartListWorker::insert(
        const QStringList   &contents,
        const QString       &fileNameStr,
        const int           &partType){

    bool partErased = false;

    QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

    if (i != _colourParts.end()){
        _colourParts.erase(i);
        partErased = true;
    }
    ColourPart colourPartEntry(contents, fileNameStr, partType);
    _colourParts.insert(fileNameStr.toLower(), colourPartEntry);

    if (! partErased)
      _partList << fileNameStr.toLower();
    //logNotice() << "02 INSERT PART CONTENTS - UID: " << fileNameStr.toLower()  <<  " partType: " << partType;
}

bool ColourPartListWorker::partAlreadyInList(const QString &fileNameStr)
{
  QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

  if (i != _colourParts.end()) {
      return true;
    }
  return false;
}

void ColourPartListWorker::remove(const QString &fileNameStr)
{
    QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

  if (i != _colourParts.end()) {
    _colourParts.erase(i);
    _partList.removeAll(fileNameStr.toLower());
    //logWarn() << "REMOVE COLOUR PART: " << fileNameStr.toLower() << " from contents and _partList";
  }
}

void ColourPartListWorker::fileSectionHeader(const int &option, const QString &heading){

    static const QString fmtDateTime("MM-dd-yyyy hh:mm:ss");

    switch(option)
    {
    case FADESTEP_INTRO_HEADER:
        _fadeStepColourParts  << QString("# File: fadeStepColorParts.lst  Generated on: %1").arg(QDateTime::currentDateTime().toString(fmtDateTime));
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# This list captures the static coloured parts (or their components) to support part fade";
        _fadeStepColourParts  << "# Parts on this list are copied to their respective fade directory and modified.";
        _fadeStepColourParts  << "# Colour codes are replaced with colour code 16";
        _fadeStepColourParts  << "# Copied files are appended with '-fade', for example, ...\\fade\\parts\\99499-fade.dat";
        _fadeStepColourParts  << "# Part identifiers with spaces will not be properly recoginzed.";
        _fadeStepColourParts  << "# This file is automatically generated from Configuration=>Generate Fade Colour Parts List";
        _fadeStepColourParts  << "# However, it can also be modified manually from Configuration=>Edit Fade Colour Parts List";
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# Fade colour parts were generate from the following list of search directories:";
        _fadeStepColourParts  << "";
        break;
      case FADESTEP_FILE_HEADER:
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# Part ID (LDraw Name:)		Part Absolute File Path		Part Description (LDraw Description) - for reference only";
        _fadeStepColourParts  << "";
          break;
    case FADESTEP_COLOUR_PARTS_HEADER:
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# Static colour parts";
        _fadeStepColourParts  << heading;
        _fadeStepColourParts  << "";
        break;
    case FADESTEP_COLOUR_CHILDREN_PARTS_HEADER:
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# Parts with no static colour element but has subparts or primitives with static colour elements";
        _fadeStepColourParts  << heading;
        _fadeStepColourParts  << "";
        break;
    case FADESTEP_FILE_STATUS:
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# ++++++++++++++++++++++++";
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << heading;
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












