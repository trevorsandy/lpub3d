/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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
#include "step.h"
#include "paths.h"
#include "lpub.h"
#include "name.h"
#include "application.h"

#ifdef WIN32
#include <clocale>
#endif // WIN32

PartWorker::PartWorker(QObject *parent) : QObject(parent)
{
  _doReload               = false;
  _didInitLDSearch        = false;
  _resetSearchDirSettings = false;
  _endThreadNowRequested  = false;

  _excludedSearchDirs << ".";
  _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("PARTS"));
  _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("P"));
  _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/parts"));
  _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/p"));
  _fadePartDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/fade/parts"));
  _fadePrimDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/fade/p"));

}

/*
 * LDraw search directories preferences.
 */
void PartWorker::ldsearchDirPreferences(){

  emit Application::instance()->splashMsgSig("70% - Search directory preferences loading...");

  QSettings Settings;
  QString const LdrawiniFilePathKey("LDrawIniFile");
  QString const LdSearchDirsKey("LDSearchDirs");

  // qDebug() << QString(tr("01 ldrawIniFoundReg(Original) = %1").arg((ldrawIniFoundReg ? "True" : "False")));

  if (Settings.contains(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey))) {
      QString ldrawiniFilePath = Settings.value(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey)).toString();
      QFileInfo ldrawiniInfo(ldrawiniFilePath);
      if (ldrawiniInfo.exists()) {
          Preferences::ldrawiniFile = ldrawiniInfo.absoluteFilePath();
          Preferences::ldrawiniFound = true;
        } else {
          Settings.remove(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey));
          Settings.remove(QString("%1/%2").arg(SETTINGS,LdSearchDirsKey));
        }
    } else if (ldPartsDirs.initLDrawSearchDirs()) {
      QFileInfo ldrawiniInfo(ldPartsDirs.getSearchDirsOrigin());
      if (ldrawiniInfo.exists()) {
          Preferences::ldrawiniFile = ldrawiniInfo.absoluteFilePath();
          Preferences::ldrawiniFound = true;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey), Preferences::ldrawiniFile);
          //qDebug() << QString(tr("01 Using LDraw.ini file form loadLDrawSearchDirs(): ").arg(Preferences::ldrawiniFile));
        } else {
          Settings.remove(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey));
          //qDebug() << QString(tr("  -Failed to get Ldraw.ini, valid file (from Preferences) does not exist."));
        }
    } else {
      emit messageSig(true,QString("Unable to initialize Ldrawini. Using default search directories."));
    }

  setDoFadeStep((gui->page.meta.LPub.fadeStep.fadeStep.value() || Preferences::enableFadeStep));
  if (!doFadeStep()) {
      _excludedSearchDirs << _fadePartDir;
      _excludedSearchDirs << _fadePrimDir;
  }

  if (!Preferences::ldrawiniFound && !_resetSearchDirSettings &&
      Settings.contains(QString("%1/%2").arg(SETTINGS,LdSearchDirsKey))) {    // ldrawini not found and not reset so load registry key
      Preferences::ldSearchDirs = Settings.value(QString("%1/%2").arg(SETTINGS,LdSearchDirsKey)).toStringList();
    } else if (loadLDrawSearchDirs()){                                        //ldraw.ini found or reset so load from disc file
      Settings.setValue(QString("%1/%2").arg(SETTINGS,LdSearchDirsKey), Preferences::ldSearchDirs);
    } else {
      Settings.remove(QString("%1/%2").arg(SETTINGS,LdSearchDirsKey));
      emit messageSig(true,QString("Unable to load search directories."));
    }
}
/*
 * Load LDraw search directories into Preferences.
 */
