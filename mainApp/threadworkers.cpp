/****************************************************************************
**
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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
#include <quazip.h>
#include <quazipfile.h>

#include "threadworkers.h"
#include "step.h"
#include "paths.h"
#include "lpub.h"
#include "meta.h"
#include "version.h"
#include "application.h"
#include "editwindow.h"
#include "lpub_preferences.h"
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtConcurrent>
#endif

//#ifdef WIN32
//#include <clocale>
//#endif // WIN32

PartWorker::PartWorker(QString archiveFile, QObject *parent) : QObject(parent)
{
  _ldrawArchiveFile       = archiveFile;
  _endThreadNowRequested  = false;
}

PartWorker::PartWorker(bool onDemand, QObject *parent) : QObject(parent)
{
  _resetSearchDirSettings = false;
  _endThreadNowRequested  = false;
  _ldrawCustomArchive     = Preferences::validLDrawCustomArchive;

  if (! onDemand) {
    _ldSearchDirsKey = Preferences::ldrawSearchDirsKey;
    _lsynthPartsDir  = QDir::toNativeSeparators(QString("%1/LSynthParts").arg(Preferences::lpubDataPath));
    _customPartDir   = QDir::toNativeSeparators(QString("%1/%2custom/parts").arg(Preferences::lpubDataPath).arg(Preferences::validLDrawLibrary));
    _customPrimDir   = QDir::toNativeSeparators(QString("%1/%2custom/p").arg(Preferences::lpubDataPath).arg(Preferences::validLDrawLibrary));

    _excludedSearchDirs << ".";
    _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawLibPath).arg("parts"));
    _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawLibPath).arg("p"));
    if (Preferences::usingDefaultLibrary) {
      _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawLibPath).arg("unofficial/parts"));
      _excludedSearchDirs << QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawLibPath).arg("unofficial/p"));
    }

    setDoFadeStep(Preferences::enableFadeSteps);
    setDoHighlightStep(Preferences::enableHighlightStep && !gui->suppressColourMeta());
  }
}

/*
 * LDraw search directories preferences.
 */
