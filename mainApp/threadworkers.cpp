/****************************************************************************
**
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public License (GPL) version 3.0
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

PartWorker::PartWorker(QString ldrawArchiveFile, QObject *parent) : QObject(parent)
{
    _ldrawArchiveFile       = ldrawArchiveFile;
    _endThreadNowRequested  = false;
}

PartWorker::PartWorker(QObject *parent) : QObject(parent)
{
  setDoFadeStep(Preferences::enableFadeSteps);
  setDoHighlightStep(Preferences::enableHighlightStep && !gui->suppressColourMeta());

  _resetSearchDirSettings = false;
  _endThreadNowRequested  = false;

  _excludedSearchDirs << ".";
  _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawLibPath).arg("parts"));
  _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawLibPath).arg("p"));
  if (Preferences::usingDefaultLibrary) {
      _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawLibPath).arg("unofficial/parts"));
      _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawLibPath).arg("unofficial/p"));
  }
  _customPartDir = QDir::toNativeSeparators(QString("%1/%2custom/parts").arg(Preferences::lpubDataPath).arg(Preferences::validLDrawLibrary));
  _customPrimDir = QDir::toNativeSeparators(QString("%1/%2custom/p").arg(Preferences::lpubDataPath).arg(Preferences::validLDrawLibrary));

  _lsynthPartsDir = QDir::toNativeSeparators(QString("%1/LSynthParts").arg(Preferences::lpubDataPath));

  _ldSearchDirsKey = Preferences::ldrawSearchDirsKey;
  _ldrawCustomArchive = Preferences::validLDrawCustomArchive;
}

/*
 * LDraw search directories preferences.
 */
void PartWorker::ldsearchDirPreferences(){

  bool fadeStep = (gui->page.meta.LPub.fadeStep.fadeStep.value() || Preferences::enableFadeSteps);
  bool highlightStep = (gui->page.meta.LPub.highlightStep.highlightStep.value() || Preferences::enableHighlightStep);
  setDoFadeStep(fadeStep);
  setDoHighlightStep(highlightStep && !gui->suppressColourMeta());

  if (!_resetSearchDirSettings && !Preferences::lpub3dLoaded) {
      emit Application::instance()->splashMsgSig("50% - Search directory preferences loading...");
    } else {
      emit gui->messageSig(LOG_INFO,"Reset search directories...");
    }

  QSettings Settings;
  QString const LdrawiniFilePathKey("LDrawIniFile");

  // qDebug() << QString(tr("01 ldrawIniFoundReg(Original) = %1").arg((ldrawIniFoundReg ? "True" : "False")));

  // Check for and load LDrawINI
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey))) {
      QString ldrawiniFilePath = Settings.value(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey)).toString();
      QFileInfo ldrawiniInfo(ldrawiniFilePath);
      if (ldrawiniInfo.exists()) {
          Preferences::ldrawiniFile = ldrawiniInfo.absoluteFilePath();
          Preferences::ldrawiniFound = true;
        } else {
          Settings.remove(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey));
          Settings.remove(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey));
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
          //qDebug() << QString(tr("  -Failed to get LDraw.ini, valid file (from Preferences) does not exist."));
        }
    } else {
      emit gui->messageSig(LOG_INFO, QString("Unable to initialize LDrawINI. Using default search directories."));
    }

  if (!doFadeStep() && !doHighlightStep()) {
      _excludedSearchDirs << _customPartDir;
      _excludedSearchDirs << _customPrimDir;
  } else {
      Paths::mkCustomDirs();
  }

  emit gui->messageSig(LOG_INFO,(doFadeStep() ? QString("Fade Previous Steps is ON.") : QString("Fade Previous Steps is OFF.")));
  emit gui->messageSig(LOG_INFO,(doHighlightStep() ? QString("Highlight Current Step is ON.") : QString("Highlight Current Step is OFF.")));

  // LDrawINI not found and not reset so load registry key
  if (!Preferences::ldrawiniFound && !_resetSearchDirSettings &&
      Settings.contains(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey))) {
      emit gui->messageSig(LOG_INFO, QString("LDrawINI not found, loading LDSearch directories from registry key..."));
      QStringList searchDirs = Settings.value(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey)).toStringList();
      bool customDirsIncluded = false;
      foreach (QString searchDir, searchDirs) {
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
              emit gui->messageSig(LOG_INFO, QString("Add search directory: %1").arg(searchDir));
          } else {
              emit gui->messageSig(LOG_INFO, QString("Search directory is empty and will be ignored: %1").arg(searchDir));
          }
      }
      // If fade step enabled but custom directories not defined in ldSearchDirs, add custom directories
      if ((doFadeStep() || doHighlightStep()) && !customDirsIncluded) {
          // We must force the custom directories for LDView as they are needed by ldview ini files
          if (Preferences::preferredRenderer == RENDERER_LDVIEW) {
              Preferences::ldSearchDirs << _customPartDir;
              emit gui->messageSig(LOG_INFO, QString("Add custom part directory: %1").arg(_customPartDir));
              Preferences::ldSearchDirs << _customPrimDir;
              emit gui->messageSig(LOG_INFO, QString("Add custom primitive directory %1").arg(_customPrimDir));
              customDirsIncluded = true;
          } else {
              if (QDir(_customPartDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                Preferences::ldSearchDirs << _customPartDir;
                customDirsIncluded = true;
                emit gui->messageSig(LOG_INFO, QString("Add custom part directory: %1").arg(_customPartDir));
              } else {
                emit gui->messageSig(LOG_INFO, QString("Custom part directory is empty and will be ignored: %1").arg(_customPartDir));
              }
              if (QDir(_customPrimDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                Preferences::ldSearchDirs << _customPrimDir;
                customDirsIncluded = true;
                emit gui->messageSig(LOG_INFO, QString("Add custom primitive directory: %1").arg(_customPrimDir));
              } else {
                emit gui->messageSig(LOG_INFO, QString("Custom primitive directory is empty and will be ignored: %1").arg(_customPrimDir));
              }
          }
          // update the registry if custom directory included
          if (customDirsIncluded) {
              Settings.setValue(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey), Preferences::ldSearchDirs);
          }
       }
    } else if (loadLDrawSearchDirs()){                                        //ldraw.ini found or reset so load local paths
      Settings.setValue(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey), Preferences::ldSearchDirs);
      emit gui->messageSig(LOG_INFO, QString("Loading LDraw parts search directories..."));
    } else {
      Settings.remove(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey));
      emit gui->messageSig(LOG_ERROR, QString("Unable to load search directories."));
    }

    // Add LSynth path to search directory list
    bool addSearchDir    = Preferences::addLSynthSearchDir;
    bool dirInSearchList = false;
    QStringList saveSearchDirs;
    foreach (QString ldSearchDir, Preferences::ldSearchDirs){
        if (addSearchDir) {
            if (QDir::toNativeSeparators(ldSearchDir.toLower()) == _lsynthPartsDir.toLower()) {
                dirInSearchList = true;
                break;
            }
        } else if (QDir::toNativeSeparators(ldSearchDir.toLower()) != _lsynthPartsDir.toLower()) {
                saveSearchDirs << ldSearchDir;
        }
    }
    if (addSearchDir && !dirInSearchList)
        Preferences::ldSearchDirs << getLSynthDir();
    if (!addSearchDir && Preferences::ldSearchDirs.size() > saveSearchDirs.size())
        Preferences::ldSearchDirs = saveSearchDirs;

    updateLDSearchDirs();
}