bool PartWorker::loadLDrawSearchDirs(){

    emit Application::instance()->splashMsgSig("70% - Search directories loading...");

    StringList ldrawSearchDirs;
    if (ldPartsDirs.loadLDrawSearchDirs("")){
        ldrawSearchDirs = ldPartsDirs.getLDrawSearchDirs();
        Preferences::ldSearchDirs.clear();
        for (StringList::const_iterator it = ldrawSearchDirs.begin();
             it != ldrawSearchDirs.end(); it++)
        {
            const char *dir = it->c_str();
            QString ldrawSearchDir = QString(dir);
            bool excludeSearchDir = false;
            foreach (QString excludedDir, _excludedSearchDirs){
                if ((excludeSearchDir =
                     ldrawSearchDir.toLower().contains(excludedDir.toLower()))) {
                    break;
                }
            }
            if (! excludeSearchDir){
                // check if empty
                if (QDir(ldrawSearchDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                    Preferences::ldSearchDirs << ldrawSearchDir;
                    // qDebug() << "->INCLUDDE LDRAW SEARCH DIR: " << ldrawSearchDir;
                }
            }
        }

    } else {
        logError() << "ldPartsDirs.loadLDrawSearchDirs("") failed.";
        return false;
    }
    return true;
}

/* Add qualified search directories to LDSEARCHDIRS string
   This is used to pass search directories to ldglite.
   This function will only execute if the preferred renderer is LDGLite
   and there are more than 0 search directories in Preferences::ldgliteSearchDirs.
*/
void PartWorker::populateLdgLiteSearchDirs(){
    if (Preferences::preferredRenderer == "LDGLite" && !Preferences::ldSearchDirs.isEmpty()){

        emit Application::instance()->splashMsgSig("70% - LDGlite Search directories loading...");

        // Define excluded directories
        QStringList ldgliteExcludedDirs = _excludedSearchDirs;
        ldgliteExcludedDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("MODELS"))
                            << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial"))
                            << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/LSynth"));
        // Clear directories
        Preferences::ldgliteSearchDirs.clear();
        int count = 0;                    // set delimeter from 2nd entry
        bool fadeDirsIncluded = false;
        // Recurse ldraw search directories
        foreach (QString ldgliteSearchDir, Preferences::ldSearchDirs){
            //Check if Unofficial root directory
            bool foundUnofficialRootDir = false;
            QString unofficialRootDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial"));
            if ((foundUnofficialRootDir =
                 ldgliteSearchDir.toLower() == unofficialRootDir.toLower())) {
//           logDebug() << "<-FOUND UNOFFICIAL DIR: " << ldgliteSearchDir;
                QDir unofficialDir(unofficialRootDir);
                // Get sub directories
                QStringList unofficialSubDirs = unofficialDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::SortByMask);
                if (unofficialSubDirs.count() > 0){
                    // Recurse unofficial subdirectories for excluded directories
                    foreach (QString unofficialSubDirName, unofficialSubDirs){
                        // Exclude invalid directories
                        bool excludeSearchDir = false;
                        QString unofficialDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(unofficialRootDir).arg(unofficialSubDirName));
                        foreach (QString excludedDir, ldgliteExcludedDirs){
                            if ((excludeSearchDir =
                                 unofficialDirPath.toLower() == excludedDir.toLower())) {
                                break;
                            }
                        }
                        if (!excludeSearchDir){
                            // check if empty
                            if (QDir(unofficialDirPath).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                                count++;
                                count > 1 ? Preferences::ldgliteSearchDirs.append(QString("|%1").arg(unofficialDirPath)):
                                            Preferences::ldgliteSearchDirs.append(unofficialDirPath);
//                          logDebug() << "->INCLUDE LDGLITE UNOFFICIAL LDRAW SEARCH SUB DIR: " << unofficialDirPath;
                            }
                        }
                    }
                }
            } else {
                // Exclude invalid directories
                bool excludeSearchDir = false;
                foreach (QString excludedDir, ldgliteExcludedDirs){
                    if ((excludeSearchDir =
                         ldgliteSearchDir.toLower() == excludedDir.toLower())) {
                        break;
                    }
                }
                if (!excludeSearchDir){
                    // check if empty
                    if (QDir(ldgliteSearchDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                        count++;
                        count > 1 ? Preferences::ldgliteSearchDirs.append(QString("|%1").arg(ldgliteSearchDir)):
                                    Preferences::ldgliteSearchDirs.append(ldgliteSearchDir);
//                      logDebug() << "->INCLUDE LDGLITE LDRAW SEARCH DIR: " << ldgliteSearchDir;
                    }
                }
                // Check if fade directories included
                if (Preferences::ldrawiniFound && doFadeStep() && !fadeDirsIncluded){
                    fadeDirsIncluded = (ldgliteSearchDir.toLower() == _fadePartDir.toLower() ||
                                        ldgliteSearchDir.toLower() == _fadePrimDir.toLower());
                }
            }
        }
        // If using ldraw.ini and fade step enabled but fade directories not defined in ldraw.ini, add fade directories
        if (Preferences::ldrawiniFound && doFadeStep() && !fadeDirsIncluded) {
            if (QDir(_fadePartDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                count ++;
                count > 1 ? Preferences::ldgliteSearchDirs.append(QString("|%1").arg(_fadePartDir)):
                            Preferences::ldgliteSearchDirs.append(QString("%1").arg(_fadePartDir));
            }
            if (QDir(_fadePrimDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                count++;
                count > 1 ? Preferences::ldgliteSearchDirs.append(QString("|%1").arg(_fadePrimDir)):
                            Preferences::ldgliteSearchDirs.append(QString("%1").arg(_fadePrimDir));
            }
            count ++;
//          logDebug() << "->INCLUDE LDGLITE LDRAW SEARCH DIR: " << QString("%1 %2").arg(_fadePartDir).arg(_fadePrimDir);
        }

//       logDebug() << "--FINAL DIR COUNT: (" << count << ") " << Preferences::ldgliteSearchDirs;
    }
}

/*
 * Process LDraw search directories part files.
 */
void PartWorker::processLDSearchDirParts(){

  if (Preferences::ldSearchDirs.size() > 0)
    processPartsArchive(Preferences::ldSearchDirs, "search directory");
 // qDebug() << "\nFinished Processing Search Directory Parts.";

 }

/*
 * Create fade version of static colour part files.
 */
void PartWorker::processFadePartsArchive(){
    if (doFadeStep()) {
        QStringList fadePartsDirs;
        Paths::mkfadedirs();
        foreach(QDir fadeDir, Paths::fadeDirs){
            if(fadeDir.entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0)
                fadePartsDirs << fadeDir.absolutePath();
        }
        if (fadePartsDirs.size() > 0) {
            if (!processPartsArchive(fadePartsDirs, "colour fade")){
                QString error = QString("Process fade parts archive failed!.");
                emit messageSig(false,error);
                logError() << error;
            }
        }
    }
}

void PartWorker::processFadeColourParts()
{
  if (doFadeStep()) {
       _timer.start();
      _fadedParts = 0;

      setDidInitLDSearch(true);

      QStringList fadePartsDirs;
      QStringList contents;
      QStringList colourPartList;

      emit progressBarInitSig();
      emit progressMessageSig("Parse Model File");
      Paths::mkfadedirs();

      ldrawFile = gui->getLDrawFile();
      // porcess top-level submodels
      emit progressRangeSig(1, ldrawFile._subFileOrder.size());
      for (int i = 0; i < ldrawFile._subFileOrder.size() && endThreadNotRequested(); i++) {
          QString subfileNameStr = ldrawFile._subFileOrder[i].toLower();
          contents = ldrawFile.contents(subfileNameStr);
          emit progressSetValueSig(i);
          logInfo() << "00 PROCESSING SUBFILE:" << subfileNameStr;
          for (int i = 0; i < contents.size() && endThreadNotRequested(); i++) {
              QString line = contents[i];
              QStringList tokens;
              split(line,tokens);
              if (tokens.size() == 15 && tokens[0] == "1") {
                  // check if colored part and create fade version if yes
                  QString fileNameStr  = tokens[tokens.size()-1];
                  // validate part is static colour part;
                  if (FadeStepColorParts::getStaticColorPartInfo(fileNameStr)){
                      bool entryExists = false;
                      QString dirName = fileNameStr.section(":::",1,1).split("\\").first();
                      QString fileName = fileNameStr.section(":::",1,1).split("\\").last();
                      //logDebug() << "FileDir:" << dirName << "FileName:" << fileName;
                      QDir fadeFileDirPath;
                      if (dirName == fileName){
                          fadeFileDirPath = Paths::fadePartDir;
                      } else  if (dirName == "s"){
                          fadeFileDirPath = Paths::fadeSubDir;
                      } else  if (dirName == "p"){
                          fadeFileDirPath = Paths::fadePrimDir;
                      } else  if (dirName == "8"){
                          fadeFileDirPath = Paths::fadePrim8Dir;
                      } else if (dirName == "48"){
                          fadeFileDirPath = Paths::fadePrim48Dir;
                      } else {
                          fadeFileDirPath = Paths::fadePartDir;
                      }
                      QFileInfo fadeFileInfo(fadeFileDirPath,fileName.replace(".dat","-fade.dat"));
                      if(fadeFileInfo.exists()){
                          logNotice() << "01 COLOUR PART EXIST - IGNORING:" << fileNameStr;
                          entryExists = true;
                      }
                      if (!entryExists)
                          foreach(QString colourPart, colourPartList){
                              if (colourPart == fileNameStr){
                                  entryExists = true;
                                  break;
                              }
                          }
                      if (!entryExists) {
                          logNotice() << "01 SUBMIT COLOUR PART INFO:" << fileNameStr << " Line: " << i ;
                          colourPartList << fileNameStr;
                      }
                  }
              }
          }
      }
      emit progressSetValueSig(ldrawFile._subFileOrder.size());

      if (colourPartList.size() > 0) {
          if (!processColourParts(colourPartList)) {
              QString error = QString("Process fade colour parts failed!.");
              emit messageSig(false,error);
              logError() << error;
              emit removeProgressStatusSig();
              emit fadeColourFinishedSig();
              return;
          }

          createFadePartFiles();

          // Append fade parts to unofficial library for LeoCAD's consumption
          if(!fadePartsDirs.size() == 0){
              fadePartsDirs.empty();
          }
          foreach(QDir fadeDir, Paths::fadeDirs){
              if(fadeDir.entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0)
                  fadePartsDirs << fadeDir.absolutePath();
          }

          if (_fadedParts > 0 && fadePartsDirs.size() > 0) {
              if (!processPartsArchive(fadePartsDirs, "colour fade")){
                  QString error = QString("Process fade parts archive failed!.");
                  emit messageSig(false,error);
                  logError() << error;
                  emit removeProgressStatusSig();
                  emit fadeColourFinishedSig();
                  return;
              }
          }
      }

      int secs = _timer.elapsed() / 1000;
      int mins = (secs / 60) % 60;
      secs = secs % 60;
      int msecs = _timer.elapsed() % 1000;

      QString time = QString("Elapsed time is %1:%2:%3")
      .arg(mins, 2, 10, QLatin1Char('0'))
      .arg(secs,  2, 10, QLatin1Char('0'))
      .arg(msecs,  3, 10, QLatin1Char('0'));

      QString fileStatus = _fadedParts == 1 ? QString("%1 fade part created and library updated. %2.").arg(_fadedParts).arg(time) :
                           _fadedParts > 1 ? QString("%1 fade parts created and library updated. %2.").arg(_fadedParts).arg(time) :
                                           QString("Fade parts verified. %1.").arg(time);
      emit removeProgressStatusSig();
      emit fadeColourFinishedSig();
      emit messageSig(true,fileStatus);

      logInfo() << fileStatus;
    }
}

bool PartWorker::processColourParts(const QStringList &colourPartList) {

    // Archive library files
    QString fileStatus;

    QString officialLib;
    QString unofficialLib;
    QFileInfo archiveFileInfo(QDir::toNativeSeparators(Preferences::lpub3dLibFile));
    if (archiveFileInfo.exists()) {
        officialLib = archiveFileInfo.absoluteFilePath();
        unofficialLib = QString("%1/%2").arg(archiveFileInfo.absolutePath(),VER_LPUB3D_UNOFFICIAL_ARCHIVE);
    } else {
        fileStatus = QString("Archive file does not exist: %1. The process will terminate.").arg(archiveFileInfo.absoluteFilePath());
        emit messageSig(true, fileStatus);
        logError() << fileStatus;
        return false;
    }

    emit progressResetSig();
    emit progressMessageSig("Process Colour Parts...");
    emit progressRangeSig(1, colourPartList.size());

    int partCount = 0;
    QStringList inputContents;
    QStringList childrenColourParts;

    foreach (QString partEntry, colourPartList) {

        bool partFound = false;

        QString cpPartEntry = partEntry;
        bool unOffLib = cpPartEntry.section(":::",0,0) == "u";
        //logInfo() << "Library Type:" << (unOffLib ? "Unofficial Library" : "Official Library");

        QString libPartName = cpPartEntry.section(":::",1,1).split("\\").last();
        //logInfo() << "Lib Part Name:" << libPartName;

        QString libDirName = cpPartEntry.section(":::",1,1).split("\\").first();

        emit progressSetValueSig(partCount++);

        QuaZip zip(unOffLib ? unofficialLib : officialLib);
        if (!zip.open(QuaZip::mdUnzip)) {
            logError() << QString("Failed to open archive: %1 @ %2").arg(zip.getZipError()).arg(unOffLib ? unofficialLib : officialLib);
            return false;
        }

        for(bool f=zip.goToFirstFile(); f&&endThreadNotRequested(); f=zip.goToNextFile()) {

            QFileInfo libPartFile(zip.getCurrentFileName());
            if (libPartFile.fileName().toLower() == libPartName && ! partAlreadyInList(libPartName)) {

                partFound = true;
                QByteArray qba;
                QuaZipFile zipFile(&zip);
                if (zipFile.open(QIODevice::ReadOnly)) {
                    qba = zipFile.readAll();
                    zipFile.close();
                } else {
                    logError() << QString("Failed to OPEN Part file :%1").arg(zip.getCurrentFileName());
                    return false;
                }
                // extract content
                QTextStream in(&qba);
                while (! in.atEnd() && endThreadNotRequested()) {
                    QString line = in.readLine(0);
                    _partFileContents << line.toLower();

                    // check if line is a colour part
                    QStringList tokens;
                    split(line,tokens);
                    if (tokens.size() == 15 && tokens[0] == "1") {
                        QString childFileNameStr  = tokens[tokens.size()-1];
                        // validate part is static colour part;
                        if (FadeStepColorParts::getStaticColorPartInfo(childFileNameStr)){
                            bool entryExists = false;
                            QString dirName = childFileNameStr.section(":::",1,1).split("\\").first();
                            QString fileName = childFileNameStr.section(":::",1,1).split("\\").last();
                            //logDebug() << "FileDir:" << dirName << "FileName:" << fileName;
                            QDir fadeFileDirPath;
                            if (dirName == fileName){
                                fadeFileDirPath = Paths::fadePartDir;
                            } else  if (dirName == "s"){
                                fadeFileDirPath = Paths::fadeSubDir;
                            } else  if (dirName == "p"){
                                fadeFileDirPath = Paths::fadePrimDir;
                            } else  if (dirName == "8"){
                                fadeFileDirPath = Paths::fadePrim8Dir;
                            } else if (dirName == "48"){
                                fadeFileDirPath = Paths::fadePrim48Dir;
                            } else {
                                fadeFileDirPath = Paths::fadePartDir;
                            }
                            QFileInfo fadeFileInfo(fadeFileDirPath,fileName.replace(".dat","-fade.dat"));
                            if(fadeFileInfo.exists()){
                                logNotice() << "03 CHILD COLOUR PART EXIST - IGNORING:" << childFileNameStr;
                                entryExists = true;
                            }
                            if (!entryExists)
                                foreach(QString childColourPart, childrenColourParts){
                                    if (childColourPart == childFileNameStr){
                                        entryExists = true;
                                        break;
                                    }
                                }
                            if (!entryExists) {
                                logNotice() << "03 SUBMIT CHILD COLOUR PART INFO:" << childFileNameStr;
                                childrenColourParts << childFileNameStr;
                            }
                        }
                    }
                }
                // determine part type
                int partType = -1;
                if (libDirName == libPartName){
                    partType=LD_PARTS;
                } else  if (libDirName == "s"){
                    partType=LD_SUB_PARTS;
                } else  if (libDirName == "p"){
                    partType=LD_PRIMITIVES;
                } else  if (libDirName == "8"){
                    partType=LD_PRIMITIVES_8;
                } else if (libDirName == "48"){
                    partType=LD_PRIMITIVES_48;
                } else {
                    partType=LD_PARTS;
                }
                // add content to ColourParts map
                insert(_partFileContents, libPartName, partType, true);
                inputContents = _partFileContents;
                _partFileContents.clear();
                break;

            } else if (libPartFile.fileName().toLower() == libPartName && partAlreadyInList(libPartName)) {
                partFound = true;
                logTrace() << "Part already in list:" << libPartName;
                break;
            }
        }
        if (!partFound) {
            fileStatus = QString("Part file %1 not found in %2.").arg(libPartName).arg(unOffLib ? "Unofficial Library" : "Official Library");
            emit messageSig(false, fileStatus);
            logError() << fileStatus;
        }

        zip.close();

        if (zip.getZipError() != UNZ_OK) {
            logError() << QString("zip close error: %1").arg(zip.getZipError());
            return false;
        }
    }
    emit progressSetValueSig(colourPartList.size());

    // recurse part file content to check if any children are colour parts
    if (childrenColourParts.size() > 0)
        processColourParts(childrenColourParts);

    QString message = "Colour parts content created.";
    emit messageSig(true,message);
    logInfo() << message;

    return true;
}


bool PartWorker::createFadePartFiles(){

    int maxValue            = _partList.size();
    emit progressResetSig();
    emit progressMessageSig("Creating Fade Colour Parts");
    emit progressRangeSig(1, maxValue);

    QString materialColor  ="16";  // Internal Common Material Colour (main)
    QString edgeColor      ="24";  // Internal Common Material Color (edge)

    QStringList fadePartContent;
    QString fadePartFile;

    for(int part = 0; part < _partList.size() && endThreadNotRequested(); part++){

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
            QString fadeFile = cp.value()._fileNameStr;
            QFileInfo fadeStepColorPartFileInfo(fadePartDirPath,fadeFile.replace(".dat","-fade.dat"));
            if (fadeStepColorPartFileInfo.exists()){
                logNotice() << "PART ALREADY EXISTS: " << fadeStepColorPartFileInfo.absoluteFilePath();
                continue;
            }
            fadePartFile = fadeStepColorPartFileInfo.absoluteFilePath();
            //logTrace() << "A. PART CONTENT ABSOLUTE FILEPATH: " << fadeStepColorPartFileInfo.absoluteFilePath();

            // process fade part contents
            for (int i = 0; i < cp.value()._contents.size() && endThreadNotRequested(); i++) {
                QString line =  cp.value()._contents[i];
                QStringList tokens;
                QString fileNameStr;

                split(line,tokens);
                if (tokens.size() == 15 && tokens[0] == "1") {
                    fileNameStr = tokens[tokens.size()-1].toLower();
                    QString searchFileNameStr = fileNameStr;
                    // check if part at this line has a matching colour part in the colourPart list - if yes, rename with '-fade'
                    searchFileNameStr = searchFileNameStr.split("\\").last();
                    QMap<QString, ColourPart>::iterator cpc = _colourParts.find(searchFileNameStr);
                    if (cpc != _colourParts.end()){
                        if (cpc.value()._fileNameStr == searchFileNameStr){
                            fileNameStr = fileNameStr.replace(".dat","-fade.dat");
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

                    //QString oldColour(tokens[1]);       //logging only: show colour lines
                    tokens[1] = materialColor;
                    //logTrace() << "D. CHANGE CHILD PART COLOUR: " << fileNameStr << " NewColour: " << tokens[1] << " OldColour: " << oldColour;
                }
                line = tokens.join(" ");
                fadePartContent << line;
            }
            //logTrace() << "04 SAVE COLOUR PART: " << fadePartFile;
            if(saveFadeFile(fadePartFile, fadePartContent))
                _fadedParts++;
            fadePartContent.clear();
        }
    }
    emit progressSetValueSig(maxValue);
    return true;
}


/*
     * Write faded part files to fade directory.
     */
bool PartWorker::saveFadeFile(
        const QString     &fileName,
        const QStringList &fadePartContent) {

    QFile file(fileName);
    if ( ! file.open(QFile::WriteOnly | QFile::Text)) {
        QString message = QString("Failed to open %1 for writing: %2").arg(fileName).arg(file.errorString());
        emit messageSig(true,message);
        logError() << message;
        return false;

    } else {
        QTextStream out(&file);
        for (int i = 0; i < fadePartContent.size(); i++) {
            out << fadePartContent[i] << endl;
        }
        file.close();
        logNotice() << "05 WRITE FADE PART TO DISC:" << fileName;
        return true;
    }
}


void PartWorker::insert(
        const QStringList   &contents,
        const QString       &fileNameStr,
        const int           &partType,
        const bool          &unOff){

    bool partErased = false;
    QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

    if (i != _colourParts.end()){
        _colourParts.erase(i);
        partErased = true;
//      logNotice() << "PART ALREADY IN LIST - PART ERASED" << i.value()._fileNameStr << ", UnOff Lib:" << i.value()._unOff;
    }

    ColourPart colourPartEntry(contents, fileNameStr, partType, unOff);
    _colourParts.insert(fileNameStr, colourPartEntry);

    if (! partErased)
      _partList << fileNameStr;
//    logNotice() << "02 INSERT (COLOUR PART ENTRY) - UID: " << fileNameStr  <<  " fileNameStr: " << fileNameStr <<  " partType: " << partType <<  " unOff: " << unOff ;
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
    //logNotice() << "REMOVE COLOUR PART: " << fileNameStr.toLower() << " from contents and _partList";
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


bool PartWorker::processPartsArchive(const QStringList &ldPartsDirs, const QString &comment = QString("")){

  // Append fade parts to unofficial library for 3D Viewer's consumption
  QFileInfo libFileInfo(Preferences::lpub3dLibFile);
  QString archiveFile = QDir::toNativeSeparators(QString("%1/%2").arg(libFileInfo.absolutePath(),VER_LPUB3D_UNOFFICIAL_ARCHIVE));
  QString returnMessage = QString("Archiving %1 parts to : %2.").arg(comment,archiveFile);
  logInfo() << QString("Archiving %1 parts to %2.").arg(comment,archiveFile);
  logInfo() << (doFadeStep() ? QString("Fade Step is ON.") : QString("Fade Step is OFF."));

  if (okToEmitToProgressBar()) {
      emit progressResetSig();
    } else {
      emit Application::instance()->splashMsgSig(QString("75% - Archiving %1 parts...").arg(comment));
    }

  if (okToEmitToProgressBar())
      emit progressRangeSig(0, 0);

  int archivedPartCount = 0;

  for (int i = 0; i < ldPartsDirs.size(); i++){

      QDir foo = ldPartsDirs[i];

      if (!archiveParts.Archive( archiveFile,
                                 foo.absolutePath(),
                                 returnMessage,
                                 QString("Append %1 parts").arg(comment)))
      {
          if (okToEmitToProgressBar())
              emit messageSig(false,returnMessage);
          else
              logError() << returnMessage;

          return false;
      }
      bool ok;
      int partCount = returnMessage.toInt(&ok);
      QString breakdown;
      if (ok){
//          breakdown = partCount == 1 && archivedPartCount == 0 ? tr("part") :
//                      partCount != 1 && archivedPartCount == 0 ? tr("parts") :
//                                                                 tr("[%1 + %2] parts").arg(archivedPartCount).arg(partCount);
          archivedPartCount += partCount;
          breakdown = partCount == 1 && archivedPartCount == 0 ? tr("part") :
                      partCount != 1 && archivedPartCount == 0 ? tr("parts") :
                                                                 tr("[Total %1] parts").arg(archivedPartCount);
      }
      logInfo() << tr("Archived %1 %2 from %3").arg(partCount).arg(breakdown).arg(foo.absolutePath());
  }

  // Reload unofficial library parts into memory - only if initial library load already done !
  if (didInitLDSearch() && archivedPartCount > 0) {

      if (!g_App->mLibrary->ReloadUnoffLib()){
          returnMessage = tr("Failed to reload unofficial parts library into memory.");
          if (okToEmitToProgressBar()) {
              emit messageSig(false,returnMessage);
          } else {
              logError() << returnMessage;
          }
          return false;
      } else {
          returnMessage = tr("Reloaded unofficial parts library into memory.").arg(archivedPartCount);
          if (okToEmitToProgressBar()) {
              emit messageSig(true,returnMessage);
          } else {
              logInfo() << returnMessage;
          }
      }
  }

  if (archivedPartCount > 0)
      returnMessage = tr("Finished. Archived and loaded %1 %2 parts into memory.").arg(archivedPartCount).arg(comment);
  else
      returnMessage = tr("Finished. No %1 parts archived.").arg(comment);

  logInfo() << returnMessage;
  if (okToEmitToProgressBar()) {
      emit messageSig(true, returnMessage);
  } else {
      emit Application::instance()->splashMsgSig(tr("80% - Finished archiving %1 parts.").arg(comment));
  }
  return true;
}


ColourPart::ColourPart(
        const QStringList   &contents,
        const QString       &fileNameStr,
        const int           &partType,
        const bool          &unOff)
{
    _contents         = contents;
    _fileNameStr      = fileNameStr;
    _partType         = partType;
    _unOff            = unOff;
}


ColourPartListWorker::ColourPartListWorker(QObject *parent) : QObject(parent)
{
    _endThreadNowRequested = false;
    _cpLines = 0;
    _filePath = "";
}


/*
 * build colour part listing
 *
 */

void ColourPartListWorker::generateFadeColourPartsList()
{
    // Archive library files
    QStringList archiveFiles;
    QFileInfo lpub3dLibFileInfo(QDir::toNativeSeparators(Preferences::lpub3dLibFile));
    if (lpub3dLibFileInfo.exists()) {
        archiveFiles << QString("%1/%2").arg(lpub3dLibFileInfo.absolutePath(),VER_LPUB3D_UNOFFICIAL_ARCHIVE);
        archiveFiles << lpub3dLibFileInfo.absoluteFilePath();
    }

    //library directories
    QStringList partTypeDirs;
    partTypeDirs << "p       Primatives";
    partTypeDirs << "p/8     Primatives - Low Resolution";
    partTypeDirs << "p/48    Primatives - High Resolution";
    partTypeDirs << "parts   Parts";
    partTypeDirs << "parts/s Sub-parts";

    _timer.start();
    _colWidthFileName = 0;

    int libCount = 1;

    fileSectionHeader(FADESTEP_INTRO_HEADER);
    _fadeStepColourParts  << "# Archive Libraries:";
    foreach (QString archiveFile, archiveFiles) {
      QString library = archiveFile == lpub3dLibFileInfo.absoluteFilePath() ? "Official Library" : "Unofficial Library";
      _fadeStepColourParts  << QString("# %1. %2: %3").arg(libCount++).arg(library).arg(archiveFile);
    }
    int dirCount = 1;
    _fadeStepColourParts  << "";
    _fadeStepColourParts  << "# Library Directories:";
    foreach (QString partTypeDir, partTypeDirs){
      _fadeStepColourParts  << QString("# %1. %2").arg(dirCount++).arg(partTypeDir);
    }
    fileSectionHeader(FADESTEP_FILE_HEADER);

    emit progressBarInitSig();
    foreach (QString archiveFile, archiveFiles) {
       if(!processArchiveParts(archiveFile)){
           QString error = QString("Process colour parts list failed!.");
           emit messageSig(false,error);
           logError() << error;
           emit removeProgressStatusSig();
           emit colourPartListFinishedSig();
           return;
       }
    }

    processChildren();

    writeFadeFile();

    int secs = _timer.elapsed() / 1000;
    int mins = (secs / 60) % 60;
    secs = secs % 60;
    int msecs = _timer.elapsed() % 1000;

    QString time = QString("Elapsed time is %1:%2:%3")
    .arg(mins, 2, 10, QLatin1Char('0'))
    .arg(secs,  2, 10, QLatin1Char('0'))
    .arg(msecs,  3, 10, QLatin1Char('0'));

    QString fileStatus = QString("Colour parts list successfully created with %1 entries. %2.").arg(QString::number(_cpLines)).arg(time);
    fileSectionHeader(FADESTEP_FILE_STATUS, QString("# %1").arg(fileStatus));
    bool append = true;
    writeFadeFile(append);

    emit removeProgressStatusSig();
    emit colourPartListFinishedSig();
    emit messageSig(true, fileStatus);

    logInfo() << fileStatus;
}

bool ColourPartListWorker::processArchiveParts(const QString &archiveFile) {

    bool isUnOffLib = true;
    QString library = "Unofficial Library";

    if (archiveFile.contains(VER_LDRAW_OFFICIAL_ARCHIVE)) {
        library = "Official Library";
        isUnOffLib = false;
    }

    QuaZip zip(archiveFile);
    if (!zip.open(QuaZip::mdUnzip)) {
        logError() << QString("! zip.open(): %1 @ %2").arg(zip.getZipError()).arg(archiveFile);
        return false;
    }

    // get part count
    emit progressRangeSig(0, 0);
    emit progressMessageSig("Generating " + library + " Colour Parts...");

    int partCount = 1;
    for(bool f=zip.goToFirstFile(); f; f=zip.goToNextFile()) {
        if (zip.getCurrentFileName().toLower().split(".").last() != "dat") {
            continue;

        } else {
            partCount++;
        }
    }
    logInfo() << QString("Processing %1 - Parts Count: %2").arg(library).arg(partCount);

    emit progressResetSig();
    emit progressRangeSig(1, partCount);
    partCount = 0;

    for(bool f=zip.goToFirstFile(); f&&endThreadNotRequested(); f=zip.goToNextFile()) {

        // set file and extract content
        if (zip.getCurrentFileName().toLower().split(".").last() != "dat") {
            continue;

          } else {

            QString libFileName = zip.getCurrentFileName();
            libFileName = isUnOffLib ? libFileName : libFileName.remove(0,6);  // Remove 'ldraw/' prefix from official file path

            QByteArray qba;
            QuaZipFile zipFile(&zip);
            if (zipFile.open(QIODevice::ReadOnly)) {
                qba = zipFile.readAll();
                zipFile.close();
            } else {
                logError() << QString("Failed to OPEN Part file :%1").arg(libFileName);
                return false;
            }

            // convert to text stream and populate contents
            QTextStream in(&qba);
            while (! in.atEnd() && endThreadNotRequested()) {
                QString line = in.readLine(0);
                _partFileContents << line.toLower();
            }

            // add content to ColourParts map
            insert(_partFileContents, libFileName,-1,isUnOffLib);

            // process file contents
            processFileContents(libFileName,isUnOffLib);

            emit progressSetValueSig(partCount++);
        }
    }
    emit progressSetValueSig(partCount);
    logInfo() << QString("Finished %1").arg(library);

    zip.close();
    if (zip.getZipError() != UNZ_OK) {
        logError() << QString("zip.close() zipError(): %1").arg(zip.getZipError());
        return false;
    }
    return true;
}

/*
 * parse colour part file to determine children parts with static colour.
 */
void ColourPartListWorker::processFileContents(const QString &libFileName, const bool isUnOffLib){

    QString materialColor  ="16";  // Internal Common Material Colour (main)
    QString edgeColor      ="24";  // Internal Common Material Color (edge)
    QString fileEntry;
    QString fileName;
    QString colour;
    QString libType        = isUnOffLib ? "U" : "O";
    QString libEntry       = libFileName;
    QString libFilePath    = libEntry.remove("/" + libEntry.split("/").last());
    //logTrace() << "Processing libFileName" << libFileName;
    bool hasColour = false;

    for (int i = 0; i < _partFileContents.size() && endThreadNotRequested(); i++){
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
                    fileName = libFileName.split("/").last();
                    emit messageSig(false,QString("Part: %1 \nhas no 'Name:' attribute. Using library path name %2 instead.\n"
                                                  "You may want to update the part content and fade colour parts list.")
                                    .arg(fileName).arg(libFileName));
                }
                fileEntry = QString("%1:::%2:::%3").arg(fileName).arg(libType).arg(_partFileContents[0].remove(0,2));
                remove(libFileName);
                //logNotice() << "Remove from list as it is a known colour part: " << libFileName;
                break;
            }
        } // token size
    }
    _partFileContents.clear();
    if(hasColour) {
         if (libFilePath != _filePath){
             fileSectionHeader(FADESTEP_COLOUR_PARTS_HEADER, QString("# Library path: %1").arg(libFilePath));
             _filePath = libFilePath;
         }
        _cpLines++;
        _fadeStepColourParts  << fileEntry.toLower();
        if (fileName.size() > _colWidthFileName)
            _colWidthFileName = fileName.size();
        //logNotice() << "ADD COLOUR PART: " << fileName << " libFileName: " << libFileName << " Colour: " << colour;
    }
}