void PartWorker::ldsearchDirPreferences() {

  setDoFadeStep(Preferences::enableFadeSteps);
  setDoHighlightStep(Preferences::enableHighlightStep && !gui->suppressColourMeta());

  if (!_resetSearchDirSettings && !Preferences::lpub3dLoaded) {
    emit Application::instance()->splashMsgSig(tr("50% - Search directory preferences loading..."));
  } else {
    emit gui->messageSig(LOG_INFO,tr("Reset search directories..."));
  }

  QSettings Settings;
  QString const LdrawiniFilePathKey("LDrawIniFile");

  // qDebug() << QString(tr("01 ldrawIniFoundReg(Original) = %1").arg((ldrawIniFoundReg ? "True" : "False")));

  // Check for and load LDrawINI
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey))) {
      QString const ldrawiniFilePath = Settings.value(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey)).toString();
      QFileInfo ldrawiniInfo(ldrawiniFilePath);
      if (ldrawiniInfo.exists()) {
          Preferences::ldrawiniFile = ldrawiniInfo.absoluteFilePath();
          Preferences::ldrawiniFound = true;
          gui->addEditLDrawIniFileAction();
        } else {
          Settings.remove(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey));
          Settings.remove(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey));
        }
    } else if (ldPartsDirs.initLDrawSearchDirs()) {
      QFileInfo ldrawiniInfo(ldPartsDirs.getSearchDirsOrigin());
      if (ldrawiniInfo.exists()) {
          Preferences::ldrawiniFile = ldrawiniInfo.absoluteFilePath();
          Preferences::ldrawiniFound = true;
          gui->addEditLDrawIniFileAction();
          Settings.setValue(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey), Preferences::ldrawiniFile);
          //qDebug() << QString(tr("01 Using LDraw.ini file form loadLDrawSearchDirs(): ").arg(Preferences::ldrawiniFile));
        } else {
          Settings.remove(QString("%1/%2").arg(SETTINGS,LdrawiniFilePathKey));
          //qDebug() << QString(tr("  -Failed to get LDraw.ini, valid file (from Preferences) does not exist."));
        }
    } else {
      emit gui->messageSig(LOG_INFO, tr("Unable to initialize LDrawINI. Using default search directories."));
    }

  if (!doFadeStep() && !doHighlightStep()) {
      _excludedSearchDirs << _customPartDir;
      _excludedSearchDirs << _customPrimDir;
  } else {
      Paths::mkCustomDirs();
  }

  emit gui->messageSig(LOG_INFO,(doFadeStep() ? tr("Fade Steps is ON.") : tr("Fade Steps is OFF.")));
  emit gui->messageSig(LOG_INFO,(doHighlightStep() ? tr("Highlight Step is ON.") : tr("Highlight Step is OFF.")));

  // LDrawINI not found and not reset so load registry key
  bool ldSearchDirsSettingKeyExist = Settings.contains(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey));
  if (!Preferences::ldrawiniFound && !_resetSearchDirSettings && ldSearchDirsSettingKeyExist) {
      emit gui->messageSig(LOG_INFO, tr("LDrawINI not found, loading LDSearch directories from registry key..."));
      QStringList searchDirs = Settings.value(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey)).toStringList();
      bool customDirsIncluded = false;
      // Process fade and highlight custom directories...
      Q_FOREACH (QString  const &searchDir, searchDirs) {
          if (QDir(searchDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
              // Skip custom directory if not doFadeStep or not doHighlightStep
              QString const customDir = QDir::toNativeSeparators(searchDir.toLower());
              if ((!doFadeStep() && !doHighlightStep()) && (customDir == _customPartDir.toLower() || customDir == _customPrimDir.toLower()))
                  continue;

              // If doFadeStep or doHighlightStep, check if custom directories included
              if ((doFadeStep() || doHighlightStep()) && !customDirsIncluded) {
                  customDirsIncluded = (customDir.toLower() == _customPartDir.toLower() ||
                                        customDir.toLower() == _customPrimDir.toLower());
                }
              Preferences::ldSearchDirs << searchDir;
              emit gui->messageSig(LOG_INFO, tr("Added search directory: %1").arg(searchDir));
          } else {
              emit gui->messageSig(LOG_NOTICE, tr("Search directory is empty and will be ignored: %1").arg(searchDir));
          }
      }
      // If fade step enabled but custom directories not defined in ldSearchDirs, add custom directories
      if ((doFadeStep() || doHighlightStep()) && !customDirsIncluded) {
          // We must force the custom directories for LDView as they are needed by ldview ini files
          if (Preferences::preferredRenderer == RENDERER_LDVIEW) {
              Preferences::ldSearchDirs << _customPartDir;
              emit gui->messageSig(LOG_INFO, tr("Add custom part directory: %1").arg(_customPartDir));
              Preferences::ldSearchDirs << _customPrimDir;
              emit gui->messageSig(LOG_INFO, tr("Add custom primitive directory %1").arg(_customPrimDir));
              customDirsIncluded = true;
          } else {
              if (QDir(_customPartDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                Preferences::ldSearchDirs << _customPartDir;
                customDirsIncluded = true;
                emit gui->messageSig(LOG_INFO, tr("Add custom part directory: %1").arg(_customPartDir));
              } else {
                emit gui->messageSig(LOG_INFO, tr("Custom part directory is empty and will be ignored: %1").arg(_customPartDir));
              }
              if (QDir(_customPrimDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                Preferences::ldSearchDirs << _customPrimDir;
                customDirsIncluded = true;
                emit gui->messageSig(LOG_INFO, tr("Add custom primitive directory: %1").arg(_customPrimDir));
              } else {
                emit gui->messageSig(LOG_INFO, tr("Custom primitive directory is empty and will be ignored: %1").arg(_customPrimDir));
              }
          }
          // update the registry if custom directory included
          if (customDirsIncluded) {
              Settings.setValue(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey), Preferences::ldSearchDirs);
          }
       }
    } else if (loadLDrawSearchDirs()) { //ldraw.ini not found and settingKey not found or reset so load local paths
      Settings.setValue(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey), Preferences::ldSearchDirs);
      emit gui->messageSig(LOG_INFO, tr("Loading LDraw parts search directories..."));
    } else {
      Settings.remove(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey));
      emit gui->messageSig(LOG_ERROR, tr("Unable to load search directories."));
    }

    // Add LSynth path to search directory list
    bool addSearchDir    = Preferences::addLSynthSearchDir;
    bool dirInSearchList = false;
    QStringList saveSearchDirs;
    Q_FOREACH (QString const &ldSearchDir, Preferences::ldSearchDirs) {
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
bool PartWorker::loadLDrawSearchDirs() {

  if (!_resetSearchDirSettings) {
    emit Application::instance()->splashMsgSig(tr("60% - Search directories loading..."));
  } else {
    emit gui->messageSig(LOG_INFO,tr("Reset - search directories loading..."));
  }

  setDoFadeStep(Preferences::enableFadeSteps);
  StringList  ldrawSearchDirs;

  if (ldPartsDirs.loadLDrawSearchDirs("")) {
      ldrawSearchDirs = ldPartsDirs.getLDrawSearchDirs();
      if (_resetSearchDirSettings)
          _saveLDSearchDirs = Preferences::ldSearchDirs;
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
          QString const ldrawSearchDir(dir);
          // check for Unofficial root directory
          if (!foundUnofficialRootDir)
            foundUnofficialRootDir = ldrawSearchDir.toLower() == unofficialRootDir.toLower();

          bool excludeSearchDir = false;
          Q_FOREACH (QString const &excludedDir, _excludedSearchDirs) {
              if ((excludeSearchDir =
                   ldrawSearchDir.toLower().contains(excludedDir.toLower()))) {
                  break;
                }
            }
          if (! excludeSearchDir) {
              // check if empty
              if (QDir(ldrawSearchDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                  Preferences::ldSearchDirs << ldrawSearchDir;
                  emit gui->messageSig(LOG_INFO, tr("Added search directory: %1").arg(ldrawSearchDir));
                }
            }
          // Check if custom directories included
          if ((doFadeStep() || doHighlightStep()) && !customDirsIncluded) {
              customDirsIncluded = (ldrawSearchDir.toLower() == _customPartDir.toLower() ||
                                  ldrawSearchDir.toLower() == _customPrimDir.toLower());
            }
        }
      // If fade step enabled but custom directories not defined in ldSearchDirs, add custom directories
      if ((doFadeStep() || doHighlightStep()) && !customDirsIncluded) {
          if (QDir(_customPartDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
              Preferences::ldSearchDirs << _customPartDir;
              emit gui->messageSig(LOG_INFO, tr("Add custom part directory: %1").arg(_customPartDir));
            } else {
              emit gui->messageSig(LOG_INFO, tr("Custom part directory is empty and will be ignored: %1").arg(_customPartDir));
            }
          if (QDir(_customPrimDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
              Preferences::ldSearchDirs << _customPrimDir;
              emit gui->messageSig(LOG_INFO, tr("Add custom primitive directory: %1").arg(_customPrimDir));
            } else {
              emit gui->messageSig(LOG_INFO, tr("Custom primitive directory is empty and will be ignored: %1").arg(_customPrimDir));
            }
        }
      // Add subdirectories from Unofficial root directory
      if (foundUnofficialRootDir) {
          QDir unofficialDir(unofficialRootDir);
          // Get sub directories
          QStringList unofficialSubDirs = unofficialDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::SortByMask);
//#ifdef QT_DEBUG_MODE
//          logDebug() << "unofficialSubDirs:" << unofficialSubDirs;
//#endif
          if (unofficialSubDirs.count() > 0) {
              // Recurse unofficial subdirectories for excluded directories
              Q_FOREACH (QString const &unofficialSubDirName, unofficialSubDirs) {
                  // Exclude invalid directories
                  bool excludeSearchDir = false;
                  QString const unofficialSubDir = QDir::toNativeSeparators(QString("%1/%2").arg(unofficialRootDir).arg(unofficialSubDirName));
                  Q_FOREACH (QString const &excludedDir, _excludedSearchDirs) {
                      if ((excludeSearchDir =
                           unofficialSubDir.toLower() == excludedDir.toLower())) {
                          break;
                      }
                  }
                  // check if paths are empty
                  if (!excludeSearchDir) {
                      // First, check if there are files in the subDir
                      bool dirIsEmpty = true;
                      if (QDir(unofficialSubDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                          Preferences::ldSearchDirs << unofficialSubDir;
                          dirIsEmpty = false;
                          emit gui->messageSig(LOG_INFO, tr("Added search directory: %1").arg(unofficialSubDir));
                      }
                      // Second, check if there are subSubDirs in subDir - e.g. ...unofficial/custom/textures
                      if (QDir(unofficialSubDir).entryInfoList(QDir::Dirs|QDir::NoSymLinks).count() > 0) {
                          // 1. get the unofficial subDir path - e.g. .../unofficial/custom/
                          QDir subSubDir(unofficialSubDir);
                          // 2. get list of subSubDirs in subDir path - e.g. .../custom/parts, .../custom/textures
                          QStringList subSubDirs = subSubDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::SortByMask);
                          // 3. search each subSubDir for files and subSubSubDir
                          Q_FOREACH (QString const &subSubDirName, subSubDirs) {
                              // 4. get the unofficialSubSubDir path - e.g. .../unofficial/custom/textures
                              QString const unofficialSubSubDir = QDir::toNativeSeparators(QString("%1/%2").arg(unofficialSubDir).arg(subSubDirName));
                              // First, check if there are files in subSubSubDir
                              if (QDir(unofficialSubSubDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                                  Preferences::ldSearchDirs << unofficialSubSubDir;
                                  dirIsEmpty = false;
                                  emit gui->messageSig(LOG_INFO, tr("Added search directory: %1").arg(unofficialSubSubDir));
                              }
                              // Second, check if there are subSubSubDirs in subDir - e.g. ...unofficial/custom/textures/model
                              if (QDir(unofficialSubSubDir).entryInfoList(QDir::Dirs|QDir::NoSymLinks).count() > 0) {
                                  // 5. get the unofficial subDir path - e.g. .../unofficial/custom/
                                  QDir subSubSubDir(unofficialSubSubDir);
                                  // 6. get list of subSubSubDirs in subDir path - e.g. .../custom/textures/model1, .../custom/textures/model2
                                  QStringList subSubSubDirs = subSubSubDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::SortByMask);
                                  // 7. search each subSubSubDir for files and subfolders
                                  Q_FOREACH (QString const &subSubDirName, subSubSubDirs) {
                                      // 8. get the unofficialSubSubSubDir path - e.g. .../unofficial/custom/textures/model
                                      QString const unofficialSubSubSubDir = QDir::toNativeSeparators(QString("%1/%2").arg(unofficialSubSubDir).arg(subSubDirName));
                                      // Exclude 'parts/s', 'p/8' and 'p/48' sub-directories
                                      excludeSearchDir = false;
                                      QStringList _excludedDirs = QStringList()
                                              << QDir::toNativeSeparators(QString("parts/s"))
                                              << QDir::toNativeSeparators(QString("p/8"))
                                              << QDir::toNativeSeparators(QString("p/48"));
                                      Q_FOREACH (QString const &excludedDir, _excludedDirs) {
                                          if ((excludeSearchDir =
                                               unofficialSubSubSubDir.toLower().endsWith(excludedDir.toLower()))) {
                                              break;
                                          }
                                      }
                                      // If subSubSubDir is not excluded - e.g. ...unofficial/custom/textures/parts/s...
                                      if (!excludeSearchDir) {
                                          // 9. Check if there are files in subDir
                                          if (QDir(unofficialSubSubSubDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                                              Preferences::ldSearchDirs << unofficialSubSubSubDir;
                                              dirIsEmpty = false;
                                              emit gui->messageSig(LOG_INFO, tr("Added search directory: %1").arg(unofficialSubSubSubDir));
                                          }
                                          if (dirIsEmpty) {
                                              emit gui->messageSig(LOG_NOTICE, tr("Search directory is empty and will be ignored: %1").arg( unofficialSubSubSubDir));
                                          }
                                      }
                                  } // For each subSubSubDir
                              }
                              if (dirIsEmpty) {
                                  emit gui->messageSig(LOG_NOTICE, tr("Search directory is empty and will be ignored: %1").arg( unofficialSubSubDir));
                              }
                          } // For each subSubDir
                      }
                      if (dirIsEmpty) {
                          emit gui->messageSig(LOG_NOTICE, tr("Search directory is empty and will be ignored: %1").arg( unofficialSubDir));
                      }
                  }
              } // For each unofficialSubDir
          }
      }

      if (_resetSearchDirSettings) {
          processLDSearchDirParts();
          _saveLDSearchDirs.clear();
      }

    } else {
      emit gui->messageSig(LOG_ERROR, tr("Load Search directories (ldPartsDirs.loadLDrawSearchDirs) call failed."));
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
    if (Preferences::preferredRenderer == RENDERER_LDGLITE && !Preferences::ldSearchDirs.isEmpty()) {

        emit Application::instance()->splashMsgSig(tr("85% - LDGlite Search directories loading..."));
//#ifdef QT_DEBUG_MODE
//        logDebug() << "SEARCH DIRECTORIES TO PROCESS" << Preferences::ldSearchDirs ;
//#endif
        emit gui->messageSig(LOG_INFO, tr("LDGlite Search Directories..."));

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
        Q_FOREACH (QString const &ldgliteSearchDir, Preferences::ldSearchDirs) {
            // Exclude invalid directories
            bool excludeSearchDir = false;
            // Skip over customDirs if fade or highlight step
            if (!(doFadeStep() || doHighlightStep()) &&
                !(ldgliteSearchDir.toLower() == QString(_customPartDir).toLower() ||
                  ldgliteSearchDir.toLower() == QString(_customPrimDir).toLower())) {
                Q_FOREACH (QString const &excludedDir, ldgliteExcludedDirs) {
                    if ((excludeSearchDir =
                         ldgliteSearchDir.toLower() == excludedDir.toLower())) {
                        break;
                    }
                }
            }
            if (!excludeSearchDir) {
                // check if empty
                if (QDir(ldgliteSearchDir).entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0) {
                    count++;
                    count > 1 ? Preferences::ldgliteSearchDirs.append(QString("|%1").arg(ldgliteSearchDir)):
                                Preferences::ldgliteSearchDirs.append(ldgliteSearchDir);
                    emit gui->messageSig(LOG_INFO, tr("Add ldglite search directory: %1").arg(ldgliteSearchDir));
                }else {
                    emit gui->messageSig(LOG_NOTICE, tr("Ldglite search directory is empty and will be ignored: %1").arg(ldgliteSearchDir));
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
    bool customDirs = false;
    if (!Preferences::ldSearchDirs.contains(_customPartDir)) {
        Preferences::ldSearchDirs << _customPartDir;
        emit gui->messageSig(LOG_INFO, tr("Added custom part directory: %1").arg(_customPartDir));
        customDirs = true;
    }
    if (!Preferences::ldSearchDirs.contains(_customPrimDir)) {
        Preferences::ldSearchDirs << _customPrimDir;
        emit gui->messageSig(LOG_INFO, tr("Added custom primitive directory: %1").arg(_customPrimDir));
        if (!customDirs)
            customDirs = true;
    }
    updateLDSearchDirs(true /*archive*/, customDirs);
}

/*
 * Remove customPartDir and customPrim dir from ldSearchDirs
 * when fade and highlight step is disabled
 */

void PartWorker::removeCustomDirs() {
    if (Preferences::ldSearchDirs.contains(_customPartDir)) {
        Preferences::ldSearchDirs.removeAll(_customPartDir);
        emit gui->messageSig(LOG_INFO, tr("Removed custom part directory: %1").arg(_customPartDir));
    }
    if (Preferences::ldSearchDirs.contains(_customPrimDir)) {
        Preferences::ldSearchDirs.removeAll(_customPrimDir);
        emit gui->messageSig(LOG_INFO, tr("Removed custom primitive directory: %1").arg(_customPrimDir));
    }
    updateLDSearchDirs();
}

/*
 * Update ldSearch Directory list and archvie new parts if any
 */

void PartWorker::updateLDSearchDirs(bool archive /*false*/, bool custom /*false*/) {
    // Update the registry
    QSettings Settings;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey), Preferences::ldSearchDirs);

    // Update LDView extra search directories
    QString const couldNotUpdate = tr("Could not update %1. Missing [ExtraSearchDirs] section.");
    Preferences::setMessageLogging(DEFAULT_LOG_LEVEL);
    if (Preferences::ldviewInstalled && !Preferences::setLDViewExtraSearchDirs(Preferences::ldviewIni))
       emit gui->messageSig(LOG_ERROR, couldNotUpdate.arg(Preferences::ldviewIni));
    if (Preferences::ldviewInstalled && Preferences::povRayInstalled && !Preferences::setLDViewExtraSearchDirs(Preferences::ldviewPOVIni))
       emit gui->messageSig(LOG_ERROR, couldNotUpdate.arg(Preferences::ldviewPOVIni));
    if (Preferences::povRayInstalled && !Preferences::setLDViewExtraSearchDirs(Preferences::nativeExportIni))
       emit gui->messageSig(LOG_ERROR, couldNotUpdate.arg(Preferences::nativeExportIni));
    Preferences::setMessageLogging();

    // Update LDGLite extra search directories
    if (Preferences::ldgliteInstalled && Preferences::preferredRenderer == RENDERER_LDGLITE)
       populateLdgLiteSearchDirs();

    // Archive search directory parts
    if (archive) {
        QStringList dirs;
        if (custom) {
            QDir dir(_customPartDir);
            if (dir.exists()) {
                if (dir.entryInfoList(QDir::NoDotAndDotDot|QDir::Dirs|QDir::Files|QDir::NoSymLinks).count())
                    dirs << dir.absolutePath();
                dir.setPath(_customPrimDir);
                if (dir.exists() && dir.entryInfoList(QDir::NoDotAndDotDot|QDir::Dirs|QDir::Files|QDir::NoSymLinks).count())
                   dirs << dir.absolutePath();
            } else {
                Paths::mkCustomDirs();
            }
            if (dirs.size())
                processPartsArchive(dirs, "custom directory");
        } else if (_updateLDSearchDirs.size()) {
            Q_FOREACH (const QString &searchDir, _updateLDSearchDirs) {
                QDir dir(searchDir);
                if (dir.exists()) {
                    if (dir.entryInfoList(QDir::NoDotAndDotDot|QDir::Dirs|QDir::Files|QDir::NoSymLinks).count())
                        dirs << dir.absolutePath();
                } else {
                    dir.mkpath(".");
                }
            }
            _updateLDSearchDirs.clear();
            if (dirs.size())
                processPartsArchive(dirs, "update search directory");
        } else {
            processLDSearchDirParts();
        }
    }
}

/*
 * SLOT instance of update ldSearch Directory list and archvie new parts if any
 */

void PartWorker::updateLDSearchDirsParts()
{
    updateLDSearchDirs(true);
}

/*
 * Get LSynth directory path
 */

QString PartWorker::getLSynthDir() {

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
void PartWorker::processLDSearchDirParts() {

    QStringList dirs;

    // Automatically load default LSynth when add to search directory is disabled
    if (!Preferences::addLSynthSearchDir) {
        QString dir = getLSynthDir();
        if (!dir.isEmpty())
            dirs.append(dir);
    }

    if (_resetSearchDirSettings) {
        Q_FOREACH (QString const &searchDir, Preferences::ldSearchDirs) {
            if (_saveLDSearchDirs.contains(searchDir))
                continue;
            dirs.append(searchDir);
        }
    } else {
        dirs += Preferences::ldSearchDirs;
    }

    if (dirs.size())
        processPartsArchive(dirs, "search directory");

    // qDebug() << "\nFinished Processing Search Directory Parts.";
}

/*
 * scan LDraw library files for static colored parts and create fade copy
 */
void PartWorker::processFadeColourParts(bool overwrite, bool setup)
{
  //qDebug() << QString("Received overwrite fade parts = %1").arg(overwriteCustomParts ? "True" : "False");
  if (doFadeStep())
    processCustomColourParts(FADE_PART, overwrite, setup);
}

/*
 * scan LDraw library files for static colored parts and create highlight copy
 */
void PartWorker::processHighlightColourParts(bool overwrite, bool setup)
{
  //qDebug() << QString("Received overwrite highlight parts = %1").arg(overwriteCustomParts ? "True" : "False");
  if (doHighlightStep())
    processCustomColourParts(HIGHLIGHT_PART, overwrite, setup);
}

void PartWorker::processCustomColourParts(PartType partType, bool overwrite, bool setup)
{
  Q_UNUSED(setup)

  QString nameMod;
  if (partType == FADE_PART)
    nameMod = LPUB3D_COLOUR_FADE_SUFFIX;
  else if (partType == HIGHLIGHT_PART)
    nameMod = LPUB3D_COLOUR_HIGHLIGHT_SUFFIX;

  _timer.start();
  _customParts = 0;

  QStringList customPartsDirs;
  QStringList colourPartList;
  int existingCustomParts = 0;

  //DISABLE PROGRESS BAR - CAUSING MESSAGEBAR OVERLOAD
  //emit progressBarInitSig();
  //emit progressMessageSig("Parse Model File");
  Paths::mkCustomDirs();

  int subfiles = lpub->ldrawFile._subFileOrder.size();

  // process top-level submodels
  //emit progressRangeSig(1, subfiles);

  // in this block, we find colour parts and their children, append the nameMod
  // and confirm that the part exist if the part is not found, we submit it to be created
  for (int i = 0; i < subfiles && endThreadNotRequested(); i++) {
      const QString &subFileString = lpub->ldrawFile._subFileOrder[i].toLower();
      const QStringList &contents = lpub->ldrawFile.contents(subFileString);
      //emit progressSetValueSig(i);
      emit gui->messageSig(LOG_INFO,tr("00 PROCESSING SUBFILE CUSTOM COLOR PARTS FOR SUBMODEL: %1").arg(subFileString));
      for (int i = 0; i < contents.size() && endThreadNotRequested(); i++) {
          QString line = contents[i];
          QStringList tokens;
          split(line,tokens);
          if (tokens.size() == 15 && tokens[0] == "1") {
              // check if colored part and create custom version if yes
              QString fileString = LDrawColourParts::getLDrawColourPartInfo(tokens[tokens.size()-1]);
              // validate part is static color part;
              if (!fileString.isEmpty()) {
                  QString fileDir;
                  QString libType  = fileString.section(":::",0,0);
                  QString fileName = fileString.section(":::",1,1);
                  if ((fileString.indexOf("\\") != -1)) {
                     fileDir  = fileString.section(":::",1,1).split("\\").first();
                     fileName = fileString.section(":::",1,1).split("\\").last();
                  }
//#ifdef QT_DEBUG_MODE
//                  logDebug() << "FileDir:" << fileDir << "FileName:" << fileName;
//#endif
                  QDir customFileDirPath;
                  if (libType == "g"){ // generated part
                     customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(QDir::currentPath()).arg(Paths::tmpDir)));
                  } else if (fileDir.isEmpty()) {
                      customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir)));
                  } else if (fileDir == "s") {
                      customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customSubDir)));
                  } else if (fileDir == "p") {
                      customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrimDir)));
                  } else if (fileDir == "8") {
                      customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim8Dir)));
                  } else if (fileDir == "48") {
                      customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim48Dir)));
                  } else {
                      customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir)));
                  }
                  bool entryExists = false;
                  QString const customFileName = fileName.replace(".dat", "-" + nameMod + ".dat");
                  QFileInfo customFileInfo(customFileDirPath,customFileName);
                  entryExists = customFileInfo.exists();
                  if (!entryExists) {
                      // we stop here for generated part if not found
                      if (libType == "g") {
                          QString const fileStatus = tr("Generated part file %1 not found in %2.<br>"
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
                      Q_FOREACH (QString const &colourPart, colourPartList) {
                          if (colourPart == fileString) {
                              entryExists = true;
                              break;
                          }
                      }
                  }
                  // add part entry to list
                  if (!entryExists || overwrite) {
                      colourPartList << fileString;
                      emit gui->messageSig(LOG_NOTICE, tr("01 SUBMIT COLOUR PART INFO: %1 Line: %2").arg(fileString.replace(":::", " ")).arg(i));
                  } else {
                      emit gui->messageSig(LOG_NOTICE, tr("01 COLOUR PART EXIST - IGNORING: %1").arg(fileString.replace(":::", " ")));
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
          QString const error = tr("Process %1 color parts failed!.").arg(nameMod);
          emit gui->messageSig(LOG_ERROR,error);
          //emit progressStatusRemoveSig();
          emit customColourFinishedSig();
          return;
      }

      // Create the custom part
      createCustomPartFiles(partType);

      // Populate custom parts dirs
      Q_FOREACH (QDir const &customDir, Paths::customDirs) {
          if(customDir.entryInfoList(QDir::Files|QDir::NoSymLinks).count() > 0)
              customPartsDirs << customDir.absolutePath();
      }
      // Remove Duplicates
      customPartsDirs.removeDuplicates();

      if (_customParts > 0 && customPartsDirs.size() > 0) {
          // transfer to ldSearchDirs
          bool updateLDGLiteSearchDirs = false;
          Q_FOREACH (QString const &customPartDir, customPartsDirs) {
              bool customDirsIncluded = false;
              QString const customDir = QDir::toNativeSeparators(customPartDir.toLower());
              // check if custom directories included
              Q_FOREACH (QString const &ldSearchDir, Preferences::ldSearchDirs ) {
                  QString const searchDir = QDir::toNativeSeparators(ldSearchDir.toLower());
                  if (customDirsIncluded)
                      break;
                  customDirsIncluded = (searchDir == customDir);
              }
              // If not included add custom directories and update registry and LDView and LDGLite extra search directories
              if (!customDirsIncluded) {
                  Preferences::ldSearchDirs << QDir::toNativeSeparators(customPartDir);
#ifdef QT_DEBUG_MODE
                  logDebug() << "Add " + nameMod + " part directory to ldSearchDirs:" << customPartDir;
#endif
                  QSettings Settings;
                  Settings.setValue(QString("%1/%2").arg(SETTINGS,_ldSearchDirsKey), Preferences::ldSearchDirs);

                  QString const couldNotUpdate = tr("Could not update %1. Missing [ExtraSearchDirs] section.");
                  Preferences::setMessageLogging(DEFAULT_LOG_LEVEL);
                  if (Preferences::ldviewInstalled && !Preferences::setLDViewExtraSearchDirs(Preferences::ldviewIni))
                      emit gui->messageSig(LOG_ERROR, couldNotUpdate.arg(Preferences::ldviewIni));
                  if (Preferences::ldviewInstalled && Preferences::povRayInstalled && !Preferences::setLDViewExtraSearchDirs(Preferences::ldviewPOVIni))
                      emit gui->messageSig(LOG_ERROR, couldNotUpdate.arg(Preferences::ldviewPOVIni));
                  if (Preferences::povRayInstalled && !Preferences::setLDViewExtraSearchDirs(Preferences::nativeExportIni))
                      emit gui->messageSig(LOG_ERROR, couldNotUpdate.arg(Preferences::nativeExportIni));
                  Preferences::setMessageLogging();
                  updateLDGLiteSearchDirs = Preferences::ldgliteInstalled && Preferences::preferredRenderer == RENDERER_LDGLITE;
              }
          }
          if (updateLDGLiteSearchDirs)
              populateLdgLiteSearchDirs();
      }
  }

  // Archive custom parts
  if (colourPartList.size() > 0 || (existingCustomParts > 0 && overwrite)) {
      // Process archive files
      QString const comment = tr("color %1").arg(nameMod);
      if (!processPartsArchive(customPartsDirs, comment, overwrite)) {
          QString error = tr("Process %1 parts archive failed!.").arg(nameMod);
          emit gui->messageSig(LOG_ERROR,error);
          //emit progressStatusRemoveSig();
          emit customColourFinishedSig();
          return;
      }
  }

  // summary stats
  qint64 elapsed = _timer.elapsed();
  int milliseconds = int(elapsed % 1000);
  elapsed /= 1000;
  int seconds = int(elapsed % 60);
  elapsed /= 60;
  int minutes = int(elapsed % 60);
  elapsed /= 60;
  int hours = int(elapsed % 24);

  const QString time = tr("Elapsed time is %1:%2:%3:%4")
  .arg(hours, 2, 10, QLatin1Char('0'))
  .arg(minutes, 2, 10, QLatin1Char('0'))
  .arg(seconds,  2, 10, QLatin1Char('0'))
  .arg(milliseconds,  3, 10, QLatin1Char('0'));

  int allCustomParts = _customParts += existingCustomParts;

  const QString fileStatus = allCustomParts > 0
                                 ? tr("%1 %2 %3 created. Archive library %4. %5.")
                                      .arg(_customParts)
                                      .arg(nameMod)
                                      .arg(allCustomParts == 1
                                                ? tr("part")
                                                : tr("parts"))
                                      .arg(_partsArchived
                                                ? tr("updated")
                                                : tr("not updated, custom parts already archived"))
                                      .arg(time)
                                 : tr("No %2 parts created.").arg(nameMod);

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
        fileStatus = tr("Archive file does not exist: %1. The process will terminate.").arg(archiveFileInfo.absoluteFilePath());
        emit gui->messageSig(LOG_ERROR, fileStatus);
        return false;
    }

    //DISABLE PROGRESS BAR - CAUSING MESSAGEBAR OVERLOAD
    //emit progressResetSig();
    //emit progressMessageSig("Process Color Parts...");
    //emit progressRangeSig(1, colourPartList.size());
    //int partCount = 0;

    int partsProcessed = 0;
    QStringList childrenColourParts;

    Q_FOREACH (QString const &partEntry, colourPartList) {

        bool partFound = false;

        QString cpPartEntry = partEntry;
        bool unOffLib = cpPartEntry.section(":::",0,0) == "u";
        //emit gui->messageSig(LOG_INFO,QString("Library Type: %1").arg((unOffLib ? "Unofficial Library" : "Official Library"));

        QString libPartDir;
        QString libPartName = cpPartEntry.section(":::",1,1);
        if ((cpPartEntry.indexOf("\\") != -1)) {
           libPartDir  = cpPartEntry.section(":::",1,1).split("\\").first();
           libPartName = cpPartEntry.section(":::",1,1).split("\\").last();
        }
        //emit gui->messageSig(LOG_INFO,QString("Lib Part Name: %1").arg(libPartName));

        //emit progressSetValueSig(partCount++);

        QuaZip zip(unOffLib ? unofficialLib : officialLib);
        if (!zip.open(QuaZip::mdUnzip)) {
            emit gui->messageSig(LOG_ERROR, tr("Could not open archive to add content. Return code %1.<br>"
                                               "Archive file %2 may be open in another program.")
                                               .arg(zip.getZipError()).arg(QFileInfo(unOffLib ? unofficialLib : officialLib).fileName()));
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
                    emit gui->messageSig(LOG_ERROR, tr("Failed to OPEN Part file :%1").arg(zip.getCurrentFileName()));
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
                        QString childFileString = LDrawColourParts::getLDrawColourPartInfo(tokens[tokens.size()-1]);
                        // validate part is static color part;
                        if (!childFileString.isEmpty()) {
                            QString fileDir;
                            QString fileName = childFileString.section(":::",1,1);
                            if ((childFileString.indexOf("\\") != -1)) {
                               fileDir  = childFileString.section(":::",1,1).split("\\").first();
                               fileName = childFileString.section(":::",1,1).split("\\").last();
                            }
//#ifdef QT_DEBUG_MODE
//                            logDebug() << "FileDir:" << fileDir << "FileName:" << fileName;
//#endif
                            QDir customFileDirPath;
                            if (fileDir.isEmpty()) {
                               customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir)));
                            } else  if (fileDir == "s") {
                                customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customSubDir)));
                            } else  if (fileDir == "p") {
                                customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrimDir)));
                            } else  if (fileDir == "8") {
                                customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim8Dir)));
                            } else if (fileDir == "48") {
                                customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim48Dir)));
                            } else {
                                customFileDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir)));
                            }
                            bool entryExists = false;
                            QString customFileName = fileName.replace(".dat", "-" + nameMod + ".dat");
                            QFileInfo customFileInfo(customFileDirPath,customFileName);
                            entryExists = customFileInfo.exists();
                            // check if child part entry already in list
                            if (!entryExists) {
                                Q_FOREACH (QString const &childColourPart, childrenColourParts) {
                                    if (childColourPart == childFileString) {
                                        entryExists = true;
                                        break;
                                    }
                                }
                            }
                            // add chile part entry to list
                            if (!entryExists) {
                                childrenColourParts << childFileString;
                                emit gui->messageSig(LOG_NOTICE, tr("03 SUBMIT CHILD COLOUR PART INFO: %1").arg(childFileString.replace(":::", " ")));
                            } else {
                                emit gui->messageSig(LOG_NOTICE, tr("03 CHILD COLOUR PART EXIST - IGNORING: %1").arg(childFileString.replace(":::", " ")));
                            }
                        }
                    }
                }
                // determine part type
                int ldrawPartType = -1;
                if (libPartDir == libPartName) {
                    ldrawPartType = LD_PARTS;
                } else  if (libPartDir == "s") {
                    ldrawPartType = LD_SUB_PARTS;
                } else  if (libPartDir == "p") {
                    ldrawPartType = LD_PRIMITIVES;
                } else  if (libPartDir == "8") {
                    ldrawPartType = LD_PRIMITIVES_8;
                } else if (libPartDir == "48") {
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
                emit gui->messageSig(LOG_TRACE, tr("Part already in list: %1").arg(libPartName));
                break;
            }
        }

        if (!partFound) {
            QString const lib = Preferences::usingDefaultLibrary ? QLatin1String("Unofficial") : QLatin1String("Custom Parts");
            fileStatus = tr("Part file %1 not found in %2. Be sure the %3 fadeStepColorParts.lst file is up to date.")
                             .arg(cpPartEntry.replace(":::", " "))
                             .arg(unOffLib ? tr("%1 Library").arg(lib) : QLatin1String("Official Library"))
                             .arg(Preferences::validLDrawLibrary);
            emit gui->messageSig(LOG_ERROR, fileStatus);
        }

        zip.close();

        if (zip.getZipError() != UNZ_OK) {
            emit gui->messageSig(LOG_ERROR, tr("zip close error. Return code %1.").arg(zip.getZipError()));
            return false;
        }
    }
    //emit progressSetValueSig(colourPartList.size());

    // recurse part file content to check if any children are color parts
    if (childrenColourParts.size() > 0)
        processColourParts(childrenColourParts, partType);

    QString const message = tr("%1 Color %2 content processed.")
                                .arg(partsProcessed)
                                .arg(partsProcessed > 1 ? tr("parts") : tr("part"));
    emit gui->messageSig(LOG_INFO,message);

    return true;
}