/*
 * Load LDraw search directories into Preferences.
 */
bool PartWorker::loadLDrawSearchDirs(){

  if (!_resetSearchDirSettings) {
      emit Application::instance()->splashMsgSig("60% - Search directories loading...");
    } else {
      emit gui->messageSig(LOG_INFO,"Reset - search directories loading...");
    }

  setDoFadeStep(gui->page.meta.LPub.fadeStep.fadeStep.value());
  StringList  ldrawSearchDirs;
  if (ldPartsDirs.loadLDrawSearchDirs("")){
      ldrawSearchDirs = ldPartsDirs.getLDrawSearchDirs();
      Preferences::ldSearchDirs.clear();
      bool foundUnofficialRootDir = false;
      bool customDirsIncluded = false;
      QString unofficialRootDir;
      if (Preferences::usingDefaultLibrary)
          unofficialRootDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawLibPath).arg("unofficial"));
      else
         unofficialRootDir = QDir::toNativeSeparators(QString("%1").arg(Preferences::ldrawLibPath));
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
                  emit gui->messageSig(LOG_INFO, QString("Add search directory: %1").arg(ldrawSearchDir));
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
              emit gui->messageSig(LOG_INFO, QString("Add custom part directory: %1").arg(_customPartDir));
            } else {
              emit gui->messageSig(LOG_INFO, QString("Custom part directory is empty and will be ignored: %1").arg(_customPartDir));
            }
          if (QDir(_customPrimDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
              Preferences::ldSearchDirs << _customPrimDir;
              emit gui->messageSig(LOG_INFO, QString("Add custom primitive directory: %1").arg(_customPrimDir));
            } else {
              emit gui->messageSig(LOG_INFO, QString("Custom primitive directory is empty and will be ignored: %1").arg(_customPrimDir));
            }
        }
      // Add subdirectories from Unofficial root directory
      if (foundUnofficialRootDir) {
          QDir unofficialDir(unofficialRootDir);
          // Get sub directories
          QStringList unofficialSubDirs = unofficialDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::SortByMask);
#ifdef QT_DEBUG_MODE
          //logDebug() << "unofficialSubDirs:" << unofficialSubDirs;
#endif
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
                          emit gui->messageSig(LOG_INFO, QString("Add search directory: %1").arg(unofficialSubDir));
                        } else if (QDir(unofficialSubDir).entryInfoList(QDir::Dirs|QDir::NoSymLinks).count() > 0) {
                          QDir subSubDir(unofficialSubDir);
                          QStringList subSubDirs = subSubDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::SortByMask);
                          foreach (QString subSubDirName, subSubDirs){
                              QString unofficialSubSubDir = QDir::toNativeSeparators(QString("%1/%2").arg(unofficialSubDir).arg(subSubDirName));
                              if (QDir(unofficialSubSubDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                                  Preferences::ldSearchDirs << unofficialSubSubDir;
                                  emit gui->messageSig(LOG_INFO, QString("Add search directory: %1").arg(unofficialSubSubDir));
                                } else {
                                  emit gui->messageSig(LOG_INFO, QString("Search directory is empty and will be ignored: %1").arg( unofficialSubSubDir));
                                }
                            }
                        } else {
                          emit gui->messageSig(LOG_INFO, QString("Search directory is empty and will be ignored: %1").arg( unofficialSubDir));
                        }
                    }
                }
            }
        }

      if (_resetSearchDirSettings) {
          processLDSearchDirParts();
        }

    } else {
      emit gui->messageSig(LOG_ERROR, QString("ldPartsDirs.loadLDrawSearchDirs("") failed."));
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
#ifdef QT_DEBUG_MODE
        //logDebug() << "SEARCH DIRECTORIES TO PROCESS" << Preferences::ldSearchDirs ;
#endif
        emit gui->messageSig(LOG_INFO, QString("LDGlite Search Directories..."));

        // Define excluded directories
        QStringList ldgliteExcludedDirs = _excludedSearchDirs;
        if (Preferences::usingDefaultLibrary) {
            ldgliteExcludedDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawLibPath).arg("models"))
                                << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawLibPath).arg("unofficial"))
                                << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawLibPath).arg("unofficial/lsynth"));
        }
        // Clear directories
        Preferences::ldgliteSearchDirs.clear();
        int count = 0;                    // set delimiter from 2nd entry
        // Recurse ldraw search directories
        foreach (QString ldgliteSearchDir, Preferences::ldSearchDirs){
            // Exclude invalid directories
            bool excludeSearchDir = false;
            // Skip over customDirs if fade or highlight step
            if (!(doFadeStep() || doHighlightStep()) &&
                !(ldgliteSearchDir.toLower() == QString(_customPartDir).toLower() ||
                  ldgliteSearchDir.toLower() == QString(_customPrimDir).toLower())) {
                foreach (QString excludedDir, ldgliteExcludedDirs){
                    if ((excludeSearchDir =
                         ldgliteSearchDir.toLower() == excludedDir.toLower())) {
                        break;
                    }
                }
            }
            if (!excludeSearchDir){
                // check if empty
                if (QDir(ldgliteSearchDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                    count++;
                    count > 1 ? Preferences::ldgliteSearchDirs.append(QString("|%1").arg(ldgliteSearchDir)):
                                Preferences::ldgliteSearchDirs.append(ldgliteSearchDir);
                    emit gui->messageSig(LOG_INFO, QString("Add ldglite search directory: %1").arg(ldgliteSearchDir));
                }else {
                    emit gui->messageSig(LOG_INFO, QString("Ldglite search directory is empty and will be ignored: %1").arg(ldgliteSearchDir));
                }
            }
        }
    }
}

/*
 * Add customPartDir and customPrim dir to ldSearchDirs
 * when fade or highlight step is enabled
 */

void PartWorker::addCustomDirs() {
    if (!Preferences::ldSearchDirs.contains(_customPartDir)) {
        Preferences::ldSearchDirs << _customPartDir;
        emit gui->messageSig(LOG_INFO, QString("Add custom part directory: %1").arg(_customPartDir));
    }
    if (!Preferences::ldSearchDirs.contains(_customPrimDir)) {
        Preferences::ldSearchDirs << _customPrimDir;
        emit gui->messageSig(LOG_INFO, QString("Add custom primitive directory: %1").arg(_customPrimDir));
    }
    updateLDSearchDirs(true);
}

/*
 * Update ldSearch Directory list and archvie new parts if any
 */

