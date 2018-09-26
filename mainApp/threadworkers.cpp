/****************************************************************************
**
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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
#include "ldrawini.h"
#include "step.h"
#include "paths.h"
#include "lpub.h"
#include "application.h"

#ifdef WIN32
#include <clocale>
#endif // WIN32

PartWorker::PartWorker(QObject *parent) : QObject(parent)
{
  setDoFadeStep(Preferences::enableFadeSteps);
  setDoHighlightStep(Preferences::enableHighlightStep && !gui->suppressColourMeta());

  _resetSearchDirSettings = false;
  _endThreadNowRequested  = false;

  _excludedSearchDirs << ".";
  _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("parts"));
  _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("p"));
  _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("unofficial/parts"));
  _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("unofficial/p"));
  _customPartDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir));
  _customPrimDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrimDir));

}

/*
 * LDraw search directories preferences.
 */
void PartWorker::ldsearchDirPreferences(){

  bool fadeStep = (gui->page.meta.LPub.fadeStep.fadeStep.value() || Preferences::enableFadeSteps);
  bool highlightStep = (gui->page.meta.LPub.highlightStep.highlightStep.value() || Preferences::enableHighlightStep);
  setDoFadeStep(fadeStep);
  setDoHighlightStep(highlightStep && !gui->suppressColourMeta());

  if (!_resetSearchDirSettings) {
      emit Application::instance()->splashMsgSig("50% - Search directory preferences loading...");
    } else {
      emit messageSig(LOG_INFO,"Reset search directories...");
      emit messageSig(LOG_INFO,"Reset - search directory preferences loading...");
    }

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
      emit messageSig(LOG_STATUS, QString("Unable to initialize Ldrawini. Using default search directories."));
    }

  if (!doFadeStep() && !doHighlightStep()) {
      _excludedSearchDirs << _customPartDir;
      _excludedSearchDirs << _customPrimDir;
  } else {
      Paths::mkCustomDirs();
  }

  emit messageSig(LOG_INFO,(doFadeStep() ? QString("Fade Previous Steps is ON.") : QString("Fade Previous Steps is OFF.")));
  emit messageSig(LOG_INFO,(doHighlightStep() ? QString("Highlight Current Step is ON.") : QString("Highlight Current Step is OFF.")));

  if (!Preferences::ldrawiniFound && !_resetSearchDirSettings &&
      Settings.contains(QString("%1/%2").arg(SETTINGS,LdSearchDirsKey))) {    // ldrawini not found and not reset so load registry key
      emit messageSig(LOG_STATUS, QString("ldraw.ini not found, loading ldSearch directories from registry key..."));
      QStringList searchDirs = Settings.value(QString("%1/%2").arg(SETTINGS,LdSearchDirsKey)).toStringList();
      bool customDirsIncluded = false;
      foreach (QString searchDir, searchDirs){
          if (QDir(searchDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
              // Skip custom directory if not doFadeStep or not doHighlightStep
              QString customDir = QDir::toNativeSeparators(searchDir.toLower());
              if ((!doFadeStep() && !doHighlightStep()) && (customDir == _customPartDir.toLower() || customDir == _customPrimDir.toLower()))
                  continue;

              // If doFadeStep or doHighlightStep, check if custom directories included
              if ((doFadeStep() || doHighlightStep()) && !customDirsIncluded){
                  customDirsIncluded = (customDir.toLower() == _customPartDir.toLower() ||
                                      customDir.toLower() == _customPrimDir.toLower());
                }
              Preferences::ldSearchDirs << searchDir;
              emit messageSig(LOG_STATUS, QString("Add search directory: %1").arg(searchDir));
          } else {
              emit messageSig(LOG_STATUS, QString("Search directory is empty and will be ignored: %1").arg(searchDir));
          }
      }
      // If fade step enabled but custom directories not defined in ldSearchDirs, add custom directories
      if ((doFadeStep() || doHighlightStep()) && !customDirsIncluded) {
          // We must force the custom directories for LDView as they are needed by ldview ini files
          if (Preferences::preferredRenderer == RENDERER_LDVIEW) {
              Preferences::ldSearchDirs << _customPartDir;
              emit messageSig(LOG_STATUS, QString("Add custom part directory: %1").arg(_customPartDir));
              Preferences::ldSearchDirs << _customPrimDir;
              emit messageSig(LOG_STATUS, QString("Add custom primitive directory %1").arg(_customPrimDir));
              customDirsIncluded = true;
          } else {
              if (QDir(_customPartDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                Preferences::ldSearchDirs << _customPartDir;
                customDirsIncluded = true;
                emit messageSig(LOG_STATUS, QString("Add custom part directory: %1").arg(_customPartDir));
              } else {
                emit messageSig(LOG_STATUS, QString("Custom part directory is empty and will be ignored: %1").arg(_customPartDir));
              }
              if (QDir(_customPrimDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                Preferences::ldSearchDirs << _customPrimDir;
                customDirsIncluded = true;
                emit messageSig(LOG_STATUS, QString("Add custom primitive directory: %1").arg(_customPrimDir));
              } else {
                emit messageSig(LOG_STATUS, QString("custom primitive directory is empty and will be ignored: %1").arg(_customPrimDir));
              }
          }
          // update the registry if custom directory included
          if (customDirsIncluded){
              QSettings Settings;
              Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDSearchDirs"), Preferences::ldSearchDirs);
          }
       }
    } else if (loadLDrawSearchDirs()){                                        //ldraw.ini found or reset so load local paths
      Settings.setValue(QString("%1/%2").arg(SETTINGS,LdSearchDirsKey), Preferences::ldSearchDirs);
      emit messageSig(LOG_STATUS, QString("Ldraw.ini found or search directory reset selected, loading ldSearch directories..."));
    } else {
      Settings.remove(QString("%1/%2").arg(SETTINGS,LdSearchDirsKey));
      emit messageSig(LOG_ERROR, QString("Unable to load search directories."));
    }

    // Update LDView extra search directories
    if (!Preferences::setLDViewExtraSearchDirs(Preferences::ldviewIni))
       emit messageSig(LOG_ERROR, qPrintable(QString("Could not update %1").arg(Preferences::ldviewIni)));
    if (!Preferences::setLDViewExtraSearchDirs(Preferences::ldviewPOVIni))
       emit messageSig(LOG_ERROR, qPrintable(QString("Could not update %1").arg(Preferences::ldviewPOVIni)));
    if (!Preferences::setLDViewExtraSearchDirs(Preferences::nativePOVIni))
       emit messageSig(LOG_ERROR, qPrintable(QString("Could not update %1").arg(Preferences::nativePOVIni)));

    // Update LDGLite extra search directories
    if (Preferences::preferredRenderer == RENDERER_LDGLITE)
        populateLdgLiteSearchDirs();
}

/*
 * Load LDraw search directories into Preferences.
 */
bool PartWorker::loadLDrawSearchDirs(){

  if (!_resetSearchDirSettings) {
      emit Application::instance()->splashMsgSig("60% - Search directories loading...");
    } else {
      emit messageSig(LOG_INFO,"Reset - search directories loading...");
    }

  setDoFadeStep(gui->page.meta.LPub.fadeStep.fadeStep.value());
  StringList  ldrawSearchDirs;
  if (ldPartsDirs.loadLDrawSearchDirs("")){
      ldrawSearchDirs = ldPartsDirs.getLDrawSearchDirs();
      Preferences::ldSearchDirs.clear();
      bool foundUnofficialRootDir = false;
      bool customDirsIncluded = false;
      QString unofficialRootDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("unofficial"));
      for (StringList::const_iterator it = ldrawSearchDirs.begin();
           it != ldrawSearchDirs.end(); it++)
        {
          const char *dir = it->c_str();
          QString ldrawSearchDir = QString(dir);
          // check for Unofficial root directory
          if (!foundUnofficialRootDir)
            foundUnofficialRootDir = ldrawSearchDir.toLower() == unofficialRootDir.toLower();

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
                  emit messageSig(LOG_STATUS, QString("Add search directory: %1").arg(ldrawSearchDir));
                }
            }
          // Check if custom directories included
          if ((doFadeStep() || doHighlightStep()) && !customDirsIncluded){
              customDirsIncluded = (ldrawSearchDir.toLower() == _customPartDir.toLower() ||
                                  ldrawSearchDir.toLower() == _customPrimDir.toLower());
            }
        }
      // If fade step enabled but custom directories not defined in ldSearchDirs, add custom directories
      if ((doFadeStep() || doHighlightStep()) && !customDirsIncluded) {
          if (QDir(_customPartDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
              Preferences::ldSearchDirs << _customPartDir;
              emit messageSig(LOG_STATUS, QString("Add custom part directory: %1").arg(_customPartDir));
            } else {
              emit messageSig(LOG_STATUS, QString("Custom part directory is empty and will be ignored: %1").arg(_customPartDir));
            }
          if (QDir(_customPrimDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
              Preferences::ldSearchDirs << _customPrimDir;
              emit messageSig(LOG_STATUS, QString("Add custom primitive directory: %1").arg(_customPrimDir));
            } else {
              emit messageSig(LOG_STATUS, QString("Custom primitive directory is empty and will be ignored: %1").arg(_customPrimDir));
            }
        }
      // Add subdirectories from Unofficial root directory
      if (foundUnofficialRootDir) {
          QDir unofficialDir(unofficialRootDir);
          // Get sub directories
          QStringList unofficialSubDirs = unofficialDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::SortByMask);
          //logDebug() << "unofficialSubDirs:" << unofficialSubDirs;
          if (unofficialSubDirs.count() > 0){
              // Recurse unofficial subdirectories for excluded directories
              foreach (QString unofficialSubDirName, unofficialSubDirs){
                  // Exclude invalid directories
                  bool excludeSearchDir = false;
                  QString unofficialSubDir = QDir::toNativeSeparators(QString("%1/%2").arg(unofficialRootDir).arg(unofficialSubDirName));
                  foreach (QString excludedDir, _excludedSearchDirs){
                      if ((excludeSearchDir =
                           unofficialSubDir.toLower() == excludedDir.toLower())) {
                          break;
                        }
                    }
                  if (!excludeSearchDir){
                      // check if empty
                      if (QDir(unofficialSubDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                          Preferences::ldSearchDirs << unofficialSubDir;
                          emit messageSig(LOG_STATUS, QString("Add search directory: %1").arg(unofficialSubDir));
                        } else if (QDir(unofficialSubDir).entryInfoList(QDir::Dirs|QDir::NoSymLinks).count() > 0) {
                          QDir subSubDir(unofficialSubDir);
                          QStringList subSubDirs = subSubDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::SortByMask);
                          foreach (QString subSubDirName, subSubDirs){
                              QString unofficialSubSubDir = QDir::toNativeSeparators(QString("%1/%2").arg(unofficialSubDir).arg(subSubDirName));
                              if (QDir(unofficialSubSubDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                                  Preferences::ldSearchDirs << unofficialSubSubDir;
                                  emit messageSig(LOG_STATUS, QString("Add search directory: %1").arg(unofficialSubSubDir));
                                } else {
                                  emit messageSig(LOG_STATUS, QString("Search directory is empty and will be ignored: %1").arg( unofficialSubSubDir));
                                }
                            }
                        } else {
                          emit messageSig(LOG_STATUS, QString("Search directory is empty and will be ignored: %1").arg( unofficialSubDir));
                        }
                    }
                }
            }
        }

      if (_resetSearchDirSettings) {
          processLDSearchDirParts();
        }

    } else {
      emit messageSig(LOG_ERROR, QString("ldPartsDirs.loadLDrawSearchDirs("") failed."));
      return false;
    }
  return true;
}

/* Add qualified search directories to LDSEARCHDIRS string
   This is used to pass search directories to ldglite.
   This function will only execute if the preferred renderer is LDGLite
   and there are more than 0 search directories in Preferences::ldgliteSearchDirs.
*/
void PartWorker::populateLdgLiteSearchDirs() {
    if (Preferences::preferredRenderer == RENDERER_LDGLITE && !Preferences::ldSearchDirs.isEmpty()){

        emit Application::instance()->splashMsgSig("85% - LDGlite Search directories loading...");
        //logDebug() << "SEARCH DIRECTORIES TO PROCESS" << Preferences::ldSearchDirs ;
        emit messageSig(LOG_STATUS, QString("LDGlite Search Directories..."));

        // Define excluded directories
        QStringList ldgliteExcludedDirs = _excludedSearchDirs;
        ldgliteExcludedDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("models"))
                            << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("unofficial"))
                            << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("unofficial/lsynth"));
        // Clear directories
        Preferences::ldgliteSearchDirs.clear();
        int count = 0;                    // set delimeter from 2nd entry
        // Recurse ldraw search directories
        foreach (QString ldgliteSearchDir, Preferences::ldSearchDirs){
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
                    emit messageSig(LOG_STATUS, QString("Add ldglite search directory: %1").arg(ldgliteSearchDir));
                }else {
                    emit messageSig(LOG_STATUS, QString("Ldglite search directory is empty and will be ignored: %1").arg(ldgliteSearchDir));
                }
            }
        }
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
 * scan LDraw library files for static colored parts and create fade copy
 */