bool PartWorker::createCustomPartFiles(const PartType partType, bool  overwriteCustomParts) {

     QString nameMod, colourPrefix;
     if (partType == FADE_PART) {
       nameMod = LPUB3D_COLOUR_FADE_SUFFIX;
       colourPrefix = LPUB3D_COLOUR_FADE_PREFIX;
     } else if (partType == HIGHLIGHT_PART) {
       nameMod = LPUB3D_COLOUR_HIGHLIGHT_SUFFIX;
       colourPrefix = LPUB3D_COLOUR_HIGHLIGHT_PREFIX;
     }

    //DISABLE PROGRESS BAR - CAUSING MESSAGEBAR OVERLOAD
    //int maxValue            = _partList.size();
    //emit progressResetSig();
    //emit progressMessageSig("Creating Custom Color Parts");
    //emit progressRangeSig(1, maxValue);

    QStringList customPartContent, customPartColourList;
    QString customPartFile;

    for(int part = 0; part < _partList.size() && endThreadNotRequested(); part++) {

        //emit progressSetValueSig(part);

        QMap<QString, ColourPart>::iterator cp = _colourParts.find(_partList[part]);

        if(cp != _colourParts.end()) {

            // prepare absoluteFilePath for custom file
            QDir customPartDirPath;
            switch (cp.value()._partType)
            {
            case LD_PARTS:
                customPartDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPartDir)));
                break;
            case LD_SUB_PARTS:
                customPartDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customSubDir)));
                break;
            case LD_PRIMITIVES:
                customPartDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrimDir)));
                break;
            case LD_PRIMITIVES_8:
                customPartDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim8Dir)));
                break;
            case LD_PRIMITIVES_48:
                customPartDirPath.setPath(QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customPrim48Dir)));
                break;
            }
            QString customFile = cp.value()._fileNameStr;
            QFileInfo customStepColourPartFileInfo(customPartDirPath,customFile.replace(".dat", "-" + nameMod + ".dat"));
            if (customStepColourPartFileInfo.exists() && !overwriteCustomParts) {
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
                    if (!FadeMetaAdded && Preferences::enableFadeSteps && partType == FADE_PART) {
                       customPartContent.insert(i,QString("0 !FADE %1").arg(Preferences::fadeStepsOpacity));
                       FadeMetaAdded = true;
                    }
                    // Insert opening silhouette meta
                    if (!SilhouetteMetaAdded && Preferences::enableHighlightStep && partType == HIGHLIGHT_PART) {
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
                    if (cpc != _colourParts.end()) {
                        if (cpc.value()._fileNameStr == searchFileNameStr) {
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
                    // QString oldColour(tokens[1]);          // logging only: show color lines
                    // Insert color code for fade or silhouette part
                    if (partType == FADE_PART || partType == HIGHLIGHT_PART) {
                        // generate custom color entry
                        QString const colourCode = tokens[1];
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
                if (i+1 == cp.value()._contents.size()) {
                    if (FadeMetaAdded) {
                       customPartContent.append(QString("0 !FADE"));
                    }
                    if (SilhouetteMetaAdded) {
                       customPartContent.append(QString("0 !SILHOUETTE"));
                    }
                }
            }

            // add the custom part color list to the header of the custom part contents
            customPartColourList.removeDuplicates(); // remove dupes

            int insertionPoint = 0; // skip the first line (title)
            QStringList words;
            // scan part header...
            for (int i = insertionPoint; i < customPartContent.size(); i++) {
                insertionPoint = i;
                // Upper case first title first letter
                if (insertionPoint == 0) {
                   QString line = customPartContent[i];
                   split(line, words);
                   for (int j = 0; j < words.size(); j++) {
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
                customPartColourList.removeDuplicates();  // remove dupes
                customPartContent.insert(insertionPoint,"0 // LPub3D part custom colours");
                for (int i = 0; i < customPartColourList.size(); i++) {
                    insertionPoint++;
                    customPartContent.insert(insertionPoint,customPartColourList.at(i));
                }
                customPartContent.insert(++insertionPoint,"0");
            }

            //emit gui->messageSig(LOG_TRACE,tr("04 SAVE CUSTGOM COLOUR PART: %1").arg(customPartFile));
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
        QString const message = tr("Failed to open %1 for writing: %2").arg(fileName).arg(file.errorString());
        emit gui->messageSig(LOG_ERROR, message);
        return false;

    } else {
        QTextStream out(&file);
        for (int i = 0; i < customPartContent.size(); i++) {

            out << customPartContent[i] << lpub_endl;
        }
        file.close();
        emit gui->messageSig(LOG_NOTICE,tr("05 WRITE CUSTOM PART TO DISC: %1").arg(fileName));
        return true;
    }
}


void PartWorker::insert(
        const QStringList   &contents,
        const QString       &fileNameStr,
        const int           &partType,
        const bool          &unOff) {

    bool partErased = false;
    QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

    if (i != _colourParts.end()) {
        _colourParts.erase(i);
        partErased = true;
//        emit gui->messageSig(LOG_NOTICE,tr("PART ALREADY IN LIST - PART ERASED %1, UnOff Lib: %2").arg(i.value()._fileNameStr).arg(i.value()._unOff));
    }

    ColourPart colourPartEntry(contents, fileNameStr, partType, unOff);
    _colourParts.insert(fileNameStr, colourPartEntry);

    if (! partErased)
      _partList << fileNameStr;
//    emit gui->messageSig(LOG_NOTICE,tr("02 INSERT (COLOUR PART ENTRY) - UID: %1  fileNameStr: %2 partType: %3 unOff: %4").arg(fileNameStr).arg(fileNameStr).arg(partType).arg(unOff));
}


int PartWorker::size(const QString &fileNameStr) {

  QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

  int mySize;
  if (i ==  _colourParts.end()) {
    mySize = 0;
  } else {
    mySize = i.value()._contents.size();
  }
  return mySize;
}

QStringList PartWorker::contents(const QString &fileNameStr) {

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
//    emit gui->messageSig(LOG_NOTICE,tr("REMOVE COLOUR PART: %1 - from contents and _partList").arg(fileNameStr.toLower()));
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


bool PartWorker::endThreadEventLoopNow() {
//  emit gui->messageSig(LOG_TRACE,tr("endThreadEventLoopNow: %1").arg(_endThreadNowRequested));
  return _endThreadNowRequested;
}


void PartWorker::requestEndThreadNow() {
//  emit gui->messageSig(LOG_TRACE,tr("requestEndThreadNow: %1").arg(_endThreadNowRequested));
  _endThreadNowRequested = true;
}


bool PartWorker::processPartsArchive(const QStringList &ldPartsDirs, const QString &comment, bool overwriteCustomParts) {

  // Used by refresh unofficial LDraw parts routine - Gui::refreshLDrawUnoffParts()
  if (Preferences::skipPartsArchive) {
      Preferences::skipPartsArchive = false;
      return true;
  }

  auto emitSplashMessage = [this] (const QString message) {
      if (! okToEmitToProgressBar()) {
          emit Application::instance()->splashMsgSig(message);
      }
  };

  // Append custom parts to custom parts library for Visual Editor's consumption
  //DISABLE PROGRESS BAR - CAUSING MESSAGEBAR OVERLOAD
  //QTime dt;
  QElapsedTimer t, tf;
  bool reloadLibrary = true;
  QString archiveFile = _ldrawArchiveFile;
  if (archiveFile.isEmpty()) {
      QFileInfo libFileInfo(Preferences::lpub3dLibFile);
      archiveFile = QDir::toNativeSeparators(QString("%1/%2").arg(libFileInfo.absolutePath(),_ldrawCustomArchive));
  } else {
      _ldrawArchiveFile = QString();
      reloadLibrary = false;
  }
  QString returnMessage = tr("Archiving %1 parts to : %2.").arg(comment,archiveFile);
  int returnMessageSeverity = 1; // 1=Error, 2=Notice
  emit gui->messageSig(LOG_INFO,tr("Archiving %1 parts to %2.").arg(comment,archiveFile));

  emitSplashMessage(tr("60% - Archiving %1 parts, please wait...").arg(comment));

  //if (okToEmitToProgressBar())
  //    emit progressRangeSig(0, 0);

  int partCount = 0;
  int totalPartCount = 0;
  QString summary;

  tf.start();

  //emit progressMessageSig(tr("Archiving %1 parts...\nProcessing: %2")
  //                        .arg(comment).arg(QDir::toNativeSeparators(ldPartsDirs[0])));

  for (int i = 0; i < ldPartsDirs.size() && endThreadNotRequested(); i++) {
      t.start();

      QDir partDir(ldPartsDirs[i]);

  //    emit progressSetValueSig(i);

      emitSplashMessage(tr("60% - Archiving %1, please wait...")
                           .arg(QDir(ldPartsDirs[i]).dirName()));

      if (!archiveParts.Archive( archiveFile,
                                 partDir.absolutePath(),
                                 returnMessage,
                                 returnMessageSeverity,
                                 tr("Append %1 parts").arg(comment),
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
      if (ok) {
          totalPartCount += partCount;
          summary = totalPartCount == 0 ? "parts" :
                    totalPartCount == 1 ? tr("[Total %1] part").arg(totalPartCount) :
                                          tr("[Total %1] parts").arg(totalPartCount);
      }

   //   emit progressMessageSig(tr("Archiving %1 parts...\nProcessing: %2\nArchived %3 %4")
   //                              .arg(comment)
   //                              .arg(QDir::toNativeSeparators(ldPartsDirs[i]))
   //                              .arg(partCount)
   //                              .arg(summary));

      emit gui->messageSig(LOG_INFO, tr("Archived %1 %2 from %3 %4")
                                        .arg(partCount)
                                        .arg(summary)
                                        .arg(partDir.absolutePath())
                                        .arg(gui->elapsedTime(t.elapsed())));
  }

  // Archive parts
  QString partsLabel = tr("parts");
  if (totalPartCount > 0) {
      // Reload unofficial library parts into memory - only if initial library load already done !
      if (Preferences::lpub3dLoaded && reloadLibrary) {
          if (!gui->ReloadUnofficialPiecesLibrary()) {
              returnMessage = tr("Failed to reload %1 archive parts library into memory.")
                                 .arg(Preferences::validLDrawLibrary);
              emit gui->messageSig(LOG_ERROR,returnMessage);
              return false;
          } else {
              partsLabel = totalPartCount == 1 ? tr("part") : tr("parts");
              returnMessage = tr("Reloaded %1 archive parts library into memory with %2 new %3.")
                                 .arg(Preferences::validLDrawLibrary).arg(totalPartCount).arg(partsLabel);
              emit gui->messageSig(LOG_INFO,returnMessage);
          }
      }
      partsLabel = totalPartCount == 1 ? tr("parts") : tr("parts");
      returnMessage = tr("Finished. Archived %1 %2 %3.\n%4")
                         .arg(totalPartCount).arg(comment).arg(partsLabel).arg(gui->elapsedTime(tf.elapsed()));

      emit partsArchiveResultSig(totalPartCount);

      emitSplashMessage(tr("70% - Finished. Archived %1 %2 parts.").arg(totalPartCount).arg(comment));

      _partsArchived = true;

  } else {
      returnMessage = tr("Finished. Parts exist in %1 archive library. No new %2 parts archived.")
                         .arg(Preferences::validLDrawLibrary).arg(comment);

      emitSplashMessage(tr("70% - Finished. No new %1 parts archived.").arg(comment));

      _partsArchived = false;
  }

  //emit progressMessageSig(returnMessage);

  emit gui->messageSig(LOG_INFO,returnMessage.replace("\n", " "));

  //if (okToEmitToProgressBar()) {
  //    // time delay to display archive totals
  //    dt = QTime::currentTime().addSecs(3);
  //    while (QTime::currentTime() < dt)
  //        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  //}

  emit partsArchiveFinishedSig();

  return true;
}

void PartWorker::processPartsArchive() {
    QString const comment = tr("custom");
    if (!processPartsArchive(
                Preferences::ldSearchDirs,
                comment,
                true /*overwriteCustomParts*/)) {
        QString error = tr("Process %1 parts archive failed!.").arg(comment);
        emit gui->messageSig(LOG_ERROR,error);
    }
    emit partsArchiveFinishedSig();
}

bool PartWorker::okToEmitToProgressBar()
{
    return (Preferences::lpub3dLoaded && Preferences::modeGUI);
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
    _filePath = QString();
    _ldrawCustomArchive    = Preferences::validLDrawCustomArchive;
}

/*
 * build color part listing
 *
 */

void ColourPartListWorker::generateCustomColourPartsList()
{
    QFileInfo colourFileList(Preferences::ldrawColourPartsFile);
    if (!colourFileList.exists())
        colourFileList.setFile(QString("%1/extras/%2").arg(Preferences::lpubDataPath, Preferences::validLDrawColorParts));

    // Archive library files
    QStringList archiveFiles;
    QFileInfo lpub3dLibFileInfo(QDir::toNativeSeparators(Preferences::lpub3dLibFile));
    if (lpub3dLibFileInfo.exists())
        archiveFiles << lpub3dLibFileInfo.absoluteFilePath();
    if (QFileInfo(QString("%1/%2").arg(lpub3dLibFileInfo.absolutePath(),_ldrawCustomArchive)).exists())
        archiveFiles << QString("%1/%2").arg(lpub3dLibFileInfo.absolutePath(),_ldrawCustomArchive);

    //library directories
    QStringList partTypeDirs;
    partTypeDirs << QLatin1String("p       Primitives");
    partTypeDirs << QLatin1String("p/8     Primitives - Low Resolution");
    partTypeDirs << QLatin1String("p/48    Primitives - High Resolution");
    partTypeDirs << QLatin1String("parts   Parts");
    partTypeDirs << QLatin1String("parts/s Sub-parts");

    _timer.start();
    _colWidthFileName = 0;

    int libCount = 1;

    fileSectionHeader(FADESTEP_INTRO_HEADER);
    _ldrawStaticColourParts  << QLatin1String("# Archive Libraries:");
    QString const lib = Preferences::validLDrawLibrary;
    QString const unoffLib = Preferences::usingDefaultLibrary ? QString("Unofficial %1").arg(lib)  : QString("%1 Custom").arg(lib);
    QString const offLib = Preferences::usingDefaultLibrary ? QString("Official %1").arg(lib) : lib;
    Q_FOREACH (QString const &archiveFile, archiveFiles) {
        QString const library = archiveFile == lpub3dLibFileInfo.absoluteFilePath() ? QString("%1 Library").arg(offLib) : QString("%1 Library").arg(unoffLib);
      _ldrawStaticColourParts  << QString("# %1. %2: %3").arg(libCount++).arg(library).arg(archiveFile);
    }
    int dirCount = 1;
    _ldrawStaticColourParts  << QString();
    _ldrawStaticColourParts  << QLatin1String("# Library Directories:");
    Q_FOREACH (QString const &partTypeDir, partTypeDirs) {
      _ldrawStaticColourParts  << QString("# %1. %2").arg(dirCount++).arg(partTypeDir);
    }
    _ldrawStaticColourParts  << QLatin1String("# ----------------------Do not delete above this line----------------------------------");
    _ldrawStaticColourParts  << QString();

    fileSectionHeader(FADESTEP_FILE_HEADER);

    emit progressBarInitSig();
    Q_FOREACH (QString const &archiveFile, archiveFiles) {
       if(!processArchiveParts(archiveFile)) {
           QString const error = tr("Process color parts list failed!.");
           emit gui->messageSig(LOG_ERROR,error);
           emit progressStatusRemoveSig();
           emit colourPartListFinishedSig();
           return;
       }
    }

    processChildren();

    writeLDrawColourPartFile(/*append=false*/);

    qint64 elapsed = _timer.elapsed();
    int milliseconds = int(elapsed % 1000);
    elapsed /= 1000;
    int seconds = int(elapsed % 60);
    elapsed /= 60;
    int minutes = int(elapsed % 60);
    elapsed /= 60;
    int hours = int(elapsed % 24);

    const QString time = tr("Elapsed time is %1:%2:%3:%4")
    .arg(hours, 2, 10, QLatin1Char('0'))
    .arg(minutes, 2, 10, QLatin1Char('0'))
    .arg(seconds,  2, 10, QLatin1Char('0'))
    .arg(milliseconds,  3, 10, QLatin1Char('0'));

    const QString fileStatus = tr("%1 Color Parts List successfully created with %2 entries. %3.")
                                       .arg(Preferences::validLDrawLibrary).arg(QString::number(_cpLines)).arg(time);
    fileSectionHeader(FADESTEP_FILE_STATUS, QString("# %1").arg(fileStatus));

    writeLDrawColourPartFile(true/*append*/);

    QSettings Settings;
    Preferences::ldrawColourPartsFile = colourFileList.absoluteFilePath();
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile"), Preferences::ldrawColourPartsFile);

    emit progressStatusRemoveSig();
    emit colourPartListFinishedSig();
    emit colorPartsListResultSig(_cpLines);
    emit gui->messageSig(LOG_INFO_STATUS,fileStatus);
}

bool ColourPartListWorker::processArchiveParts(const QString &archiveFile) {

   QString lib = Preferences::validLDrawLibrary;
   bool isUnOffLib = true;
   QString library = Preferences::usingDefaultLibrary ? tr("Unofficial %1 Library").arg(lib)  : tr("%1 Custom Library").arg(lib);

    if (archiveFile.contains(Preferences::validLDrawPartsArchive)) {
        library = Preferences::usingDefaultLibrary ? tr("Official %1 Library").arg(lib) : tr("%1 Library").arg(lib);
        isUnOffLib = false;
    }

    QuaZip zip(archiveFile);
    if (!zip.open(QuaZip::mdUnzip)) {
        emit gui->messageSig(LOG_ERROR, tr("Could not open archive to add content. Return code %1.<br>"
                                           "Archive file %2 may be open in another program.")
                                            .arg(zip.getZipError()).arg(archiveFile));
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
    emit gui->messageSig(LOG_INFO,tr("Processing Archive Parts for %1 - Parts Count: %2")
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
                emit gui->messageSig(LOG_ERROR, tr("Failed to OPEN Part file :%1").arg(libFileName));
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
    emit gui->messageSig(LOG_INFO,tr("Finished Processing %1 Parent Color Parts").arg(library));

    zip.close();
    if (zip.getZipError() != UNZ_OK) {
        emit gui->messageSig(LOG_ERROR,tr("Failed to close archive file. Return code %1.").arg(zip.getZipError()));
        return false;
    }
    return true;
}

/*
 * parse color part file to determine children parts with static color.
 */
void ColourPartListWorker::processFileContents(const QString &libFileName, const bool isUnOffLib) {
    QString fileEntry, fileName, color;
    QString libEntry          = libFileName;
    QString const libType     = isUnOffLib ? QLatin1String("U") : QLatin1String("O");
    QString const libFilePath = libEntry.remove("/" + libEntry.split("/").last());
    //emit gui->messageSig(LOG_TRACE,tr("Processing libFileName %1").arg(libFileName));
    bool hasColour = false;

    for (int i = 0; i < _partFileContents.size() && endThreadNotRequested(); i++) {
        QString line = _partFileContents[i];
        QStringList tokens;

        //emit gui->messageSig(LOG_INFO, tr("File contents CHECK: %1").arg(line));
        split(line,tokens);
        if (tokens.size() == 3 && line.contains(QLatin1String("Name:"), Qt::CaseInsensitive))
            fileName  = tokens[tokens.size()-1];

        //      if (tokens.size() == 15 && tokens[0] == "1") {
         if((tokens.size() == 15 && tokens[0] == "1") ||
            (tokens.size() == 8  && tokens[0] == "2") ||
            (tokens.size() == 11 && tokens[0] == "3") ||
            (tokens.size() == 14 && tokens[0] == "4") ||
            (tokens.size() == 14 && tokens[0] == "5")) {
            color = tokens[1];
            if (color == LDRAW_EDGE_MATERIAL_COLOUR || color == LDRAW_MAIN_MATERIAL_COLOUR) {
                continue;

            } else {
                hasColour = true;
//#ifdef QT_DEBUG_MODE
//                emit gui->messageSig(LOG_TRACE,tr("CONTENTS COLOUR LINE: %1 FILE: %2").arg(line).arg(libFileName));
//#endif
                if (fileName.isEmpty()) {
                    fileName = libFileName.split("/").last();
                    emit gui->messageSig(LOG_ERROR,tr("Part: %1 \nhas no 'Name:' attribute. Using library path name %2 instead.\n"
                                                      "You may want to update the part content and custom color parts list.")
                                                      .arg(fileName).arg(libFileName));
                }
                fileEntry = QString("%1:::%2:::%3").arg(fileName).arg(libType).arg(_partFileContents[0].remove(0,2));
                remove(libFileName);
                //emit gui->messageSig(LOG_TRACE,tr("Remove from list as it is a known color part: %1").arg(libFileName));
                break;
            }
        } // token size
    }
    _partFileContents.clear();
    if(hasColour) {
         if (libFilePath != _filePath) {
             fileSectionHeader(FADESTEP_COLOUR_PARTS_HEADER, QString("# Library path: %1").arg(libFilePath));
             _filePath = libFilePath;
         }
        _cpLines++;
        _ldrawStaticColourParts  << fileEntry.toLower();
        if (fileName.size() > _colWidthFileName)
            _colWidthFileName = fileName.size();
        //emit gui->messageSig(LOG_NOTICE,tr("ADD COLOUR PART: %1  libFileName: %2 Color: %3").arg(fileName).arg(libFileName).arg(color);
    }
}

void ColourPartListWorker::processChildren() {

    emit progressResetSig();
    emit progressMessageSig("Processing Child Color Parts...");
    emit progressRangeSig(1, _partList.size());
    emit gui->messageSig(LOG_INFO,tr("Processing Child Color Parts - Count: %1").arg(_partList.size()));

    QString     filePath = "";
    for(int part = 0; part < _partList.size() && endThreadNotRequested(); part++) {
        QString     parentFileNameStr;
        bool gotoMainLoop = false;

        emit progressSetValueSig(part);
        QMap<QString, ColourPart>::iterator ap = _colourParts.find(_partList[part]);

        if(ap != _colourParts.end()) {

             QString libFileName = ap.value()._fileNameStr;
             QString libFilePath    = libFileName.remove("/" + libFileName.split("/").last());

            // process ColourPart content
            for (int i = 0; i < ap.value()._contents.size() && ! gotoMainLoop && endThreadNotRequested(); i++) {
                QString line =  ap.value()._contents[i];
                QStringList tokens;

                split(line,tokens);
                if (tokens.size() == 3 && line.contains("Name:", Qt::CaseInsensitive))
                    parentFileNameStr = tokens[tokens.size()-1];                           // short name of parent part

                if (tokens.size() == 15 && tokens[0] == "1") {

                    QString childFileString = tokens[tokens.size()-1];                     // child part name in parent part
                    // check childFileName in _ldrawStaticColourParts list
                    for (int j = 0; j < _ldrawStaticColourParts.size() && ! gotoMainLoop && endThreadNotRequested(); ++j) {

                        if (_ldrawStaticColourParts.at(j).contains(childFileString) && _ldrawStaticColourParts.at(j).contains(QRegExp("\\b"+childFileString.replace("\\","\\\\")+"[^\n]*"))) {
                             if (libFilePath != filePath) {
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
                            //emit gui->messageSig(LOG_INFO,tr("ADD CHILD COLOUR PART: %1").arg(libFileName));
                            gotoMainLoop = true;
                        }
                    }
                }
            }
        }
    }
    emit progressSetValueSig(_partList.size());
    emit gui->messageSig(LOG_INFO,tr("Finished Processing Child Color Parts."));
}

void ColourPartListWorker::writeLDrawColourPartFile(bool append) {

    if (! _ldrawStaticColourParts.empty())
    {
        QFile file(Preferences::ldrawColourPartsFile);
        if ( ! file.open(append ? QFile::Append | QFile::Text : QFile::WriteOnly | QFile::Text)) {
            emit gui->messageSig(LOG_ERROR,tr("Failed to OPEN colour parts file %1 for writing:<br>%2").arg(file.fileName()).arg(file.errorString()));
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
                            << partDescription   << qSetFieldWidth(0) << lpub_endl;

            } else if (cpLine.section(":::",0,0) == QLatin1String("# File Name")) {
                out << left << qSetFieldWidth(_colWidthFileName+1)    << cpLine.section(":::",0,0)
                            << qSetFieldWidth(9)           << cpLine.section(":::",1,1)
                            << cpLine.section(":::",2,2)   << qSetFieldWidth(0) << lpub_endl;

            } else {
                out << cpLine << lpub_endl;
            }
        }
        file.close();

        if(!append) {
            gui->messageSig(LOG_INFO,tr("Lines written to %1: %2")
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
        const bool          &unOff) {

    bool partErased = false;

    QMap<QString, ColourPart>::iterator i = _colourParts.find(fileNameStr.toLower());

    if (i != _colourParts.end()) {
        _colourParts.erase(i);
        partErased = true;
//        qDebug() << "PART ALREADY IN LIST - PART ERASED" << i.value()._fileNameStr << ", UnOff Lib:" << i.value()._unOff;
    }
    ColourPart colourPartEntry(contents, fileNameStr, partType, unOff);
    _colourParts.insert(fileNameStr.toLower(), colourPartEntry);

    if (! partErased)
      _partList << fileNameStr.toLower();
//    gui->messageSig(LOG_INFO,tr("02 INSERT PART CONTENTS - UID: %1  partType: %2  unOff: %3").arg(fileNameStr.toLower()).arg(partType).arg(unOff));
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
//        gui->messageSig(LOG_NOTICE,tr("REMOVE COLOUR PART: %1 from contents and _partList").arg(fileNameStr.toLower()));
    }
}

void ColourPartListWorker::fileSectionHeader(const int &option, const QString &heading) {

    static const QString fmtDateTime("MM-dd-yyyy hh:mm:ss");
    QString const lib = Preferences::validLDrawLibrary;
    QString const unoffLib = Preferences::usingDefaultLibrary ? QString("Unofficial %1").arg(lib)  : QString("%1 Custom").arg(lib);
    QString const offLib = Preferences::usingDefaultLibrary ? QString("Official %1").arg(lib) : lib;

    switch(option)
    {
    case FADESTEP_INTRO_HEADER:
        _ldrawStaticColourParts  << QString("# File: %1  Generated on: %2")
                                    .arg(Preferences::validLDrawColorParts)
                                    .arg(QDateTime::currentDateTime().toString(fmtDateTime));
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << tr("# This space-delimited list captures the LDraw static color parts (and their subfiles) to support");
        _ldrawStaticColourParts  << tr("# step fade and step highlight. Parts on this list are identified in the LDraw library and copied to");
        _ldrawStaticColourParts  << tr("# their respective custom directory. Copied files are modified as described in the following");
        _ldrawStaticColourParts  << tr("# lines. If fade step is enabled, color codes are replaced with a custom code using the standard");
        _ldrawStaticColourParts  << tr("# color code prefixed with [" LPUB3D_COLOUR_FADE_PREFIX "].");
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << tr("# If using a single fade step color, color codes are replaced with main material color");
        _ldrawStaticColourParts  << tr("# code 16 using the fade color set in Preferences. If part highlight is enabled, edge");
        _ldrawStaticColourParts  << tr("# color values are replaced with the color value set in Preferences. If part highlight is");
        _ldrawStaticColourParts  << tr("# enabled, color codes are replaced with a custom code using the standard color code ");
        _ldrawStaticColourParts  << tr("# prefixed with [" LPUB3D_COLOUR_HIGHLIGHT_PREFIX "].");
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << tr("# When fade step is enabled, custom generated files are appended with '" FADE_SFX "',");
        _ldrawStaticColourParts  << tr("# for example, ...\\custom\\parts\\99499"  FADE_SFX ".dat");
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << tr("# When highlight step is enabled, custom generated files are appended with '" HIGHLIGHT_SFX "',");
        _ldrawStaticColourParts  << tr("# for example, ...\\custom\\parts\\99499" HIGHLIGHT_SFX ".dat");
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << tr("# Part identifiers with spaces will not be properly recognized.");
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << tr("# This file is automatically generated from:");
        _ldrawStaticColourParts  << tr("#    Configuration=>Generate Static Color Parts List");
        _ldrawStaticColourParts  << tr("# However, it can also be edited manually from:");
        _ldrawStaticColourParts  << tr("#    Configuration=>Edit Parameter Files=>Edit LDraw Static Color Parts List");
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << tr("# LPub3D will attempt to load the regular expression below first, if the");
        _ldrawStaticColourParts  << tr("# load fails, LPub3D will load the hard-coded (default) regular expression.");
        _ldrawStaticColourParts  << tr("# If you wish to modify the file import, you can edit this regular expression.");
        _ldrawStaticColourParts  << tr("# It would be wise to backup the default entry before performing an update - copy");
        _ldrawStaticColourParts  << tr("# and paste to a new line with starting phrase other than 'The Regular Expression...'");
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << tr("# The Regular Expression used is: ^(\\b.*[^\\s]\\b)(?:\\s)\\s+(u|o)\\s+(.*)$");
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << tr("# There are three defined columns in this file:");
        _ldrawStaticColourParts  << tr("# 1. File Name: The part file name as defined in the LDraw Library.");
        _ldrawStaticColourParts  << tr("# 2. Lib Type: Indicator 'U' = %1 Library and 'O'= %2 Library.").arg(unoffLib, offLib);
        _ldrawStaticColourParts  << tr("# 3. Description: The part file description taken from the first line of the part file.");
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << tr("# LDraw static color parts were generated from the following list of libraries and directories:");
        _ldrawStaticColourParts  << QString();
        break;
      case FADESTEP_FILE_HEADER:
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << QLatin1String("# File Name:::Lib Type:::Description");
        _ldrawStaticColourParts  << QString();
          break;
    case FADESTEP_COLOUR_PARTS_HEADER:
        _ldrawStaticColourParts  << QString();
          _ldrawStaticColourParts<< QLatin1String("# Static color parts");
        _ldrawStaticColourParts  << heading;
        _ldrawStaticColourParts  << QString();
        break;
    case FADESTEP_COLOUR_CHILDREN_PARTS_HEADER:
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << QLatin1String("# Parts with no static color element but has subparts or primitives with static color elements");
        _ldrawStaticColourParts  << heading;
        _ldrawStaticColourParts  << QString();
        break;
    case FADESTEP_FILE_STATUS:
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << QLatin1String("# ++++++++++++++++++++++++");
        _ldrawStaticColourParts  << QString();
        _ldrawStaticColourParts  << heading;
        _ldrawStaticColourParts  << QString();
        break;
    }
}

bool ColourPartListWorker::endThreadEventLoopNow() {
//    gui->messageSig(LOG_NOTICE,tr("endThreadEventLoopNow: %1").arg(_endThreadNowRequested));
    return _endThreadNowRequested;
}

void ColourPartListWorker::requestEndThreadNow() {
//    gui->messageSig(LOG_NOTICE,tr("requestEndThreadNow: %1").arg(_endThreadNowRequested));
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
        emit gui->messageSig(LOG_ERROR,tr(", Failed to extract archive %1").arg(mArchive));
    }
    emit finishedSig();
}

bool ExtractWorker::extractDir(QuaZip &zip, const QString &dir)
{
    if(!zip.open(QuaZip::mdUnzip)) {
        emit gui->messageSig(LOG_ERROR, tr("Could not open archive to add content. Return code %1.<br>"
                                           "Archive file %2 may be open in another program.")
                                            .arg(zip.getZipError()).arg(mArchive));
        return false;
    }

    QDir directory(dir);
    QStringList extracted;
    if (!zip.goToFirstFile()) {
        emit gui->messageSig(LOG_ERROR,tr("Failed to go to first archive file. Return Code %1")
                                            .arg(zip.getZipError()));
        return false;
    }

    int i = 0;
    do {
        QString name = zip.getCurrentFileName();
        QString absFilePath = directory.absoluteFilePath(name);
        if (!extractFile(&zip, "", absFilePath)) {
            removeFile(extracted);
            emit gui->messageSig(LOG_ERROR,tr("Failed extract archive file %1").arg(absFilePath));
            return false;
        }
        extracted.append(absFilePath);
        emit setValueSig(++i);
    } while (zip.goToNextFile() && ! mEndWorkNow);

    // Chiudo il file zip
    zip.close();
    if(zip.getZipError() != UNZ_OK) {
        removeFile(extracted);
        emit gui->messageSig(LOG_ERROR,tr("Failed to close archive %1. Return Code %2")
                                            .arg(mArchive).arg(zip.getZipError()));
        return false;
    }

    emit resultSig(extracted.count());

    return true;
}

void ExtractWorker::requestEndWorkNow() {
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

QMutex countMutex(QMutex::Recursive);

int CountPageWorker::countPage(
    Meta            *meta,
    LDrawFile       *ldrawFile,
    FindPageOptions  &opts,
    const QString    &addLine)
{
  QMutexLocker countLocker(&countMutex);

  Gui::pageProcessRunning = PROC_COUNT_PAGE;

  opts.flags.countInstances = meta->LPub.countInstance.value();

  //* local displayPageNum used to set breakpoint condition (e.g. displayPageNum > 7)
#ifdef QT_DEBUG_MODE
  int displayPageNum = Gui::displayPageNum;
  Q_UNUSED(displayPageNum)
#endif
  //*/

  Where topOfStep = opts.current;
  if (opts.flags.countPageContains) {
      gui->skipHeader(opts.current);
      topOfStep = opts.current;
      opts.flags.countPageContains = false;
  } else if (Preferences::buildModEnabled) {
      // set the topOfStep, lineNumber from findPage is automatically incremented
      ldrawFile->getTopOfStep(topOfStep.modelName, topOfStep.modelIndex, topOfStep.lineNumber);
  }
  Where topOfSteps = topOfStep;

  if (opts.pageNum == 1 + Gui::pa && opts.current.modelName == ldrawFile->topLevelFile()) {
      if (!opts.stepNumber)
          opts.stepNumber = 1 + Gui::sa;
#ifdef QT_DEBUG_MODE
      emit gui->messageSig(LOG_NOTICE, QString("COUNTPAGE - Page 000 topOfPage First Page         (opt) - LineNumber %2, ModelName %3")
                                               .arg(opts.current.lineNumber, 3, 10, QChar('0')).arg(opts.current.modelName));
#endif
      Gui::topOfPages.clear();
      Gui::topOfPages.append(opts.current);
  }

  opts.flags.numLines = ldrawFile->size(opts.current.modelName);

  ldrawFile->setRendered(opts.current.modelName,
                         opts.renderModelColour,
                         opts.renderParentModel,
                         opts.isMirrored,
                         opts.stepNumber,
                         opts.flags.countInstances,
                         true/*countPage*/);

  Rc rc;
  QStringList bfxParts;

  BuildModFlags           buildMod = opts.flags.buildMod;
  QMap<int, QString>      buildModKeys;
  QMap<int, QVector<int>> buildModAttributes;

  bool buildModExists    = false;
  int  buildModStepIndex = BM_INVALID_INDEX;

  auto insertAttribute =
          [&buildMod,
           &topOfStep] (
          QMap<int, QVector<int>> &buildModAttributes,
          int index, const Where &here)
  {
      QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildMod.level);
      if (i == buildModAttributes.end()) {
          QVector<int> modAttributes = { 0, 0, 0, 1, 0, topOfStep.modelIndex, 0, 0 };
          modAttributes[index] = here.lineNumber;
          buildModAttributes.insert(buildMod.level, modAttributes);
      } else {
          i.value()[index] = here.lineNumber;
      }
  };

  auto insertBuildModification =
         [&opts,
          &ldrawFile,
          &buildModAttributes,
          &buildModKeys,
          &topOfStep] (int buildModLevel)
  {
      int buildModStepIndex = ldrawFile->getBuildModStepIndex(topOfStep.modelIndex, topOfStep.lineNumber);
      QString buildModKey = buildModKeys.value(buildModLevel);
      QVector<int> modAttributes = { 0, 0, 0, 0, 0, -1, 0, 0 };

      QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
      if (i == buildModAttributes.end()) {
          emit gui->messageSig(LOG_ERROR, QString("Invalid BuildMod Entry for key: %1").arg(buildModKey));
          return;
      }
      modAttributes = i.value();

      modAttributes[BM_DISPLAY_PAGE_NUM] = gui->saveDisplayPageNum;
      modAttributes[BM_STEP_PIECES]      = opts.flags.partsAdded;
      modAttributes[BM_MODEL_NAME_INDEX] = topOfStep.modelIndex;
      modAttributes[BM_MODEL_LINE_NUM]   = topOfStep.lineNumber;
      modAttributes[BM_MODEL_STEP_NUM]   = opts.stepNumber;

#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_TRACE, QString(
                  "Insert CountPage BuildMod StepIndex: %1, "
                  "Action: Apply(64), "
                  "Attributes: %2 %3 %4 %5 %6 %7 %8 %9, "
                  "ModKey: %10, "
                  "Level: %11, "
                  "Model: %12")
                  .arg(buildModStepIndex)                      // Attribute Default Initial:
                  .arg(modAttributes.at(BM_BEGIN_LINE_NUM))    // 0         0       this
                  .arg(modAttributes.at(BM_ACTION_LINE_NUM))   // 1         0       this
                  .arg(modAttributes.at(BM_END_LINE_NUM))      // 2         0       this
                  .arg(modAttributes.at(BM_DISPLAY_PAGE_NUM))  // 3         0       this
                  .arg(modAttributes.at(BM_STEP_PIECES))       // 4         0       this
                  .arg(modAttributes.at(BM_MODEL_NAME_INDEX))  // 5        -1       this
                  .arg(modAttributes.at(BM_MODEL_LINE_NUM))    // 6         0       this
                  .arg(modAttributes.at(BM_MODEL_STEP_NUM))    // 7         0       this
                  .arg(buildModKey)
                  .arg(buildModLevel)
                  .arg(topOfStep.modelName));
#endif

      ldrawFile->insertBuildMod(buildModKey,
                                modAttributes,
                                buildModStepIndex);
  };

//#ifndef QT_DEBUG_MODE
//  auto documentPageCount = [&] ()
//  {
//      emit gui->messageSig(LOG_INFO_STATUS, tr("Counting document page %1...")
//                           .arg(QStringLiteral("%1").arg(opts.pageNum - 1, 4, 10, QLatin1Char('0'))));
//  };
//#endif

  for ( ;
        opts.current.lineNumber < opts.flags.numLines && ! Gui::abortProcess();
        opts.current.lineNumber++) {

//* local optsPageNum used to set breakpoint condition (e.g. optsPageNum > 7)
#ifdef QT_DEBUG_MODE
      int optsPageNum = opts.pageNum;
      int saveDisplayPageNum = gui->saveDisplayPageNum;
      Q_UNUSED(optsPageNum)
      Q_UNUSED(saveDisplayPageNum)
#endif
//*/

      // if reading include file, return to current line, do not advance

      if (static_cast<Rc>(opts.flags.includeFileRc) != EndOfIncludeFileRc && opts.flags.includeFileFound) {
         opts.current.lineNumber--;
      }

      // scan through the model counting pages. do as little as possible

      QString line = ldrawFile->readLine(opts.current.modelName,opts.current.lineNumber).trimmed();

      if (line.startsWith("0 GHOST ")) {
          line = line.mid(8).trimmed();
      }

      switch (line.toLatin1()[0]) {
      case '1':
          // process type 1 line...
          if (! opts.flags.partIgnore) {

              if (Gui::firstStepPageNum == -1) {
                  Gui::firstStepPageNum = opts.pageNum;
              }
              Gui::lastStepPageNum = opts.pageNum;

              QStringList tokens;

              split(line,tokens);

              if (tokens.size() == 15) {

                  QString type = tokens[tokens.size()-1];
                  QString colorType = tokens[1]+type;

                  bool contains = ldrawFile->isSubmodel(type);

                  // if submodel
                  if (contains) {

                      // check if submodel is in current step build modification
                      bool buildModRendered = false;

                      // if not callout or assembled/rotated callout or parseBuildMods, process the submodel
                      // note that we accept callouts if parseBuildMods is true to parse any specified build mods

                      bool validCallout = opts.flags.callout && meta->LPub.callout.begin.value() != CalloutBeginMeta::Unassembled;

                      bool validSubmodel = (!opts.displayModel && (!opts.flags.callout || opts.flags.parseBuildMods)) || (validCallout) || (opts.displayModel && validCallout);

                      if (validSubmodel) {

                          // inherit colour number if material colour
                          if (tokens[1] == LDRAW_MAIN_MATERIAL_COLOUR) {
                              if (!addLine.isEmpty()) {
                                  QStringList addTokens;
                                  split(addLine, addTokens);
                                  if (addTokens.size() == 15)
                                      opts.renderModelColour = addTokens[1];
                              }
                          } else {
                              opts.renderModelColour = tokens[1];
                          }

                          // check if submodel was rendered
                          bool rendered = ldrawFile->rendered(type,
                                                              opts.renderModelColour,
                                                              opts.current.modelName,
                                                              ldrawFile->mirrored(tokens),
                                                              opts.stepNumber,
                                                              opts.flags.countInstances,
                                                              true /*countPage*/);

                          buildModRendered = Preferences::buildModEnabled && (buildMod.ignore ||
                                                                              ldrawFile->getBuildModRendered(buildMod.key, colorType, true/*countPage*/));

                          // if the submodel was not rendered, and (is not in the buffer exchange call setRendered for the submodel.
                          if (! rendered && ! buildModRendered && (! opts.flags.bfxStore2 || ! bfxParts.contains(colorType))) {

                              if (! buildMod.ignore || ! buildModRendered) {

                                  opts.isMirrored = ldrawFile->mirrored(tokens);

                                  // add submodel to the model modelStack - it can also be a callout
                                  SubmodelStack tos(opts.current.modelName,opts.current.lineNumber,opts.stepNumber);
                                  meta->submodelStack << tos;
                                  Where current2(type,ldrawFile->getSubmodelIndex(type),0);

                                  // pass contains to countPageContains, buildMod, parseBuildMods and callout - if parseBuildMods
                                  FindPageFlags flags2;    //= opts.flags; //passing all flags crashes main model formatting remove
                                  flags2.countPageContains = contains;
                                  flags2.buildMod          = buildMod;
                                  if (opts.flags.parseBuildMods) {
                                      // as we are processing callouts we must pass their flag
                                      // so we do not set their step or end model as a new page
                                      flags2.callout        = opts.flags.callout;
                                      flags2.parseBuildMods = opts.flags.parseBuildMods;
                                      const QString levelKey = QString("CountPages BuildMod Key: %1, ParentModel: %2, LineNumber: %3")
                                                                       .arg(buildMod.key)
                                                                       .arg(opts.current.modelName)
                                                                       .arg(opts.current.lineNumber);
                                      flags2.buildModLevel  = buildMod.state == BM_BEGIN ? getLevel(levelKey, BM_CURRENT) : opts.flags.buildModLevel;
                                  }

                                  ldrawFile->setModelStartPageNumber(current2.modelName,opts.pageNum);

                                  // save rotStep, clear it, and restore it afterwards
                                  // since rotsteps don't affect submodels
                                  RotStepMeta saveRotStep2 = meta->rotStep;
                                  meta->rotStep.clear();

                                  // save Default pageSize information
                                  PageSizeData pageSize2;
                                  if (Gui::exporting()) {
                                      pageSize2       = Gui::getPageSize(DEF_SIZE);
                                      opts.flags.pageSizeUpdate  = false;
#ifdef PAGE_SIZE_DEBUG
                                      logDebug() << "SM: Saving    Default Page size info at PageNumber:" << opts.pageNum
                                                 << "W:"    << pageSize2.sizeW << "H:"    << pageSize2.sizeH
                                                 << "O:"    <<(pageSize2.orientation == Portrait ? "Portrait" : "Landscape")
                                                 << "ID:"   << pageSize2.sizeID
                                                 << "Model:" << opts.current.modelName;
#endif
                                  }

                                  // set the step number and parent model where the submodel will be rendered
                                  FindPageOptions modelOpts(
                                              opts.pageNum,
                                              current2,
                                              opts.pageSize,
                                              flags2,
                                              opts.modelStack,
                                              opts.pageDisplayed,
                                              opts.displayModel,
                                              opts.updateViewer,
                                              opts.isMirrored,
                                              opts.printing,
                                              opts.stepNumber,
                                              opts.contStepNumber,
                                              opts.groupStepNumber,
                                              opts.renderModelColour,
                                              opts.current.modelName /*renderParentModel*/);

                                  const TraverseRc drc = static_cast<TraverseRc>(countPage(meta, ldrawFile, modelOpts, line));
                                  if (drc == HitAbortProcess)
                                      return static_cast<int>(drc);

                                  meta->rotStep = saveRotStep2;             // restore old rotstep
                                  if (meta->submodelStack.size())
                                      meta->submodelStack.pop_back();       // remove where we stopped in the parent model

                                  // terminate build modification countPage at end of submodel
                                  if (Gui::buildModJumpForward && ! opts.flags.callout && modelOpts.pageNum >= gui->saveDisplayPageNum) {
                                      opts.flags.parseBuildMods = modelOpts.flags.parseBuildMods;
                                      if (! opts.flags.parseBuildMods)
                                          return static_cast<int>(drc);
                                  }

                                  if (Gui::exporting()) {
                                      Gui::removePageSize(DEF_SIZE);
                                      Gui::insertPageSize(DEF_SIZE, pageSize2); // restore old Default pageSize information
#ifdef PAGE_SIZE_DEBUG
                                      logDebug() << "SM: Restoring Default Page size info at PageNumber:" << opts.pageNum
                                                 << "W:"    << Gui::getPageSize(DEF_SIZE).sizeW << "H:"    << Gui::getPageSize(DEF_SIZE).sizeH
                                                 << "O:"    << (Gui::getPageSize(DEF_SIZE).orientation == Portrait ? "Portrait" : "Landscape")
                                                 << "ID:"   << Gui::getPageSize(DEF_SIZE).sizeID
                                                 << "Model:" << opts.current.modelName;
#endif
                                  } // Exporting

                              } // ! BuildModIgnore

                          } // ! Rendered && (! BfxStore2 || ! BfxParts.contains(colorType))

                      } // ! Callout || (Callout && CalloutMode != CalloutBeginMeta::Unassembled)

                      // add submodel to buildMod rendered list
                      if (Preferences::buildModEnabled &&
                          buildMod.state == BM_BEGIN   &&
                          ! buildModRendered) {
                          ldrawFile->setBuildModRendered(QString("%1;%2").arg(COUNT_PAGE_PREFIX).arg(buildMod.key), colorType);
                      }

                  } // Contains [IsSubmodel]

                  if (opts.flags.bfxStore1) {
                      bfxParts << colorType;
                  }

              } // Type 1 Line

          } // ! PartIgnore
        case '2':
        case '3':
        case '4':
        case '5':
          // if opts.displayModel, we have a custom display
          if (! opts.displayModel && ! buildMod.ignore) {
              ++opts.flags.partsAdded;
            } // ! BuildModIgnore, for each line
            break;

        case '0':

          // intercept include file flag

          if (static_cast<Rc>(opts.flags.includeFileRc) != EndOfIncludeFileRc) {
              if (opts.flags.resetIncludeRc) {
                  rc = IncludeRc;                    // return to IncludeRc to parse another line
              } else {
                  rc = static_cast<Rc>(opts.flags.includeFileRc); // execute the Rc returned by include(...)
                  opts.flags.resetIncludeRc = true;  // reset to run include(...) to parse another line
              }
          } else {
              rc = meta->parse(line,opts.current);    // continue
          }

          switch (rc) {
            case StepGroupBeginRc:
              opts.flags.stepGroup = true;
              topOfSteps = topOfStep;

              // Steps within step group modify bfxStore2 as they progress
              // so we must save bfxStore2 and use the saved copy when
              // we call drawPage for a step group.
              opts.flags.stepGroupBfxStore2 = opts.flags.bfxStore2;
              break;

            case StepGroupEndRc:
              if (opts.flags.stepGroup && ! opts.flags.noStep2) {
                  opts.flags.stepGroup = false;

                  // terminate parse build modifications
                  if (opts.flags.parseBuildMods) {
                      // terminate parse build mods at end of display page when called from gui::countPage for jump to page
                      if (Gui::buildModJumpForward && ! opts.flags.callout) {
                          // we will be at the bottom of the current (display) page as pageNum is advanced at the end of the page step,
                          // so set pageNum + 1 to use the correct page number to determine when to terminate the buildMod parse.
                          // Use <= comparison between (pageNum + 1 and displayPageNum) to parse buildMod up to display page.
                          opts.flags.parseBuildMods = ((opts.pageNum + 1) <= gui->saveDisplayPageNum);
                      }
                      // terminate parse build modification for steps after first step in step group
                      else if (opts.flags.parseStepGroupBM) {
                          opts.flags.parseStepGroupBM = opts.flags.parseBuildMods = false;
                      }
                  }

                  // ignored when processing buildMod display
                  if (Gui::exporting()) {
                      Gui::getPageSizes().remove(opts.pageNum);
                      if (opts.flags.pageSizeUpdate) {
                          opts.flags.pageSizeUpdate = false;
                          Gui::insertPageSize(opts.pageNum,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                          logTrace() << "SG: Inserting New Page size info     at PageNumber:" << opts.pageNum
                                     << "W:"    << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                                     << "O:"    <<(opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                                     << "ID:"   << opts.pageSize.sizeID
                                     << "Model:" << opts.current.modelName;
#endif
                      } else {
                          Gui::insertPageSize(opts.pageNum,Gui::getPageSize(DEF_SIZE));
#ifdef PAGE_SIZE_DEBUG
                          logTrace() << "SG: Inserting Default Page size info at PageNumber:" << opts.pageNum
                                     << "W:"    << Gui::getPageSize(DEF_SIZE).sizeW << "H:"    << Gui::getPageSize(DEF_SIZE).sizeH
                                     << "O:"    << (Gui::getPageSize(DEF_SIZE).orientation == Portrait ? "Portrait" : "Landscape")
                                     << "ID:"   << Gui::getPageSize(DEF_SIZE).sizeID
                                     << "Model:" << opts.current.modelName;
#endif
                      }
                  } // Exporting

#ifdef QT_DEBUG_MODE
                  emit gui->messageSig(LOG_NOTICE, QString("COUNTPAGE - Page %1 topOfPage StepGroup End      (tos) - LineNumber %2, ModelName %3")
                                       .arg(opts.pageNum, 3, 10, QChar('0')).arg(topOfStep.lineNumber, 3, 10, QChar('0')).arg(topOfStep.modelName));
#endif
                  ++opts.pageNum;
                  Gui::topOfPages.append(topOfStep/*opts.current*/);  // TopOfSteps(Page) (Next StepGroup), BottomOfSteps(Page) (Current StepGroup)
//#ifndef QT_DEBUG_MODE
//                  if (Preferences::debugLogging)
//                      documentPageCount();
//#endif
                } // StepGroup && ! NoStep2
              opts.flags.noStep2 = false;

              // terminate build modification countPage at end of step group
              if (Gui::buildModJumpForward && ! opts.flags.callout && opts.pageNum > gui->saveDisplayPageNum) {
                  if (! opts.flags.parseBuildMods)
                      opts.flags.numLines = opts.current.lineNumber;
              }
              break;

            case BuildModApplyRc:
            case BuildModRemoveRc:
              if (Preferences::buildModEnabled) {
                  Where current = opts.current;
                  if (lpub->mi.scanForwardNoParts(current, StepMask|StepGroupMask) == StepGroupEndRc)
                      gui->parseErrorSig(QString("BUILD_MOD %1 '%2' must be placed after MULTI_STEP END")
                                                 .arg(rc == BuildModRemoveRc ? QString("REMOVE") : QString("APPLY"))
                                                 .arg(meta->LPub.buildMod.key()), opts.current,Preferences::ParseErrors,false,false);
                  // special case where we have BUILD_MOD and NOSTEP commands in the same single STEP
                  if (! opts.flags.parseNoStep && ! opts.pageDisplayed && ! opts.flags.stepGroup && opts.flags.noStep)
                      opts.flags.parseNoStep = meta->LPub.parseNoStep.value();
                  // Parse buildMod actions to display page when jumping forward, excluding callouts and
                  // step-groups, and parse lines after MULTI_STEP END in the last STEP of a submodel.
                  // Do not parse callouts when jumping ahead because the current step will be assigned any
                  // action change which will cause callouts to display the 'final' action for the page when
                  // the intention would be to have the drawPage iterate the callout's STEPS to render each
                  // STEP's action accordingly. Build modification actions in callouts and step-groups are
                  // parsed in drawPage. Likewise build modification actions are not parsed during count
                  // page processing step-groups STEPs after the first STEP.
                  if (opts.flags.parseBuildMods && ! opts.flags.parseStepGroupBM && ! opts.flags.callout) {
                      if (opts.flags.partsAdded)
                          emit gui->parseErrorSig(QString("BUILD_MOD REMOVE/APPLY action command must be placed before step parts"),
                                                  opts.current,Preferences::BuildModErrors,false,false);
                      buildModStepIndex = ldrawFile->getBuildModStepIndex(topOfStep.modelIndex, topOfStep.lineNumber, true/*index check*/);
                      // if we are processing lines in the last step of a submodel, the topOfStep.lineNumber
                      // likely will not reflect a valid value so we'll have to manually set it accordingly...
                      if (buildModStepIndex == BM_INVALID_INDEX) {
                          ldrawFile->getTopOfStep(topOfStep.modelName, topOfStep.modelIndex, topOfStep.lineNumber);
                          buildModStepIndex = ldrawFile->getBuildModStepIndex(topOfStep.modelIndex, topOfStep.lineNumber);
                      }
                      buildMod.key = meta->LPub.buildMod.key();
                      if (ldrawFile->buildModContains(buildMod.key)) {
                          buildMod.action = ldrawFile->getBuildModAction(buildMod.key, buildModStepIndex);
                          if (ldrawFile->getBuildModActionPrevIndex(buildMod.key, buildModStepIndex, rc) < buildModStepIndex)
                              emit gui->parseErrorSig(tr("Redundant build modification meta command '%1' - this command can be removed.")
                                                      .arg(buildMod.key), opts.current,Preferences::BuildModErrors,false,false,QMessageBox::Icon::Information);
                      } else {
                          const QString action = rc == BuildModApplyRc ? tr("Apply") : tr("Remove");
                          emit gui->parseErrorSig(tr("Jump forward BuildMod key '%1' for %2 action was not found.")
                                                  .arg(buildMod.key).arg(action),
                                                  opts.current,Preferences::BuildModErrors,false,false);
                      }
                      if ((Rc)buildMod.action != rc) {
#ifdef QT_DEBUG_MODE 
                      const QString message = tr("Jump forward Build Mod Reset Setup - Key: '%1', Current Action: %2, Next Action: %3") 
                                                 .arg(buildMod.key) 
                                                 .arg(buildMod.action == BuildModRemoveRc ? "Remove(65)" : "Apply(64)") 
                                                 .arg(rc == BuildModRemoveRc ? "Remove(65)" : "Apply(64)"); 
                      emit gui->messageSig(LOG_NOTICE, message); 
                      //qDebug() << qPrintable(QString("DEBUG: %1").arg(message)); 
#endif 
                          // get the viewerStepKey for the current step
                          const QString viewerStepKey = QString("%1;%2;%3%4")
                                                        .arg(topOfStep.modelIndex)
                                                        .arg(topOfStep.lineNumber)
                                                        .arg(opts.stepNumber)
                                                        .arg(lpub->mi.viewerStepKeySuffix(topOfStep, nullptr, true/*step check*/));

                          // set BuildMod action for step if exists
                          if (lpub->ldrawFile.setViewerStepHasBuildModAction(viewerStepKey, true))
                              ldrawFile->setBuildModAction(buildMod.key, buildModStepIndex, rc); 
 
                      }
                      buildMod.state = BM_NONE;
                  } // opts.flags.parseBuildMods && ! opts.flags.parseStepGroupBM
              } // Preferences::buildModEnabled
              break;

            case BuildModBeginRc:
              if (! Preferences::buildModEnabled) {
                  buildMod.ignore = true;
                  break;
              }
              if (opts.displayModel) {
                  emit gui->parseErrorSig(tr("Build modifications are not supported in display model Step"),
                                             opts.current,Preferences::BuildModErrors,false,false);
                  buildMod.ignore = true;
                  break;
              }
              if (!buildMod.countPage && buildMod.state == BM_BEGIN) {
                  QString const message = tr("BUILD_MOD BEGIN '%1' encountered but '%2' was already defined in this STEP.<br><br>"
                                             "Multiple build modifications per STEP are not allowed.")
                                             .arg(meta->LPub.buildMod.key()).arg(buildMod.key);
                  emit gui->parseErrorSig(message,opts.current,Preferences::BuildModErrors,false,false);
                  buildMod.ignore = true;
                  break;
              }
              buildMod.key = meta->LPub.buildMod.key();
              buildMod.level = getLevel(buildMod.key, BM_BEGIN);
              buildMod.action = BuildModApplyRc;
              buildMod.ignore = false;
              // parse build modifications
              if (opts.flags.parseBuildMods) {
                  buildModExists = ldrawFile->buildModContains(buildMod.key);
                  if (! buildModExists && ! buildModKeys.contains(buildMod.level)) {
                    buildModKeys.insert(buildMod.level, buildMod.key);
                    insertAttribute(buildModAttributes, BM_BEGIN_LINE_NUM, opts.current);
                  }
              }
              buildMod.state = BM_BEGIN;
              break;

            case BuildModEndModRc:
              if (! Preferences::buildModEnabled) {
                  buildMod.ignore = getLevel(QString(), BM_END);
                  break;
              }
              if (buildMod.state == BM_BEGIN)
                  if (buildMod.action == BuildModApplyRc)
                      buildMod.ignore = true;
              // parse build modifications
              if (opts.flags.parseBuildMods) {
                  if (buildMod.level > 1 && buildMod.key.isEmpty())
                      emit gui->parseErrorSig("Key required for nested build mod meta command",
                                              opts.current,Preferences::BuildModErrors,false,false);
                  if (buildMod.state != BM_BEGIN)
                      emit gui->parseErrorSig(QString("Required meta BUILD_MOD BEGIN not found"),
                                              opts.current, Preferences::BuildModErrors,false,false);
                  insertAttribute(buildModAttributes, BM_ACTION_LINE_NUM, opts.current);
              }
              buildMod.state = BM_END_MOD;
              break;

            case BuildModEndRc:
              if (! Preferences::buildModEnabled)
                  break;
              // parse build modifications
              if (opts.flags.parseBuildMods) {
                  if (buildMod.state != BM_END_MOD)
                      emit gui->parseErrorSig(QString("Required meta BUILD_MOD END_MOD not found"),
                                              opts.current, Preferences::BuildModErrors,false,false);
                  insertAttribute(buildModAttributes, BM_END_LINE_NUM, opts.current);
              }
              if (buildMod.state == BM_END_MOD) {
                  buildMod.level = getLevel(QString(), BM_END);
                  if (buildMod.level == opts.flags.buildModLevel)
                      buildMod.ignore = false;
              }
              buildMod.state = BM_END;
              break;

            case RotStepRc:
            case StepRc:
              if (opts.flags.partsAdded && ! opts.flags.noStep) {
                  // parse build modifications
                  if (opts.flags.parseBuildMods) {
                      // terminate parse build mods at end of display page when called from gui::countPage for jump to page
                      if (Gui::buildModJumpForward && ! opts.flags.callout && ! opts.flags.stepGroup) {
                          // we will be at the bottom of the current (display) page as pageNum is advanced at the end of the page step,
                          // so set pageNum + 1 to use the correct page number to determine when to terminate the buildMod parse.
                          // Use <= comparison between (pageNum + 1 and displayPageNum) to parse buildMod up to display page.
                          opts.flags.parseBuildMods = ((opts.pageNum + 1) <= gui->saveDisplayPageNum);
                      }
                      // BuildMod create
                      if (buildModKeys.size()) {
                          if (buildMod.state != BM_END)
                              emit gui->parseErrorSig(QString("Required meta BUILD_MOD END not found"),
                                                      opts.current, Preferences::BuildModErrors,false,false);
                          Q_FOREACH (int buildModLevel, buildModKeys.keys())
                              insertBuildModification(buildModLevel);
                      }
                      buildModKeys.clear();
                      buildModAttributes.clear();
                  }

                  opts.stepNumber  += ! opts.flags.coverPage && ! opts.flags.stepPage;

                  // Added callout step parse for parse build modifications so
                  // exclude from page number increment and topOfPages indices
                  if ( ! opts.flags.stepGroup && ! opts.flags.callout) {
                      if (Gui::exporting()) {
                          Gui::getPageSizes().remove(opts.pageNum);
                          if (opts.flags.pageSizeUpdate) {
                              opts.flags.pageSizeUpdate = false;
                              Gui::insertPageSize(opts.pageNum,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                              logTrace() << "ST: Inserting New Page size info     at PageNumber:" << opts.pageNum
                                         << "W:"    << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                                         << "O:"    <<(opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                                         << "ID:"   << opts.pageSize.sizeID
                                         << "Model:" << opts.current.modelName;
#endif
                            } else {
                              Gui::insertPageSize(opts.pageNum,Gui::getPageSize(DEF_SIZE));
#ifdef PAGE_SIZE_DEBUG
                              logTrace() << "ST: Inserting Default Page size info at PageNumber:" << opts.pageNum
                                         << "W:"    << Gui::getPageSize(DEF_SIZE).sizeW << "H:"    << Gui::getPageSize(DEF_SIZE).sizeH
                                         << "O:"    << (Gui::getPageSize(DEF_SIZE).orientation == Portrait ? "Portrait" : "Landscape")
                                         << "ID:"   << Gui::getPageSize(DEF_SIZE).sizeID
                                         << "Model:" << opts.current.modelName;
#endif
                          }
                      } // Exporting

#ifdef QT_DEBUG_MODE
                      emit gui->messageSig(LOG_NOTICE, QString("COUNTPAGE - Page %1 topOfPage Step, Not Group    (opt) - LineNumber %2, ModelName %3")
                                           .arg(opts.pageNum, 3, 10, QChar('0')).arg(opts.current.lineNumber, 3, 10, QChar('0')).arg(opts.current.modelName));
#endif
                      ++opts.pageNum;
                      Gui::topOfPages.append(opts.current); // Set TopOfStep (Step)
//#ifndef QT_DEBUG_MODE
//                      if (Preferences::debugLogging)
//                          documentPageCount();
//#endif

                  } // ! StepGroup and ! Callout (Single step)

                  topOfStep = opts.current;
                  opts.flags.partsAdded = 0;
                  opts.flags.coverPage = false;
                  opts.flags.stepPage = false;
                  opts.flags.bfxStore2 = opts.flags.bfxStore1;
                  opts.flags.bfxStore1 = false;
                  if ( ! opts.flags.bfxStore2) {
                        bfxParts.clear();
                  } // ! BfxStore2
                  if ( ! buildMod.ignore) {
                      ldrawFile->clearBuildModRendered(true/*countPage*/);
                  } // ! BuildMod.ignore
                  if (! opts.flags.parseBuildMods && opts.flags.stepGroup && topOfSteps != topOfStep) {
                      if (opts.pageNum == Gui::displayPageNum + 1) {
                          opts.flags.parseStepGroupBM = opts.flags.parseBuildMods = true;
                      } // we only care about the next page during each page count run
                  } // enable parse build modifications, except actions, for steps after first step in step group
                  meta->pop();
               } // PartsAdded && ! NoStep

              buildMod.clear();
              meta->LPub.buildMod.clear();
              opts.flags.noStep2 = opts.flags.noStep;
              opts.flags.noStep = false;
              opts.flags.parseNoStep = false;
              opts.displayModel = false;

              // terminate build modification countPage at end of step
              if (Gui::buildModJumpForward && ! opts.flags.callout && opts.pageNum > gui->saveDisplayPageNum) {
                  if (! opts.flags.parseBuildMods && ! opts.flags.stepGroup)
                      opts.flags.numLines = opts.current.lineNumber;
              }
              break;

            case CalloutBeginRc:
              opts.flags.callout = true;
              break;

            case CalloutEndRc:
              opts.flags.callout = false;
              meta->LPub.callout.placement.clear();
              break;

            case InsertCoverPageRc:
              opts.flags.coverPage  = true;
              opts.flags.partsAdded = true;
              break;

            case InsertPageRc:
              opts.flags.stepPage   = true;
              opts.flags.partsAdded = true;
              Gui::lastStepPageNum  = opts.pageNum;
              break;

            case InsertFinalModelRc:
            case InsertDisplayModelRc:
              Gui::lastStepPageNum = opts.pageNum;
              if (rc == InsertDisplayModelRc)
                  opts.displayModel = true;
              break;

            case PartBeginIgnRc:
              opts.flags.partIgnore = true;
              break;

            case PartEndRc:
              opts.flags.partIgnore = false;
              break;

              // Any of the metas that can change csiParts needs
              // to be processed here

              /* Buffer exchange */
            case BufferStoreRc:
              opts.flags.bfxStore1 = true;
              bfxParts.clear();
              break;

            case BufferLoadRc:
              // special case where we have BUFEXCHG load and NOSTEP commands in the same single STEP
              if (! opts.flags.parseNoStep && ! opts.pageDisplayed && ! opts.flags.stepGroup && opts.flags.noStep)
                  opts.flags.parseNoStep = meta->LPub.parseNoStep.value();
              opts.flags.partsAdded = true;
              break;

            case PartNameRc:
            case PartTypeRc:
            case MLCadGroupRc:
            case LDCadGroupRc:
            case LeoCadModelRc:
            case LeoCadPieceRc:
            case LeoCadCameraRc:
            case LeoCadLightRc:
            case LeoCadLightTypeRc:
            case LeoCadLightPOVRayRc:
            case LeoCadLightShadowless:
            case LeoCadSynthRc:
            case LeoCadGroupBeginRc:
            case LeoCadGroupEndRc:
               opts.flags.partsAdded = true;
               break;

            /*
            case IncludeRc:
              opts.flags.includeFileRc = gui->includePub(*meta,opts.flags.includeLineNum,opts.flags.includeFileFound); // includeHere and inserted are include(...) vars
              if (opts.flags.includeFileRc == static_cast<int>(IncludeFileErrorRc)) {
                  opts.flags.includeFileRc = static_cast<int>(EndOfIncludeFileRc);
                  emit gui->parseErrorSig(tr("INCLUDE file was not resolved."),
                                          opts.current,Preferences::IncludeFileErrors,false,false);  // file parse error
              } else if (static_cast<Rc>(opts.flags.includeFileRc) != EndOfIncludeFileRc) {  // still reading so continue
                  opts.flags.resetIncludeRc = false;                                         // do not reset, allow includeFileRc to execute
                  continue;
              }
              break;
            */

            case PageSizeRc:
              {
                if (Gui::exporting()) {
                    opts.flags.pageSizeUpdate  = true;

                    opts.pageSize.sizeW  = meta->LPub.page.size.valueInches(0);
                    opts.pageSize.sizeH  = meta->LPub.page.size.valueInches(1);
                    opts.pageSize.sizeID = meta->LPub.page.size.valueSizeID();

                    if (meta->LPub.page.size.valueOrientation() != InvalidOrientation) {
                      opts.pageSize.orientation = meta->LPub.page.size.valueOrientation();
                      meta->LPub.page.orientation.setValue(opts.pageSize.orientation);
                    }

                    Gui::removePageSize(DEF_SIZE);
                    Gui::insertPageSize(DEF_SIZE,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                    logTrace() << "1. New Page Size entry for Default  at PageNumber:" << opts.pageNum
                               << "W:"  << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                               << "O:"  << (opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                               << "ID:" << opts.pageSize.sizeID
                               << "Model:" << opts.current.modelName;
#endif
                  }
              }
              break;

            case CountInstanceRc:
              opts.flags.countInstances = meta->LPub.countInstance.value();
              break;

            case PageOrientationRc:
              {
                if (Gui::exporting()) {
                    opts.flags.pageSizeUpdate      = true;

                    if (opts.pageSize.sizeW == 0.0f)
                      opts.pageSize.sizeW    = Gui::getPageSize(DEF_SIZE).sizeW;
                    if (opts.pageSize.sizeH == 0.0f)
                      opts.pageSize.sizeH    = Gui::getPageSize(DEF_SIZE).sizeH;
                    if (opts.pageSize.sizeID.isEmpty())
                      opts.pageSize.sizeID   = Gui::getPageSize(DEF_SIZE).sizeID;
                    opts.pageSize.orientation= meta->LPub.page.orientation.value();

                    Gui::removePageSize(DEF_SIZE);
                    Gui::insertPageSize(DEF_SIZE,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                    logTrace() << "1. New Orientation entry for Default at PageNumber:" << opts.pageNum
                               << "W:"  << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                               << "O:"  << (opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                               << "ID:" << opts.pageSize.sizeID
                               << "Model:" << opts.current.modelName;
#endif
                  }
              }
              break;

            case NoStepRc:
              opts.flags.noStep = true;
              break;
            default:
              break;
            } // Switch Rc

          break;
        } // Switch First Character of Line

    } // For Every Line

  if (! Gui::abortProcess()) {
      // last step in submodel
      // terminate parse build modifications
      if (opts.flags.parseBuildMods) {
          // terminate parse build mods at end of display page when called from gui::countPage for jump to page
          if (Gui::buildModJumpForward && ! opts.flags.stepGroup) {
              // we will be at the bottom of the current (display) page as pageNum is advanced at the end of the page step,
              // so set pageNum + 1 to use the correct page number to determine when to terminate the buildMod parse.
              // Use <= comparison between (pageNum + 1 and displayPageNum) to parse buildMod up to display page.
              opts.flags.parseBuildMods = ((opts.pageNum + 1) <= gui->saveDisplayPageNum);
          }
      }
      // Added callout step parse for parse build modifications so
      // exclude from page number increment and topOfPages indices
      if (opts.flags.partsAdded && ! opts.flags.callout && (! opts.flags.noStep || opts.flags.parseNoStep)) {
          if (Gui::exporting()) {
              Gui::getPageSizes().remove(opts.pageNum);
              if (opts.flags.pageSizeUpdate) {
                  opts.flags.pageSizeUpdate = false;
                  Gui::insertPageSize(opts.pageNum,opts.pageSize);
#ifdef PAGE_SIZE_DEBUG
                  logTrace() << "PG: Inserting New Page size info     at PageNumber:" << opts.pageNum
                             << "W:"    << opts.pageSize.sizeW << "H:"    << opts.pageSize.sizeH
                             << "O:"    <<(opts.pageSize.orientation == Portrait ? "Portrait" : "Landscape")
                             << "ID:"   << opts.pageSize.sizeID
                             << "Model:" << opts.current.modelName;
#endif
              } else {
                  Gui::insertPageSize(opts.pageNum,Gui::getPageSize(DEF_SIZE));
#ifdef PAGE_SIZE_DEBUG
                  logTrace() << "PG: Inserting Default Page size info at PageNumber:" << opts.pageNum
                             << "W:"    << Gui::getPageSize(DEF_SIZE).sizeW << "H:"    << Gui::getPageSize(DEF_SIZE).sizeH
                             << "O:"    << (Gui::getPageSize(DEF_SIZE).orientation == Portrait ? "Portrait" : "Landscape")
                             << "ID:"   << Gui::getPageSize(DEF_SIZE).sizeID
                             << "Model:" << opts.current.modelName;
#endif
              }
          } // Exporting

          // BuildMod create
          if (buildModKeys.size()) {
              if (buildMod.state != BM_END)
                  emit gui->parseErrorSig(QString("Required meta BUILD_MOD END not found"),
                                          opts.current, Preferences::BuildModErrors,false,false);
              Q_FOREACH (int buildModLevel, buildModKeys.keys())
                  insertBuildModification(buildModLevel);
          }
#ifdef QT_DEBUG_MODE
          emit gui->messageSig(LOG_NOTICE, QString("COUNTPAGE - Page %1 topOfPage Step, Submodel End (opt) - LineNumber %2, ModelName %3")
                               .arg(opts.pageNum, 3, 10, QChar('0')).arg(opts.current.lineNumber, 3, 10, QChar('0')).arg(opts.current.modelName));
#endif
          ++opts.pageNum;
          opts.flags.parseNoStep = false;
          Gui::topOfPages.append(opts.current); // Set TopOfStep (Last Step)
//#ifndef QT_DEBUG_MODE
//          if (Preferences::debugLogging)
//              documentPageCount();
//#endif
      } // Last Step in Submodel
  } // ! abortProcess

  return Gui::abortProcess() ? static_cast<int>(HitAbortProcess) : static_cast<int>(HitNothing);
} // CountPageWorker::countPage()

/**********************************************
 *
 *  Editor loader calls
 *
 **********************************************/

bool LoadModelWorker::_detached = false;

QMutex loadMutex;

void LoadModelWorker::setPlainText(const QString &content)
{
    QMetaObject::invokeMethod(
    _detached ? cmdModEditor : cmdEditor,      // obj
                "setPlainText",                // member
                Qt::QueuedConnection,          // connection type
                Q_ARG(QString, content));      // val1
}

void LoadModelWorker::setPagedContent(const QStringList &content)
{
    QMetaObject::invokeMethod(
    _detached ? cmdModEditor : cmdEditor,      // obj
                "setPagedContent",             // member
                Qt::QueuedConnection,          // connection type
                Q_ARG(QStringList, content));  // val1
}

void LoadModelWorker::setSubFiles(const QStringList &subFiles)
{
    QMetaObject::invokeMethod(
    _detached ? cmdModEditor : cmdEditor,      // obj
                "setSubFiles",                 // member
                Qt::QueuedConnection,          // connection type
                Q_ARG(QStringList, subFiles)); // val1
}

void LoadModelWorker::setLineCount(const int count)
{
    QMetaObject::invokeMethod(
    _detached ? cmdModEditor : cmdEditor,      // obj
                "setLineCount",                // member
                Qt::QueuedConnection,          // connection type
                Q_ARG(int, count));            // val1
}

int LoadModelWorker::loadModel(LDrawFile *ldrawFile, const QString &filePath)
{
    QMutexLocker loadLocker(&loadMutex);

#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_DEBUG,QString("3.  Editor loading..."));
#endif

    int lineCount = 0;
    QString content;
    QStringList contentList;
    QString fileName = filePath;

    if (_detached) {

        if (QDir::fromNativeSeparators(fileName).count("/")) {

            // open file for read
            QFile file(fileName);
            if (!file.open(QFile::ReadOnly | QFile::Text)) {
                emit gui->messageSig(LOG_ERROR,QString("Cannot read editor display file %1:\n%2.")
                                                       .arg(file.fileName()).arg(file.errorString()));
                return 1;
            }

            // get content and set codec
            QTextStream in(&file);
            in.setCodec(QTextCodec::codecForName("UTF-8"));
            content = in.readAll();
            contentList = content.split("\n");

            file.close();

        } else if (ldrawFile) {
            fileName = QFileInfo(filePath).fileName();
            contentList = ldrawFile->contents(fileName);
            content = contentList.join("\n");
        } else {
            emit gui->messageSig(LOG_ERROR,QString("No suitable data source detected for %1").arg(fileName));
            return 1;
        }

        QStringList subFiles;
        QRegExp sofRx("^0\\s+FILE\\s+(.*)$");  //start of file
        for (int i = 0; i < contentList.size(); i++) {
             if(contentList.at(i).contains(sofRx))
                 subFiles.append(sofRx.cap(1));
        }
        setSubFiles(subFiles);
    }

    // set line count
    lineCount = contentList.size();
    if (lineCount) {
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG,QString("3a. Editor set line count to %1").arg(lineCount));
#endif
        setLineCount(lineCount);
    } else {
        emit gui->messageSig(LOG_ERROR,QString("No lines detected in %1").arg(fileName));
        return 1;
    }

    // set content
    if (Preferences::editorBufferedPaging && lineCount > Preferences::editorLinesPerPage) {
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG,QString("3b. Editor load paged text started..."));
#endif
        if (contentList.size())
            setPagedContent(contentList);
    } else {
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG,QString("3b. Editor load plain text started..."));
#endif
        if (!content.isEmpty())
            setPlainText(content);
    }

    return 0;
} // LoadModelWorker::loadModel()