void PartWorker::updateLDSearchDirs(bool archive /*false*/) {
    // Update the registry
    QSettings Settings;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey), Preferences::ldSearchDirs);

    // Update LDView extra search directories
    if (!Preferences::setLDViewExtraSearchDirs(Preferences::ldviewIni))
       emit gui->messageSig(LOG_ERROR, QString("Could not update %1").arg(Preferences::ldviewIni));
    if (!Preferences::setLDViewExtraSearchDirs(Preferences::ldviewPOVIni))
       emit gui->messageSig(LOG_ERROR, QString("Could not update %1").arg(Preferences::ldviewPOVIni));
    if (!Preferences::setLDViewExtraSearchDirs(Preferences::nativeExportIni))
       emit gui->messageSig(LOG_ERROR, QString("Could not update %1").arg(Preferences::nativeExportIni));

    // Update LDGLite extra search directories
    if (Preferences::preferredRenderer == RENDERER_LDGLITE)
       populateLdgLiteSearchDirs();

    // Archive search directory parts
    if (archive)
       processLDSearchDirParts();
}

/*
 * Get LSynth directory path
 */

QString PartWorker::getLSynthDir(){

    if (Preferences::archiveLSynthParts) {
        QFileInfo outFile;
        QDir dir(_lsynthPartsDir);
        if (!dir.exists())
            dir.mkdir(_lsynthPartsDir);
        enum numLSynthFiles { lsynthFiles = 34 };
        if (dir.entryInfoList(QDir::Files|QDir::NoSymLinks).count() == lsynthFiles)
            return dir.absolutePath();
        const QString lsynthFilePaths[lsynthFiles] =
        {
            ":/lsynth/572a.dat",":/lsynth/757.dat"  ,":/lsynth/LS00.dat" ,":/lsynth/LS01.dat",
            ":/lsynth/LS02.dat",":/lsynth/LS03.dat" ,":/lsynth/LS04.dat" ,":/lsynth/LS05.dat",
            ":/lsynth/LS06.dat",":/lsynth/LS07.dat" ,":/lsynth/LS08.dat" ,":/lsynth/LS09.dat",
            ":/lsynth/LS10.dat",":/lsynth/LS100.dat",":/lsynth/LS101.dat",":/lsynth/LS102.dat",
            ":/lsynth/LS11.dat",":/lsynth/LS12.dat" ,":/lsynth/LS20.dat" ,":/lsynth/LS21.dat",
            ":/lsynth/LS22.dat",":/lsynth/LS23.dat" ,":/lsynth/LS30.dat" ,":/lsynth/LS40.dat",
            ":/lsynth/LS41.dat",":/lsynth/LS50.dat" ,":/lsynth/LS51.dat" ,":/lsynth/LS60.dat",
            ":/lsynth/LS61.dat",":/lsynth/LS70.dat" ,":/lsynth/LS71.dat" ,":/lsynth/LS80.dat",
            ":/lsynth/LS90.dat",":/lsynth/LS91.dat"
        };
        for (int i = 0; i < lsynthFiles; i++) {
            qDebug() << "\nLSynth filePath: [" << i << "] " << lsynthFilePaths[i];
            QFileInfo inFile(lsynthFilePaths[i]);
            outFile.setFile(QString("%1/%2").arg(dir.absolutePath(), inFile.fileName()));
            if (!outFile.exists())
                QFile::copy(inFile.absoluteFilePath(), outFile.absoluteFilePath());
        }
        return dir.absolutePath();
    }
    return QString();
}

/*
 * Process LDraw search directories part files.
 */
void PartWorker::processLDSearchDirParts(){

  QStringList dirs;

  if (!Preferences::addLSynthSearchDir) {
     QString dir;
     if (!(dir = getLSynthDir()).isEmpty())
        dirs.append(dir);
  }

  dirs += Preferences::ldSearchDirs;

  if (dirs.size() > 0)
    processPartsArchive(dirs, "search directory");

  // qDebug() << "\nFinished Processing Search Directory Parts.";
}

/*
 * scan LDraw library files for static colored parts and create fade copy
 */
void PartWorker::processFadeColourParts(bool overwriteCustomParts)
{
  //qDebug() << QString("Received overwrite fade parts = %1").arg(overwriteCustomParts ? "True" : "False");
  if (doFadeStep())
    processCustomColourParts(FADE_PART, overwriteCustomParts);
}

/*
 * scan LDraw library files for static colored parts and create highlight copy
 */
void PartWorker::processHighlightColourParts(bool overwriteCustomParts)
{
  //qDebug() << QString("Received overwrite highlight parts = %1").arg(overwriteCustomParts ? "True" : "False");
  if (doHighlightStep())
    processCustomColourParts(HIGHLIGHT_PART, overwriteCustomParts);
}