void ColourPartListWorker::processChildren(){

    emit progressResetSig();
    emit progressMessageSig("Processing Child Colour Parts...");
    emit progressRangeSig(1, _partList.size());
    logInfo() << QString("Processing Child Colour Parts - Count: %1").arg(_partList.size());

    QString     filePath = "";
    for(int part = 0; part < _partList.size() && endThreadNotRequested(); part++){
        QString     parentFileNameStr;
        bool gotoMainLoop = false;

        emit progressSetValueSig(part);
        QMap<QString, ColourPart>::iterator ap = _colourParts.find(_partList[part]);

        if(ap != _colourParts.end()){

             QString libFileName = ap.value()._fileNameStr;
             QString libFilePath    = libFileName.remove("/" + libFileName.split("/").last());

            // process ColourPart content
            for (int i = 0; i < ap.value()._contents.size() && ! gotoMainLoop && endThreadNotRequested(); i++) {
                QString line =  ap.value()._contents[i];
                QStringList tokens;

                split(line,tokens);
                if (tokens.size() == 3 && line.contains("Name:", Qt::CaseInsensitive))
                    parentFileNameStr = tokens[tokens.size()-1];                            // short name of parent part

                if (tokens.size() == 15 && tokens[0] == "1") {

                    QString childFileNameStr = tokens[tokens.size()-1];                     // child part name in parent part
                    // check childFileName in _fadeStepColourParts list
                    for (int j = 0; j < _fadeStepColourParts.size() && ! gotoMainLoop && endThreadNotRequested(); ++j){

                        if (_fadeStepColourParts.at(j).contains(childFileNameStr) && _fadeStepColourParts.at(j).contains(QRegExp("\\b"+childFileNameStr.replace("\\","\\\\")+"[^\n]*"))){
                             if (libFilePath != filePath){
                                 fileSectionHeader(FADESTEP_COLOUR_CHILDREN_PARTS_HEADER, QString("# Library path: %1").arg(libFilePath));
                                 filePath = libFilePath;
                             }
                            _cpLines++;
                            QString fileEntry, libType;
                            libType = ap.value()._unOff ? "U" : "O";
                            fileEntry = QString("%1:::%2:::%3:::%4").arg(parentFileNameStr).arg(libType).arg(ap.value()._contents[0].remove(0,2));
                            _fadeStepColourParts  << fileEntry.toLower();
                            if (parentFileNameStr.size() > _colWidthFileName)
                                _colWidthFileName = parentFileNameStr.size();
                            //logInfo() << "ADD CHILD COLOUR PART: " << libFileName;
                            gotoMainLoop = true;
                        }
                    }
                }
            }
        }
    }
    emit progressSetValueSig(_partList.size());
    logInfo() << QString("Finished Processing Child Colour Parts.");
}