void PartWorker::processFadeColourParts(bool overwriteCustomParts)
{
  //qDebug() << qPrintable(QString("Received overwrite fade parts = %1").arg(overwriteCustomParts ? "True" : "False"));
  if (doFadeStep())
    processCustomColourParts(FADE_PART, overwriteCustomParts);
}

/*
 * scan LDraw library files for static colored parts and create highlight copy
 */
void PartWorker::processHighlightColourParts(bool overwriteCustomParts)
{
  //qDebug() << qPrintable(QString("Received overwrite highlight parts = %1").arg(overwriteCustomParts ? "True" : "False"));
  if (doHighlightStep())
    processCustomColourParts(HIGHLIGHT_PART, overwriteCustomParts);
}

void PartWorker::processCustomColourParts(PartType partType, bool overwriteCustomParts)
{
  QString nameMod;
  if (partType == FADE_PART)
    nameMod = "fade";
  else if (partType == HIGHLIGHT_PART)
    nameMod = "highlight";

  _timer.start();
  _customParts = 0;

  QStringList customPartsDirs;
  QStringList contents;
  QStringList colourPartList;
  int existingCustomParts = 0;

  emit progressBarInitSig();
  emit progressMessageSig("Parse Model File");
  Paths::mkCustomDirs();

  ldrawFile = gui->getLDrawFile();
  // process top-level submodels
  emit progressRangeSig(1, ldrawFile._subFileOrder.size());

  for (int i = 0; i < ldrawFile._subFileOrder.size() && endThreadNotRequested(); i++) {
      QString subfileNameStr = ldrawFile._subFileOrder[i].toLower();
      contents = ldrawFile.contents(subfileNameStr);
      emit progressSetValueSig(i);
      emit messageSig(LOG_INFO,QString("00 PROCESSING SUBFILE: %1").arg(subfileNameStr));
      for (int i = 0; i < contents.size() && endThreadNotRequested(); i++) {
          QString line = contents[i];
          QStringList tokens;
          split(line,tokens);
          if (tokens.size() == 15 && tokens[0] == "1") {
              // check if colored part and create custom version if yes
              QString fileNameStr = gui->ldrawColourParts.getLDrawColourPartInfo(tokens[tokens.size()-1]);
              // validate part is static colour part;
              if (!fileNameStr.isEmpty()){
                  bool entryExists = false;
                  QString dirName = fileNameStr.section(":::",1,1).split("\\").first();
                  QString fileName = fileNameStr.section(":::",1,1).split("\\").last();
                  //logDebug() << "FileDir:" << dirName << "FileName:" << fileName;
                  QDir customFileDirPath;
                  if (dirName == fileName){
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir));
                  } else  if (dirName == "s"){
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customSubDir));
                  } else  if (dirName == "p"){
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrimDir));
                  } else  if (dirName == "8"){
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim8Dir));
                  } else if (dirName == "48"){
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim48Dir));
                  } else {
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir));
                  }
                  QFileInfo customFileInfo(customFileDirPath,fileName.replace(".dat", "-" + nameMod + ".dat"));
                  if(customFileInfo.exists()){
                      logNotice() << "01 COLOUR PART EXIST - IGNORING:" << fileNameStr;
                      entryExists = true;
                      if (!customPartsDirs.contains(customFileInfo.absolutePath()))
                        customPartsDirs << customFileInfo.absolutePath();
                      existingCustomParts++;
                  }
                  // check if part entry already in list
                  if (!entryExists)
                      foreach(QString colourPart, colourPartList){
                          if (colourPart == fileNameStr){
                              entryExists = true;
                              break;
                          }
                      }
                  // add part entry to list
                  if (!entryExists) {
                      logNotice() << "01 SUBMIT COLOUR PART INFO:" << fileNameStr << " Line: " << i ;
                      colourPartList << fileNameStr;
                  }
              }
          }
      }
  }

  emit progressSetValueSig(ldrawFile._subFileOrder.size());

  // We have new parts to be created.
  if (colourPartList.size() > 0) {
      // check if part has children color part(s)
      if (!processColourParts(colourPartList, partType)) {
          QString error = QString("Process %1 colour parts failed!.").arg(nameMod);
          emit messageSig(LOG_ERROR,error);
          emit progressStatusRemoveSig();
          emit customColourFinishedSig();
          return;
      }

      // Create the custom part
      createCustomPartFiles(partType);

      // Populate custom parts dirs
      foreach(QDir customDir, Paths::customDirs){
          if(customDir.entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0)
              customPartsDirs << customDir.absolutePath();
      }
      // Remove Duplicates
      customPartsDirs = customPartsDirs.toSet().toList();

      if (_customParts > 0 && customPartsDirs.size() > 0) {
          // transfer to ldSearchDirs
          bool updateLDGLiteSearchDirs = false;
          foreach (QString customPartDir, customPartsDirs){
              bool customDirsIncluded = false;
              QString customDir = QDir::toNativeSeparators(customPartDir.toLower());
              // check if custom directories included
              foreach(QString ldSearchDir, Preferences::ldSearchDirs ) {
                  QString searchDir = QDir::toNativeSeparators(ldSearchDir.toLower());
                  if (customDirsIncluded)
                    break;
                  customDirsIncluded = (searchDir == customDir);
              }
              // If not included add custom directories and update registry and LDView and LDGLite extra search directories
              if (!customDirsIncluded){
                  Preferences::ldSearchDirs << QDir::toNativeSeparators(customPartDir);
                  logDebug() << "Add " + nameMod + " part directory to ldSearchDirs:" << customPartDir;
                  QSettings Settings;
                  Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDSearchDirs"), Preferences::ldSearchDirs);

                  if (!Preferences::setLDViewExtraSearchDirs(Preferences::ldviewIni))
                     emit messageSig(LOG_ERROR, qPrintable(QString("Could not update %1").arg(Preferences::ldviewIni)));
                  if (!Preferences::setLDViewExtraSearchDirs(Preferences::ldviewPOVIni))
                     emit messageSig(LOG_ERROR, qPrintable(QString("Could not update %1").arg(Preferences::ldviewPOVIni)));
                  if (!Preferences::setLDViewExtraSearchDirs(Preferences::nativePOVIni))
                     emit messageSig(LOG_ERROR, qPrintable(QString("Could not update %1").arg(Preferences::nativePOVIni)));
                  updateLDGLiteSearchDirs = true;
              }
          }
          if (updateLDGLiteSearchDirs)
              populateLdgLiteSearchDirs();
      }
  }

  // Archive custom parts
  if (colourPartList.size() > 0 || (existingCustomParts > 0 && overwriteCustomParts)){
      // Process archive files
      QString comment = QString("colour %1").arg(nameMod);
      if (!processPartsArchive(customPartsDirs, comment, overwriteCustomParts)){
          QString error = QString("Process %1 parts archive failed!.").arg(nameMod);
          emit messageSig(LOG_ERROR,error);
          emit progressStatusRemoveSig();
          emit customColourFinishedSig();
          return;
      }
  }

  // summary stats
  int secs = _timer.elapsed() / 1000;
  int mins = (secs / 60) % 60;
  secs = secs % 60;
  int msecs = _timer.elapsed() % 1000;

  QString time = QString("Elapsed time is %1:%2:%3")
  .arg(mins, 2, 10, QLatin1Char('0'))
  .arg(secs,  2, 10, QLatin1Char('0'))
  .arg(msecs,  3, 10, QLatin1Char('0'));

  int allCustomParts = _customParts += existingCustomParts;

  QString fileStatus = allCustomParts > 0 ? QString("%1 %2 %3 created. Archive library %4. %5.")
                                                    .arg(_customParts)
                                                    .arg(nameMod)
                                                    .arg(allCustomParts == 1 ? "part" : "parts")
                                                    .arg(_partsArchived ? "updated" : "not updated, custom parts already archived")
                                                    .arg(time) :
                                             QString("No %2 parts created.").arg(nameMod);

  emit progressStatusRemoveSig();
  emit customColourFinishedSig();
  emit messageSig(LOG_STATUS,fileStatus);

  emit messageSig(LOG_INFO,fileStatus);
}