void PartWorker::processCustomColourParts(PartType partType, bool overwriteCustomParts)
{
  QString nameMod;
  if (partType == FADE_PART)
    nameMod = LPUB3D_COLOUR_FADE_SUFFIX;
  else if (partType == HIGHLIGHT_PART)
    nameMod = LPUB3D_COLOUR_HIGHLIGHT_SUFFIX;

  _timer.start();
  _customParts = 0;

  QStringList customPartsDirs;
  QStringList contents;
  QStringList colourPartList;
  int existingCustomParts = 0;

  //DISABLE PROGRESS BAR - CAUSING MESSAGEBAR OVERLOAD
  //emit progressBarInitSig();
  //emit progressMessageSig("Parse Model File");
  Paths::mkCustomDirs();

  ldrawFile = gui->getLDrawFile();
  // process top-level submodels
  //emit progressRangeSig(1, ldrawFile._subFileOrder.size());

  // in this block, we find colour parts and their children, append the nameMod
  // and confirm that the part exist if the part is not found, we submit it to be created
  for (int i = 0; i < ldrawFile._subFileOrder.size() && endThreadNotRequested(); i++) {
      QString subFileString = ldrawFile._subFileOrder[i].toLower();
      contents = ldrawFile.contents(subFileString);
      //emit progressSetValueSig(i);
      emit gui->messageSig(LOG_INFO,QString("00 PROCESSING SUBFILE CUSTOM COLOR PARTS: %1").arg(subFileString));
      for (int i = 0; i < contents.size() && endThreadNotRequested(); i++) {
          QString line = contents[i];
          QStringList tokens;
          split(line,tokens);
          if (tokens.size() == 15 && tokens[0] == "1") {
              // check if colored part and create custom version if yes
              QString fileString = gui->ldrawColourParts.getLDrawColourPartInfo(tokens[tokens.size()-1]);
              // validate part is static color part;
              if (!fileString.isEmpty()){
                  QString fileDir  = QString();
                  QString libType  = fileString.section(":::",0,0);
                  QString fileName = fileString.section(":::",1,1);
                  if ((fileString.indexOf("\\") != -1)) {
                     fileDir  = fileString.section(":::",1,1).split("\\").first();
                     fileName = fileString.section(":::",1,1).split("\\").last();
                  }
#ifdef QT_DEBUG_MODE
                  //logDebug() << "FileDir:" << fileDir << "FileName:" << fileName;
#endif
                  QDir customFileDirPath;
                  if (libType == "g"){ // generated part
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(QDir::currentPath()).arg(Paths::tmpDir));
                  } else if (fileDir.isEmpty()){
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir));
                  } else if (fileDir == "s"){
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customSubDir));
                  } else if (fileDir == "p"){
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrimDir));
                  } else if (fileDir == "8"){
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim8Dir));
                  } else if (fileDir == "48"){
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim48Dir));
                  } else {
                      customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir));
                  }
                  bool entryExists = false;
                  QString customFileName = fileName.replace(".dat", "-" + nameMod + ".dat");
                  QFileInfo customFileInfo(customFileDirPath,customFileName);
                  entryExists = customFileInfo.exists();
                  if (!entryExists) {
                      // we stop here for generated part if not found
                      if (libType == "g") {
                          QString fileStatus = QString("Generated part file %1 not found in %2.<br>"
                                                       "Be sure your model file correctly reflects this part.")
                                  .arg(fileString.replace(":::", " "))
                                  .arg(customFileDirPath.absolutePath());
                          emit gui->messageSig(LOG_ERROR, fileStatus);
                          continue;
                      }
                      // add part directory to list if not already added
                      if (! customPartsDirs.contains(customFileInfo.absolutePath()))
                        customPartsDirs << customFileInfo.absolutePath();
                      existingCustomParts++;
                      // check if part entry already in list
                      foreach(QString colourPart, colourPartList){
                          if (colourPart == fileString){
                              entryExists = true;
                              break;
                          }
                      }
                  }
                  // add part entry to list
                  if (!entryExists) {
                      colourPartList << fileString;
                      logNotice() << "01 SUBMIT COLOUR PART INFO:" << fileString.replace(":::", " ") << " Line: " << i ;
                  } else {
                      logNotice() << "01 COLOUR PART EXIST - IGNORING:" << fileString.replace(":::", " ");
                  }
              }
          }
      }
  }

  //emit progressSetValueSig(ldrawFile._subFileOrder.size());

  // We have new parts to be created.
  if (colourPartList.size() > 0) {
      // check if part has children color part(s)
      if (!processColourParts(colourPartList, partType)) {
          QString error = QString("Process %1 color parts failed!.").arg(nameMod);
          emit gui->messageSig(LOG_ERROR,error);
          //emit progressStatusRemoveSig();
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
#ifdef QT_DEBUG_MODE
                  logDebug() << "Add " + nameMod + " part directory to ldSearchDirs:" << customPartDir;
#endif
                  QSettings Settings;
                  Settings.setValue(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey), Preferences::ldSearchDirs);

                  if (!Preferences::setLDViewExtraSearchDirs(Preferences::ldviewIni))
                     emit gui->messageSig(LOG_ERROR, QString("Could not update %1").arg(Preferences::ldviewIni));
                  if (!Preferences::setLDViewExtraSearchDirs(Preferences::ldviewPOVIni))
                     emit gui->messageSig(LOG_ERROR, QString("Could not update %1").arg(Preferences::ldviewPOVIni));
                  if (!Preferences::setLDViewExtraSearchDirs(Preferences::nativeExportIni))
                     emit gui->messageSig(LOG_ERROR, QString("Could not update %1").arg(Preferences::nativeExportIni));
                  if (!updateLDGLiteSearchDirs)
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
      QString comment = QString("color %1").arg(nameMod);
      if (!processPartsArchive(customPartsDirs, comment, overwriteCustomParts)){
          QString error = QString("Process %1 parts archive failed!.").arg(nameMod);
          emit gui->messageSig(LOG_ERROR,error);
          //emit progressStatusRemoveSig();
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

  //emit progressStatusRemoveSig();
  emit customColourFinishedSig();
  emit gui->messageSig(LOG_INFO,fileStatus);
}

bool PartWorker::processColourParts(const QStringList &colourPartList, const PartType partType) {

    QString nameMod;
    if (partType == FADE_PART)
      nameMod = LPUB3D_COLOUR_FADE_SUFFIX;
    else if (partType == HIGHLIGHT_PART)
      nameMod = LPUB3D_COLOUR_HIGHLIGHT_SUFFIX;

    // Archive library files
    QString fileStatus;

    QString officialLib;
    QString unofficialLib;
    QFileInfo archiveFileInfo(QDir::toNativeSeparators(Preferences::lpub3dLibFile));
    if (archiveFileInfo.exists()) {
        officialLib = archiveFileInfo.absoluteFilePath();
        unofficialLib = QString("%1/%2").arg(archiveFileInfo.absolutePath(),_ldrawCustomArchive);
    } else {
        fileStatus = QString("Archive file does not exist: %1. The process will terminate.").arg(archiveFileInfo.absoluteFilePath());
        emit gui->messageSig(LOG_ERROR, fileStatus);
        return false;
    }

    //DISABLE PROGRESS BAR - CAUSING MESSAGEBAR OVERLOAD
    //emit progressResetSig();
    //emit progressMessageSig("Process Color Parts...");
    //emit progressRangeSig(1, colourPartList.size());

    int partCount = 0;
    int partsProcessed = 0;
    QStringList childrenColourParts;

    foreach (QString partEntry, colourPartList) {

        bool partFound = false;

        QString cpPartEntry = partEntry;
        bool unOffLib = cpPartEntry.section(":::",0,0) == "u";
        //emit gui->messageSig(LOG_INFO,QString("Library Type: %1").arg((unOffLib ? "Unofficial Library" : "Official Library"));

        QString libPartDir  = QString();
        QString libPartName = cpPartEntry.section(":::",1,1);
        if ((cpPartEntry.indexOf("\\") != -1)) {
           libPartDir  = cpPartEntry.section(":::",1,1).split("\\").first();
           libPartName = cpPartEntry.section(":::",1,1).split("\\").last();
        }
        //emit gui->messageSig(LOG_INFO,QString("Lib Part Name: %1").arg(libPartName));

        //emit progressSetValueSig(partCount++);

        QuaZip zip(unOffLib ? unofficialLib : officialLib);
        if (!zip.open(QuaZip::mdUnzip)) {
            emit gui->messageSig(LOG_ERROR, QString("Failed to open archive: %1 @ %2").arg(zip.getZipError()).arg(unOffLib ? unofficialLib : officialLib));
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
                    emit gui->messageSig(LOG_ERROR, QString("Failed to OPEN Part file :%1").arg(zip.getCurrentFileName()));
                    return false;
                }
                // extract content
                QTextStream in(&qba);
                while (! in.atEnd() && endThreadNotRequested()) {
                    QString line = in.readLine(0);
                    _partFileContents << line.toLower();

                    // check if line is a color part
                    QStringList tokens;
                    split(line,tokens);
                    if (tokens.size() == 15 && tokens[0] == "1") {
                        // validate part is static color part;
                        QString childFileString = gui->ldrawColourParts.getLDrawColourPartInfo(tokens[tokens.size()-1]);
                        // validate part is static color part;
                        if (!childFileString.isEmpty()){
                            QString fileDir  = QString();
                            QString fileName = childFileString.section(":::",1,1);
                            if ((childFileString.indexOf("\\") != -1)) {
                               fileDir  = childFileString.section(":::",1,1).split("\\").first();
                               fileName = childFileString.section(":::",1,1).split("\\").last();
                            }
#ifdef QT_DEBUG_MODE
                            //logDebug() << "FileDir:" << fileDir << "FileName:" << fileName;
#endif
                            QDir customFileDirPath;
                            if (fileDir.isEmpty()){
                                customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir));
                            } else  if (fileDir == "s"){
                                customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customSubDir));
                            } else  if (fileDir == "p"){
                                customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrimDir));
                            } else  if (fileDir == "8"){
                                customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim8Dir));
                            } else if (fileDir == "48"){
                                customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim48Dir));
                            } else {
                                customFileDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir));
                            }
                            bool entryExists = false;
                            QString customFileName = fileName.replace(".dat", "-" + nameMod + ".dat");
                            QFileInfo customFileInfo(customFileDirPath,customFileName);
                            entryExists = customFileInfo.exists();
                            // check if child part entry already in list
                            if (!entryExists) {
                                foreach(QString childColourPart, childrenColourParts){
                                    if (childColourPart == childFileString){
                                        entryExists = true;
                                        break;
                                    }
                                }
                            }
                            // add chile part entry to list
                            if (!entryExists) {
                                childrenColourParts << childFileString;
                                logNotice() << "03 SUBMIT CHILD COLOUR PART INFO:" << childFileString.replace(":::", " ");
                            } else {
                                logNotice() << "03 CHILD COLOUR PART EXIST - IGNORING:" << childFileString.replace(":::", " ");
                            }
                        }
                    }
                }
                // determine part type
                int ldrawPartType = -1;
                if (libPartDir == libPartName){
                    ldrawPartType = LD_PARTS;
                } else  if (libPartDir == "s"){
                    ldrawPartType = LD_SUB_PARTS;
                } else  if (libPartDir == "p"){
                    ldrawPartType = LD_PRIMITIVES;
                } else  if (libPartDir == "8"){
                    ldrawPartType = LD_PRIMITIVES_8;
                } else if (libPartDir == "48"){
                    ldrawPartType = LD_PRIMITIVES_48;
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
            QString lib = Preferences::usingDefaultLibrary ? "Unofficial" : "Custom Parts";
            fileStatus = QString("Part file %1 not found in %2. Be sure the %3 fadeStepColorParts.lst file is up to date.")
                    .arg(partEntry.replace(":::", " "))
                    .arg(unOffLib ? lib+" Library" : "Official Library")
                    .arg(Preferences::validLDrawLibrary);
            emit gui->messageSig(LOG_ERROR, fileStatus);
        }

        zip.close();

        if (zip.getZipError() != UNZ_OK) {
            emit gui->messageSig(LOG_ERROR, QString("zip close error: %1").arg(zip.getZipError()));
            return false;
        }
    }
    //emit progressSetValueSig(colourPartList.size());

    // recurse part file content to check if any children are color parts
    if (childrenColourParts.size() > 0)
        processColourParts(childrenColourParts, partType);

    QString message = QString("%1 Color %2 content processed.")
        .arg(partsProcessed)
        .arg(partsProcessed > 1 ? "parts" : "part");
    emit gui->messageSig(LOG_INFO,message);

    return true;
}