void ColourPartListWorker::writeFadeFile(bool append){

    if (! _fadeStepColourParts.empty())
    {
        QFileInfo colourFileList(Preferences::fadeStepColorPartsFile);
        QFile file(colourFileList.absoluteFilePath());
        if ( ! file.open(append ? QFile::Append | QFile::Text : QFile::WriteOnly | QFile::Text)) {
           logError() << QString("Failed to OPEN colourFileList %1 for writing:\n%2").arg(file.fileName()).arg(file.errorString());
            emit messageSig(false,QString("Failed to OPEN colourFileList %1 for writing:\n%2").arg(file.fileName()).arg(file.errorString()));
            return;
        }
        QTextStream out(&file);
        for (int i = 0; i < _fadeStepColourParts.size() && endThreadNotRequested(); i++) {
            QString cpLine = _fadeStepColourParts[i];
            if (cpLine.section(":::",0,0).split(".").last() == "dat") {
                QString partNumber      = cpLine.section(":::",0,0);
                QString libraryType     = cpLine.section(":::",1,1);
                QString partDescription = cpLine.section(":::",2,2);
                out << left << qSetFieldWidth(_colWidthFileName+1)    << partNumber
                            << qSetFieldWidth(9) << libraryType
                            << partDescription   << qSetFieldWidth(0) << endl;

            } else if (cpLine.section(":::",0,0) == "# File Name"){
                out << left << qSetFieldWidth(_colWidthFileName+1)    << cpLine.section(":::",0,0)
                            << qSetFieldWidth(9)           << cpLine.section(":::",1,1)
                            << cpLine.section(":::",2,2)   << qSetFieldWidth(0) << endl;

            } else {
                out << cpLine << endl;
            }
        }
        file.close();

        if(!append) {
            logInfo() << QString("Lines written to fadeStepColorParts.lst: %1").arg(_fadeStepColourParts.size()+5);
        }
        _fadeStepColourParts.clear();
    }
}