bool PartWorker::processColourParts(const QStringList &colourPartList, const PartType partType) {

    QString nameMod;
    if (partType == FADE_PART)
      nameMod = "fade";
    else if (partType == HIGHLIGHT_PART)
      nameMod = "highlight";

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
        emit messageSig(LOG_ERROR, fileStatus);
        return false;
    }

    emit progressResetSig();
    emit progressMessageSig("Process Colour Parts...");
    emit progressRangeSig(1, colourPartList.size());

    int partCount = 0;
    int partsProcessed = 0;
    QStringList childrenColourParts;

    foreach (QString partEntry, colourPartList) {

        bool partFound = false;

        QString cpPartEntry = partEntry;
        bool unOffLib = cpPartEntry.section(":::",0,0) == "u";
        //emit messageSig(LOG_INFO,QString("Library Type: %1").arg((unOffLib ? "Unofficial Library" : "Official Library"));

        QString libPartName = cpPartEntry.section(":::",1,1).split("\\").last();
        //emit messageSig(LOG_INFO,QString("Lib Part Name: %1").arg(libPartName));

        QString libDirName = cpPartEntry.section(":::",1,1).split("\\").first();

        emit progressSetValueSig(partCount++);

        QuaZip zip(unOffLib ? unofficialLib : officialLib);
        if (!zip.open(QuaZip::mdUnzip)) {
            emit messageSig(LOG_ERROR, QString("Failed to open archive: %1 @ %2").arg(zip.getZipError()).arg(unOffLib ? unofficialLib : officialLib));
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
                    emit messageSig(LOG_ERROR, QString("Failed to OPEN Part file :%1").arg(zip.getCurrentFileName()));
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
                        // validate part is static colour part;
                        QString childFileNameStr = gui->ldrawColourParts.getLDrawColourPartInfo(tokens[tokens.size()-1]);
                        // validate part is static colour part;
                        if (!childFileNameStr.isEmpty()){
                            bool entryExists = false;
                            QString dirName = childFileNameStr.section(":::",1,1).split("\\").first();
                            QString fileName = childFileNameStr.section(":::",1,1).split("\\").last();
                            //logDebug() << "FileDir:" << dirName << "FileName:" << fileName;
                            QDir customFileDirPath;
                            if (dirName == fileName){
                                customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir));
                            } else  if (dirName == "s"){
                                customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customSubDir));
                            } else  if (dirName == "p"){
                                customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrimDir));
                            } else  if (dirName == "8"){
                                customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim8Dir));
                            } else if (dirName == "48"){
                                customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim48Dir));
                            } else {
                                customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir));
                            }
                            QFileInfo customFileInfo(customFileDirPath,fileName.replace(".dat", "-" + nameMod + ".dat"));
                            if(customFileInfo.exists()){
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
                int ldrawPartType = -1;
                if (libDirName == libPartName){
                    ldrawPartType=LD_PARTS;
                } else  if (libDirName == "s"){
                    ldrawPartType=LD_SUB_PARTS;
                } else  if (libDirName == "p"){
                    ldrawPartType=LD_PRIMITIVES;
                } else  if (libDirName == "8"){
                    ldrawPartType=LD_PRIMITIVES_8;
                } else if (libDirName == "48"){
                    ldrawPartType=LD_PRIMITIVES_48;
                } else {
                    ldrawPartType=LD_PARTS;
                }
                // add content to ColourParts map
                insert(_partFileContents, libPartName, ldrawPartType, true);
                _partFileContents.clear();
                partsProcessed++;
                break;

            } else if (libPartFile.fileName().toLower() == libPartName && partAlreadyInList(libPartName)) {
                partFound = true;
                logTrace() << "Part already in list:" << libPartName;
                break;
            }
        }
        if (!partFound) {
            fileStatus = QString("Part file %1 not found in %2. Be sure the fadeStepColorParts.lst file is up to date.").arg(libPartName).arg(unOffLib ? "Unofficial Library" : "Official Library");
            emit messageSig(LOG_ERROR, fileStatus);
        }

        zip.close();

        if (zip.getZipError() != UNZ_OK) {
            emit messageSig(LOG_ERROR, QString("zip close error: %1").arg(zip.getZipError()));
            return false;
        }
    }
    emit progressSetValueSig(colourPartList.size());

    // recurse part file content to check if any children are colour parts
    if (childrenColourParts.size() > 0)
        processColourParts(childrenColourParts, partType);

    QString message = QString("%1 Colour %2 content processed.")
        .arg(partsProcessed)
        .arg(partsProcessed > 1 ? "parts" : "part");
    emit messageSig(LOG_STATUS,message);
    emit messageSig(LOG_INFO,message);

    return true;
}