bool PartWorker::createCustomPartFiles(const PartType partType){

     QString nameMod, colourPrefix;
     if (partType == FADE_PART){
       nameMod = LPUB3D_COLOUR_FADE_SUFFIX;
       colourPrefix = LPUB3D_COLOUR_FADE_PREFIX;
     } else if (partType == HIGHLIGHT_PART) {
       nameMod = LPUB3D_COLOUR_HIGHLIGHT_SUFFIX;
       colourPrefix = LPUB3D_COLOUR_HIGHLIGHT_PREFIX;
     }

    int maxValue            = _partList.size();
    //DISABLE PROGRESS BAR - CAUSING MESSAGEBAR OVERLOAD
    //emit progressResetSig();
    //emit progressMessageSig("Creating Custom Color Parts");
    //emit progressRangeSig(1, maxValue);

    QStringList customPartContent, customPartColourList;
    QString customPartFile;

    for(int part = 0; part < _partList.size() && endThreadNotRequested(); part++) {

        //emit progressSetValueSig(part);

        QMap<QString, ColourPart>::iterator cp = _colourParts.find(_partList[part]);

        if(cp != _colourParts.end()){

            // prepare absoluteFilePath for custom file
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

            // process custom part contents
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
                    // check if part at this line has a matching color part in the colourPart list - if yes, rename with '-fade' or '-highlight'
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
                    //QString oldColour(tokens[1]);          //logging only: show color lines
                    QString colourCode;
                    // Insert color code for fade part
                    if (partType == FADE_PART){
                        // generate custom color entry - if fadeStepsUseColour, set color to material color (16), without prefix
                        colourCode = Preferences::fadeStepsUseColour ? LDRAW_MAIN_MATERIAL_COLOUR : tokens[1];
                        // add color line to local list - if fadeStepsUseColour, no need to create entry
                        if (!Preferences::fadeStepsUseColour && !gui->colourEntryExist(customPartColourList,colourCode,partType))
                            customPartColourList << gui->createColourEntry(colourCode,partType);
                        // set custom color - if fadeStepsUseColour, do not add custom color prefix
                        tokens[1] = Preferences::fadeStepsUseColour ? colourCode : QString("%1%2").arg(colourPrefix).arg(colourCode);
                        //logTrace() << "D. CHANGE CHILD PART COLOUR: " << fileNameStr << " NewColour: " << tokens[1] << " OldColour: " << oldColour;
                    }
                    // Insert color code for silhouette part
                    if (partType == HIGHLIGHT_PART){
                        // generate custom color entry - always
                        colourCode = tokens[1];
                        // add color line to local list - always request to create entry
                        if (!gui->colourEntryExist(customPartColourList,colourCode,partType))
                            customPartColourList << gui->createColourEntry(colourCode,partType);
                        // set custom color - if fadeStepsUseColour, do not add custom color prefix
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

            // add the custom part color list to the header of the custom part contents
            customPartColourList.toSet().toList(); // remove dupes

            int insertionPoint = 0; // skip the first line (title)
            QStringList words;
            // scan part header...
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
                   customPartContent[i] = words.join(" ");
                }
                else
                if (!isHeader(customPartContent[i]) && !QString(customPartContent[i]).isEmpty())
                  break;
            }

            // insert color entries after header
            if (!customPartColourList.isEmpty()) {
                customPartColourList.toSet().toList();  // remove dupes
                customPartContent.insert(insertionPoint,"0 // LPub3D part custom colours");
                for (int i = 0; i < customPartColourList.size(); i++) {
                    insertionPoint++;
                    customPartContent.insert(insertionPoint,customPartColourList.at(i));
                }
                customPartContent.insert(++insertionPoint,"0");
            }

            //logTrace() << "04 SAVE CUSTGOM COLOUR PART: " << customPartFile;
            if(saveCustomFile(customPartFile, customPartContent))
                _customParts++;

            customPartContent.clear();
            customPartColourList.clear();
        }
    }
    //emit progressSetValueSig(maxValue);
    return true;
}