void ColourPartListWorker::insert(
        const QStringList   &contents,
        const QString       &fileNameStr,
        const int           &partType,
        const bool          &unOff){

    bool partErased = false;

    QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

    if (i != _colourParts.end()){
        _colourParts.erase(i);
        partErased = true;
//        qDebug() << "PART ALREADY IN LIST - PART ERASED" << i.value()._fileNameStr << ", UnOff Lib:" << i.value()._unOff;
    }
    ColourPart colourPartEntry(contents, fileNameStr, partType, unOff);
    _colourParts.insert(fileNameStr.toLower(), colourPartEntry);

    if (! partErased)
      _partList << fileNameStr.toLower();
    //logNotice() << "02 INSERT PART CONTENTS - UID: " << fileNameStr.toLower()  <<  " partType: " << partType <<  " unOff: " << unOff ;
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
        //logNotice() << "REMOVE COLOUR PART: " << fileNameStr.toLower() << " from contents and _partList";
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
        _fadeStepColourParts  << "# There are three defined columns:";
        _fadeStepColourParts  << "# 1. File Name: The part file name as defined in the LDraw Library.";
        _fadeStepColourParts  << "# 2. Lib Type: Indicator 'U'=Unofficial Library and 'O'= Official Library.";
        _fadeStepColourParts  << "# 3. Description: The part file description taken from the first line of the part file.";
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# Fade colour parts were generated from the following list of libraries and directories:";
        _fadeStepColourParts  << "";
        break;
      case FADESTEP_FILE_HEADER:
        _fadeStepColourParts  << "";
        _fadeStepColourParts  << "# File Name:::Lib Type:::Description";
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