bool PartWorker::createCustomPartFiles(const PartType partType){

     QString nameMod, colourPrefix;
     if (partType == FADE_PART){
       nameMod = "fade";
       colourPrefix = LPUB3D_COLOUR_FADE_PREFIX;
     } else if (partType == HIGHLIGHT_PART) {
       nameMod = "highlight";
       colourPrefix = LPUB3D_COLOUR_HIGHLIGHT_PREFIX;
     }

    int maxValue            = _partList.size();
    emit progressResetSig();
    emit progressMessageSig("Creating Costom Colour Parts");
    emit progressRangeSig(1, maxValue);

    QStringList customPartContent, customPartColourList;
    QString customPartFile;

    for(int part = 0; part < _partList.size() && endThreadNotRequested(); part++) {

        emit progressSetValueSig(part);

        QMap<QString, ColourPart>::iterator cp = _colourParts.find(_partList[part]);

        if(cp != _colourParts.end()){

            // prepare absoluteFilePath for costom file
            QDir customPartDirPath;
            switch (cp.value()._partType)
            {
            case LD_PARTS:
                customPartDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir));
                break;
            case LD_SUB_PARTS:
                customPartDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customSubDir));
                break;
            case LD_PRIMITIVES:
                customPartDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrimDir));
                break;
            case LD_PRIMITIVES_8:
                customPartDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim8Dir));
                break;
            case LD_PRIMITIVES_48:
                customPartDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim48Dir));
                break;
            }
            QString customFile = cp.value()._fileNameStr;
            QFileInfo customStepColourPartFileInfo(customPartDirPath,customFile.replace(".dat", "-" + nameMod + ".dat"));
            if (customStepColourPartFileInfo.exists()){
                logNotice() << "PART ALREADY EXISTS: " << customStepColourPartFileInfo.absoluteFilePath();
                continue;
            } else {
                logNotice() << "CREATE CUSTOM PART: " << customStepColourPartFileInfo.absoluteFilePath();
            }
            customPartFile = customStepColourPartFileInfo.absoluteFilePath();
            //logTrace() << "A. PART CONTENT ABSOLUTE FILEPATH: " << customStepColourPartFileInfo.absoluteFilePath();

            bool FadeMetaAdded = false;
            bool SilhouetteMetaAdded = false;

            // process costom part contents
            for (int i = 0; i < cp.value()._contents.size() && endThreadNotRequested(); i++) {
                QString line =  cp.value()._contents[i];
                QStringList tokens;
                QString fileNameStr;

                split(line,tokens);
                if (tokens.size() == 15 && tokens[0] == "1") {
                    // Insert opening fade meta
                    if (!FadeMetaAdded && Preferences::enableFadeSteps && partType == FADE_PART){
                       customPartContent.insert(i,QString("0 !FADE %1").arg(Preferences::fadeStepsOpacity));
                       FadeMetaAdded = true;
                    }
                    // Insert opening silhouette meta
                    if (!SilhouetteMetaAdded && Preferences::enableHighlightStep && partType == HIGHLIGHT_PART){
                       customPartContent.insert(i,QString("0 !SILHOUETTE %1 %2")
                                                          .arg(Preferences::highlightStepLineWidth)
                                                          .arg(Preferences::highlightStepColour));
                       SilhouetteMetaAdded = true;
                    }
                    fileNameStr = tokens[tokens.size()-1].toLower();
                    QString searchFileNameStr = fileNameStr;
                    // check if part at this line has a matching colour part in the colourPart list - if yes, rename with '-fade' or '-highlight'
                    searchFileNameStr = searchFileNameStr.split("\\").last();
                    QMap<QString, ColourPart>::iterator cpc = _colourParts.find(searchFileNameStr);
                    if (cpc != _colourParts.end()){
                        if (cpc.value()._fileNameStr == searchFileNameStr){
                            fileNameStr = fileNameStr.replace(".dat", "-" + nameMod + ".dat");
                        }
                    }
                    tokens[tokens.size()-1] = fileNameStr;
                }
                // check if coloured line...
                if((tokens.size() && tokens[0].size() == 1    &&
                    tokens[0] >= "1" && tokens[0] <= "5")     &&
                    (tokens[1] != LDRAW_MAIN_MATERIAL_COLOUR) &&
                    (tokens[1] != LDRAW_EDGE_MATERIAL_COLOUR)) {
                    //QString oldColour(tokens[1]);          //logging only: show colour lines
                    QString colourCode;
                    // Insert color code for fade part
                    if (partType == FADE_PART){
                        // generate costom colour entry - if fadeStepsUseColour, set colour to material colour (16), without prefix
                        colourCode = Preferences::fadeStepsUseColour ? LDRAW_MAIN_MATERIAL_COLOUR : tokens[1];
                        // add colour line to local list - if fadeStepsUseColour, no need to create entry
                        if (!Preferences::fadeStepsUseColour && !gui->colourEntryExist(customPartColourList,colourCode,partType))
                            customPartColourList << gui->createColourEntry(colourCode,partType);
                        // set costom colour - if fadeStepsUseColour, do not add costom colour prefix
                        tokens[1] = Preferences::fadeStepsUseColour ? colourCode : QString("%1%2").arg(colourPrefix).arg(colourCode);
                        //logTrace() << "D. CHANGE CHILD PART COLOUR: " << fileNameStr << " NewColour: " << tokens[1] << " OldColour: " << oldColour;
                    }
                    // Insert color code for silhouette part
                    if (partType == HIGHLIGHT_PART){
                        // generate costom colour entry - always
                        colourCode = tokens[1];
                        // add colour line to local list - always request to create entry
                        if (!gui->colourEntryExist(customPartColourList,colourCode,partType))
                            customPartColourList << gui->createColourEntry(colourCode,partType);
                        // set costom colour - if fadeStepsUseColour, do not add costom colour prefix
                        tokens[1] = QString("%1%2").arg(colourPrefix).arg(colourCode);
                    }
                }
                line = tokens.join(" ");
                customPartContent << line;

                // Insert closing fade and silhouette metas
                if (i+1 == cp.value()._contents.size()){
                    if (FadeMetaAdded){
                       customPartContent.append(QString("0 !FADE"));
                    }
                    if (SilhouetteMetaAdded){
                       customPartContent.append(QString("0 !SILHOUETTE"));
                    }
                }
            }

            // add the costom part colour list to the header of the costom part contents
            customPartColourList.toSet().toList(); // remove dupes

            int insertionPoint = 0; // skip the first line (title)
            QStringList words;
            // scan past header...
            for (int i = insertionPoint; i < customPartContent.size(); i++) {
                insertionPoint = i;
                // Upper case first title first letter
                if (insertionPoint == 0){
                   QString line = customPartContent[i];
                   split(line, words);
                   for (int j = 0; j < words.size(); j++){
                      QString word = QString(words[j]);
                      word[0]      = word[0].toUpper();
                      words[j]     = word;
                   }
                   words << (partType == FADE_PART ? "- Fade" : "- Highlight");
                   customPartContent[i] = words.join(" ");
                }
                else
                if (!isHeader(customPartContent[i]) && !QString(customPartContent[i]).isEmpty())
                  break;
            }
            // insert colour entries after header
            customPartContent.insert(insertionPoint,"0 // LPub3D part custom colours");
            for (int i = 0; i < customPartColourList.size(); i++) {
                insertionPoint++;
                customPartContent.insert(insertionPoint,customPartColourList.at(i));
            }
            customPartContent.insert(++insertionPoint,"0");

            //logTrace() << "04 SAVE CUSTGOM COLOUR PART: " << customPartFile;
            if(saveCustomFile(customPartFile, customPartContent))
                _customParts++;

            customPartContent.clear();
            customPartColourList.clear();
        }
    }
    emit progressSetValueSig(maxValue);
    return true;
}