/*
 * Write custom part files to custom directory.
 */
bool PartWorker::saveCustomFile(
        const QString     &fileName,
        const QStringList &customPartContent) {

    QFile file(fileName);
    if ( ! file.open(QFile::WriteOnly | QFile::Text)) {
        QString message = QString("Failed to open %1 for writing: %2").arg(fileName).arg(file.errorString());
        emit gui->messageSig(LOG_ERROR, message);
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
}


bool PartWorker::processPartsArchive(const QStringList &ldPartsDirs, const QString &comment, bool overwriteCustomParts){

  if (Preferences::skipPartsArchive) {
      Preferences::skipPartsArchive = false;
      return true;
  }

  // Append custom parts to custom parts library for 3D Viewer's consumption
  QTime t, tf;
  bool reloadLibrary = true;
  QString archiveFile = _ldrawArchiveFile;
  if (archiveFile.isEmpty()) {
      QFileInfo libFileInfo(Preferences::lpub3dLibFile);
      archiveFile = QDir::toNativeSeparators(QString("%1/%2").arg(libFileInfo.absolutePath(),_ldrawCustomArchive));
  } else {
      _ldrawArchiveFile = QString();
      reloadLibrary = false;
  }
  QString returnMessage = QString("Archiving %1 parts to : %2.").arg(comment,archiveFile);
  int returnMessageSeverity = 1; // 1=Error, 2=Notice
  emit gui->messageSig(LOG_INFO,QString("Archiving %1 parts to %2.").arg(comment,archiveFile));

  if (okToEmitToProgressBar()) {
      //emit progressResetSig();
      ;
    } else {
      emit Application::instance()->splashMsgSig(QString("60% - Archiving %1 parts, please wait...").arg(comment));
    }

  //if (okToEmitToProgressBar())
  //    emit progressRangeSig(0, 0);

  int partCount = 0;
  int totalPartCount = 0;

  tf.start();
  for (int i = 0; i < ldPartsDirs.size() && endThreadNotRequested(); i++){
      t.start();

      QDir partDir(ldPartsDirs[i]);

      emit progressSetValueSig(i);

      QString progressMessage = QString("Archiving custom parts...\nProcessing: " +
                                        QDir::toNativeSeparators(ldPartsDirs[i]));
      emit progressMessageSig(progressMessage);

      if (!archiveParts.Archive( archiveFile,
                                 partDir.absolutePath(),
                                 returnMessage,
                                 returnMessageSeverity,
                                 QString("Append %1 parts").arg(comment),
                                 overwriteCustomParts))
      {
         if (returnMessageSeverity == 1)
             emit gui->messageSig(LOG_ERROR,returnMessage);
         else
             emit gui->messageSig(LOG_NOTICE,returnMessage);
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
      emit gui->messageSig(LOG_INFO,tr("Archived %1 %2 from %3. %4")
                                       .arg(partCount).arg(summary).arg(partDir.absolutePath())
                                       .arg(gui->elapsedTime(t.elapsed())));
  }

  // Archive parts
  QString partsLabel = "parts";
  if (totalPartCount > 0) {
      // Reload unofficial library parts into memory - only if initial library load already done !
      if (Preferences::lpub3dLoaded && reloadLibrary) {
          if (!gApplication->mLibrary->ReloadUnoffLib()){
              returnMessage = tr("Failed to reload custom %1 parts library into memory.")
                                 .arg(Preferences::validLDrawLibrary);
              emit gui->messageSig(LOG_ERROR,returnMessage);
              return false;
          } else {
              partsLabel = totalPartCount == 1 ? "part" : "parts";
              returnMessage = tr("Reloaded custom %1 parts library into memory with %2 new %3.")
                                 .arg(Preferences::validLDrawLibrary).arg(totalPartCount).arg(partsLabel);
          }
      }
      partsLabel = totalPartCount == 1 ? "part" : "parts";
      returnMessage = tr("Finished. Archived %1 %2 %3. %4")
                         .arg(totalPartCount).arg(comment).arg(partsLabel).arg(gui->elapsedTime(tf.elapsed()));

      emit partsArchiveResultSig(totalPartCount);
      _partsArchived = true;
  } else {
      returnMessage = tr("Finished. Parts exist in custom %1 archive. No new %2 parts archived.")
                         .arg(Preferences::validLDrawLibrary).arg(comment);
      _partsArchived = false;
  }
  emit gui->messageSig(LOG_INFO,returnMessage);

  if (!okToEmitToProgressBar()) {
      emit Application::instance()->splashMsgSig(tr("70% - Finished archiving %1 parts.").arg(comment));
  }
  return true;
}

void PartWorker::processPartsArchive() {
    QString comment = QString("custom");
    if (!processPartsArchive(
                Preferences::ldSearchDirs,
                comment,
                true /*overwriteCustomParts*/)) {
        QString error = QString("Process %1 parts archive failed!.").arg(comment);
        emit gui->messageSig(LOG_ERROR,error);
    }
    emit partsArchiveFinishedSig();
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
    _ldrawCustomArchive    = Preferences::validLDrawCustomArchive;
}

/*
 * build color part listing
 *
 */

void ColourPartListWorker::generateCustomColourPartsList()
{
    // Archive library files
    QStringList archiveFiles;
    QFileInfo lpub3dLibFileInfo(QDir::toNativeSeparators(Preferences::lpub3dLibFile));
    if (lpub3dLibFileInfo.exists())
        archiveFiles << lpub3dLibFileInfo.absoluteFilePath();
    if (QFileInfo(QString("%1/%2").arg(lpub3dLibFileInfo.absolutePath(),_ldrawCustomArchive)).exists())
        archiveFiles << QString("%1/%2").arg(lpub3dLibFileInfo.absolutePath(),_ldrawCustomArchive);

    //library directories
    QStringList partTypeDirs;
    partTypeDirs << "p       Primitives";
    partTypeDirs << "p/8     Primitives - Low Resolution";
    partTypeDirs << "p/48    Primitives - High Resolution";
    partTypeDirs << "parts   Parts";
    partTypeDirs << "parts/s Sub-parts";

    _timer.start();
    _colWidthFileName = 0;

    int libCount = 1;

    fileSectionHeader(FADESTEP_INTRO_HEADER);
    _ldrawStaticColourParts  << "# Archive Libraries:";
    QString lib = Preferences::validLDrawLibrary;
    QString unoffLib = Preferences::usingDefaultLibrary ? "Unofficial "+lib  : lib+" Custom";
    QString offLib = Preferences::usingDefaultLibrary ? "Official "+lib : lib;
    foreach (QString archiveFile, archiveFiles) {
      QString library = archiveFile == lpub3dLibFileInfo.absoluteFilePath() ? offLib+" Library" : unoffLib+" Library";
      _ldrawStaticColourParts  << QString("# %1. %2: %3").arg(libCount++).arg(library).arg(archiveFile);
    }
    int dirCount = 1;
    _ldrawStaticColourParts  << "";
    _ldrawStaticColourParts  << "# Library Directories:";
    foreach (QString partTypeDir, partTypeDirs){
      _ldrawStaticColourParts  << QString("# %1. %2").arg(dirCount++).arg(partTypeDir);
    }
    _ldrawStaticColourParts  << "# ----------------------Do not delete above this line----------------------------------";
    _ldrawStaticColourParts  << "";

    fileSectionHeader(FADESTEP_FILE_HEADER);

    emit progressBarInitSig();
    foreach (QString archiveFile, archiveFiles) {
       if(!processArchiveParts(archiveFile)){
           QString error = QString("Process color parts list failed!.");
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

    QString fileStatus = QString("%1 Color Parts List successfully created with %2 entries. %3.")
                                   .arg(Preferences::validLDrawLibrary).arg(QString::number(_cpLines)).arg(time);
    fileSectionHeader(FADESTEP_FILE_STATUS, QString("# %1").arg(fileStatus));
    bool append = true;
    writeLDrawColourPartFile(append);

    emit progressStatusRemoveSig();
    emit colourPartListFinishedSig();
    emit messageSig(LOG_INFO_STATUS,fileStatus);
}

bool ColourPartListWorker::processArchiveParts(const QString &archiveFile) {

   QString lib = Preferences::validLDrawLibrary;
   bool isUnOffLib = true;
   QString library = Preferences::usingDefaultLibrary ? "Unofficial "+lib+" Library"  : lib+" Custom Library";

    if (archiveFile.contains(Preferences::validLDrawPartsArchive)) {
        library = Preferences::usingDefaultLibrary ? "Official "+lib+" Library" : lib+" Library";
        isUnOffLib = false;
    }

    QuaZip zip(archiveFile);
    if (!zip.open(QuaZip::mdUnzip)) {
        emit messageSig(LOG_ERROR, QString("Failed to open archive file %1. Error code [%2]")
                             .arg(archiveFile).arg(zip.getZipError()));
        return false;
    }

    // get part count
    emit progressRangeSig(0, 0);
    emit progressMessageSig("Generating " + library + " Color Parts...");

    int partCount = 1;
    for(bool f=zip.goToFirstFile(); f; f=zip.goToNextFile()) {
        if (zip.getCurrentFileName().toLower().split(".").last() != "dat") {
            continue;

        } else {
            partCount++;
        }
    }
    emit messageSig(LOG_INFO,QString("Processing Archive Parts for %1 - Parts Count: %2")
                    .arg(library).arg(partCount));

    emit progressResetSig();
    emit progressRangeSig(1, partCount);
    partCount = 0;

    for(bool f = zip.goToFirstFile(); f && endThreadNotRequested(); f = zip.goToNextFile()) {

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
    emit messageSig(LOG_INFO,QString("Finished Processing %1 Parent Color Parts").arg(library));

    zip.close();
    if (zip.getZipError() != UNZ_OK) {
        emit messageSig(LOG_ERROR, QString("zip.close() zipError(): %1").arg(zip.getZipError()));
        return false;
    }
    return true;
}

/*
 * parse color part file to determine children parts with static color.
 */
void ColourPartListWorker::processFileContents(const QString &libFileName, const bool isUnOffLib){

    QString materialColor  ="16";  // Internal Common Material Color (main)
    QString edgeColor      ="24";  // Internal Common Material Color (edge)
    QString fileEntry;
    QString fileName;
    QString color;
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
            color = tokens[1];
            if (color == edgeColor || color == materialColor){
                continue;

            } else {
                hasColour = true;
#ifdef QT_DEBUG_MODE
                //emit messageSig(LOG_TRACE,QString("CONTENTS COLOUR LINE: %1 FILE: %2").arg(line).arg(libFileName));
#endif
                if (fileName.isEmpty()){
                    fileName = libFileName.split("/").last();
                    emit messageSig(LOG_ERROR,QString("Part: %1 \nhas no 'Name:' attribute. Using library path name %2 instead.\n"
                                                      "You may want to update the part content and custom color parts list.")
                                                      .arg(fileName).arg(libFileName));
                }
                fileEntry = QString("%1:::%2:::%3").arg(fileName).arg(libType).arg(_partFileContents[0].remove(0,2));
                remove(libFileName);
                //logNotice() << "Remove from list as it is a known color part: " << libFileName;
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
        //logNotice() << "ADD COLOUR PART: " << fileName << " libFileName: " << libFileName << " Color: " << color;
    }
}

void ColourPartListWorker::processChildren(){

    emit progressResetSig();
    emit progressMessageSig("Processing Child Color Parts...");
    emit progressRangeSig(1, _partList.size());
    emit messageSig(LOG_INFO,QString("Processing Child Color Parts - Count: %1").arg(_partList.size()));

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

                    QString childFileString = tokens[tokens.size()-1];                     // child part name in parent part
                    // check childFileName in _ldrawStaticColourParts list
                    for (int j = 0; j < _ldrawStaticColourParts.size() && ! gotoMainLoop && endThreadNotRequested(); ++j){

                        if (_ldrawStaticColourParts.at(j).contains(childFileString) && _ldrawStaticColourParts.at(j).contains(QRegExp("\\b"+childFileString.replace("\\","\\\\")+"[^\n]*"))){
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
    emit messageSig(LOG_INFO,QString("Finished Processing Child Color Parts."));
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
                QString library         = cpLine.section(":::",1,1);
                QString partDescription = cpLine.section(":::",2,2);
                out << left << qSetFieldWidth(_colWidthFileName+1)    << partNumber
                            << qSetFieldWidth(9) << library
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
                         .arg(Preferences::validLDrawColorParts)
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
    QString lib = Preferences::validLDrawLibrary;
    QString unoffLib = Preferences::usingDefaultLibrary ? "Unofficial "+lib  : lib+" Custom";
    QString offLib = Preferences::usingDefaultLibrary ? "Official "+lib : lib;

    switch(option)
    {
    case FADESTEP_INTRO_HEADER:
        _ldrawStaticColourParts  << QString("# File: %1  Generated on: %2")
                                    .arg(Preferences::validLDrawColorParts)
                                    .arg(QDateTime::currentDateTime().toString(fmtDateTime));
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# This space-delimited list captures the LDraw static color parts (and their subfiles) to support";
        _ldrawStaticColourParts  << "# step fade and step highlight. Parts on this list are identified in the LDraw library and copied to";
        _ldrawStaticColourParts  << "# their respective custom directory. Copied files are modified as described in the following";
        _ldrawStaticColourParts  << "# lines. If fade step is enabled, color codes are replaced with a custom code using the standard";
        _ldrawStaticColourParts  << "# color code prefixed with [" LPUB3D_COLOUR_FADE_PREFIX "].";
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# If using a single fade step color, color codes are replaced with main material color";
        _ldrawStaticColourParts  << "# code 16 using the fade color set in Preferences. If part highlight is enabled, edge";
        _ldrawStaticColourParts  << "# color values are replaced with the color value set in Preferences. If part highlight is";
        _ldrawStaticColourParts  << "# enabled, color codes are replaced with a custom code using the standard color code ";
        _ldrawStaticColourParts  << "# prefixed with [" LPUB3D_COLOUR_HIGHLIGHT_PREFIX "].";
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# When fade step is enabled, custom generated files are appended with '" FADE_SFX "',";
        _ldrawStaticColourParts  << "# for example, ...\\custom\\parts\\99499"  FADE_SFX ".dat";
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# When highlight step is enabled, custom generated files are appended with '" HIGHLIGHT_SFX "',";
        _ldrawStaticColourParts  << "# for example, ...\\custom\\parts\\99499" HIGHLIGHT_SFX ".dat";
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# Part identifiers with spaces will not be properly recognized.";
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# This file is automatically generated from:";
        _ldrawStaticColourParts  << "#    Configuration=>Generate Static Color Parts List";
        _ldrawStaticColourParts  << "# However, it can also be edited manually from:";
        _ldrawStaticColourParts  << "#    Configuration=>Edit Parameter Files=>Edit LDraw Static Color Parts List";
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# LPub3D will attempt to load the regular expression below first, if the";
        _ldrawStaticColourParts  << "# load fails, LPub3D will load the hard-coded (default) regular expression.";
        _ldrawStaticColourParts  << "# If you wish to modify the file import, you can edit this regular expression.";
        _ldrawStaticColourParts  << "# It would be wise to backup the default entry before performing an update - copy";
        _ldrawStaticColourParts  << "# and paste to a new line with starting phrase other than 'The Regular Expression...'";
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# The Regular Expression used is: ^(\\b.*[^\\s]\\b)(?:\\s)\\s+(u|o)\\s+(.*)$";
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# There are three defined columns in this file:";
        _ldrawStaticColourParts  << "# 1. File Name: The part file name as defined in the LDraw Library.";
        _ldrawStaticColourParts  << "# 2. Lib Type: Indicator 'U' = " + unoffLib + " Library and 'O'= " + offLib + " Library.";
        _ldrawStaticColourParts  << "# 3. Description: The part file description taken from the first line of the part file.";
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# LDraw static color parts were generated from the following list of libraries and directories:";
        _ldrawStaticColourParts  << "";
        break;
      case FADESTEP_FILE_HEADER:
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# File Name:::Lib Type:::Description";
        _ldrawStaticColourParts  << "";
          break;
    case FADESTEP_COLOUR_PARTS_HEADER:
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# Static color parts";
        _ldrawStaticColourParts  << heading;
        _ldrawStaticColourParts  << "";
        break;
    case FADESTEP_COLOUR_CHILDREN_PARTS_HEADER:
        _ldrawStaticColourParts  << "";
        _ldrawStaticColourParts  << "# Parts with no static color element but has subparts or primitives with static color elements";
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

ExtractWorker::ExtractWorker(
        const QString &archive,
        const QString &destination)
    : mArchive(archive),
      mDestination(destination),
      mEndWorkNow(false)
{
}

void ExtractWorker::doWork() {
    QuaZip zip(mArchive);
    if (!extractDir(zip, mDestination)) {
        emit gui->messageSig(LOG_ERROR,QString(", Failed to extract archive %1").arg(mArchive));
    }
    emit finished();
}

bool ExtractWorker::extractDir(QuaZip &zip, const QString &dir)
{
    if(!zip.open(QuaZip::mdUnzip)) {
        emit gui->messageSig(LOG_ERROR,QString("Failed to open archive %1").arg(mArchive));
        return false;
    }

    QDir directory(dir);
    QStringList extracted;
    if (!zip.goToFirstFile()) {
        emit gui->messageSig(LOG_ERROR,QString("Failed to go to first archive file"));
        return false;
    }

    int i = 0;
    do {
        QString name = zip.getCurrentFileName();
        QString absFilePath = directory.absoluteFilePath(name);
        if (!extractFile(&zip, "", absFilePath)) {
            removeFile(extracted);
            emit gui->messageSig(LOG_ERROR,QString("Failed extract archive file %1").arg(absFilePath));
            return false;
        }
        extracted.append(absFilePath);
        emit progressSetValue(++i);
    } while (zip.goToNextFile() && ! mEndWorkNow);

    // Chiudo il file zip
    zip.close();
    if(zip.getZipError()!=0) {
        removeFile(extracted);
        emit gui->messageSig(LOG_ERROR,QString("Failed to close archive %1.").arg(mArchive));
        return false;
    }

    emit result(extracted.count());

    return true;
}

void ExtractWorker::requestEndWorkNow(){
    mEndWorkNow = true;
}

bool ExtractWorker::copyData(QIODevice &inFile, QIODevice &outFile)
{
    while (!inFile.atEnd()) {
        char buf[4096];
        qint64 readLen = inFile.read(buf, 4096);
        if (readLen <= 0)
            return false;
        if (outFile.write(buf, readLen) != readLen)
            return false;
    }
    return true;
}

bool ExtractWorker::extractFile(QuaZip* zip, QString fileName, QString fileDest) {
    // zip: oggetto dove aggiungere il file
    // filename: nome del file reale
    // fileincompress: nome del file all'interno del file compresso

    // Controllo l'apertura dello zip
    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdUnzip) return false;

    // Apro il file compresso
    if (!fileName.isEmpty())
        zip->setCurrentFile(fileName);
    QuaZipFile inFile(zip);
    if(!inFile.open(QIODevice::ReadOnly) || inFile.getZipError()!=UNZ_OK) return false;

    // Controllo esistenza cartella file risultato
    QDir curDir;
    if (fileDest.endsWith('/')) {
        if (!curDir.mkpath(fileDest)) {
            return false;
        }
    } else {
        if (!curDir.mkpath(QFileInfo(fileDest).absolutePath())) {
            return false;
        }
    }

    QuaZipFileInfo64 info;
    if (!zip->getCurrentFileInfo(&info))
        return false;

    QFile::Permissions srcPerm = info.getPermissions();
    if (fileDest.endsWith('/') && QFileInfo(fileDest).isDir()) {
        if (srcPerm != 0) {
            QFile(fileDest).setPermissions(srcPerm);
        }
        return true;
    }

    // Apro il file risultato
    QFile outFile;
    outFile.setFileName(fileDest);
    if(!outFile.open(QIODevice::WriteOnly)) return false;

    // Copio i dati
    if (!copyData(inFile, outFile) || inFile.getZipError()!=UNZ_OK) {
        outFile.close();
        removeFile(QStringList(fileDest));
        return false;
    }
    outFile.close();

    // Chiudo i file
    inFile.close();
    if (inFile.getZipError()!=UNZ_OK) {
        removeFile(QStringList(fileDest));
        return false;
    }

    if (srcPerm != 0) {
        outFile.setPermissions(srcPerm);
    }
    return true;
}

bool ExtractWorker::removeFile(QStringList listFile) {
    bool ret = true;
    // Per ogni file
    for (int i=0; i<listFile.count(); i++) {
        // Lo elimino
        ret = ret && QFile::remove(listFile.at(i));
    }
    return ret;
}