/*
 * Write custom part files to costom directory.
 */
bool PartWorker::saveCustomFile(
        const QString     &fileName,
        const QStringList &customPartContent) {

    QFile file(fileName);
    if ( ! file.open(QFile::WriteOnly | QFile::Text)) {
        QString message = QString("Failed to open %1 for writing: %2").arg(fileName).arg(file.errorString());
        emit messageSig(LOG_ERROR, message);
        return false;

    } else {
        QTextStream out(&file);
        for (int i = 0; i < customPartContent.size(); i++) {

            out << customPartContent[i] << endl;
        }
        file.close();
        logNotice() << "05 WRITE CUSTOM PART TO DISC:" << fileName;
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


bool PartWorker::processPartsArchive(const QStringList &ldPartsDirs, const QString &comment, bool overwriteCustomParts){

  // Append costom parts to unofficial library for 3D Viewer's consumption
  QFileInfo libFileInfo(Preferences::lpub3dLibFile);
  QString archiveFile = QDir::toNativeSeparators(QString("%1/%2").arg(libFileInfo.absolutePath(),VER_LPUB3D_UNOFFICIAL_ARCHIVE));
  QString returnMessage = QString("Archiving %1 parts to : %2.").arg(comment,archiveFile);
  emit messageSig(LOG_INFO,"Archive parts...");
  emit messageSig(LOG_INFO,QString("Archiving %1 parts to %2.").arg(comment,archiveFile));

  if (okToEmitToProgressBar()) {
      emit progressResetSig();
    } else {
      emit Application::instance()->splashMsgSig(QString("60% - Archiving %1 parts...").arg(comment));
    }

  if (okToEmitToProgressBar())
      emit progressRangeSig(0, 0);

  int partCount = 0;
  int totalPartCount = 0;

  for (int i = 0; i < ldPartsDirs.size(); i++){

      QDir foo = ldPartsDirs[i];

      if (!archiveParts.Archive( archiveFile,
                                 foo.absolutePath(),
                                 returnMessage,
                                 QString("Append %1 parts").arg(comment),
                                 overwriteCustomParts))
      {
         emit messageSig(LOG_ERROR,returnMessage);
         continue;
      }
      bool ok;
      partCount = returnMessage.toInt(&ok);
      QString summary;
      if (ok){
          totalPartCount += partCount;
          summary = totalPartCount == 0 ? "parts" :
                    totalPartCount == 1 ? tr("[Total %1] part").arg(totalPartCount) :
                                          tr("[Total %1] parts").arg(totalPartCount);

      }
      emit messageSig(LOG_INFO,tr("Archived %1 %2 from %3").arg(partCount).arg(summary).arg(foo.absolutePath()));
  }

  // Reload unofficial library parts into memory - only if initial library load already done !
  QString partsLabel = "parts";
  if (Preferences::lpub3dLoaded && totalPartCount > 0) {

      if (!gApplication->mLibrary->ReloadUnoffLib()){
          returnMessage = tr("Failed to reload unofficial parts library into memory.");
          emit messageSig(LOG_ERROR,returnMessage);
          return false;
      } else {
          partsLabel = totalPartCount == 1 ? "part" : "parts";
          returnMessage = tr("Reloaded unofficial library into memory with %1 new %2.").arg(totalPartCount).arg(partsLabel);
          emit messageSig(LOG_INFO,returnMessage);
      }
  } else if (totalPartCount > 0) {
      partsLabel = totalPartCount == 1 ? "part" : "parts";
      returnMessage = tr("Finished. Archived and loaded %1 %2 %3 into memory.").arg(totalPartCount).arg(comment).arg(partsLabel);
      _partsArchived = true;
  } else {
      returnMessage = tr("Finished. No %1 parts archived. Unofficial library not reloaded.").arg(comment);
      _partsArchived = false;
  }

  emit messageSig(LOG_INFO,returnMessage);
  if (!okToEmitToProgressBar()) {
      emit Application::instance()->splashMsgSig(tr("70% - Finished archiving %1 parts.").arg(comment));
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

void ColourPartListWorker::generateCustomColourPartsList()
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
    _ldrawStaticColourParts  << "# Archive Libraries:";
    foreach (QString archiveFile, archiveFiles) {
      QString library = archiveFile == lpub3dLibFileInfo.absoluteFilePath() ? "Official Library" : "Unofficial Library";
      _ldrawStaticColourParts  << QString("# %1. %2: %3").arg(libCount++).arg(library).arg(archiveFile);
    }
    int dirCount = 1;
    _ldrawStaticColourParts  << "";
    _ldrawStaticColourParts  << "# Library Directories:";
    foreach (QString partTypeDir, partTypeDirs){
      _ldrawStaticColourParts  << QString("# %1. %2").arg(dirCount++).arg(partTypeDir);
    }
    fileSectionHeader(FADESTEP_FILE_HEADER);

    emit progressBarInitSig();
    foreach (QString archiveFile, archiveFiles) {
       if(!processArchiveParts(archiveFile)){
           QString error = QString("Process colour parts list failed!.");
           emit messageSig(LOG_ERROR,error);
           emit progressStatusRemoveSig();
           emit colourPartListFinishedSig();
           return;
       }
    }

    processChildren();

    writeLDrawColourPartFile();

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
    writeLDrawColourPartFile(append);

    emit progressStatusRemoveSig();
    emit colourPartListFinishedSig();
    emit messageSig(LOG_STATUS, fileStatus);

    emit messageSig(LOG_INFO,fileStatus);
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
        emit messageSig(LOG_ERROR, QString("! zip.open(): %1 @ %2").arg(zip.getZipError()).arg(archiveFile));
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
    emit messageSig(LOG_INFO,QString("Processing %1 - Parts Count: %2").arg(library).arg(partCount));

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
                emit messageSig(LOG_ERROR, QString("Failed to OPEN Part file :%1").arg(libFileName));
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
    emit messageSig(LOG_INFO,QString("Finished %1").arg(library));

    zip.close();
    if (zip.getZipError() != UNZ_OK) {
        emit messageSig(LOG_ERROR, QString("zip.close() zipError(): %1").arg(zip.getZipError()));
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

        //emit messageSig(LOG_INFO, QString("File contents CHECK: %1").arg(line));
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
                //emit messageSig(LOG_INFO,QString("File contents VERIFY: %1  COLOUR: %2 %3").arg(line).arg(colour));
                if (fileName.isEmpty()){
                    fileName = libFileName.split("/").last();
                    emit messageSig(LOG_ERROR,QString("Part: %1 \nhas no 'Name:' attribute. Using library path name %2 instead.\n"
                                                      "You may want to update the part content and costom colour parts list.")
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
        _ldrawStaticColourParts  << fileEntry.toLower();
        if (fileName.size() > _colWidthFileName)
            _colWidthFileName = fileName.size();
        //logNotice() << "ADD COLOUR PART: " << fileName << " libFileName: " << libFileName << " Colour: " << colour;
    }
}

void ColourPartListWorker::processChildren(){

    emit progressResetSig();
    emit progressMessageSig("Processing Child Colour Parts...");
    emit progressRangeSig(1, _partList.size());
    emit messageSig(LOG_INFO,QString("Processing Child Colour Parts - Count: %1").arg(_partList.size()));

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
                    // check childFileName in _ldrawStaticColourParts list
                    for (int j = 0; j < _ldrawStaticColourParts.size() && ! gotoMainLoop && endThreadNotRequested(); ++j){

                        if (_ldrawStaticColourParts.at(j).contains(childFileNameStr) && _ldrawStaticColourParts.at(j).contains(QRegExp("\\b"+childFileNameStr.replace("\\","\\\\")+"[^\n]*"))){
                             if (libFilePath != filePath){
                                 fileSectionHeader(FADESTEP_COLOUR_CHILDREN_PARTS_HEADER, QString("# Library path: %1").arg(libFilePath));
                                 filePath = libFilePath;
                             }
                            _cpLines++;
                            QString fileEntry, libType;
                            libType = ap.value()._unOff ? "U" : "O";
                            fileEntry = QString("%1:::%2:::%3:::%4").arg(parentFileNameStr).arg(libType).arg(ap.value()._contents[0].remove(0,2));
                            _ldrawStaticColourParts  << fileEntry.toLower();
                            if (parentFileNameStr.size() > _colWidthFileName)
                                _colWidthFileName = parentFileNameStr.size();
                            //emit messageSig(LOG_INFO,QString("ADD CHILD COLOUR PART: %1").arg(libFileName));
                            gotoMainLoop = true;
                        }
                    }
                }
            }
        }
    }
    emit progressSetValueSig(_partList.size());
    emit messageSig(LOG_INFO,QString("Finished Processing Child Colour Parts."));
}

void ColourPartListWorker::writeLDrawColourPartFile(bool append){

    if (! _ldrawStaticColourParts.empty())
    {
        QFileInfo colourFileList(Preferences::ldrawColourPartsFile);
        QFile file(colourFileList.absoluteFilePath());
        if ( ! file.open(append ? QFile::Append | QFile::Text : QFile::WriteOnly | QFile::Text)) {
            emit messageSig(LOG_ERROR,QString("Failed to OPEN colourFileList %1 for writing:\n%2").arg(file.fileName()).arg(file.errorString()));
            return;
        }
        QTextStream out(&file);
        for (int i = 0; i < _ldrawStaticColourParts.size() && endThreadNotRequested(); i++) {
            QString cpLine = _ldrawStaticColourParts[i];
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
            messageSig(LOG_INFO,QString("Lines written to %1: %2")
                         .arg(VER_LDRAW_COLOR_PARTS_FILE)
                         .arg(_ldrawStaticColourParts.size()+5));
        }
        _ldrawStaticColourParts.clear();
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
        _ldrawStaticColourParts  << QString("# File: %1  Generated on: %2")
                                    .arg(VER_LDRAW_COLOR_PARTS_FILE)
                                    .arg(QDateTime::currentDateTime().toString(fmtDateTime));
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# This list captures the LDraw static colour parts (and their subfiles) to support step fade";
        _ldrawStaticColourParts  << "# and step highlight.";
        _ldrawStaticColourParts  << "# Parts on this list are identified in the LDraw library and copied to their respective";
        _ldrawStaticColourParts  << "# custom directory.";
        _ldrawStaticColourParts  << "# Copied files are modified as described in the following lines.";
        _ldrawStaticColourParts  << "# If fade step is enabled, colour codes are replaced with a custom code using the standard";
        _ldrawStaticColourParts  << "# colour code prefixed with " << LPUB3D_COLOUR_FADE_PREFIX << ".";
        _ldrawStaticColourParts  << "# If using a single fade step colour, colour codes are replaced with main material colour";
        _ldrawStaticColourParts  << "# code 16 using the fade colour set in Preferences";
        _ldrawStaticColourParts  << "# If part highlight is enabled, edge colour values are replaced with the colour value set";
        _ldrawStaticColourParts  << "# in Preferences";
        _ldrawStaticColourParts  << "# If part highlight is enabled, colour codes are replaced with a custom code using the standard";
        _ldrawStaticColourParts  << "# colour code prefixed with " << LPUB3D_COLOUR_HIGHLIGHT_PREFIX << ".";
        _ldrawStaticColourParts  << "# When fade step is enabled, custom generated files are appended with '-fade',";
        _ldrawStaticColourParts  << "# for example, ...\\custom\\parts\\99499-fade.dat";
        _ldrawStaticColourParts  << "# When highlight step is enabled, custom generated files are appended with '-highlight',";
        _ldrawStaticColourParts  << "# for example, ...\\custom\\parts\\99499-highlight.dat";
        _ldrawStaticColourParts  << "# Part identifiers with spaces will not be properly recoginzed.";
        _ldrawStaticColourParts  << "# This file is automatically generated from Configuration=>Generate Static Colour Parts List";
        _ldrawStaticColourParts  << "# However, it can also be modified manually from Configuration=>Edit Static Colour Parts List";
        _ldrawStaticColourParts  << "# There are three defined columns in this file:";
        _ldrawStaticColourParts  << "# 1. File Name: The part file name as defined in the LDraw Library.";
        _ldrawStaticColourParts  << "# 2. Lib Type: Indicator 'U'=Unofficial Library and 'O'= Official Library.";
        _ldrawStaticColourParts  << "# 3. Description: The part file description taken from the first line of the part file.";
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# LDraw static colour parts were generated from the following list of libraries and directories:";
        _ldrawStaticColourParts  << "";
        break;
      case FADESTEP_FILE_HEADER:
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# File Name:::Lib Type:::Description";
        _ldrawStaticColourParts  << "";
          break;
    case FADESTEP_COLOUR_PARTS_HEADER:
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# Static colour parts";
        _ldrawStaticColourParts  << heading;
        _ldrawStaticColourParts  << "";
        break;
    case FADESTEP_COLOUR_CHILDREN_PARTS_HEADER:
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# Parts with no static colour element but has subparts or primitives with static colour elements";
        _ldrawStaticColourParts  << heading;
        _ldrawStaticColourParts  << "";
        break;
    case FADESTEP_FILE_STATUS:
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# ++++++++++++++++++++++++";
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << heading;
        _ldrawStaticColourParts  << "";
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












