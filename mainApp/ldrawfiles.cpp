
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This class reads in, manages and writes out LDraw files.  While being
 * edited an MPD file, or a top level LDraw files and any sub-model files
 * are maintained in memory using this class.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "version.h"
#include "ldrawfiles.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <QMessageBox>
#include <QFile>
#include <QRegExp>
#include <QHash>
#include <functional>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtConcurrent>
#endif
#include "paths.h"

#include "lpub.h"
#include "ldrawfilesload.h"
#include "lc_library.h"
#include "pieceinf.h"

#include "lc_previewwidget.h"

QList<QRegExp> LDrawFile::_fileRegExp;
QList<QRegExp> LDrawHeaderRegExp;
QList<QRegExp> LDrawUnofficialPartRegExp;
QList<QRegExp> LDrawUnofficialSubPartRegExp;
QList<QRegExp> LDrawUnofficialPrimitiveRegExp;
QList<QRegExp> LDrawUnofficialShortcutRegExp;
const QString LDrawUnofficialType[UNOFFICIAL_NUM] =
{
    QLatin1String("Unofficial Submodel"),  // UNOFFICIAL_SUBMODEL
    QLatin1String("Unofficial Part"),      // UNOFFICIAL_PART
    QLatin1String("Unofficial Subpart"),   // UNOFFICIAL_SUBPART
    QLatin1String("Unofficial Primitive"), // UNOFFICIAL_PRIMITIVE
    QLatin1String("Unofficial Shortcut"),  // UNOFFICIAL_SHORTUCT
    QLatin1String("Unofficial Other File") // UNOFFICIAL_OTHER
};

/********************************************
 *
 * routines for nested levels
 *
 ********************************************/

QList<HiarchLevel*> LDrawFile::_currentLevels;
QList<HiarchLevel*> LDrawFile::_allLevels;

HiarchLevel* addLevel(const QString& key, bool create)
{
    // if level object with specified key exists...
    for (HiarchLevel* level : LDrawFile::_allLevels)
        // return existing level object
        if (level->key == key)
            return level;

    // else if create object spedified...
    if (create) {
        // create a new level object
        HiarchLevel* level = new HiarchLevel(key);
        // add to 'all' level objects list
        LDrawFile::_allLevels.append(level);
        // return level object
        return level;
    }

    return nullptr;
}

int getLevel(const QString& key, int position)
{
    if (position == BM_BEGIN) {
        // ensure key is specified
        if (key.isEmpty())
            emit gui->messageSig(LOG_ERROR, QString("BUILD_MOD - GetLevel: Invalid request. Key not specified."));

        // add level object to 'all' levels
        HiarchLevel* level = addLevel(key, true);

        // if there are 'currentLevel' objects...
        if (LDrawFile::_currentLevels.size())
            // set last 'current' level object as parent of this level object
            level->level = LDrawFile::_currentLevels[LDrawFile::_currentLevels.size() - 1];
        else
            // set this level object parent to nullptr
            level->level = nullptr;

        // append this level object to 'current' level objects list
        LDrawFile::_currentLevels.append(level);

    } else if (position == BM_END) {
        // if there are 'current' level objects...
        if (LDrawFile::_currentLevels.size()) {
            // return absolute level, remove last level object from 'currentLevels' - reset to parent level or BM_BASE_LEVEL [0]
            LDrawFile::_currentLevels.removeAt(LDrawFile::_currentLevels.size() - 1);
/*
#ifdef QT_DEBUG_MODE
            const QString message = QString("BUILD_MOD - Get absolute level [%1] from 'currentLevels'%2")
                                            .arg(LDrawFile::_currentLevels.size()).arg(!key.isEmpty() ? QString(" - Key: %1").arg(key) : "");

            emit gui->messageSig(LOG_DEBUG, message);
#endif
//*/
        }
    }
//*
#ifdef QT_DEBUG_MODE
    else
    if (position == BM_CURRENT) {
        const QString message = QString("DEBUG: Build Modification - Get absolute level [%1] from 'currentLevels'%2")
                                        .arg(LDrawFile::_currentLevels.size()).arg(!key.isEmpty() ? QString(" - %1").arg(key) : "");
        qDebug() << qPrintable(message);
    }
#endif
//*/

    // return the absolute level from the 'current' list
    return LDrawFile::_currentLevels.size();
}

/********************************************
 *
 ********************************************/

QStringList LDrawFile::_subFileOrder;
QStringList LDrawFile::_subFileOrderNoUnoff;
QStringList LDrawFile::_includeFileList;
QStringList LDrawFile::_buildModList;
QStringList LDrawFile::_loadedParts;
QString LDrawFile::_file           = "";
QString LDrawFile::_description    = PUBLISH_DESCRIPTION_DEFAULT;
QString LDrawFile::_name           = "";
QString LDrawFile::_author         = VER_PRODUCTNAME_STR;
QString LDrawFile::_category       = "";
int     LDrawFile::_emptyInt;
int     LDrawFile::_partCount      = 0;
int     LDrawFile::_savedLines     = 0;
int     LDrawFile::_uniquePartCount= 0;
bool    LDrawFile::_currFileIsUTF8 = false;
bool    LDrawFile::_loadAborted    = false;
bool    LDrawFile::_loadBuildMods  = false;
bool    LDrawFile::_loadUnofficialParts = true;
bool    LDrawFile::_hasUnofficialParts = false;
bool    LDrawFile::_helperPartsNotInArchive = false;

LDrawSubFile::LDrawSubFile(
  const QStringList &contents,
  QDateTime         &datetime,
  int                unofficialPart,
  bool               generated,
  bool               includeFile,
  const QString     &subFilePath,
  const QString     &description)
{
  _contents << contents;
  _subFilePath = subFilePath;
  _description = description;
  _datetime = datetime;
  _modified = false;
  _numSteps = 0;
  _buildMods = 0;
  _instances = 0;
  _mirrorInstances = 0;
  _rendered = false;
  _mirrorRendered = false;
  _changedSinceLastWrite = true;
  _unofficialPart = unofficialPart;
  _generated = generated;
  _includeFile = includeFile;
  _startPageNumber = 0;
  _lineTypeIndexes.clear();
  _subFileIndexes.clear();
  _smiContents.clear();
  _prevStepPosition = { 0,0,0 };
}

/* Only used to store fade or highlight content */

CfgSubFile::CfgSubFile(
  const QStringList &contents,
  const QString     &subFilePath)
{
    _contents << contents;
    _subFilePath = subFilePath;
}

/* initialize new Build Mod */
BuildMod::BuildMod(const QVector<int> &modAttributes,
                   int                stepIndex)
{
    _modStepIndex = stepIndex;
    _modAttributes << modAttributes;
    _modActions.insert(stepIndex, BuildModApplyRc);
}

/* initialize new Build Mod Step */
BuildModStep::BuildModStep(const int      buildModStepIndex,
                           const int      buildModAction,
                           const QString &buildModKey)
{
    _buildModStepIndex = buildModStepIndex;
    _buildModAction = buildModAction;
    _buildModKey = buildModKey;
}

/* initialize viewer step*/
ViewerStep::ViewerStep(const QStringList &stepKey,
                       const QStringList &rotatedViewerContents,
                       const QStringList &rotatedContents,
                       const QStringList &unrotatedContents,
                       const QString     &filePath,
                       const QString     &imagePath,
                       const QString     &csiKey,
                       bool               multiStep,
                       bool               calledOut,
                       int                viewType)
{
    _rotatedViewerContents << rotatedViewerContents;
    _rotatedContents       << rotatedContents;
    _unrotatedContents     << unrotatedContents;
    _partCount = 0;
    _filePath  = filePath;
    _imagePath = imagePath;
    _csiKey    = csiKey;
    _modified  = false;
    _multiStep = multiStep;
    _calledOut = calledOut;
    _viewType  = viewType;
    _hasBuildModAction = false;
    if (viewType == Options::PLI) // Parts do not have modelIndex or lineNumber
        _stepKey   = { BM_NONE, 0, stepKey[BM_STEP_NUM_KEY].toInt() };
    else
        _stepKey   = { stepKey[BM_STEP_MODEL_KEY].toInt(), stepKey[BM_STEP_LINE_KEY].toInt(), stepKey[BM_STEP_NUM_KEY].toInt() };
}

void LDrawFile::empty()
{
  _subFiles.clear();
  _configuredSubFiles.clear();
  _subFileOrder.clear();
  _subFileOrderNoUnoff.clear();
  _viewerSteps.clear();
  _buildMods.clear();
  _buildModSteps.clear();
  _buildModStepIndexes.clear();
  _buildModRendered.clear();
  _buildModList.clear();
  _includeFileList.clear();
  _loadedParts.clear();
  _name.clear();
  _author.clear();
  _file.clear();
  _helperPartsNotInArchive = false;
  _mpd                   = false;
  _loadAborted           = false;
  _loadBuildMods         = false;
  _loadUnofficialParts   = true;
  _hasUnofficialParts    = false;
  _partCount             =  0;
  _uniquePartCount       =  0;
  _buildModNextStepIndex = -1;
  _buildModPrevStepIndex =  0;
}

void LDrawFile::normalizeHeader(const QString &fileName,int missing)
{
  auto setHeaders = [this, &fileName, &missing](int lineNumber) {
    QString line;
    if (missing == NameMissing) {
      line = QString("0 Name: %1").arg(fileName);
      insertLine(fileName, lineNumber, line);
    } else if (missing == AuthorMissing) {
      lineNumber++;
      line = QString("0 Author: %1").arg(Preferences::defaultAuthor);
      insertLine(fileName, lineNumber, line);
    } else {
      line = QString("0 Author: %1").arg(Preferences::defaultAuthor);
      insertLine(fileName, lineNumber, line);
      lineNumber++;
      line = QString("0 Name: %1").arg(fileName);
      insertLine(fileName, lineNumber, line);
    }
  };

  int numLines = size(fileName);
  for (int lineNumber = 0; lineNumber < numLines; lineNumber++) {
    QString line = readLine(fileName, lineNumber);
    if (missing) {
      int here = hdrDescNotFound ? 0 : 1;
      if (lineNumber == here) {
          setHeaders(lineNumber);
          break;
      }
    } else {
      int p;
      for (p = 0; p < line.size(); ++p) {
        if (line[p] != ' ')
          break;
      }
      if (line[p] >= '1' && line[p] <= '5') {
        if (lineNumber == 0)
            setHeaders(lineNumber);
        break;
      }
    }
  }
}

/* Add a new subFile */

void LDrawFile::insert(const QString &mcFileName,
                      QStringList    &contents,
                      QDateTime      &datetime,
                      int             unofficialPart,
                      bool            generated,
                      bool            includeFile,
                      const QString  &subFilePath,
                      const QString  &description)
{
  QString    fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    _subFiles.erase(i);
  }
  const QString modelDesc = description.isEmpty() ? QFileInfo(mcFileName).baseName() : description;
  LDrawSubFile subFile(contents,datetime,unofficialPart,generated,includeFile,subFilePath,modelDesc);
  _subFiles.insert(fileName,subFile);
  if (includeFile) {
      _includeFileList << fileName;
  } else {
      _subFileOrder << mcFileName;
      if (unofficialPart == UNOFFICIAL_SUBMODEL)
          _subFileOrderNoUnoff << mcFileName;
      if (unofficialPart > UNOFFICIAL_UNKNOWN)
          _hasUnofficialParts = true;
  }
}

/* Add a new modSubFile - Only used to insert fade or highlight content */

void LDrawFile::insertConfiguredSubFile(const QString &mcFileName,
                                        QStringList    &contents,
                                        const QString  &subFilePath)
{
  QString    fileName = mcFileName.toLower();
  QMap<QString, CfgSubFile>::iterator i = _configuredSubFiles.find(fileName);

  if (i != _configuredSubFiles.end()) {
    _configuredSubFiles.erase(i);
  }
  CfgSubFile subFile(contents,subFilePath);
  _configuredSubFiles.insert(fileName,subFile);
}

/* return the number of lines in the loaded model file */

int LDrawFile::loadedLines()
{
  int lines = 0;
  for (int i = 0; i < _subFileOrder.size(); i++) {
    QString subFileName = _subFileOrder.at(i).toLower();
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(subFileName);

    if (f != _subFiles.end() && ! f.value()._generated) {
      lines += f.value()._contents.size();
    }
  }
  return lines;
}

/* return the number of steps in the loaded model file */

int LDrawFile::loadedSteps()
{
  int steps = 0;
  for (int i = 0; i < _subFileOrder.size(); i++) {
    QString subFileName = _subFileOrder.at(i).toLower();
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(subFileName);

    if (f != _subFiles.end() && ! f.value()._generated) {
      steps += f.value()._numSteps;
    }
  }
  return steps;
}

/* return the number of lines in the subfile */

int LDrawFile::size(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();

  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    return i.value()._contents.size();
  }
  return 0;
}

/* Only used to return fade or highlight content size */

int LDrawFile::configuredSubFileSize(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();

  QMap<QString, CfgSubFile>::iterator i = _configuredSubFiles.find(fileName);

  if (i != _configuredSubFiles.end()) {
    return i.value()._contents.size();
  }
  return 0;
}

bool LDrawFile::isMpd()
{
  return _mpd;
}

QString LDrawFile::description(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    return i.value()._description;
  }
  return QFileInfo(mcFileName).completeBaseName();
}

int LDrawFile::isUnofficialPart(const QString &name)
{
  QString fileName = name.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._unofficialPart;
  }
  return UNOFFICIAL_UNKNOWN;
}

bool LDrawFile::isIncludeFile(const QString &name)
{
  QString fileName = name.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._includeFile;
  }
  return false;
}

/* return the name of the top level file */

QString LDrawFile::topLevelFile()
{
  if (_subFileOrder.size()) {
    return _subFileOrder.at(0).toLower();
  } else {
    return _emptyString;
  }
}

int LDrawFile::fileOrderIndex(const QString &file)
{
  for (int i = 0; i < _subFileOrder.size(); i++) {
    if (_subFileOrder.at(i).toLower() == file.toLower()) {
      return i;
    }
  }
  return -1;
}

/* return the number of steps within the file */

int LDrawFile::numSteps(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._numSteps;
  }
  return 0;
}

/* return the model start page number value */

int LDrawFile::getModelStartPageNumber(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._startPageNumber;
  }
  return 0;
}

QDateTime LDrawFile::lastModified(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._datetime;
  }
  return QDateTime();
}

bool LDrawFile::contains(const QString &file, bool searchAll)
{
  if (searchAll)
      for (int i = 0; i < _subFileOrder.size(); i++) {
        if (_subFileOrder.at(i).toLower() == file.toLower()) {
          return true;
        }
      }
  else
      for (int i = 0; i < _subFileOrderNoUnoff.size(); i++) {
        if (_subFileOrderNoUnoff[i].toLower() == file.toLower()) {
          return true;
        }
      }
  return false;
}

bool LDrawFile::isSubmodel(const QString &file)
{
  QString fileName = file.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
      return i.value()._unofficialPart == UNOFFICIAL_SUBMODEL && !i.value()._generated;
      //return ! i.value()._generated; // added on revision 368 - to generate csiSubModels for 3D render
  }
  return false;
}

bool LDrawFile::modified()
{
  bool modified = false;
  for (const QString &key : _subFiles.keys())
    modified |= _subFiles[key]._modified;
  return modified;
}

bool LDrawFile::modified(const QString &mcFileName, bool reset)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    const bool modified = i.value()._modified;
    if (reset) {
      i.value()._modified = false;
#ifdef QT_DEBUG_MODE
      if (reset && modified)
          emit gui->messageSig(LOG_DEBUG, QString("Reset Submodel: %1, Modified: [No].").arg(mcFileName));
#endif
    }
    return modified;
  } else {
    return false;
  }
}

bool LDrawFile::modified(const QStringList &parsedStack, bool reset)
{
  bool result = false;
  for (const QString &fileName : parsedStack) {
    LDrawSubFile &subFile = _subFiles[fileName];
    result |= subFile._modified;
    if (reset)
       subFile._modified = false;
  }
  return result;
}

bool LDrawFile::modified(const QVector<int> &parsedIndexes, bool reset)
{
#ifdef QT_DEBUG_MODE
    int count = 0;
    QString modifiedSubmodels;
#endif
    bool result = false;
    for (const int index : parsedIndexes) {
        const QString &fileName = getSubmodelName(index);
        LDrawSubFile &subFile = _subFiles[fileName];
        const bool modified = subFile._modified;
        result |= modified;
#ifdef QT_DEBUG_MODE
        if (modified) {
            modifiedSubmodels.append(QString("%1 (%2), ").arg(fileName).arg(index));
            count++;
        }
#endif
        if (reset)
           subFile._modified = false;
    }

#ifdef QT_DEBUG_MODE
    if (result) {
        modifiedSubmodels = modifiedSubmodels.trimmed();
        modifiedSubmodels.chop(1);
        emit gui->messageSig(LOG_TRACE, QString("LDrawFile Modified %1 %2 %3")
                                                .arg(count).arg(count == 1 ? "Submodel:" : "Submodels:")
                                                .arg(modifiedSubmodels));
    }
#endif
    return result;
}

void LDrawFile::setModified(const QString &mcFileName, bool modified)
{
  const QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    i.value()._modified = modified;
    i.value()._changedSinceLastWrite = modified;
  }
}

QStringList LDrawFile::contents(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    return i.value()._contents;
  } else {
    return _emptyList;
  }
}

void LDrawFile::setContents(const QString &mcFileName, const QStringList &contents)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._modified = true;
    //i.value()._datetime = QDateTime::currentDateTime();
    i.value()._contents = contents;
    i.value()._changedSinceLastWrite = true;
  }
}

void LDrawFile::setSmiContent(const QString &mcFileName, const QStringList &smiContents)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._smiContents = smiContents;
  }
}

QStringList LDrawFile::smiContents(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    return i.value()._smiContents;
  } else {
    return _emptyList;
  }
}

void LDrawFile::setSubFilePath(
                 const QString     &mcFileName,
                 const QString &subFilePath)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._subFilePath = subFilePath;
  }
}

QStringList LDrawFile::getSubModels()
{
    QStringList subModel;
    for (int i = 0; i < _subFileOrder.size(); i++) {
      QString modelName = _subFileOrder.at(i).toLower();
      QMap<QString, LDrawSubFile>::iterator it = _subFiles.find(modelName);
      if (it->_unofficialPart == UNOFFICIAL_SUBMODEL && !it->_generated) {
          subModel << modelName;
      }
    }
    return subModel;
}

QString LDrawFile::getSubFilePath(const QString &fileName)
{
  QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
  if (f != _subFiles.end()) {
    return f.value()._subFilePath;
  }
  return QString();
}

QStringList LDrawFile::getSubFilePaths()
{
  QStringList subFilesPaths;
  for (int i = 0; i < _subFileOrder.size(); i++) {
    QString filePath = getSubFilePath(_subFileOrder.at(i).toLower());
    if (!filePath.isEmpty())
        subFilesPaths << filePath;
  }
  if (_includeFileList.size()){
      for (int i = 0; i < _includeFileList.size(); i++) {
        QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(_includeFileList[i]);
        if (f != _subFiles.end()) {
            if (!f.value()._subFilePath.isEmpty()) {
                subFilesPaths << f.value()._subFilePath;
        }
      }
    }
  }
  return subFilesPaths;
}

void LDrawFile::setModelStartPageNumber(
        const QString &mcFileName,
        const int     &startPageNumber)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end())
    i.value()._startPageNumber = startPageNumber;
}

/* return the last fade position value */

int LDrawFile::getPrevStepPosition(
        const QString &mcFileName,
        const int     &mcLineNumber,
        const int     &mcStepNumber)
{
  int position = 0;
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
      if (mcStepNumber == i.value()._prevStepPosition.at(PS_STEP_NUM))
          position = i.value()._prevStepPosition.at(PS_LAST_POS);
      else
          position = i.value()._prevStepPosition.at(PS_POS);
  }

  if (position && Preferences::buildModEnabled) {
      QVector<int> indexKey = { getSubmodelIndex(mcFileName), mcLineNumber };
      const int currentStepIndex = _buildModStepIndexes.indexOf(indexKey);

      if (currentStepIndex == BM_INVALID_INDEX) {
          emit gui->messageSig(LOG_ERROR, QString("Could not find an index for TopOfStep fileName: %1 and lineNumber: %2")
                               .arg(mcFileName).arg(mcLineNumber));
          return position;
      }

      for (const QString &modKey : _buildMods.keys()) {
          const QMap<int,int> &modActions = _buildMods[modKey]._modActions;
          for (int stepIndex : modActions.keys()) {
              if (currentStepIndex == stepIndex) {
                  const QVector<int> &modAttributes = _buildMods[modKey]._modAttributes;
                  const int action = modActions.last();
#ifdef QT_DEBUG_MODE
                  const int oldPosition = position;
                  emit gui->messageSig(LOG_TRACE, QString("%1 BuildMod Last Action: %2, ActionStepIndex: %3, ModKey: '%4'")
                                                          .arg(action == BuildModApplyRc ? "Check" : "Adjust")
                                                          .arg(action == BuildModApplyRc ? "Apply(64)" : "Remove(65)")
                                                          .arg(modActions.lastKey()).arg(modKey));
#endif
                  if (action == BuildModRemoveRc && modAttributes.size() > BM_ACTION_LINE_NUM) {
                      const int modBegin      = modAttributes.at(BM_BEGIN_LINE_NUM);
                      const int modAction     = modAttributes.at(BM_ACTION_LINE_NUM);
                      const int modEnd        = modAttributes.at(BM_END_LINE_NUM);
                      const int modAdjustment = (modAction - modBegin) - (modEnd - modAction);
                      position                -= modAdjustment;
#ifdef QT_DEBUG_MODE
                      emit gui->messageSig(LOG_TRACE, QString("Adjust BuildMod Step %1 Position, (Action [%3] - Begin [%2]) - (End [%4] - Action [%3]) = Adjustment: [%5], "
                                                              "New Pos: [%6], Old Pos: [%7]")
                                                              .arg(mcStepNumber).arg(modBegin).arg(modAction).arg(modEnd).arg(modAdjustment)
                                                              .arg(position).arg(oldPosition));
#endif
                  }
              }
          }
      }
  }
  return position;
}

void LDrawFile::setPrevStepPosition(
        const QString &mcFileName,
        const int     &mcStepNumber,
        const int     &prevStepPosition)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    const int lastStepPosition = i.value()._prevStepPosition.size() ? i.value()._prevStepPosition.at(PS_POS) : PS_POS ;
    if (lastStepPosition != prevStepPosition) {
        QVector<int> stepPositions = { prevStepPosition, lastStepPosition, mcStepNumber };
        i.value()._prevStepPosition = stepPositions;
    }
  }
}

/* set all previous step positions to 0 */

void LDrawFile::clearPrevStepPositions()
{
  for (const QString &key : _subFiles.keys()) {
    _subFiles[key]._prevStepPosition.clear();
  }
}

/* Check the pngFile lastModified date against its submodel file */
bool LDrawFile::older(const QString &fileName,
                      const QDateTime &lastModified){
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    QDateTime fileDatetime = i.value()._datetime;
    if (fileDatetime > lastModified) {
      return false;
    }
  }
  return true;
}

bool LDrawFile::older(const QStringList &parsedStack,
                      const QDateTime &lastModified)
{
  for (const QString &fileName : parsedStack) {
    QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
    if (i != _subFiles.end()) {
      QDateTime fileDatetime = i.value()._datetime;
      if (fileDatetime > lastModified) {
        return false;
      }
    }
  }
  return true;
}

QStringList LDrawFile::subFileOrder() {
  if (_loadUnofficialParts)
    return _subFileOrder;
  else
    return _subFileOrderNoUnoff;
}

int LDrawFile::subFileOrderSize() {
    if (_loadUnofficialParts)
      return _subFileOrder.size();
    else
      return _subFileOrderNoUnoff.size();
}

QStringList LDrawFile::includeFileList() {
  return _includeFileList;
}

QString LDrawFile::getSubmodelName(int submodelIndx)
{
    if (submodelIndx > BM_INVALID_INDEX && submodelIndx < _subFileOrder.size())
        return _subFileOrder.at(submodelIndx).toLower();
    return QString();
}

int LDrawFile::getSubmodelIndex(const QString &mcFileName)
{
    const QString fileName = mcFileName.toLower();
    return _subFileOrder.indexOf(fileName);
}

/* marshall subFile 'child' indexes */

QVector<int> LDrawFile::getSubmodelIndexes(const QString &fileName)
{
    QVector<int> indexes, parsedIndexes;
    const QString mcFileName = fileName.toLower();
    const int mcModelIndex = getSubmodelIndex(mcFileName);

    if (mcFileName == topLevelFile()) {
        QMap<QString, LDrawSubFile>::const_iterator it = _subFiles.constBegin();
        while (it != _subFiles.constEnd()) {
            if (it.value()._unofficialPart == UNOFFICIAL_SUBMODEL && !it.value()._generated)
                parsedIndexes << getSubmodelIndex(it.key());
            ++it;
        }

        // remove top level file index
        parsedIndexes.takeFirst();

        return parsedIndexes;
    }

    auto getIndexes = [this, &parsedIndexes] (const int index, QVector<int> &indexes)
    {
        const QString mcFileName = getSubmodelName(index).toLower();
        QMap<QString, LDrawSubFile>::iterator it = _subFiles.find(mcFileName);
        if (it != _subFiles.end()) {
            if (it.value()._unofficialPart == UNOFFICIAL_SUBMODEL && !it.value()._generated) {
                for(int i : it.value()._subFileIndexes) {
                    if (!indexes.contains(i) && !parsedIndexes.contains(i))
                        indexes << i;
                }
            }
        }
    };

    getIndexes(mcModelIndex, indexes);

    while (!indexes.isEmpty()) {
        const int modelIndex = indexes.takeFirst();
        if (!parsedIndexes.contains(modelIndex))
            getIndexes(modelIndex, indexes);
        parsedIndexes << modelIndex;
    }

    return parsedIndexes;
}

// The Line Type Index is the position of the type 1 line in the parsed subfile written to temp
// This function returns the position (Relative Type Index) of the type 1 line in the subfile content
int LDrawFile::getLineTypeRelativeIndex(int submodelIndx, int lineTypeIndx) {
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(getSubmodelName(submodelIndx).toLower());
    if (f != _subFiles.end() && f.value()._lineTypeIndexes.size() > lineTypeIndx) {
        return f.value()._lineTypeIndexes.at(lineTypeIndx);
    }
    return -1;
}

// The Relative Type Index is the position of the type 1 line in the subfile content
// This function inserts the Relative Type Index at the position (Line Type Index)
// of the type 1 line in the parsed subfile written to temp
void LDrawFile::setLineTypeRelativeIndex(int submodelIndx, int relativeTypeIndx) {
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(getSubmodelName(submodelIndx).toLower());
    if (f != _subFiles.end()) {
        f.value()._lineTypeIndexes.append(relativeTypeIndx);
    }
}

// This function sets the Line Type Indexes vector
void LDrawFile::setLineTypeRelativeIndexes(int submodelIndx, QVector<int> &relativeTypeIndxes) {
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(getSubmodelName(submodelIndx).toLower());
    if (f != _subFiles.end()) {
        f.value()._lineTypeIndexes = relativeTypeIndxes;
    }
}

// This function returns the submodel Line Type Index
int LDrawFile::getLineTypeIndex(int submodelIndx, int relativeTypeIndx) {
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(getSubmodelName(submodelIndx).toLower());
    if (f != _subFiles.end() && f.value()._lineTypeIndexes.size()) {
        for (int i = 0; i < f.value()._lineTypeIndexes.size(); ++i)
            if (f.value()._lineTypeIndexes.at(i) == relativeTypeIndx)
                return i;
    }
    return -1;
}

// This function returns a pointer to the submodel Line Type Index vector
QVector<int> *LDrawFile::getLineTypeRelativeIndexes(int submodelIndx){

    QString fileName = getSubmodelName(submodelIndx).toLower();
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
    if (f != _subFiles.end() && f.value()._lineTypeIndexes.size()) {
        return &f.value()._lineTypeIndexes;
    }
    return nullptr;
}

// This function returns the number of indexes (type 1 parts) specified for the specified submodel
int LDrawFile::getLineTypeRelativeIndexCount(int submodelIndx) {
    QString fileName = getSubmodelName(submodelIndx).toLower();
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
    if (f != _subFiles.end())
        return f.value()._lineTypeIndexes.size();
    return 0;
}

// This function resets the Line Type Indexes vector
void LDrawFile::resetLineTypeRelativeIndex(const QString &mcFileName) {
    QString fileName = mcFileName.toLower();
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
    if (f != _subFiles.end()) {
        f.value()._lineTypeIndexes.clear();
    }
}

QString LDrawFile::readLine(const QString &mcFileName, int lineNumber)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
      if (lineNumber < i.value()._contents.size())
          return i.value()._contents[lineNumber];
  }
  return QString();
}

void LDrawFile::insertLine(const QString &mcFileName, int lineNumber, const QString &line)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._contents.insert(lineNumber,line);
    i.value()._modified = true;
 //   i.value()._datetime = QDateTime::currentDateTime();
    i.value()._changedSinceLastWrite = true;
  }
}

void LDrawFile::replaceLine(const QString &mcFileName, int lineNumber, const QString &line)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._contents[lineNumber] = line;
    i.value()._modified = true;
//    i.value()._datetime = QDateTime::currentDateTime();
    i.value()._changedSinceLastWrite = true;
  }
}

void LDrawFile::deleteLine(const QString &mcFileName, int lineNumber)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._contents.removeAt(lineNumber);
    i.value()._modified = true;
//    i.value()._datetime = QDateTime::currentDateTime();
    i.value()._changedSinceLastWrite = true;
  }
}

void LDrawFile::changeContents(
                    const QString &mcFileName,
                          int      position,
                          int      charsRemoved,
                    const QString &charsAdded)
{
  QString fileName = mcFileName.toLower();
  if (charsRemoved || charsAdded.size()) {
    QString all = contents(fileName).join("\n");
    all.remove(position,charsRemoved);
    all.insert(position,charsAdded);
    setContents(fileName,all.split("\n"));
  }
}

/*  Only used by SubMeta::parse(...) to read fade or highlight content */

QString LDrawFile::readConfiguredLine(const QString &mcFileName, int lineNumber)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, CfgSubFile>::iterator i = _configuredSubFiles.find(fileName);

  if (i != _configuredSubFiles.end()) {
      if (lineNumber < i.value()._contents.size())
          return i.value()._contents[lineNumber];
  }
  return QString();
}

void LDrawFile::unrendered()
{
  for (const QString &key : _subFiles.keys()) {
    _subFiles[key]._rendered = false;
    _subFiles[key]._mirrorRendered = false;
    _subFiles[key]._renderedKeys.clear();
    _subFiles[key]._mirrorRenderedKeys.clear();
  }
}

void LDrawFile::setRendered(
        const QString &mcFileName,
        bool           mirrored,
        const QString &renderParentModel,
        int            renderStepNumber,
        int            howCounted,
        bool           countPage)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
      QString key =
        howCounted == CountAtStep ?
          QString("%1 %2").arg(renderParentModel).arg(renderStepNumber) :
        howCounted > CountFalse && howCounted < CountAtStep ?
          renderParentModel : QString();
        if (countPage)
          key.prepend("cp~");
    if (mirrored) {
      i.value()._mirrorRendered = true;
      if (!key.isEmpty() && !i.value()._mirrorRenderedKeys.contains(key)) {
        i.value()._mirrorRenderedKeys.append(key);
      }
    } else {
      i.value()._rendered = true;
      if (!key.isEmpty() && !i.value()._renderedKeys.contains(key)) {
        i.value()._renderedKeys.append(key);
      }
    }
//#ifdef QT_DEBUG_MODE
//    qDebug() << "SET RENDERED - "
//             << "COUNTPAGE:"         << "[" << qPrintable(countPage ? "YES" : "NO") << "]"
//             << "KEY:"               << "[" << qPrintable(key)                      << "]"
//             << "FileName:"          << "[" << qPrintable(fileName)                 << "]"
//             << "RenderParentModel:" << "[" << qPrintable(renderParentModel)        << "]"
//             << "HowCounted:"        << "[" << qPrintable(howCounted == CountAtStep  ? "CountAtStep"  :
//                                                          howCounted == CountAtTop   ? "CountAtTop"   :
//                                                          howCounted == CountAtModel ? "CountAtModel" :
//                                                                        QString::number(howCounted))
//             << "]"                  << "\n"
//                ;
//#endif
  }
}

bool LDrawFile::rendered(const QString &mcFileName,
        bool           mirrored,
        const QString &renderParentModel,
        int            renderStepNumber,
        int            howCounted,
        bool           countPage)
{
  bool rendered = false;
  if (howCounted == CountFalse)
      return rendered;
  bool haveKey  = false;

  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    QString key =
        howCounted == CountAtStep ?
          QString("%1 %2").arg(renderParentModel).arg(renderStepNumber) :
        howCounted > CountFalse && howCounted < CountAtStep ?
          renderParentModel : QString() ;
    QString altKey = key;
    if (countPage)
        key.prepend("cp~");
    if (mirrored) {
      // check the countPage key ('cp~' prefix) if present.
      haveKey = key.isEmpty() || (countPage && key == "cp~") ? howCounted == CountAtTop ? true : false :
                  i.value()._mirrorRenderedKeys.contains(key);
      // if no key found using countPage key, attempt to check the findPage key (no 'cp~' prefix)
      if (!haveKey && countPage) {
        haveKey = altKey.isEmpty() ? howCounted == CountAtTop ? true : false :
                    i.value()._mirrorRenderedKeys.contains(altKey);
      }
      rendered  = i.value()._mirrorRendered;
    } else {
      haveKey = key.isEmpty() || (countPage && key == "cp~") ? howCounted == CountAtTop ? true : false :
                  i.value()._renderedKeys.contains(key);
      if (!haveKey && countPage) {
        haveKey = altKey.isEmpty() ? howCounted == CountAtTop ? true : false :
                    i.value()._renderedKeys.contains(altKey);
      }
      rendered  = i.value()._rendered;
    }
//#ifdef QT_DEBUG_MODE
//    qDebug() << "RENDERED:"   << "[" << qPrintable(rendered  ? "YES" : "NO") << "]"
//             << "COUNTPAGE:"  << "[" << qPrintable(countPage ? "YES" : "NO") << "]"
//             << "HAVEKEY:"    << "[" << qPrintable(haveKey   ? "YES" : "NO") << "]"
//             << "KEY:"        << "[" << qPrintable(key)                      << "]"
//             << "ALTKEY:"     << "[" << qPrintable(altKey)                   << "]"
//             << "FileName:"   << "[" << qPrintable(fileName)                 << "]"
//             << "HowCounted:" << "[" << qPrintable(howCounted == CountAtStep  ? "CountAtStep"  :
//                                                   howCounted == CountAtTop   ? "CountAtTop"   :
//                                                   howCounted == CountAtModel ? "CountAtModel" :
//                                                                 QString::number(howCounted))
//             << "]"           << "\n"
//                ;
//#endif
    return rendered && haveKey;
  }
  return rendered;
}

int LDrawFile::instances(const QString &mcFileName, bool mirrored)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  int instances = 0;

  if (i != _subFiles.end()) {
    if (mirrored) {
      instances = i.value()._mirrorInstances;
    } else {
      instances = i.value()._instances;
    }
  }
  return instances;
}

int LDrawFile::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Cannot read LDraw file: [%1]<br>%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return 1;
    }
    QByteArray qba(file.readAll());
    file.close();

    QElapsedTimer t; t.start();

    // check file encoding
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString utfTest = codec->toUnicode(qba.constData(), qba.size(), &state);
    _currFileIsUTF8 = state.invalidChars == 0;
    utfTest = QString();

    // get rid of what's there before we load up new stuff

    empty();

    // allow files ldr suffix to allow for MPD
    enum {
        UNKNOWN_FILE,
        MPD_FILE,
        LDR_FILE
    };

    int type = UNKNOWN_FILE;

    QFileInfo fileInfo(fileName);

    QTextStream in(&qba);
    while ( ! in.atEnd()) {
        QString line = in.readLine(0);
        if (line.contains(_fileRegExp[SOF_RX])) {
            emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("File '%1' identified as Multi-Part LDraw System (MPD) Document").arg(fileInfo.fileName()));
            type = MPD_FILE;
            break;
        }
        if (line.contains(_fileRegExp[NAM_RX]) || line.contains(_fileRegExp[LDR_RX])) {
            emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("File '%1' identified as LDraw System (LDR) Document").arg(fileInfo.fileName()));
            type = LDR_FILE;
            break;
        }
    }

    if (type == UNKNOWN_FILE) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("File '%1' is not a valid LDraw (LDR) or Multi-Part LDraw (MPD) System Document.").arg(fileInfo.fileName()));
        return 1;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    topLevelModel = true;

    QFuture<void> loadFuture = QtConcurrent::run([this, fileInfo, type]() {
        if (type == MPD_FILE)
            loadMPDFile(fileInfo.absoluteFilePath());
        else
            loadLDRFile(fileInfo.absoluteFilePath());
    });
    loadFuture.waitForFinished();

    QApplication::restoreOverrideCursor();

    QFuture<void> colorPartsFuture = QtConcurrent::run([this](){ addCustomColorParts(topLevelFile()); });
    colorPartsFuture.waitForFinished();

    buildModLevel = 0 /*false*/;

    QFuture<void> countPartsFuture = QtConcurrent::run([this](){countParts(topLevelFile()); });
    countPartsFuture.waitForFinished();

    auto getCount = [] (const LoadMsgType lmt)
    {
        if (lmt == ALL_LOAD_MSG)
            return _loadedParts.size();

        bool ok;
        int count = 0;

        for (const QString &part : _loadedParts)
        {
            int mt = QString(part[0]).toInt(&ok);
            if (ok && static_cast<LoadMsgType>(mt) == lmt)
                count++;
        }

        return count;
    };

    const QString loadMessage = QObject::tr("Loaded LDraw %1 model file '%2'. Unique %3 %4. Total %5 %6. %7")
                                            .arg(type == MPD_FILE ? "MPD" : "LDR")
                                            .arg(fileInfo.fileName())
                                            .arg(_uniquePartCount == 1 ? QObject::tr("Part") : QObject::tr("Parts"))
                                            .arg(_uniquePartCount)
                                            .arg(_partCount == 1 ? QObject::tr("Part") : QObject::tr("Parts"))
                                            .arg(_partCount)
                                            .arg(gui->elapsedTime(t.elapsed()));

    if (Preferences::modeGUI) {
        bool showLoadMessages = false;
        int mpc = getCount(MISSING_LOAD_MSG);
        int ppc = getCount(PRIMITIVE_LOAD_MSG);
        int spc = getCount(SUBPART_LOAD_MSG);

        switch (Preferences::ldrawFilesLoadMsgs)
        {
            case NEVER_SHOW:
                break;
            case SHOW_ERROR:
                showLoadMessages = mpc;
                break;
            case SHOW_WARNING:
                showLoadMessages = ppc || spc;
                break;
            case SHOW_MESSAGE:
                showLoadMessages = mpc || ppc || spc;
                break;
            case ALWAYS_SHOW:
                showLoadMessages = true;
                break;
        }

        if (showLoadMessages) {
            int vpc  = getCount(VALID_LOAD_MSG);
            int msmc = getCount(MPD_SUBMODEL_LOAD_MSG);
            int lsmc = getCount(LDR_SUBMODEL_LOAD_MSG);
            int ipc  = getCount(INLINE_PART_LOAD_MSG);
            int ippc = getCount(INLINE_PRIMITIVE_LOAD_MSG);
            int ispc = getCount(INLINE_SUBPART_LOAD_MSG);
            int apc  = _partCount;
            int upc  = _uniquePartCount;
            bool delta = apc != vpc;

            _loadedParts << QObject::tr("Loaded LDraw %1 model file <b>%2</b>.%3%4%5%6%7%8%9%10%11%12%13%14%15%16%17%18")
                               /* 01 */ .arg(type == MPD_FILE ? "<b>MPD</b>" : "<b>LDR</b>")
                               /* 02 */ .arg(fileInfo.fileName())
                               /* 03 */ .arg(delta ? QObject::tr("<br>Parts count:            <b>%1</b>").arg(apc) : "")
                               /* 04 */ .arg(mpc   ? QObject::tr("<span style=\"color:red\">"
                                                                 "<br>Missing parts:          <b>%1</b></span>").arg(mpc) : "")
                               /* 05 */ .arg(vpc   ? QObject::tr("<br>Total validated parts:  <b>%1</b>").arg(vpc)  : "")
                               /* 06 */ .arg(upc   ? QObject::tr("<br>Unique validated parts: <b>%1</b>").arg(upc)  : "")
                               /* 07 */ .arg(msmc  ? QObject::tr("<br>Submodels:              <b>%1</b>").arg(msmc) : "")
                               /* 08 */ .arg(lsmc  ? QObject::tr("<br>Submodels:              <b>%1</b>").arg(lsmc) : "")
                               /* 09 */ .arg(ipc   ? QObject::tr("<br>Inline parts:           <b>%1</b>").arg(ipc)  : "")
                               /* 10 */ .arg(ippc  ? QObject::tr("<br>Inline primitives:      <b>%1</b>").arg(ippc) : "")
                               /* 11 */ .arg(ispc  ? QObject::tr("<br>Inline subparts:        <b>%1</b>").arg(ispc) : "")
                               /* 12 */ .arg(ppc   ? QObject::tr("<br>Primitive parts:        <b>%1</b>").arg(ppc)  : "")
                               /* 13 */ .arg(spc   ? QObject::tr("<br>Subparts:               <b>%1</b>").arg(spc)  : "")
                               /* 14 */ .arg(        QObject::tr("<br>Loaded steps:           <b>%1</b>").arg(loadedSteps()))
                               /* 15 */ .arg(        QObject::tr("<br>Loaded subfiles:        <b>%1</b>").arg(_subFileOrder.size()))
                               /* 16 */ .arg(        QObject::tr("<br>Loaded lines:           <b>%1</b>").arg(loadedLines()))
                               /* 17 */ .arg(QString("<br>%1").arg(gui->elapsedTime(t.elapsed())))
                               /* 18 */ .arg(mpc   ? QObject::tr("<br><br>Missing %1 %2 not found in the %3 or %4 archive.<br>"
                                                                 "If %5 custom %1, be sure %7 location is captured in the LDraw search directory list.<br>"
                                                                 "If %5 new unofficial %1, be sure the unofficial archive library is up to date.")
                                                                 .arg(mpc > 1 ? QObject::tr("parts") : QObject::tr("part"))          /* 01 */
                                                                 .arg(mpc > 1 ? QObject::tr("were") :  QObject::tr("was"))           /* 02 */
                                                                 .arg(VER_LPUB3D_UNOFFICIAL_ARCHIVE)                                 /* 03 */
                                                                 .arg(VER_LDRAW_OFFICIAL_ARCHIVE)                                    /* 04 */
                                                                 .arg(mpc > 1 ? QObject::tr("these are") : QObject::tr("this is a")) /* 05 */
                                                                 .arg(mpc > 1 ? QObject::tr("their") :     QObject::tr("its")) : "");/* 07 */

            int response = LdrawFilesLoad::showLoadMessages(_loadedParts);
            if (response == QDialog::Rejected) {
                if (mpc || ppc || spc) {
                    empty();
                    _loadAborted = true;
                    return 1;
                }
            } // load message rejected
        } // show load message
        else
        {
            emit gui->messageSig(LOG_INFO_STATUS, loadMessage);
        }
    } // modeGUI
    else
    {
        emit gui->messageSig(LOG_INFO, loadMessage);
    }

    return 0;
}

void LDrawFile::loadIncludeFile(const QString &mcFileName)
{
    QFile file(mcFileName);
    if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
        emit lpub->messageSig(LOG_ERROR, QObject::tr("Cannot read include file %1<br>%2")
                                                     .arg(mcFileName)
                                                     .arg(file.errorString()));
        return;
    }

    emit lpub->messageSig(LOG_TRACE, QObject::tr("Loading include file '%1'...").arg(mcFileName));

    QTextStream in(&file);
    in.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));

    auto isValidLine = [] (const QString &smLine) {
        if (smLine.isEmpty())
            return false;

        switch (smLine.toLatin1()[0]) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
            emit lpub->messageSig(LOG_NOTICE, QObject::tr("Invalid include line [%1].<br>"
                                                          "Part lines (type 1 to 5) are ignored in include file.").arg(smLine));
            return false;
        case '0':
            return true;
        }

        return false;
    };

    QStringList tokens;
    while (! in.atEnd()) {
        QString smLine = in.readLine(0).trimmed();
        if (isComment(smLine))
            continue;
        if (isValidLine(smLine)) {
            split(smLine,tokens);
            if (tokens.size() >= 4)
                processMetaCommand(tokens);
        }
    }
    file.close();
}

void LDrawFile::processMetaCommand(const QStringList &tokens)
{
    int number;
    bool validNumber;

    if (tokens.size() < 4)
        return;

    // Check if load external parts in command editor is disabled
    if (metaLoadUnoffPartsNotFound) {
        if (tokens.at(2) == QLatin1String("LOAD_UNOFFICIAL_PARTS_IN_EDITOR")) {
            _loadUnofficialParts = tokens.last() == "FALSE" ? false : true ;
            emit gui->messageSig(LOG_INFO, QObject::tr("Load Custom Unofficial Parts In Command Editor is %1")
                                                       .arg(_loadUnofficialParts ? QObject::tr("Enabled") : QObject::tr("Disabled")));
            metaLoadUnoffPartsNotFound = false;
        }
    }

    // Check if BuildMod is disabled
    if (metaBuildModNotFund) {
        if (tokens.at(2) == QLatin1String("BUILD_MOD_ENABLED")) {
            _loadBuildMods  = tokens.last() == "FALSE" ? false : true ;
            Preferences::buildModEnabled = _loadBuildMods;
            emit gui->messageSig(LOG_INFO, QObject::tr("Build Modifications are %1")
                                                       .arg(Preferences::buildModEnabled ? QObject::tr("Enabled") : QObject::tr("Disabled")));
            metaBuildModNotFund = false;
        }
    }

    // Check if insert final model is disabled
    if (metaFinalModelNotFound) {
        if (tokens.at(2) == QLatin1String("FINAL_MODEL_ENABLED")) {
            Preferences::finalModelEnabled = tokens.last() == "FALSE" ? false : true ;
            if (Preferences::enableFadeSteps || Preferences::enableHighlightStep)
                emit gui->messageSig(LOG_INFO, QObject::tr("Display Final Model is %1")
                                                           .arg(Preferences::finalModelEnabled ? QObject::tr("Enabled") : QObject::tr("Disabled")));
            metaFinalModelNotFound = false;
        }
    }

    // Check if Start Page Number is specified
    if (metaStartPageNumNotFound) {
        if (tokens.at(2) == QLatin1String("START_PAGE_NUMBER")) {
            number = tokens.last().toInt(&validNumber);
            Gui::pa  = validNumber ? number - 1 : 0;
            emit gui->messageSig(LOG_INFO, QObject::tr("Start Page Number Set to %1").arg(QString::number(Gui::pa)));
            metaStartPageNumNotFound = false;
        }
    }

    // Check if Start Step Number is specified
    if (metaStartStepNumNotFound) {
        if (tokens.at(2) == QLatin1String("START_STEP_NUMBER")) {
            number = tokens.last().toInt(&validNumber);
            Gui::sa  = validNumber ? number - 1 : 0;
            emit gui->messageSig(LOG_INFO, QObject::tr("Start Step Number Set to %1").arg(QString::number(Gui::sa)));
            metaStartStepNumNotFound = false;
        }
    }
}

void LDrawFile::loadMPDFile(const QString &fileName, bool externalFile)
{
    MissingHeader headerMissing = NoneMissing;

    std::function<int()> missingHeaders;
    missingHeaders = [this] ()
    {
        if (hdrNameNotFound && hdrAuthorNotFound)
            return BothMissing;
        else if (hdrNameNotFound)
            return NameMissing;
        else if (hdrAuthorNotFound)
            return AuthorMissing;
        return NoneMissing;
    };

    auto fileType = [&,this] (int isUnofficial = 0)
    {
        if (isUnofficial)
            return LDrawUnofficialType[unofficialPart];
        else
            return topLevelModel
                    ? QObject::tr("model")
                    : unofficialPart
                      ? LDrawUnofficialType[unofficialPart]
                        : hdrFILENotFound
                        ? QObject::tr("subfile")
                        : QObject::tr("submodel");
    };

    QFileInfo   fileInfo(fileName);

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        emit gui->messageSig(LOG_ERROR, QObject::tr("Cannot read mpd file %1<br>%2")
                                                    .arg(fileName)
                                                    .arg(file.errorString()));
        return;
    }

    int subfileIndx;
    bool alreadyLoaded;
    bool subfileFound        = false;
    bool partHeaderFinished  = false;
    bool stagedSubfilesFound = externalFile;
    bool modelHeaderFinished = externalFile ? true : false;
    bool sosf = false;
    bool eosf = false;

    if (topLevelModel) {
        topHeaderFinished          = false;
        topFileNotFound            = true;
        hdrFILENotFound            = true;
        hdrDescNotFound            = true;
        hdrCategNotFound           = true;
        helperPartsNotFound        = true;
        metaLoadUnoffPartsNotFound = true;
        metaBuildModNotFund        = true;
        metaFinalModelNotFound     = true;
        metaStartPageNumNotFound   = true;
        metaStartStepNumNotFound   = true;
        descriptionLine            = 0;
    }

    hdrNameNotFound   = true;
    hdrAuthorNotFound = true;
    unofficialPart    = UNOFFICIAL_UNKNOWN;

    QString subfileName, subFile, smLine;
    QStringList stagedContents, stagedSubfiles, contents, tokens;
    QStringList searchPaths = Preferences::ldSearchDirs;

    QString ldrawPath = QDir::toNativeSeparators(Preferences::ldrawLibPath);
    if (!searchPaths.contains(ldrawPath + QDir::separator() + "MODELS",Qt::CaseInsensitive))
        searchPaths.append(ldrawPath + QDir::separator() + "MODELS");
    if (!searchPaths.contains(ldrawPath + QDir::separator() + "PARTS",Qt::CaseInsensitive))
        searchPaths.append(ldrawPath + QDir::separator() + "PARTS");
    if (!searchPaths.contains(ldrawPath + QDir::separator() + "P",Qt::CaseInsensitive))
        searchPaths.append(ldrawPath + QDir::separator() + "P");
    if (!searchPaths.contains(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "PARTS",Qt::CaseInsensitive))
        searchPaths.append(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "PARTS");
    if (!searchPaths.contains(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "P",Qt::CaseInsensitive))
        searchPaths.append(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "P");

    /* Read it in the first time to put into fileList in order of appearance */

    QTextStream in(&file);
    in.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));

    while ( ! in.atEnd()) {
        QString smLine = in.readLine(0);
        stagedContents << smLine.trimmed();
    }
    file.close();

    int lineCount = stagedContents.size();

    if (topLevelModel)
        emit gui->progressBarPermInitSig();
    else
        emit gui->progressBarPermReset();
    emit gui->progressPermRangeSig(1, lineCount);
    emit gui->progressPermMessageSig(QObject::tr("Loading MPD subfile '%1'...").arg( fileInfo.fileName()));

#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_DEBUG, QString("Stage Contents Size: %1").arg(lineCount));
#endif

    QDateTime datetime = QFileInfo(fileName).lastModified();

    for (int lineIndx = 0; lineIndx < lineCount; lineIndx++) {

        smLine = stagedContents.at(lineIndx).trimmed();

        emit gui->progressPermSetValueSig(lineIndx);

        if (smLine.isEmpty())
            continue;

        bool sof = smLine.contains(_fileRegExp[SOF_RX]);  //start of submodel file
        bool eof = smLine.contains(_fileRegExp[EOF_RX]);  //end of submodel file

        // load LDCad groups
        if (!ldcadGroupsLoaded) {
            if (smLine.contains(_fileRegExp[LDG_RX])) {
                insertLDCadGroup(_fileRegExp[LDG_RX].cap(3),_fileRegExp[LDG_RX].cap(1).toInt());
                insertLDCadGroup(_fileRegExp[LDG_RX].cap(2),_fileRegExp[LDG_RX].cap(1).toInt());
            } else if (smLine.contains("0 STEP") || smLine.contains("0 ROTSTEP")) {
                ldcadGroupsLoaded = true;
            }
        }

        split(smLine,tokens);

        // subfile check;
        if ((subfileFound = tokens.size() == 15 && tokens.at(0) != "0")) {
            modelHeaderFinished = partHeaderFinished = true;
            subFile = tokens.at(14);
        } else if (isSubstitute(smLine,subFile)) {
            subfileFound = !subFile.isEmpty();
        }
        if (subfileFound) {
            PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(subFile.toLatin1().constData(), nullptr, false, false);
            if (! pieceInfo && ! LDrawFile::contains(subFile) && ! stagedSubfiles.contains(subFile)) {
                stagedSubfiles.append(subFile);
                stagedSubfilesFound = true;
            }
            // determine if helper part
            if (helperPartsNotFound) {
                if (ExcludedParts::isExcludedHelperPart(subFile)) {
                    helperPartsNotFound = false;
                    _helperPartsNotInArchive = ! pieceInfo;
                }
            }
        }

        if ((sof || !hdrFILENotFound) && !modelHeaderFinished) {
            if (sof) {
                hdrFILENotFound = false;        /* we have an LDraw submodel */
                descriptionLine = lineIndx + 1; /* next line should be description */
                if (!externalFile)
                    modelHeaderFinished = false;/* set model header flag */
                // One time populate top level file name
                if (topFileNotFound) {
                    _file = _fileRegExp[SOF_RX].cap(1).replace("." + QFileInfo(_fileRegExp[SOF_RX].cap(1)).suffix(),"");
                    topFileNotFound = false;
                }
            } else {
                if (hdrDescNotFound && lineIndx == descriptionLine) {
                    if (smLine.contains(_fileRegExp[DES_RX]) && ! isHeader(smLine)) {
                        _description = QString(smLine).remove(0, 2);
                        if (topLevelModel)
                            Preferences::publishDescription = _description;
                        hdrDescNotFound = false;
                    } else
                        _description = QFileInfo(subfileName).completeBaseName();
                }

                if (hdrNameNotFound) {
                    if (smLine.contains(_fileRegExp[NAM_RX])) {
                        if (topLevelModel)
                            _name = _fileRegExp[NAM_RX].cap(1).replace(": ","");
                        hdrNameNotFound = false;
                    }
                }

                if (hdrAuthorNotFound) {
                    if (smLine.contains(_fileRegExp[AUT_RX])) {
                        if (topLevelModel) {
                            _author = _fileRegExp[AUT_RX].cap(1).replace(": ","");
                            Preferences::defaultAuthor = _author;
                        }
                        hdrAuthorNotFound = false;
                    }
                }

                if (unofficialPart == UNOFFICIAL_UNKNOWN) {
                    unofficialPart = getUnofficialFileType(smLine);
                    if (unofficialPart) {
                        emit gui->messageSig(LOG_TRACE, QObject::tr("Subfile '%1' spcified as %2.")
                                                                    .arg(subfileName).arg(fileType()));
                    }
                }

                // One time populate model category (not used)
                if (hdrCategNotFound) {
                    if (smLine.contains(_fileRegExp[CAT_RX])) {
                        if (topLevelModel)
                            _category = _fileRegExp[CAT_RX].cap(1);
                        hdrCategNotFound = false;
                    }
                }

                // Check for include file
                if (smLine.contains(_fileRegExp[INC_RX])) {
                    const QString filePath = LPub::getFilePath(_fileRegExp[INC_RX].cap(1));
                    if (!filePath.isEmpty())
                        loadIncludeFile(filePath);
                }

                // Check meta commands
                if (!isComment(smLine))
                    processMetaCommand(tokens);
            }
        } // modelHeaderFinished

        if ((alreadyLoaded = LDrawFile::contains(subfileName))) {
            emit gui->messageSig(LOG_TRACE, QObject::tr("MPD %1 '%2' already loaded.").arg(fileType()).arg(subfileName));
            subfileIndx = stagedSubfiles.indexOf(subfileName);
            if (subfileIndx > NOT_FOUND)
                stagedSubfiles.removeAt(subfileIndx);
        }

        // processing inlined parts
        if (!sof && hdrFILENotFound) {
            if (subfileName.isEmpty() && hdrNameNotFound) {
                sosf = smLine.contains(_fileRegExp[NAM_RX]);
                if (! sosf)
                    contents << smLine;
            } else if (! hdrNameNotFound && smLine.startsWith("0")) {
                if ((eosf = smLine.contains(_fileRegExp[NAM_RX]))) {
                    const QString &lastLine = contents.last();
                    if (lastLine.contains(_fileRegExp[DES_RX]))
                        _description = contents.takeLast(); // for inline files
                } else if ((eosf = lineIndx == lineCount - 1 && smLine == "0"))
                    contents << smLine;                         // for external files
            }

            if (!sosf && !eosf) {
                if (hdrAuthorNotFound) {
                    if (smLine.contains(_fileRegExp[AUT_RX]))
                        hdrAuthorNotFound = false;
                }
                if (! partHeaderFinished && unofficialPart == UNOFFICIAL_UNKNOWN) {
                    unofficialPart = getUnofficialFileType(smLine);
                    if (unofficialPart) {
                        emit gui->messageSig(LOG_TRACE, QObject::tr("Inline file '%1' spcified as %2.")
                                                                    .arg(subfileName).arg(fileType()));
                    }
                }
            }
        }

        /* - if at start of file marker, populate subfileName
         * - if at end of file marker, clear subfileName
         */
        if (sof || eof || sosf || eosf) {
            /* - if at end of file marker
             * - insert items if subfileName not empty
             * - after insert, clear contents
             */
            if (! subfileName.isEmpty()) {
                if (! alreadyLoaded) {
                    insert(subfileName,
                           contents,
                           datetime,
                           unofficialPart,
                           false/*generated*/,
                           false/*includeFile*/,
                           externalFile ? fileInfo.absoluteFilePath() : "",
                           _description);
                    if (contents.size()) {
                        if ((headerMissing = MissingHeader(missingHeaders())))
                            normalizeHeader(subfileName, headerMissing);
                        emit gui->messageSig(LOG_NOTICE, QObject::tr("MPD %1 '%2' with %3 lines loaded.")
                                                                     .arg(fileType()).arg(subfileName).arg(QString::number(size(subfileName))));
                    }
                    topLevelModel = false;
                    unofficialPart = UNOFFICIAL_UNKNOWN;
                }
                subfileIndx = stagedSubfiles.indexOf(subfileName);
                if (subfileIndx > NOT_FOUND)
                    stagedSubfiles.removeAt(subfileIndx);

                contents.clear();
            }

            /* - if at start of file marker
             * - set subfileName of new file
             * - else if at end of file marker, clear subfileName
             */
            if (sof || sosf) {
                if (sof) {
                    hdrNameNotFound   = true;
                    hdrAuthorNotFound = true;
                    hdrFILENotFound   = false; /* we are at the beginning of an LDraw submodel */
                    modelHeaderFinished = false;
                    subfileName = _fileRegExp[SOF_RX].cap(1);
                } else/*sosf*/ {

                    hdrNameNotFound = sosf = false;
                    partHeaderFinished = false;
                    subfileName = _fileRegExp[NAM_RX].cap(1).replace(": ","");
                    contents << smLine;
                }
                if (! alreadyLoaded) {
                    emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Loading MPD subfile '%1'...").arg(subfileName));
                }
                unofficialPart = UNOFFICIAL_UNKNOWN;
            } else if (eof || eosf) {
                /* - at the end of submodel file or inline part
                */
                subfileName.clear();
                hdrNameNotFound     = true; /* reset Name capture*/
                hdrAuthorNotFound   = true; /* reset Author capture*/
                hdrDescNotFound     = true; /* reset Description capture */
                if (eof) {
                    hdrFILENotFound = true; /* we are at the end of an LDraw submodel */
                    modelHeaderFinished = false;
                } else/*eosf*/ {
                    /* - if description found, add it to the start of the part's contents
                    */
                    if (!_description.isEmpty())
                        contents << _description;
                    /* - at the Name: of a new inline part so revert by 1 line to capture
                    */
                    lineIndx--;
                    eosf = false;
                }
                /* - clear description for next model or inline part
                */
                _description.clear();
            }
        } else if (! subfileName.isEmpty() && !smLine.isEmpty()) {
            /* - after start of file - subfileName not empty
             * - add line to contents
             */
            contents << smLine;
        }
    } // iterate stagedContents

    // at end of file - NOFILE tag not specified
    if ( ! subfileName.isEmpty() && ! contents.isEmpty()) {
        if (LDrawFile::contains(subfileName)) {
            emit gui->messageSig(LOG_TRACE, QObject::tr("MPD %1 '%2' already loaded").arg(fileType()).arg(subfileName));
        } else {
            insert(subfileName,
                   contents,
                   datetime,
                   unofficialPart,
                   false/*generated*/,
                   false/*includeFile*/,
                   externalFile ? fileInfo.absoluteFilePath() : "",
                   _description);
            if (contents.size()) {
                if ((headerMissing = MissingHeader(missingHeaders())))
                    normalizeHeader(subfileName, headerMissing);
                emit gui->messageSig(LOG_NOTICE, QObject::tr("MPD %1 '%2' with %3 lines loaded.")
                                                             .arg(fileType()).arg(subfileName).arg(QString::number(size(subfileName))));
            }
        }

        contents.clear();

        subfileIndx = stagedSubfiles.indexOf(subfileName);
        if (subfileIndx > NOT_FOUND)
            stagedSubfiles.removeAt(subfileIndx);
    } // at end of file

    // resolve outstanding subfiles
    if (stagedSubfiles.size()) {
        stagedSubfiles.removeDuplicates();

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("%1 unresolved staged %2 specified.")
                                                .arg(stagedSubfiles.size()).arg(stagedSubfiles.size() == 1 ? "subfile" : "subfiles"));
#endif
        QString projectPath = QDir::toNativeSeparators(fileInfo.absolutePath());

        for (const QString &subfile : stagedSubfiles) {
#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_DEBUG, QString("Processing staged subfile '%1'...").arg(subfile));
#endif
            if (LDrawFile::contains(subfile)) {
                QString subfileType = fileType(isUnofficialPart(subfile));
                emit gui->messageSig(LOG_TRACE, QObject::tr("MPD %1 '%2' already loaded.").arg(subfileType).arg(subfile));
                continue;
            }
            // current path
            if ((subfileFound = QFileInfo(projectPath + QDir::separator() + subfile).isFile())) {
                fileInfo = QFileInfo(projectPath + QDir::separator() + subfile);
            } else
            // file path
            if ((subfileFound = QFileInfo(subfile).isFile())){
                fileInfo = QFileInfo(subfile);
            }
            else
            // extended search - LDraw subfolder paths and extra search directorie paths
            if (Preferences::extendedSubfileSearch) {
                for (QString subFilePath : searchPaths){
                    if ((subfileFound = QFileInfo(subFilePath + QDir::separator() + subfile).isFile())) {
                        fileInfo = QFileInfo(subFilePath + QDir::separator() + subfile);
                        break;
                    }
                }
            }

            bool externalSubfile = true;
            if (!subfileFound) {
                QString partFile = subfile.toUpper();
                if (partFile.startsWith("S\\"))
                    partFile.replace("S\\","S/");
                PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(partFile.toLatin1().constData(), nullptr/*CurrentProject*/, false/*CreatePlaceholder*/, false/*SearchProjectFolder*/);
                if ((subfileFound = pieceInfo) && pieceInfo->IsPartType()) {
                    emit gui->messageSig(LOG_NOTICE, QObject::tr("Staged subfile '%1' is a part.").arg(subfile));
                } else
                if ((subfileFound = lcGetPiecesLibrary()->IsPrimitive(partFile.toLatin1().constData()))) {
                    lcLibraryPrimitive* Primitive = lcGetPiecesLibrary()->FindPrimitive(partFile.toLatin1().constData());
                    if (Primitive) {
                        emit gui->messageSig(LOG_NOTICE, QObject::tr("Staged subfile '%1' is a %2.")
                                                                     .arg(subfile).arg(Primitive->mSubFile ? QObject::tr("subpart") : QObject::tr("primitive")));
                    }
                }
                externalSubfile = !subfileFound;
            }

            if (!subfileFound) {
                emit gui->messageSig(LOG_NOTICE, QObject::tr("Staged subfile '%1' not found.").arg(subfile));
            } else if (externalSubfile) {
                emit gui->messageSig(LOG_NOTICE, QObject::tr("External subfile '%1' found.").arg(subfile));

                bool savedTopLevelModel  = topLevelModel;
                bool savedUnofficialPart = unofficialPart;
                topLevelModel            = false;

                loadMPDFile(fileInfo.absoluteFilePath(), true/*external*/);

                subfileIndx = stagedSubfiles.indexOf(subfile);
                if (subfileIndx > NOT_FOUND)
                    stagedSubfiles.removeAt(subfileIndx);

                topLevelModel  = savedTopLevelModel;
                unofficialPart = savedUnofficialPart;
            }
        }
    } // resolve outstanding subfiles

    _mpd = true;

    // restore last subfileName for final processing
    if (stagedSubfilesFound && subfileName.isEmpty())
        subfileName = QFileInfo(fileName).fileName();
    else
        subfileName = fileInfo.fileName();

#ifdef QT_DEBUG_MODE
    if (!stagedSubfiles.size())
        emit gui->messageSig(LOG_DEBUG, QString("All '%1' staged subfiles processed.").arg(subfileName));

    QHashIterator<QString, int> i(_ldcadGroups);
    while (i.hasNext()) {
        i.next();
        emit gui->messageSig(LOG_TRACE, QString("LDCad Groups: Name[%1], LineID[%2].")
                             .arg(i.key()).arg(i.value()));
    }
#endif

    emit gui->progressPermSetValueSig(lineCount);
    if (!stagedSubfiles.size()) {
        emit gui->progressPermMessageSig(QString());
        emit gui->progressPermStatusRemoveSig();
    }

    emit gui->messageSig(LOG_NOTICE, QObject::tr("MPD file '%1' with %2 lines loaded.")
                                                 .arg(subfileName).arg(lineCount));
}

void LDrawFile::loadLDRFile(const QString &filePath, const QString &fileName, bool externalFile)
{
    MissingHeader headerMissing = NoneMissing;

    std::function<int()> missingHeaders;
    missingHeaders = [this] ()
    {
        if (hdrNameNotFound && hdrAuthorNotFound)
            return BothMissing;
        else if (hdrNameNotFound)
            return NameMissing;
        else if (hdrAuthorNotFound)
            return AuthorMissing;
        return NoneMissing;
    };

    auto fileType = [&,this] (int isUnofficial = 0)
    {
        if (isUnofficial)
            return LDrawUnofficialType[unofficialPart];
        else
            return topLevelModel
                    ? QObject::tr("model")
                    : unofficialPart
                      ? LDrawUnofficialType[unofficialPart]
                      : QObject::tr("subfile");
    };

    QString fullName;
    if (fileName.isEmpty())
        fullName = QDir::toNativeSeparators(filePath);
    else
        fullName = QDir::toNativeSeparators(filePath + "/" + fileName);

    QFileInfo   fileInfo(fullName);

    if (_subFiles[fileInfo.fileName()]._contents.isEmpty()) {

        QFile file(fullName);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            emit gui->messageSig(LOG_ERROR,QObject::tr("Cannot read ldr file %1<br>%2")
                                                       .arg(fullName)
                                                       .arg(file.errorString()));
            return;
        }

        int subfileIndx;
        bool alreadyLoaded;
        bool subfileFound        = false;
        bool partHeaderFinished  = false;
        bool sosf = false;
        bool eosf = false;

        if (topLevelModel) {
            topFileNotFound            = true;
            topHeaderFinished          = false;
            hdrDescNotFound            = true;
            hdrCategNotFound           = true;
            helperPartsNotFound        = true;
            metaLoadUnoffPartsNotFound = true;
            metaBuildModNotFund        = true;
            metaFinalModelNotFound     = true;
            metaStartPageNumNotFound   = true;
            metaStartStepNumNotFound   = true;
            descriptionLine            = 0;
        }

        hdrNameNotFound   = true;
        hdrAuthorNotFound = true;
        unofficialPart = UNOFFICIAL_UNKNOWN;

        QString subfileName, subFile, smLine;
        QStringList stagedContents, stagedSubfiles, contents, tokens;
        QStringList searchPaths = Preferences::ldSearchDirs;

        QString ldrawPath = QDir::toNativeSeparators(Preferences::ldrawLibPath);
        if (!searchPaths.contains(ldrawPath + QDir::separator() + "MODELS",Qt::CaseInsensitive))
            searchPaths.append(ldrawPath + QDir::separator() + "MODELS");
        if (!searchPaths.contains(ldrawPath + QDir::separator() + "PARTS",Qt::CaseInsensitive))
            searchPaths.append(ldrawPath + QDir::separator() + "PARTS");
        if (!searchPaths.contains(ldrawPath + QDir::separator() + "P",Qt::CaseInsensitive))
            searchPaths.append(ldrawPath + QDir::separator() + "P");
        if (!searchPaths.contains(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "PARTS",Qt::CaseInsensitive))
            searchPaths.append(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "PARTS");
        if (!searchPaths.contains(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "P",Qt::CaseInsensitive))
            searchPaths.append(ldrawPath + QDir::separator() + "UNOFFICIAL" + QDir::separator() + "P");

        /* Read it in the first time to put into fileList in order of appearance */

        QTextStream in(&file);
        in.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
        bool checked = false;
        while ( ! in.atEnd()) {
            smLine = in.readLine(0);
            if (!smLine.isEmpty())
                stagedContents << smLine.trimmed();
            if (!checked) {
                checked = (smLine.contains(_fileRegExp[LDR_RX]) || smLine.contains(_fileRegExp[NAM_RX]));
                if (smLine.contains(_fileRegExp[SOF_RX])) {
                    file.close();
                    stagedContents.clear();
                    QString scModelType = QString(fileType())[0].toUpper() + QString(fileType()).right(QString(fileType()).size() - 1);
                    emit gui->messageSig(LOG_INFO_STATUS, QString(scModelType + " file %1 identified as Multi-Part LDraw System (MPD) Document").arg(fileInfo.fileName()));
                    loadMPDFile(fileInfo.absoluteFilePath());
                    return;
                }
            }
        }
        file.close();

        int lineCount = stagedContents.size();

        if (topLevelModel)
            emit gui->progressBarPermInitSig();
        else
            emit gui->progressBarPermReset();
        emit gui->progressPermRangeSig(1, lineCount);
        emit gui->progressPermMessageSig(QString("Loading LDR subfile '%1'...").arg(fileInfo.fileName()));

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Stage Contents Size: %1").arg(lineCount));
#endif

        QDateTime datetime = fileInfo.lastModified();

        /* read it a second time to find submodels and check for completeness*/

        for (int lineIndx = 0; lineIndx < lineCount; lineIndx++) {

            smLine = stagedContents.at(lineIndx).trimmed();

            emit gui->progressPermSetValueSig(lineIndx);

            if (smLine.isEmpty())
                continue;

            if (subfileName.isEmpty() && hdrNameNotFound && (topLevelModel || !smLine.isEmpty())) {
                sosf = smLine.contains(_fileRegExp[NAM_RX]);
                if (!sosf) {
                    if (!smLine.contains(_fileRegExp[AUT_RX]))
                        descriptionLine = lineIndx;
                    contents << smLine;
                }
            } else if (! hdrNameNotFound && smLine.startsWith("0")) {
                if ((eosf = smLine.contains(_fileRegExp[NAM_RX]))) {
                    const QString &lastLine = contents.last();
                    if (lastLine.contains(_fileRegExp[DES_RX])) {
                        _description = contents.takeLast(); // for inline files
                    }
                } else if ((eosf = lineIndx == lineCount - 1 && smLine == "0"))
                    contents << smLine;                     // for external files
            }

            // load LDCad groups
            if (!ldcadGroupsLoaded) {
                if(smLine.contains(_fileRegExp[LDG_RX])){
                    insertLDCadGroup(_fileRegExp[LDG_RX].cap(3),_fileRegExp[LDG_RX].cap(1).toInt());
                    insertLDCadGroup(_fileRegExp[LDG_RX].cap(2),_fileRegExp[LDG_RX].cap(1).toInt());
                } else if (smLine.contains("0 STEP") || smLine.contains("0 ROTSTEP")) {
                    ldcadGroupsLoaded = true;
                }
            }

            split(smLine,tokens);

            // subfile check;
            if ((subfileFound = tokens.size() == 15 && tokens.at(0) != "0")) {
                topHeaderFinished = partHeaderFinished = true;
                subFile = tokens.at(14);
                if ((headerMissing = MissingHeader(missingHeaders()))) {
                    if (headerMissing == NameMissing || headerMissing == BothMissing) {
                        const QString missing = QString("0 Name: %1").arg(fileInfo.fileName());
                        sosf = missing.contains(_fileRegExp[NAM_RX]);
                        if (hdrAuthorNotFound) {
                            contents.insert(0,missing);
                        } else {
                            for (int lineNumber = 0; lineNumber < contents.size(); lineNumber++) {
                                const QString &line = contents.at(lineNumber);
                                if (line.contains(_fileRegExp[AUT_RX])) {
                                    contents.insert(lineNumber,missing);
                                    break;
                                }
                            }
                        }
                        contents.takeLast();
                    }
                }
            } else if (isSubstitute(smLine,subFile)) {
                subfileFound = !subFile.isEmpty();
            }
            if (subfileFound) {
                PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(subFile.toLatin1().constData(), nullptr, false, false);
                if (! pieceInfo && ! LDrawFile::contains(subFile) && ! stagedSubfiles.contains(subFile)) {
                    stagedSubfiles.append(subFile);
                }
                // determine if helper part
                if (helperPartsNotFound) {
                    if (ExcludedParts::isExcludedHelperPart(subFile)) {
                        helperPartsNotFound = false;
                        _helperPartsNotInArchive = ! pieceInfo;
                    }
                }
            }

            if (!topHeaderFinished) {
                // One time populate top level file name
                if (topFileNotFound) {
                    _file = QString(fileInfo.fileName()).replace(fileInfo.suffix(),"");
                    topFileNotFound = false;
                }

                if (!sosf || !partHeaderFinished) {
                    if (hdrDescNotFound && lineIndx == descriptionLine) {
                        if (smLine.contains(_fileRegExp[DES_RX]) && ! isHeader(smLine)) {
                            _description = QString(smLine).remove(0, 2);
                            if (topLevelModel)
                                Preferences::publishDescription = _description;
                            hdrDescNotFound = false;
                        } else
                            _description = QFileInfo(subfileName).completeBaseName();
                    }

                    if (hdrNameNotFound) {
                        if (smLine.contains(_fileRegExp[NAM_RX])) {
                            if (topLevelModel)
                                _name = _fileRegExp[NAM_RX].cap(1).replace(": ","");
                            hdrNameNotFound = false;
                        }
                    }

                    if (hdrAuthorNotFound) {
                        if (smLine.contains(_fileRegExp[AUT_RX])) {
                            if (topLevelModel) {
                                _author = _fileRegExp[AUT_RX].cap(1).replace(": ","");
                                Preferences::defaultAuthor = _author;
                            }
                            hdrAuthorNotFound = false;
                        }
                    }

                    if (unofficialPart == UNOFFICIAL_UNKNOWN) {
                        unofficialPart = getUnofficialFileType(smLine);
                        if (unofficialPart) {
                            emit gui->messageSig(LOG_TRACE, QObject::tr("Subfile '%1' spcified as %2.")
                                                                        .arg(subfileName).arg(fileType()));
                        }
                    }

                    // One time populate model category
                    if (hdrCategNotFound) {
                        if (smLine.contains(_fileRegExp[CAT_RX])) {
                            if (topLevelModel)
                                _category = _fileRegExp[CAT_RX].cap(1);
                            hdrCategNotFound = false;
                        }
                    }

                    // Check for include file
                    if (smLine.contains(_fileRegExp[INC_RX])) {
                        const QString filePath = LPub::getFilePath(_fileRegExp[INC_RX].cap(1));
                        if (!filePath.isEmpty())
                            loadIncludeFile(filePath);
                    }

                    // Check meta commands
                    if (!isComment(smLine))
                        processMetaCommand(tokens);
                }
            } // topHeaderFinished

            if ((alreadyLoaded = LDrawFile::contains(subfileName))) {
                emit gui->messageSig(LOG_TRACE, QString("LDR %1 '%2' already loaded.").arg(fileType()).arg(subfileName));
                subfileIndx = stagedSubfiles.indexOf(subfileName);
                if (subfileIndx > NOT_FOUND)
                    stagedSubfiles.removeAt(subfileIndx);
            }

            // processing inlined parts
            if (! topLevelModel && ! hdrNameNotFound) {
                if (hdrAuthorNotFound) {
                    if (smLine.contains(_fileRegExp[AUT_RX]))
                        hdrAuthorNotFound = false;
                }
                if (! partHeaderFinished && unofficialPart == UNOFFICIAL_UNKNOWN) {
                    unofficialPart = getUnofficialFileType(smLine);
                    if (unofficialPart) {
                        emit gui->messageSig(LOG_TRACE, QObject::tr("Inline file '%1' spcified as %2.")
                                                                    .arg(subfileName).arg(fileType()));
                    }
                }
            }

            /* - if at start of file marker, populate subfileName
             * - if at end of file marker, clear subfileName
             */
            if (sosf || eosf) {
                /* - if at end of file marker
                 * - insert items if subfileName not empty
                 * - after insert, clear contents
                 */
                if (! subfileName.isEmpty()) {
                    if (! alreadyLoaded) {
                        insert(subfileName,
                               contents,
                               datetime,
                               unofficialPart,
                               false/*generated*/,
                               false/*includeFile*/,
                               externalFile ? fileInfo.absoluteFilePath() : "",
                               _description);
                        if (contents.size()) {
                            if ((headerMissing = MissingHeader(missingHeaders())))
                                normalizeHeader(subfileName, headerMissing);
                            emit gui->messageSig(LOG_NOTICE, QObject::tr("MPD %1 '%2' with %3 lines loaded.")
                                                                         .arg(fileType()).arg(subfileName).arg(QString::number(size(subfileName))));
                        }
                        topLevelModel = false;
                        unofficialPart = UNOFFICIAL_UNKNOWN;
                    }
                    subfileIndx = stagedSubfiles.indexOf(subfileName);
                    if (subfileIndx > NOT_FOUND)
                        stagedSubfiles.removeAt(subfileIndx);

                    contents.clear();
                }

                /* - if at start of file marker
                 * - set subfileName of new file
                 * - else if at end of file marker, clear subfileName
                 */
                if (sosf) {
                    unofficialPart  = UNOFFICIAL_UNKNOWN;
                    hdrNameNotFound = sosf = false;
                    partHeaderFinished = subfileFound ? true : false;
                    subfileName = _fileRegExp[NAM_RX].cap(1).replace(": ","");
                    contents << smLine;
                    if (! alreadyLoaded) {
                        emit gui->messageSig(LOG_INFO_STATUS, QObject::tr("Loading LDR '%1'...").arg(subfileName));
                    }
                } else if (eosf) {
                    /* - at the end of inline part
                    */
                    subfileName.clear();
                    hdrNameNotFound   = true; /* reset Name capture*/
                    hdrAuthorNotFound = true; /* reset Author capture*/
                    hdrDescNotFound   = true; /* reset Description capture*/
                    /* - if description found, add it to the start of the part's contents
                    */
                    if (!_description.isEmpty())
                        contents << _description;
                    /* - clear description for next part
                    */
                    _description.clear();
                    /* - at the Name: of a new inline part so revert by 1 line to capture
                    */
                    lineIndx--;
                    eosf = false;
                }
            } else if (! subfileName.isEmpty() && !smLine.isEmpty()) {
                /* - after start of file - subfileName not empty
                 * - add line to contents
                 */
                contents << smLine;
            }
        } // iterate stagedContents

        // at end of file
        if ( ! subfileName.isEmpty() && ! contents.isEmpty()) {
            if (LDrawFile::contains(subfileName)) {
                emit gui->messageSig(LOG_TRACE, QObject::tr("LDR %1 '%2' already loaded.").arg(fileType()).arg(subfileName));
            } else {
                insert(subfileName,
                       contents,
                       datetime,
                       unofficialPart,
                       false/*generated*/,
                       false/*includeFile*/,
                       externalFile ? fileInfo.absoluteFilePath() : "",
                       _description);
                if (contents.size()) {
                    if ((headerMissing = MissingHeader(missingHeaders())))
                        normalizeHeader(subfileName, headerMissing);
                    emit gui->messageSig(LOG_NOTICE, QObject::tr("LDR  %1 '%2' with %3 lines loaded.")
                                                                 .arg(fileType()).arg(subfileName).arg(QString::number(size(subfileName))));
                }
            }

            contents.clear();

            subfileIndx = stagedSubfiles.indexOf(subfileName);
            if (subfileIndx > NOT_FOUND)
                stagedSubfiles.removeAt(subfileIndx);
        } // at end of file

        // resolve outstanding subfiles
        if (stagedSubfiles.size()) {
            stagedSubfiles.removeDuplicates();

#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_DEBUG, QString("%1 unresolved staged %2 specified.")
                                                    .arg(stagedSubfiles.size()).arg(stagedSubfiles.size() == 1 ? "subfile" : "subfiles"));
#endif
            QString projectPath = QDir::toNativeSeparators(fileInfo.absolutePath());

            for (QString subfile : stagedSubfiles) {
#ifdef QT_DEBUG_MODE
                emit gui->messageSig(LOG_DEBUG, QString("Processing staged subfile '%1'...").arg(subfile));
#endif
                if (LDrawFile::contains(subfile)) {
                    QString subfileType = fileType(isUnofficialPart(subfile));
                    emit gui->messageSig(LOG_TRACE, QObject::tr("LDR %1 '%2' already loaded.").arg(subfileType).arg(subfile));
                    continue;
                }
                // current path
                if ((subfileFound = QFileInfo(projectPath + QDir::separator() + subfile).isFile())) {
                    fileInfo = QFileInfo(projectPath + QDir::separator() + subfile);
                } else
                // file path
                if ((subfileFound = QFileInfo(subfile).isFile())){
                    fileInfo = QFileInfo(subfile);
                }
                else
                // extended search - LDraw subfolder paths and extra search directorie paths
                if (Preferences::extendedSubfileSearch) {
                    for (QString subFilePath : searchPaths){
                        if ((subfileFound = QFileInfo(subFilePath + QDir::separator() + subfile).isFile())) {
                            fileInfo = QFileInfo(subFilePath + QDir::separator() + subfile);
                            break;
                        }
                    }
                }

                bool externalSubfile = true;
                if (!subfileFound) {
                    QString partFile = subfile.toUpper();
                    if (partFile.startsWith("S\\"))
                        partFile.replace("S\\","S/");
                    PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(partFile.toLatin1().constData(), nullptr/*CurrentProject*/, false/*CreatePlaceholder*/, false/*SearchProjectFolder*/);
                    if ((subfileFound = pieceInfo) && pieceInfo->IsPartType()) {
                        emit gui->messageSig(LOG_NOTICE, QObject::tr("Staged subfile '%1' is a part.").arg(subfile));
                    } else
                    if ((subfileFound = lcGetPiecesLibrary()->IsPrimitive(partFile.toLatin1().constData()))) {
                        lcLibraryPrimitive* Primitive = lcGetPiecesLibrary()->FindPrimitive(partFile.toLatin1().constData());
                        if (Primitive) {
                            emit gui->messageSig(LOG_NOTICE, QObject::tr("Staged subfile '%1' is a %2.")
                                                 .arg(subfile).arg(Primitive->mSubFile ? QObject::tr("subpart") : QObject::tr("primitive")));
                        }
                    }
                    externalSubfile = !subfileFound;
                }

                if (!subfileFound) {
                    emit gui->messageSig(LOG_NOTICE, QObject::tr("Staged subfile '%1' not found.").arg(subfile));
                } else {
                    bool savedTopLevelModel  = topLevelModel;
                    bool savedUnofficialPart = unofficialPart;
                    topLevelModel            = false;

                    emit gui->messageSig(LOG_NOTICE, QObject::tr("External subfile '%1' found.").arg(subfile));

                    loadLDRFile(fileInfo.absoluteFilePath(), QString(), true/*external*/);

                    subfileIndx = stagedSubfiles.indexOf(subfile);
                    if (subfileIndx > NOT_FOUND)
                        stagedSubfiles.removeAt(subfileIndx);

                    topLevelModel  = savedTopLevelModel;
                    unofficialPart = savedUnofficialPart;
                }
            }
        } // resolve outstanding subfiles

        _mpd = false;

        subfileName = QFileInfo(fullName).fileName();

#ifdef QT_DEBUG_MODE
        if (!stagedSubfiles.size())
            emit gui->messageSig(LOG_DEBUG, QString("All '%1' staged subfiles processed.").arg(subfileName));

        QHashIterator<QString, int> i(_ldcadGroups);
        while (i.hasNext()) {
            i.next();
            emit gui->messageSig(LOG_TRACE, QString("LDCad Groups: Name[%1], LineID[%2].")
                                 .arg(i.key()).arg(i.value()));
        }
#endif

        emit gui->progressPermSetValueSig(lineCount);
        if (!stagedSubfiles.size()) {
            emit gui->progressPermMessageSig(QString());
            emit gui->progressPermStatusRemoveSig();
        }

        emit gui->messageSig(LOG_NOTICE, QObject::tr("LDR file '%1' with %2 lines loaded.")
                                                     .arg(subfileName).arg(lineCount));
    }
}

bool LDrawFile::saveFile(const QString &fileName)
{
    _savedLines = 0;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool rc;
    if (isIncludeFile(fileName)) {
      rc = saveIncludeFile(fileName);
    } else {
      rc = saveModelFile(fileName);
    }
    QApplication::restoreOverrideCursor();
    return rc;
}

bool LDrawFile::mirrored(
  const QStringList &tokens)
{
  if (tokens.size() != 15) {
    return false;
  }
  /* 5  6  7
     8  9 10
    11 12 13 */

  float a = tokens[5].toFloat();
  float b = tokens[6].toFloat();
  float c = tokens[7].toFloat();
  float d = tokens[8].toFloat();
  float e = tokens[9].toFloat();
  float f = tokens[10].toFloat();
  float g = tokens[11].toFloat();
  float h = tokens[12].toFloat();
  float i = tokens[13].toFloat();

  float a1 = a*(e*i - f*h);
  float a2 = b*(d*i - f*g);
  float a3 = c*(d*h - e*g);

  float det = (a1 - a2 + a3);

  return det < 0;
  //return a*(e*i - f*h) - b*(d*i - f*g) + c*(d*h - e*g) < 0;
}

void LDrawFile::addCustomColorParts(const QString &mcFileName,bool autoAdd)
{
  if (!Preferences::enableFadeSteps && !Preferences::enableHighlightStep)
      return;
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
  if (f != _subFiles.end()) {
    // get content size
    int j = f->_contents.size();
    // process submodel content...
    for (int i = 0; i < j; i++) {
      QStringList tokens;
      QString line = f->_contents[i];
      split(line,tokens);
      // we interrogate substitue files
      if (tokens.size() >= 6 &&
          tokens[0] == "0" &&
         (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
          tokens[2] == "PLI" &&
          tokens[3] == "BEGIN" &&
          tokens[4] == "SUB") {
        // do we have a color file ?
        if (LDrawColourParts::isLDrawColourPart(tokens[5])) {
          // parse the part lines for custom sub parts
          for (++i; i < j; i++) {
            split(f->_contents[i],tokens);
            if (tokens.size() == 15 && tokens[0] == "1") {
              if (contains(tokens[14])) {
                // we have custom part so let's add it to the custom part list
                LDrawColourParts::addLDrawColorPart(tokens[14]);
                // now lets check if the custom part has any sub parts
                addCustomColorParts(tokens[14],true/*autoadd*/);
              }
            } else if (tokens.size() == 4 &&
                       tokens[0] == "0" &&
                      (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
                      (tokens[2] == "PLI" || tokens[2] == "PART") &&
                       tokens[3] == "END") {
              break;
            }
          }
        }
      } else if (autoAdd) {
        // parse the the add part lines for custom sub parts
        for (; i < j; i++) {
          split(f->_contents[i],tokens);
          if (tokens.size() == 15 && tokens[0] == "1") {
            if (contains(tokens[14])) {
              // do we have a color part ?
              if (tokens[1] != LDRAW_MAIN_MATERIAL_COLOUR &&
                  tokens[1] != LDRAW_EDGE_MATERIAL_COLOUR)
                LDrawColourParts::addLDrawColorPart(tokens[14]);
              // now lets check if the part has any sub parts
              addCustomColorParts(tokens[14],true/*autoadd*/);
            }
          } else if (f->_contents[i] == "0 //Segments"){
            // we have reached the custpm part segments so break
            break;
          }
        }
      } else if (tokens.size() == 15 && tokens[0] == "1") {
        bool containsSubFile = contains(tokens[14]);
        if (containsSubFile) {
          addCustomColorParts(tokens[14],false/*autoadd*/);
        }
      }
    }
  }
}

void LDrawFile::countInstances(
  const QString &mcFileName,
        bool     firstStep,
        bool     isMirrored,
        bool     callout)
{
  QMutexLocker ldrawLocker(&ldrawMutex);

  //logTrace() << QString("countInstances, File: %1, Mirrored: %2, Callout: %3").arg(mcFileName,(isMirrored?"Yes":"No"),(callout?"Yes":"No"));

  /*
   * For countInstances, the BuildMod behaviour creates a sequential
   * list (Vector<int>) of all the steps in the loaded model file.
   * Step indices are appended to the _buildModStepIndexes register.
   * Each step index contains the step's parent model and the line number
   * of the STEP meta command indicating the top of the 'next' step.
   * The buildModLevel flag uses getLevel() function to determine the current
   * BuildMod when mods are nested.
   */
  Where top(mcFileName, getSubmodelIndex(mcFileName), 0);

  gui->skipHeader(top);

  Where topOfStep(top); // set after skipHeader

  setBuildModsCount(topOfStep.modelName, BM_INIT);

  bool partsAdded        = false;
  bool noStep            = false;
  bool isInsertStep      = false;
  bool stepIgnore        = false;
  bool buildModIgnore    = false;
  int  buildModState     = BM_NONE;
  int  buildModStepIndex = BM_NONE;
  int  buildModPartCount = 0;
  int  buildModAction    = BuildModNoActionRc;

  QMap<int, QString>      buildModKeys;
  QMap<int, QVector<int>> buildModAttributes;

  auto loadBuildMods = [this] ()
  {
      int stepIndexes = _buildModStepIndexes.size();

      bool result =
              _loadBuildMods && (Gui::suspendFileDisplay ||
              (stepIndexes >= _buildModPrevStepIndex &&
               stepIndexes <= _buildModNextStepIndex));

      return result;
  };

  auto insertAttribute = [this, &top] (QMap<int, QVector<int>> &buildModAttributes, int indx)
  {
      QMap<int, QVector<int>>::iterator i = buildModAttributes.find(buildModLevel);
      if (i == buildModAttributes.end()) {
          QVector<int> modAttributes = { 0, 0, 0, 1, 0, top.modelIndex, 0, 0 };
          modAttributes[indx] = top.lineNumber;
          buildModAttributes.insert(buildModLevel, modAttributes);
      } else {
          i.value()[indx] = top.lineNumber;
      }
  };

  auto insertBuildModification = [this, &buildModPartCount, &buildModAttributes, &buildModKeys, &topOfStep] (int level)
  {
      int buildModStepIndex = _buildModStepIndexes.indexOf({topOfStep.modelIndex, topOfStep.lineNumber});

      QString buildModKey = buildModKeys.value(level);
      QVector<int> modAttributes = { 0, 0, 0, 0, 0, -1, 0, 0 };

      QMap<int, QVector<int>>::iterator i = buildModAttributes.find(level);
      if (i == buildModAttributes.end()) {
          emit gui->messageSig(LOG_ERROR, QString("COUNTINSTANCE - WARNING - Invalid BuildMod Entry for key: %1").arg(buildModKey));
          return;
      }
      modAttributes = i.value();

      modAttributes[BM_DISPLAY_PAGE_NUM] = Gui::displayPageNum;
      modAttributes[BM_STEP_PIECES]      = buildModPartCount;
      modAttributes[BM_MODEL_NAME_INDEX] = topOfStep.modelIndex;
      modAttributes[BM_MODEL_LINE_NUM]   = topOfStep.lineNumber;
//*
#ifdef QT_DEBUG_MODE
      if (Gui::suspendFileDisplay) {
          emit gui->messageSig(LOG_DEBUG, QString(
                               "Insert CountInst BuildMod StepIndex: %1, "
                               "Attributes: %2 %3 %4 %5 %6 %7 %8, "
                               "ModKey: '%9', "
                               "Level: %10, "
                               "Model: %11")
                               .arg(buildModStepIndex)                      // Attribute Default Initial:
                               .arg(modAttributes.at(BM_BEGIN_LINE_NUM))    // 0         0       this
                               .arg(modAttributes.at(BM_ACTION_LINE_NUM))   // 1         0       this
                               .arg(modAttributes.at(BM_END_LINE_NUM))      // 2         0       this
                               .arg(modAttributes.at(BM_DISPLAY_PAGE_NUM))  // 3         0       this
                               .arg(modAttributes.at(BM_STEP_PIECES))       // 4         0       this
                               .arg(modAttributes.at(BM_MODEL_NAME_INDEX))  // 5        -1       this
                               .arg(modAttributes.at(BM_MODEL_LINE_NUM))    // 6         0       this
                               .arg(buildModKey)
                               .arg(level)
                               .arg(topOfStep.modelName));
      }
#endif
//*/
      insertBuildMod(buildModKey,
                     modAttributes,
                     buildModStepIndex);
  };

  if (firstStep) {
    if (top.modelName.toLower() == topLevelFile().toLower()) {
      buildModLevel = BM_BASE_LEVEL;
      _currentLevels.clear();
      _buildModStepIndexes.clear();
    }

    if (! _buildModStepIndexes.contains({ top.modelIndex, top.lineNumber }))
      _buildModStepIndexes.append({ top.modelIndex, top.lineNumber });
  }

  QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(top.modelName.toLower());
  if (f != _subFiles.end()) {
    if (f->_beenCounted) {
      if (isMirrored) {
        ++f->_mirrorInstances;
      } else {
        ++f->_instances;
      }
      // children
      for (int i = 0; i < f->_subFileIndexes.size(); i++) {
        QMap<QString, LDrawSubFile>::iterator s = _subFiles.find(getSubmodelName(f->_subFileIndexes.at(i)));
        if (s != _subFiles.end()) {
          if (isMirrored) {
            ++s->_mirrorInstances;
          } else {
            ++s->_instances;
          }
        }
      }
      return;
    }

    // get content size and reset numSteps
    int j = f->_contents.size();
    f->_numSteps = 0;

    // process submodel content...
    for (; top.lineNumber < j; top.lineNumber++) {
      QStringList tokens;
      QString line = f->_contents[top.lineNumber];
      split(line,tokens);

      //lpub3d ignore part - so set ignore step
      if (tokens.size() > 1 && tokens[0] == "0") {
        if (tokens[1] == "!LPUB" || tokens[1] == "LPUB") {
          // pli part meta commands
          if (tokens.size() == 5) {
            if ((tokens[2] == "PART"  || tokens[2] == "PLI") &&
              tokens[3] == "BEGIN"  &&
              tokens[4] == "IGN") {
              stepIgnore = true;
            }
            // build modification - commands
            else if (tokens[2] == "BUILD_MOD") {
              if (tokens[3] == "BEGIN") {
                if (! Preferences::buildModEnabled) {
                    buildModIgnore = true;
                    continue;
                }
                buildModLevel = getLevel(tokens[4], BM_BEGIN);
                buildModIgnore = false;
                buildModAction = BuildModApplyRc;
                setBuildModsCount(topOfStep.modelName);
                if (loadBuildMods()) {
                  buildModKeys.insert(buildModLevel, tokens[4]);
                  insertAttribute(buildModAttributes, BM_BEGIN_LINE_NUM);
                }
                buildModState = BM_BEGIN;
              } else if (tokens[3] == "ADD" || tokens[3] == "REMOVE") {
                if (loadBuildMods()) {
                  int newAction = tokens[3] == "ADD" ? BuildModApplyRc : BuildModRemoveRc;
                  if (buildModContains(tokens[4])) {
                    buildModStepIndex = getBuildModStepIndex(topOfStep.modelIndex, topOfStep.lineNumber);
                    if (buildModStepIndex > BM_INVALID_INDEX) {
                      buildModAction = getBuildModAction(tokens[4], buildModStepIndex);
                      if (buildModAction && (buildModAction != newAction)) {
                        setBuildModAction(tokens[4], buildModStepIndex, newAction);
                      }
                    }
                  } else {
                    emit gui->messageSig(LOG_TRACE, QString("COUNTINSTANCE - WARNING - BuildMod for key '%1' not found.").arg(tokens[4]));
                  }
                }
              }
            } // build modification commands
          // we have a meta command...
          } else if (tokens.size() == 4) {
            // lpub3d part - so set include step
            if ((tokens[2] == "PART" || tokens[2] == "PLI") &&
                 tokens[3] == "END") {
              stepIgnore = false;
            // called out
            /* Sorry, but models that are callouts are not counted as instances */
            } else if (tokens[2] == "CALLOUT" &&
                       tokens[3] == "BEGIN") {
              callout    = true;
              partsAdded = true;
              //process callout content
              for (++top.lineNumber; top.lineNumber < j; top.lineNumber++) {
                split(f->_contents[top.lineNumber],tokens);
                if (tokens.size() == 15 && tokens[0] == "1") {
                  if (contains(tokens[14],false/*searchAll*/) && ! stepIgnore && ! buildModIgnore) {
                    // add contains 'child' index to parent list
                    if (isSubmodel(tokens[14])) {
                      const int subFileIndex = getSubmodelIndex(tokens[14]);
                      if (top.modelIndex && !f->_subFileIndexes.contains(subFileIndex))
                        f->_subFileIndexes.append(subFileIndex);
                    }
                    countInstances(tokens[14], true/*firstStep*/, mirrored(tokens), callout);
                  }
                } else if (tokens.size() == 4 && tokens[0] == "0" &&
                          (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
                          tokens[2] == "CALLOUT" &&
                          tokens[3] == "END") {
                  callout = false;
                  break;
                }
              }
            // build modification - END_MOD and END commands
            } else if (tokens[2] == "BUILD_MOD") {
              if (tokens[3] == "END_MOD") {
                if (! Preferences::buildModEnabled)
                    continue;
                if (buildModAction == BuildModApplyRc)
                    buildModIgnore = true;
                if (loadBuildMods())
                  insertAttribute(buildModAttributes, BM_ACTION_LINE_NUM);
                buildModState = BM_END_MOD;
              } else if (tokens[3] == "END") {
                if (! Preferences::buildModEnabled)
                    continue;
                if (loadBuildMods() && buildModState == BM_END_MOD)
                  insertAttribute(buildModAttributes, BM_END_LINE_NUM);
                buildModLevel  = getLevel(QString(), BM_END);
                if (buildModLevel == BM_BASE_LEVEL || buildModLevel == _currentLevels.size())
                  buildModIgnore = false;
                buildModState  = BM_END;
              }
              // page insert, e.g. dispaly model, bom
            } else if (tokens[2] == "INSERT" &&
                    (tokens[3] == "PAGE" || tokens[3] == "COVER_PAGE")) {
              isInsertStep = partsAdded = true;
            } else if (tokens[1] == "BUFEXCHG") {}
          } // lines with 4 elements
        // no step
        } else if (tokens.size() == 3 &&
                  (tokens[2] == "NOSTEP" || tokens[2] == "NOFILE")) {
            noStep = true;
        // LDraw step or rotstep - so check if parts added
        } else if (tokens.size() >= 2 &&
                  (tokens[1] == "STEP" || tokens[1] == "ROTSTEP")) {
          if (! noStep) {
            if (partsAdded) {
              // parts added - increment step
              int incr = (((isMirrored && f->_mirrorInstances == 0) ||
                           (! isMirrored && f->_instances == 0)) && ! isInsertStep);
              f->_numSteps += incr;
            }
            // set step index for occurrences of STEP or ROTSTEP not ignored by BuildMod
            if (! buildModIgnore) {
              _buildModStepIndexes.append({ top.modelIndex, top.lineNumber });
              // build modification inserts
              if (loadBuildMods() && buildModKeys.size()) {
                for (int level : buildModKeys.keys())
                  insertBuildModification(level);
              }
            }
            buildModLevel     = BM_BASE_LEVEL;
            buildModState     = BM_NONE;
            buildModAction    = BuildModNoActionRc;
            buildModIgnore    = false;
            if (loadBuildMods()) {
              buildModPartCount = 0;
              buildModKeys.clear();
              buildModAttributes.clear();
            }
          } // not nostep
          // set top of next step
          topOfStep = top;
          // reset partsAdded, noStep and emptyLines
          isInsertStep = partsAdded = false;
          noStep = false;
        } // step or rotstep
      // check if subfile and process
      } else if (tokens.size() == 15 && tokens[0] >= "1" && tokens[0] <= "5") {
        if (! stepIgnore && ! buildModIgnore) {
          buildModPartCount++;
          if (contains(tokens[14],false/*searchAll*/)) {
            // add contains 'child' index to parent list
            if (isSubmodel(tokens[14])) {
              const int subFileIndex = getSubmodelIndex(tokens[14]);
              if (top.modelIndex && !f->_subFileIndexes.contains(subFileIndex))
                f->_subFileIndexes.append(subFileIndex);
            }
            countInstances(tokens[14], true /*firstStep*/, mirrored(tokens), callout);
          }
        }
        partsAdded = true;
      } // part line
    } // for each line

    // increment steps and add BuildMod step index if parts added in the last step of the sub/model and not ignored by BuildMod
    if (partsAdded && ! noStep) {
      int incr = (((isMirrored && f->_mirrorInstances == 0) ||
                   (! isMirrored && f->_instances == 0)) && ! isInsertStep);
      f->_numSteps += incr;
      if (! buildModIgnore) {
        _buildModStepIndexes.append({ top.modelIndex, top.lineNumber });
        // insert buildMod entries at end of content
        if (loadBuildMods() && buildModKeys.size()) {
          for (int level : buildModKeys.keys())
            insertBuildModification(level);
        }
      }
    } // partsAdded && ! noStep

    if ( ! callout) {
      if (isMirrored) {
        ++f->_mirrorInstances;
      } else {
        ++f->_instances;
      }
    } // callout
  } // subfile end

  f->_beenCounted = true;
}

void LDrawFile::countInstances()
{
  for (int i = 0; i < _subFileOrder.size(); i++) {
    QString fileName = _subFileOrder.at(i).toLower();
    QMap<QString, LDrawSubFile>::iterator it = _subFiles.find(fileName);
    it->_instances = 0;
    it->_mirrorInstances = 0;
    it->_beenCounted = false;
  }

#ifdef QT_DEBUG_MODE
  QElapsedTimer timer;
  timer.start();
#endif

  /*
   * For countInstances, the BuildMod behaviour creates a sequential
   * list (Vector<QString>) of all the steps in the loaded model file.
   * The buildMod flag uses a multilevel (_currentLevels) framework to
   * determine the current BuildMod when mods are nested.
   */
  QFuture<void> future = QtConcurrent::run([this]() {
      countInstances(topLevelFile(),
                     true/*firstStep*/,
                     false/*isMirrored*/,
                     false/*callout*/);
  });

  future.waitForFinished();

  _buildModStepIndexes.append({ 0/*SubmodelIndex*/, size(topLevelFile()) });

//*
#ifdef QT_DEBUG_MODE
  if (Gui::suspendFileDisplay) {
    emit gui->messageSig(LOG_DEBUG, QString("COUNT INSTANCES Step Indexes"));
    emit gui->messageSig(LOG_DEBUG, "----------------------------");
    for (int i = 0; i < _buildModStepIndexes.size(); i++)
    {
      const QVector<int> &key = _buildModStepIndexes.at(i);
      emit gui->messageSig(LOG_DEBUG, QString("StepIndex: %1, SubmodelIndex: %2: LineNumber: %3, ModelName: %4")
                           .arg(i, 3, 10, QChar('0'))         // index
                           .arg(key.at(0), 3, 10, QChar('0')) // modelIndex
                           .arg(key.at(1), 3, 10, QChar('0')) // lineNumber
                           .arg(getSubmodelName(key.at(0)))); // modelName
    }
    emit gui->messageSig(LOG_DEBUG, QString("Count steps and submodel instances - %1")
                         .arg(gui->elapsedTime(timer.elapsed())));
    emit gui->messageSig(LOG_DEBUG, "----------------------------");
  }
#endif
//*/
}

void LDrawFile::countParts(const QString &fileName) {

    Where top(fileName, getSubmodelIndex(fileName), 0);

    int topModelIndx  = top.modelIndex;

    emit gui->progressBarPermInitSig();
    emit gui->progressPermRangeSig(1, size(top.modelName));
    emit gui->progressPermMessageSig("Counting parts for " + top.modelName + "...");

    auto setStatusEntry = [&] (
            const QString &statusEntry,
            const QString &type,
            const QString &statusMessage = "",
            bool uniqueCount = false)
    {
        if (!_loadedParts.contains(statusEntry)) {
            if (uniqueCount)
                _uniquePartCount++;
            else
                _loadedParts.append(statusEntry);
            QString message;
            if (statusMessage.endsWith(" validated."))
                message = statusMessage.arg(_uniquePartCount).arg(type);
            else if (statusMessage.endsWith(" archives."))
                message = statusMessage.arg(type).arg(VER_PRODUCTNAME_STR);
            else
                message = statusMessage.arg(type);
            emit gui->messageSig(LOG_NOTICE, message);
        }
        if (uniqueCount)
            _loadedParts.append(statusEntry);
    };

    std::function<void(Where&)> countModelParts;
    countModelParts = [&] (Where& top)
    {

        QStringList content = contents(top.modelName);

        // get content size
        int lines = content.size();

        if (content.size()) {

            // initialize model parts count
            int modelPartCount = 0;

            // initialize valid line
            bool lineIncluded  = true;

            // skip the header
            gui->skipHeader(top);

            // process submodel content...
            for (; top.lineNumber < lines; top.lineNumber++) {

                QStringList tokens;
                QString line = content.at(top.lineNumber);

                if (top.modelIndex == topModelIndx)
                    emit gui->progressPermSetValueSig(top.lineNumber);

                // adjust ghost lines
                if (line.startsWith("0 GHOST "))
                    line = line.mid(8).trimmed();

                split(line,tokens);

                // meta command parse
                if (tokens.size() > 1 && tokens[0] == "0") {
                    if (tokens[1] == "!LPUB" || tokens[1] == "LPUB") {
                        // build modification - starts at BEGIN command and ends at END_MOD action
                        if (tokens.size() >= 4 &&
                            tokens[2] == "BUILD_MOD") {
                            if (tokens[3] == "BEGIN") {
                                buildModLevel = getLevel(tokens[4], BM_BEGIN);
                            } else if (tokens[3] == "END_MOD") {
                                buildModLevel = getLevel(QString(), BM_END);
                            }
                            lineIncluded = ! buildModLevel;
                        }
                        // ignore parts begin
                        if (tokens.size() == 5 && tokens[0] == "0" &&
                           (tokens[2] == "PART" || tokens[2] == "PLI") &&
                            tokens[3] == "BEGIN"  &&
                            tokens[4] == "IGN") {
                            lineIncluded = false;
                        } else
                        // ignore part end
                        if (tokens.size() == 4 &&
                           (tokens[2] == "PART" || tokens[2] == "PLI") &&
                           tokens[3] == "END") {
                            lineIncluded = true;
                        }
                    }
                } // meta command lines

                QString type;
                bool countThisLine = true;
                if ((countThisLine = tokens.size() == 15 && tokens[0] == "1"))
                    type = tokens[14];
                else if (isSubstitute(line, type))
                    countThisLine = !type.isEmpty();

                bool partIncluded = !ExcludedParts::isExcludedPart(type);

                if (countThisLine && lineIncluded && partIncluded) {
                    QString statusEntry;
                    if (contains(type)) {
                        QString description = QFileInfo(type).baseName();
                        LDrawUnofficialFileType subFileType = LDrawUnofficialFileType(isUnofficialPart(type.toLower()));
                        if (subFileType == UNOFFICIAL_SUBMODEL) {
                            statusEntry = QObject::tr("%1|%2|Submodel: %3 with %4 lines")
                                                      .arg(_mpd ? MPD_SUBMODEL_LOAD_MSG : LDR_SUBMODEL_LOAD_MSG).arg(type).arg(description).arg(size(type));
                            setStatusEntry(statusEntry, type, QObject::tr("Model [%1] is a SUBMODEL"));
                            Where top(type, getSubmodelIndex(type), 0);
                            countModelParts(top);
                        } else {
                            QString partFile = type.toUpper();
                            if (partFile.startsWith("S\\"))
                                partFile.replace("S\\","S/");
                            PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(partFile.toLatin1().constData(), nullptr/*CurrentProject*/, false/*CreatePlaceholder*/, false/*SearchProjectFolder*/);
                            if (pieceInfo && pieceInfo->IsPartType()) {
                                description = pieceInfo->m_strDescription;
                            } else
                            if (lcGetPiecesLibrary()->IsPrimitive(partFile.toLatin1().constData())) {
                                lcLibraryPrimitive* Primitive = lcGetPiecesLibrary()->FindPrimitive(partFile.toLatin1().constData());
                                description = Primitive->mName;
                            }
                            switch(subFileType) {
                            case UNOFFICIAL_PART:
                            case UNOFFICIAL_SHORTCUT:
                                _partCount++;modelPartCount++;
                                statusEntry = QObject::tr("%1|%2|Unofficial Inline - %3").arg(INLINE_PART_LOAD_MSG).arg(type).arg(description);
                                setStatusEntry(statusEntry, type, QObject::tr("Part %1 is an INLINE PART."));
                                statusEntry = QObject::tr("%1|%2|Unofficial Inline - %3").arg(VALID_LOAD_MSG).arg(type).arg(description);
                                setStatusEntry(statusEntry, type, QObject::tr("Part %1 [Inline %2] validated."),true/*unique count*/);
                                break;
                            case UNOFFICIAL_SUBPART:
                                statusEntry = QObject::tr("%1|%2|Unofficial Inline - %3").arg(INLINE_SUBPART_LOAD_MSG).arg(type).arg(description);
                                setStatusEntry(statusEntry, type, QObject::tr("Part [%1] is a INLINE SUBPART"));
                                break;
                            /* Add these primitives into the load status dialogue because they are loaded in the LDrawFile.subfiles */
                            case UNOFFICIAL_PRIMITIVE:
                                statusEntry = QObject::tr("%1|%2|Unofficial Inline - %3").arg(INLINE_PRIMITIVE_LOAD_MSG).arg(type).arg(description);
                                setStatusEntry(statusEntry, type, QObject::tr("Part [%1] is a INLINE PRIMITIVE"));
                                break;
                            default:
                                break;
                            }
                            Where top(type, getSubmodelIndex(type), 0);
                            countModelParts(top);
                        }
                    } else {
                        QString partFile = type.toUpper();
                        if (partFile.startsWith("S\\"))
                            partFile.replace("S\\","S/");
                        PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(partFile.toLatin1().constData(), nullptr/*CurrentProject*/, false/*CreatePlaceholder*/, false/*SearchProjectFolder*/);
                        if (pieceInfo && pieceInfo->IsPartType()) {
                            _partCount++;modelPartCount++;;
                            statusEntry = QObject::tr("%1|%2|%3").arg(VALID_LOAD_MSG).arg(type).arg(pieceInfo->m_strDescription);
                            setStatusEntry(statusEntry, type, QObject::tr("Part %1 [%2] validated."),true/*unique count*/);
                        } else
                        if (lcGetPiecesLibrary()->IsPrimitive(partFile.toLatin1().constData())) {
                            continue;
                            /* Do not add these primitives into the load status dialogue because they are not loaded in the LDrawFile.subfiles
                            lcLibraryPrimitive* Primitive = lcGetPiecesLibrary()->FindPrimitive(partFile.toLatin1().constData());
                            if (Primitive) {
                                const QString description = Primitive->mName;
                                if (Primitive->mSubFile) {
                                    statusEntry = QObject::tr("%1|%2|%3").arg(SUBPART_LOAD_MSG).arg(type).arg(description);
                                    setStatusEntry(statusEntry, type, QObject::tr("Part [%1] is a SUBPART"));
                                } else {
                                    statusEntry = QObject::tr("%1|%2|%3").arg(PRIMITIVE_LOAD_MSG).arg(type).arg(description);
                                    setStatusEntry(statusEntry, type, QObject::tr("Part [%1] is a PRIMITIVE"));
                                }
                            }*/
                        } else {
                            const QString message = QObject::tr("Part [%1] is not excluded, inlined, a submodel or an "
                                                                "external file and was not found in the %2 library archives.");
                            statusEntry = QObject::tr("%1|%2|Part not found!").arg(MISSING_LOAD_MSG).arg(type);
                            setStatusEntry(statusEntry, type, message);
                        }
                    }  // check archive
                } // countThisLine && lineIncluded && partIncluded
            } // process submodel content
        } // content size
    };

    countModelParts(top);

    emit gui->messageSig(LOG_STATUS, QString("%1 total %2 and %3 unique %4 counted for %5")
                                             .arg(_partCount)
                                             .arg(_partCount == 1 ? QObject::tr("part") : QObject::tr("parts"))
                                             .arg(_uniquePartCount)
                                             .arg(_uniquePartCount == 1 ? QObject::tr("part") : QObject::tr("parts"))
                                             .arg(top.modelName));
    emit gui->progressPermSetValueSig(size(top.modelName));
    emit gui->progressPermMessageSig(QString());
    emit gui->progressPermStatusRemoveSig();
}

bool LDrawFile::saveModelFile(const QString &fileName)
{
    QString writeFileName = QDir::toNativeSeparators(fileName);
    QFile file(writeFileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        emit gui->messageSig(LOG_ERROR,QString("Cannot write %1 file %2:<br>%3.")
                             .arg(_mpd ? "MPD" : "LDR")
                             .arg(writeFileName)
                             .arg(file.errorString()));
        return false;
    }

    emit gui->messageSig(LOG_INFO,QString("Saving %1 file %2.")
                         .arg(_mpd ? "MPD" : "LDR")
                         .arg(writeFileName));

    Gui::suspendFileDisplay = true;
    gui->deleteFinalModelStep();

    bool newLineIinserted = false;

    QTextStream out(&file);
    out.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));

    for (int i = 0; i < _subFileOrder.size(); i++) {

        bool addFILEMeta = false;
        bool omitNOFIlEMeta = false;
        bool isModelHeader = true;
        bool isLDCadContent = false;

        QString subFileName = _subFileOrder.at(i);
        QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(subFileName.toLower());

        if (f != _subFiles.end() && ! f.value()._generated) {

            addFILEMeta    = _mpd && !f.value()._includeFile;

            if (!f.value()._subFilePath.isEmpty()) {

                file.close();
                writeFileName = QDir::toNativeSeparators(f.value()._subFilePath);
                file.setFileName(writeFileName);

                if (!file.open(QFile::WriteOnly | QFile::Text)) {
                    emit gui->messageSig(LOG_ERROR,QString("Cannot write %1 subfile %2:<br>%3.")
                                        .arg(_mpd ? "MPD" : "LDR")
                                        .arg(writeFileName)
                                        .arg(file.errorString()));
                    gui->insertFinalModelStep();
                    Gui::suspendFileDisplay = false;
                    return false;
                }

                emit gui->messageSig(LOG_INFO,QString("Saving %1 subfile %2.")
                                     .arg(_mpd ? "MPD" : "LDR")
                                     .arg(writeFileName));

                QTextStream out(&file);
                out.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
            }

            if (addFILEMeta) {
                out << "0 FILE " << subFileName << lpub_endl;
                _savedLines++;
            }

            for (int j = 0; j < f.value()._contents.size(); j++) {

                _savedLines++;

                bool insertNewLineBefore = false;
                bool insertNewLineAfter = false;

                QString line = f.value()._contents[j];

                if (isModelHeader && line[0] == '0') {
                    if (!isLDCadContent)
                        isLDCadContent = line.contains(_fileRegExp[LDC_RX]);
                } else if (line[0] != '0') {
                    isModelHeader = false;
                    if (isLDCadContent) {
                        QStringList tokens;
                        split(line,tokens);
                        if (tokens.size() == 15 && contains(tokens.at(14)) && _subFileOrder.size() > i)
                            omitNOFIlEMeta = tokens.at(14).toLower() == _subFileOrder.at(i+1).toLower();
                    }
                }

                if (!newLineIinserted) {
                    if (isLDCadContent) {
                        QRegExp newLineBeforeRx("^\\s*0\\s+\\/\\/\\s*(Segments| Segment |Start cap |End cap |Fixed color segments)[^\n]*",Qt::CaseSensitive);
                        insertNewLineBefore = line.contains(newLineBeforeRx) && (j ? !f.value()._contents.at(j-1).startsWith("0 //Segments",Qt::CaseSensitive) : true);
                        if (!insertNewLineBefore) {
                            QRegExp newLineAfterRx("^\\s*0\\s+\\/\\/\\s*( The path is approx | License: )[^\n]*",Qt::CaseSensitive);
                            insertNewLineAfter  = line.startsWith("0 BFC ");
                            insertNewLineAfter |= line.contains(newLineAfterRx);
                        }
                        _savedLines = insertNewLineBefore || insertNewLineAfter ? _savedLines++ : _savedLines;
                    } else if (line.startsWith("0 !LICENSE ") && f.value()._contents.size() > j) {
                        insertNewLineAfter |= f.value()._contents.at(j+1).startsWith("0 !LDCAD GENERATED ",Qt::CaseSensitive);
                        _savedLines++;
                    }
/*
                    // this condition block adds a space after the folowing headers
                    else
                    if (f.value()._unofficialPart > UNOFFICIAL_SUBMODEL &&
                       (line.startsWith("0 !LICENSE ")                  ||
                        line.startsWith("0 BFC ")                       ||
                        line.startsWith("0 !HISTORY "))                 &&
                       (f.value()._contents.size() > (j+1)              &&
                        f.value()._contents[j+1] != "0"                 &&
                        f.value()._contents[j+1] != ""))
                    {
                        insertNewLineAfter = true;
                    }
//*/
                }

                if (insertNewLineBefore)
                {
                    out << lpub_endl;
                    out << line << lpub_endl;
                }
                else if (insertNewLineAfter)
                {
                    out << line << lpub_endl;
                    out << lpub_endl;
                }
                else
                    out << line << lpub_endl;

                newLineIinserted = insertNewLineBefore || insertNewLineAfter;
            }

            if (addFILEMeta) {
                if (!omitNOFIlEMeta) {
                    out << "0 NOFILE" << lpub_endl;
                    _savedLines++;
                }
                out << lpub_endl;
                _savedLines++;
            }
        }
    }

    file.close();

    gui->insertFinalModelStep();
    Gui::suspendFileDisplay = false;

    return true;
}

bool LDrawFile::saveIncludeFile(const QString &fileName){
    QString includeFileName = fileName.toLower();

    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(includeFileName);
    if (f != _subFiles.end() && f.value()._includeFile) {

      if (f.value()._modified) {
        QFile file;
        QString writeFileName;

        if (!f.value()._subFilePath.isEmpty()) {
            writeFileName = f.value()._subFilePath;
            file.setFileName(writeFileName);
        }

        if (!file.open(QFile::WriteOnly | QFile::Text)) {
          emit gui->messageSig(LOG_ERROR,QString("Cannot write include file %1:<br>%2.")
                               .arg(writeFileName)
                               .arg(file.errorString()));
          return false;
        }

        QTextStream out(&file);
        out.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));

        for (int j = 0; j < f.value()._contents.size(); j++) {
          out << f.value()._contents[j] << lpub_endl;
          _savedLines++;
        }

        file.close();
      }
    }
    return true;
}

bool LDrawFile::changedSinceLastWrite(const QString &fileName)
{
  QString mcFileName = fileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(mcFileName);
  if (i != _subFiles.end()) {
    bool value = i.value()._changedSinceLastWrite;
    i.value()._changedSinceLastWrite = false;
    return value;
  }
  return false;
}

void LDrawFile::tempCacheCleared()
{
  for (const QString &key : _subFiles.keys()) {
    _subFiles[key]._changedSinceLastWrite = true;
    _subFiles[key]._modified = true;
  }
}

void LDrawFile::insertLDCadGroup(const QString &name, int lid)
{
  QHash<QString, int>::const_iterator i = _ldcadGroups.constBegin();
  while (i != _ldcadGroups.constEnd()) {
    if (i.key() == name && i.value() == lid)
      return;
    ++i;
  }
  _ldcadGroups.insert(name,lid);
}

bool LDrawFile::ldcadGroupMatch(const QString &name, const QStringList &lids)
{
  QList<int> values = _ldcadGroups.values(name);
  for (QString lid : lids){
    if (values.contains(lid.toInt()))
      return true;
  }
  return false;
}

/* Build Modification Routines */

void LDrawFile::insertBuildMod(const QString      &buildModKey,
                               const QVector<int> &modAttributes,
                               int                 stepIndex)
{
  QString modKey  = buildModKey.toLower();
  QVector<int>  modSubmodelStack;
  QMap<int,int> modActions;
  QVector<int>  newAttributes;
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end()) {
    // Preserve actions
    modActions = i.value()._modActions;

    // Preserve submodelStack
    modSubmodelStack = i.value()._modSubmodelStack;

    // Remove action for specified stepIndex
    QMap<int,int>::iterator a = modActions.find(stepIndex);
    if (a != modActions.end())
        modActions.erase(a);

    // Update attributes
    newAttributes = { 0, 0, 0, 0, 0, -1, 0, 0 };
    for (int a = 0; a < modAttributes.size(); a++) {
      if (a < i.value()._modAttributes.size()) {
        if (modAttributes.at(a) != i.value()._modAttributes.at(a) && modAttributes.at(a) != newAttributes.at(a))
          newAttributes[a] = modAttributes.at(a);
        else
          newAttributes[a] = i.value()._modAttributes.at(a);
      } else
        newAttributes[a] = i.value()._modAttributes.at(a);
    }

    // Remove BuildMod if exist
    _buildMods.erase(i);

  } else
    newAttributes = modAttributes;

  // Initialize new BuildMod
  BuildMod buildMod(newAttributes, stepIndex);

  // Restore preserved actions
  if (modActions.size()){
    QMap<int,int>::const_iterator a = modActions.constBegin();
    while (a != modActions.constEnd()) {
      buildMod._modActions.insert(a.key(), a.value());
      ++a;
    }
  }

  // Insert new BuildMod
  _buildMods.insert(modKey, buildMod);

  // Insert BuildModStep - must come after _buildMods.insert()
  insertBuildModStep(modKey, stepIndex, BuildModApplyRc);

  // Set submodelStack items modified if exists
  if (modSubmodelStack.size()) {
      buildMod._modSubmodelStack = modSubmodelStack;
      for (const int modelIndex : modSubmodelStack)
          setModified(getSubmodelName(modelIndex),true);
  }

  // Set subfile modified
  QString modFileName = getBuildModStepKeyModelName(modKey);
  setModified(modFileName, true);

  // Update BuildMod list
  if (!_buildModList.contains(buildModKey))
      _buildModList.append(buildModKey);
}

/* This call captures steps that have buildMod command(s) */

void LDrawFile::insertBuildModStep(const QString &buildModKey,
                                   const int      stepIndex,
                                   const int      modAction)
{
    bool modBegin = false;
    QString modKey = buildModKey.toLower();
    BuildModStep newModStep(stepIndex, modAction ? modAction : getBuildModAction(modKey, stepIndex), modKey);
    QMultiMap<int, BuildModStep>::iterator i = _buildModSteps.find(stepIndex);
    while (i != _buildModSteps.end() && i.key() == stepIndex) {
        if (i.value()._buildModKey == modKey) {
            modBegin = getBuildModStepIndex(i.value()._buildModKey) == stepIndex;
            if (modBegin)
                setViewerStepHasBuildModAction(getViewerStepKey(i.key()), false);
            _buildModSteps.erase(i);
            break;
        }
        ++i;
    }

    // we update submodels in the calls that use this call:
    //   ::insertBuildMod and ::setBuildModAction

    _buildModSteps.insert(stepIndex, newModStep);

#ifdef QT_DEBUG_MODE
    int action = modBegin ? static_cast<int>(BuildModBeginRc) : modAction ? modAction : newModStep._buildModAction;
    const QString message =
            QString("Insert BuildMod Step ModStepIndex: %1, Action: %2, ModKey: '%3'")
                    .arg(stepIndex).arg(action == BuildModApplyRc ? "Apply(64)" : action == BuildModRemoveRc ? "Remove(65)" : action == BuildModBeginRc ? "Begin(61)" : "None(0)").arg(buildModKey);
    //qDebug() << qPrintable("DEBUG: " + message + "\n\n");
    emit gui->messageSig(LOG_DEBUG, message);
#endif
}

int LDrawFile::getBuildModStepAction(const QString &modelName,
                                     const     int &lineNumber)
{
    int modAction = BuildModNoActionRc;
    int modelIndex = getSubmodelIndex(modelName);
    int modStepIndex = getBuildModStepIndex(modelIndex, lineNumber);
    int modBeginStepIndex = BuildModInvalidIndexRc;

    QMap<int, BuildModStep>::const_iterator i = _buildModSteps.find(modStepIndex);
    if (i != _buildModSteps.end()) {
        modAction = i.value()._buildModAction;
        modBeginStepIndex = getBuildModStepIndex(i.value()._buildModKey);
        if(modBeginStepIndex == modStepIndex)
            modAction = BuildModBeginRc;
#ifdef QT_DEBUG_MODE
        const QString message =
                QString("Get BuildModStepAction "
                        "StepIndex: %1, "
                        "BeginIndex: %2, "
                        "StepModCount: %3, "
                        "ModAction: %4%5, "
                        "Top ModelIndex: %6, "
                        "Top LineNumber: %7, "
                        "ModKey: '%8'")
                        .arg(modStepIndex)
                        .arg(modBeginStepIndex)
                        .arg(_buildModSteps.values(modStepIndex).size())
                        .arg(modAction == BuildModBeginRc ? "Begin(61)" : modAction == BuildModApplyRc ? "Apply(64)" : modAction == BuildModRemoveRc ? "Remove(65)" : "None(0)")
                        .arg(modAction == BuildModBeginRc ? QString(", LineNumber: %1").arg(getBuildModBeginLineNumber(i.value()._buildModKey)) : "")
                        .arg(modelIndex).arg(lineNumber).arg(i.value()._buildModKey);
        //qDebug() << qPrintable("DEBUG: " + message + "\n\n");
        emit gui->messageSig(LOG_DEBUG, message);
#endif
    }

    return modAction;
}

int LDrawFile::getBuildModStepAction(const int stepIndex)
{
    QVector<int> topOfStep = _buildModStepIndexes.at(stepIndex);
    QString modelName      = getSubmodelName(topOfStep.at(BM_MODEL_NAME));
    int lineNumber         = topOfStep.at(BM_LINE_NUMBER);

    if (!modelName.isEmpty() && lineNumber > BM_BEGIN_LINE_NUM)
        return getBuildModStepAction(modelName, lineNumber);

    return static_cast<int>(BuildModNoActionRc);
}

/* This call will capture a step's build mod actions (if exists) and check if the captured action is BuildModBeginRc */

QList<QVector<int> > LDrawFile::getBuildModStepActions(
        const QString &modelName,
        const     int &lineNumber)
{
    QList<QVector<int> > modActions;
    int modelIndex = getSubmodelIndex(modelName);
    int modStepIndex = getBuildModStepIndex(modelIndex, lineNumber);
#ifdef QT_DEBUG_MODE
    int stepModCount = 1;
#endif
    QMultiMap<int, BuildModStep>::const_iterator i = _buildModSteps.find(modStepIndex);
    while (i != _buildModSteps.end() && i.key() == modStepIndex) {
        QVector<int> modAction = { BM_INVALID_INDEX, BM_INIT, BM_INIT };
        QRegExp buildModKeyRx(i.value()._buildModKey, Qt::CaseInsensitive);
        int modBeginStepIndex = getBuildModStepIndex(i.value()._buildModKey);
        modAction[BM_ACTION_KEY_INDEX] = _buildModList.indexOf(buildModKeyRx);
        if (modBeginStepIndex == modStepIndex) {        // step index has BuildModBeginRc
            modAction[BM_ACTION_LINE_NUM] = getBuildModBeginLineNumber(i.value()._buildModKey);
            modAction[BM_ACTION_CODE] = static_cast<int>(BuildModBeginRc);
        } else {
            modAction[BM_ACTION_LINE_NUM] = lineNumber; // action topOfStep.lineNumber
            modAction[BM_ACTION_CODE] = i.value()._buildModAction; // apply or remove
        }
#ifdef QT_DEBUG_MODE
        const QString message =
                QString("Get BuildModStepActions "
                        "StepIndex: %1, "
                        "BeginIndex: %2, "
                        "StepModCount: %3, "
                        "ModAction: %4%5, "
                        "Top ModelIndex: %6, "
                        "Top LineNumber: %7, "
                        "ModKey(Index): '%8(%9)'")
                        .arg(modStepIndex)
                        .arg(modBeginStepIndex)
                        .arg(stepModCount)
                        .arg(modAction[BM_ACTION_CODE] == BuildModBeginRc ? "Begin(61)" : modAction[BM_ACTION_CODE] == BuildModApplyRc ? "Apply(64)" : modAction[BM_ACTION_CODE] == BuildModRemoveRc ? "Remove(65)" : "None(0)")
                        .arg(modAction[BM_ACTION_CODE] == BuildModBeginRc ? QString(", LineNumber: %1").arg(modAction[BM_ACTION_LINE_NUM]) : "")
                        .arg(modelIndex)
                        .arg(lineNumber)
                        .arg(i.value()._buildModKey)
                        .arg(modAction[BM_ACTION_KEY_INDEX]);
        //qDebug() << qPrintable("DEBUG: " + message + "\n\n");
        emit gui->messageSig(LOG_DEBUG, message);
        stepModCount++;
#endif
        modActions.append(modAction);
        i++;
    }

    return modActions;
}

void LDrawFile::clearBuildModStep(const QString &buildModKey,const int stepIndex)
{
    bool modBegin = false;
    QString modKey = buildModKey.toLower();
    QMultiMap<int, BuildModStep>::iterator i = _buildModSteps.find(stepIndex);
    while (i != _buildModSteps.end() && i.key() == stepIndex) {
        if (i.value()._buildModStepIndex == stepIndex &&
            i.value()._buildModKey == modKey) {
            modBegin = getBuildModStepIndex(i.value()._buildModKey) == stepIndex;
            if (modBegin)
                setViewerStepHasBuildModAction(getViewerStepKey(i.key()), false);
#ifdef QT_DEBUG_MODE
            int action = modBegin ? static_cast<int>(BuildModBeginRc) : i.value()._buildModAction;
            const QString message =
                    QString("Remove BuildModStep ModStepIndex: %1, Action: %2, ModKey: '%3'")
                            .arg(stepIndex)
                            .arg(action == BuildModApplyRc ? "Apply(64)" : action == BuildModRemoveRc ? "Remove(65)" : action == BuildModBeginRc ? "Begin(61)" : "None(0)")
                            .arg(buildModKey);
            //qDebug() << qPrintable("DEBUG: " + message + "\n\n");
            emit gui->messageSig(LOG_DEBUG, message);
#endif
            _buildModSteps.erase(i);
            break;
        }
        ++i;
    }
}

void LDrawFile::clearBuildModSteps(const QString &buildModKey)
{
#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_TRACE, QString("Remove BuildModStep actions for ModKey %1...") .arg(buildModKey));
#endif
    bool modBegin = false;
    QString modKey = buildModKey.toLower();
    QMultiMap<int, BuildModStep> buildModSteps;
    QMultiMap<int, BuildModStep>::iterator i = _buildModSteps.begin();
    while (i != _buildModSteps.end()) {
        if(i.value()._buildModKey != modKey ) {
            buildModSteps.insert(i.key(),i.value());
        }
        else
        {
            modBegin = getBuildModStepIndex(i.value()._buildModKey) == i.value()._buildModStepIndex;
            if (modBegin)
                setViewerStepHasBuildModAction(getViewerStepKey(i.key()), false);
#ifdef QT_DEBUG_MODE
            int action = modBegin ? static_cast<int>(BuildModBeginRc) : i.value()._buildModAction;
            const QString message =
                    QString("Removed Step Index: %1, Action: %2")
                            .arg(i.value()._buildModStepIndex)
                            .arg(action == BuildModApplyRc ? "Apply(64)" : action == BuildModRemoveRc ? "Remove(65)" : action == BuildModBeginRc ? "Begin(61)" : "None(0)");
            //qDebug() << qPrintable("DEBUG: " + message + "\n\n");
            emit gui->messageSig(LOG_TRACE, message);
#endif
        }
        i++;
    }

    if (buildModSteps.size()) {
        _buildModSteps.clear();
        _buildModSteps = buildModSteps;
    }
}

bool LDrawFile::deleteBuildMod(const QString &buildModKey)
{
    QString modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
        _buildMods.erase(i);
        if (_buildModList.contains(buildModKey, Qt::CaseInsensitive)) {
            QRegExp buildModKeyRx(buildModKey, Qt::CaseInsensitive);
            const int buildModListIndex = _buildModList.indexOf(buildModKeyRx);
            if (buildModListIndex > -1)
                _buildModList.removeAt(buildModListIndex);
        }

        // Set subfile modified
        QString modFileName = getBuildModStepKeyModelName(modKey);
        QMap<QString, LDrawSubFile>::iterator s = _subFiles.find(modFileName);
        if (s != _subFiles.end()) {
          if (s.value()._buildMods > 0)
            s.value()._buildMods -= 1;
          s.value()._modified = true;
          s.value()._changedSinceLastWrite = true;
        }

        // Clear build mod steps
        clearBuildModSteps(modKey);

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Removed BuildMod ModKey: '%1'").arg(buildModKey));
#endif
        return true;
    }
    return false;
}

void LDrawFile::deleteBuildMods(const int stepIndex)
{
    for(const QString &key : getBuildModsList()) {
        const int index = getBuildModStepIndex(key);
        if (index >= stepIndex) {
            deleteBuildMod(key);
        }
    }
}

void LDrawFile::deleteBuildMods()
{
    for (const QString &key : getBuildModsList()) {
        deleteBuildMod(key);
    }
}

void LDrawFile::setBuildModStepKey(const QString &buildModKey, const QString &modStepKey)
{
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
        QStringList stepKeys = modStepKey.split(";");
        i.value()._modAttributes[BM_MODEL_NAME_INDEX] = stepKeys.at(BM_STEP_MODEL_KEY).toInt();
        i.value()._modAttributes[BM_MODEL_LINE_NUM]   = stepKeys.at(BM_STEP_LINE_KEY).toInt();
        i.value()._modAttributes[BM_MODEL_STEP_NUM]   = stepKeys.at(BM_STEP_NUM_KEY).toInt();
#ifdef QT_DEBUG_MODE
        int action = BuildModNoActionRc;
        QMap<int, int>::iterator ai = i.value()._modActions.find(i.value()._modStepIndex);
        if (ai != i.value()._modActions.end())
            action = ai.value();
        emit gui->messageSig(LOG_DEBUG, QString("Update BuildMod StepKey: %1;%2;%3, StepIndex: %4, Action: %5, ModKey: '%6'")
                             .arg(i.value()._modAttributes.at(BM_MODEL_NAME_INDEX))
                             .arg(i.value()._modAttributes.at(BM_MODEL_LINE_NUM))
                             .arg(i.value()._modAttributes.at(BM_MODEL_STEP_NUM))
                             .arg(i.value()._modStepIndex)
                             .arg(action ? action == BuildModApplyRc ? "Apply(64)" : "Remove(65)" : "None(0)")
                             .arg(buildModKey));
#endif
    }
}

int LDrawFile::getBuildModBeginLineNumber(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_BEGIN_LINE_NUM) {
    return i.value()._modAttributes.at(BM_BEGIN_LINE_NUM);
  }

  return 0;
}

int LDrawFile::getBuildModActionLineNumber(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_ACTION_LINE_NUM) {
    return i.value()._modAttributes.at(BM_ACTION_LINE_NUM);
  }

  return 0;
}

int LDrawFile::getBuildModEndLineNumber(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_END_LINE_NUM) {
    return i.value()._modAttributes.at(BM_END_LINE_NUM);
  }

  return 0;
}

int LDrawFile::getBuildModDisplayPageNumber(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_DISPLAY_PAGE_NUM) {
    return i.value()._modAttributes.at(BM_DISPLAY_PAGE_NUM);
  }

  return 0;
}

int LDrawFile::setBuildModDisplayPageNumber(const QString &buildModKey, int displayPageNum)
{
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
        i.value()._modAttributes[BM_DISPLAY_PAGE_NUM] = displayPageNum;

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Set BuildMod DisplayPageNumber: %1, ModKey: '%2'")
                                                .arg(i.value()._modAttributes.at(BM_DISPLAY_PAGE_NUM))
                                                .arg(buildModKey));
#endif

        return i.value()._modAttributes.at(BM_DISPLAY_PAGE_NUM);
    }

    return 0;
}

int LDrawFile::getBuildModStepPieces(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_STEP_PIECES) {
    return i.value()._modAttributes.at(BM_STEP_PIECES);
  }

  return 0;
}

int LDrawFile::setBuildModStepPieces(const QString &buildModKey, int pieces)
{
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
        i.value()._modAttributes[BM_STEP_PIECES] = pieces;
#ifdef QT_DEBUG_MODE
        int action = BuildModNoActionRc;
        QMap<int, int>::iterator ai = i.value()._modActions.find(i.value()._modStepIndex);
        if (ai != i.value()._modActions.end())
            action = ai.value();
        emit gui->messageSig(LOG_DEBUG, QString("Update BuildMod StepPieces: %1, StepIndex: %2, Action: %3, ModKey: '%4'")
                                                .arg(i.value()._modAttributes.at(BM_STEP_PIECES))
                                                .arg(i.value()._modStepIndex)
                                                .arg(action ? action == BuildModApplyRc ? "Apply(64)" : "Remove(65)" : "None(0)")
                                                .arg(buildModKey));
#endif
        return i.value()._modAttributes.at(BM_STEP_PIECES);
    }

    return 0;
}

int LDrawFile::setBuildModRendered(const QString &buildModKey, const QString &renderedModel)
{
#ifdef QT_DEBUG_MODE
    bool entryAdded = true;
#endif
    QString  modKey = buildModKey.toLower();
    QString  subModel = renderedModel.toLower();
    QMap<QString, QStringList>::iterator i = _buildModRendered.find(modKey);
    if (i == _buildModRendered.end()) {
        _buildModRendered.insert(modKey, QStringList() << subModel);
    } else if (! i.value().contains(subModel)) {
        i.value().append(subModel);
    }
#ifdef QT_DEBUG_MODE
    else {
        entryAdded = false;
    }
    if (entryAdded)
        emit gui->messageSig(LOG_DEBUG, QString("Insert BuildMod RenderedModel: %1, ModKey: '%2'").arg(renderedModel).arg(buildModKey));
#endif
    return 0;
}

bool LDrawFile::getBuildModRendered(const QString &buildModKey, const QString &renderedModel, bool countPage)
{
    QString  modKey = countPage ? "cp~"+buildModKey.toLower() : buildModKey.toLower();
    QMap<QString, QStringList>::iterator i = _buildModRendered.find(modKey);
    if (i != _buildModRendered.end()) {
        if (i.value().contains(renderedModel.toLower())) {
#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_DEBUG, QString("BuildMod RenderedModel: %1, ModKey: '%2'")
                                 .arg(renderedModel).arg((countPage ? "cp~" : "")+buildModKey));
#endif
            return true;
        }
    }
    return false;
}

void LDrawFile::clearBuildModRendered(const QString &buildModKey, const QString &renderedModel)
{
    QString modKey = buildModKey.toLower();
    QString subModel = renderedModel.toLower();
    QMap<QString, QStringList>::iterator i = _buildModRendered.find(modKey);
    if (! i.value().contains(subModel)) {
        int cleared = i.value().removeAll(subModel);
#ifdef QT_DEBUG_MODE
        if (cleared)
            emit gui->messageSig(LOG_DEBUG, QString("Clear BuildMod RenderedModel: %1, Count: %2, ModKey: '%3'")
                                 .arg(renderedModel).arg(cleared).arg(buildModKey));
#else
        Q_UNUSED(cleared)
#endif
    }
}

void LDrawFile::clearBuildModRendered(bool countPage)
{
    for (const QString &key : _buildModRendered.keys()) {
        if (countPage) {
           for (const QString &modelFile : _buildModRendered[key]) {
               if (modelFile.startsWith("cp~")) {
                   _buildModRendered[key].removeAll(modelFile);
               }
           }
        } else {
            _buildModRendered[key].clear();
        }
    }
}

void LDrawFile::setBuildModSubmodelStack(const QString &buildModKey, const QStringList &submodelStack)
{
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
        for (const QString &modelFile : submodelStack) {
            int submodelIndex = getSubmodelIndex(modelFile);
            if (!i.value()._modSubmodelStack.contains(submodelIndex)) {
                i.value()._modSubmodelStack << submodelIndex;
                setModified(modelFile, true);
            }
        }
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Update BuildMod ParentSubmodels: %1, StepIndex: %2, ModKey: '%4'")
                                                .arg(i.value()._modSubmodelStack.size())
                                                .arg(i.value()._modStepIndex)
                                                .arg(buildModKey));
#endif
    }
}

int LDrawFile::getBuildModActionPrevIndex(const QString &buildModKey, const int stepIndex, const int action)
{
    int actionStepIndex;
    if (getBuildModAction(buildModKey, stepIndex - 1, BM_LAST_ACTION, actionStepIndex) == action)
        return actionStepIndex;
    return stepIndex;
}

int LDrawFile::getBuildModAction(const QString &buildModKey, const int stepIndex)
{
    int unusedIndex;
    return getBuildModAction(buildModKey, stepIndex, BM_LAST_ACTION, unusedIndex);
}

int LDrawFile::getBuildModAction(const QString &buildModKey, const int stepIndex, const int defaultIndex)
{
    int unusedIndex = BM_INVALID_INDEX;
    return getBuildModAction(buildModKey, stepIndex, defaultIndex, unusedIndex);
}

int LDrawFile::getBuildModAction(const QString &buildModKey, const int stepIndex, const int defaultIndex, int &actionStepIndex)
{
  QString insert = QString();
  QString modKey = buildModKey.toLower();
  int action = BuildModNoActionRc;
  actionStepIndex = stepIndex;
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end()) {
      // return BuildModRemoveRc (65) for 'Future' requests where the requested stepIndex is before the buildMod stepIndex
      if (stepIndex < i.value()._modActions.firstKey() && stepIndex > BM_INVALID_INDEX) {
          actionStepIndex = i.value()._modActions.firstKey();
          action = BuildModRemoveRc;
          insert = " Future";
      } else if (stepIndex == BM_LAST_ACTION) {
          action = i.value()._modActions.last();
          actionStepIndex = i.value()._modActions.lastKey();
          insert = " Last";
      } else {
          QMap<int, int>::iterator a = i.value()._modActions.find(stepIndex);
          if (a != i.value()._modActions.end() && defaultIndex == BM_LAST_ACTION) {
              action = i.value()._modActions.value(stepIndex);
              insert = " Current";
          } else if (i.value()._modActions.size()) {
              int keyIndex = stepIndex;
              if (defaultIndex == BM_LAST_ACTION) {
                  action = i.value()._modActions.last();
                  keyIndex = i.value()._modActions.lastKey();
                  insert = " Last";
              } else if (defaultIndex == BM_PREVIOUS_ACTION) {
                  // iterate backward to get the last action index before the specified step index (account for action index gap)
                  for (; keyIndex >= BM_FIRST_INDEX; keyIndex--) {
                      if (i.value()._modActions.value(keyIndex, BM_INVALID_INDEX) > BM_INVALID_INDEX) {
                          action = i.value()._modActions.value(keyIndex);
                          insert = " Previous";
                          break;
                      }
                  }
              } else if (defaultIndex == BM_NEXT_ACTION) {
                  // iterate forward to get the next action index after the specified step index
                  for (; keyIndex < i.value()._modActions.size(); keyIndex++) {
                      if (i.value()._modActions.value(keyIndex, BM_INVALID_INDEX) > BM_INVALID_INDEX) {
                          action = i.value()._modActions.value(keyIndex);
                          insert = " Next";
                          break;
                      }
                  }
              }
              actionStepIndex = keyIndex;
          }
      }
  }

  // BuildMod update 18/07/2022 - this may not be correct as the actual
  // stepIndex request is usually before the buildMod stepIndex - hence
  // the need to 'set' the action. The 'Default' behaviour should be to
  // return BuildModRemoveRc (65) as the BuildMod is in the future.
  // The 'Future' scenario is addressed above but I'm leaving this code
  // in to see if there are any other valid use-cases for it.
  if (!action) {
     action = setBuildModAction(buildModKey, stepIndex, BuildModApplyRc);
     if (insert == " Future")
         insert = " Default (Future)";
     insert = " Default";
  }

  if (!action)
      emit gui->messageSig(LOG_ERROR, QString("Get BuildMod%1 Action: (INVALID), StepIndex: %2, ActionStepIndex: %3, ModKey: '%4'")
                                              .arg(insert)
                                              .arg(stepIndex)
                                              .arg(actionStepIndex)
                                              .arg(buildModKey));
#ifdef QT_DEBUG_MODE
  else
      emit gui->messageSig(LOG_TRACE, QString("Get BuildMod%1 Action: %2, %3ActionStepIndex: %4, ModKey: '%5'")
                                              .arg(insert)
                                              .arg(action ? action == BuildModApplyRc ? "Apply(64)" : "Remove(65)" : "None(0)")
                                              .arg(stepIndex >= 0 ? QString("StepIndex: %1, ").arg(stepIndex) : "")
                                              .arg(actionStepIndex)
                                              .arg(buildModKey));
#endif

  return action;
}

int LDrawFile::setBuildModAction(
        const QString  &buildModKey,
        const int       stepIndex,
        const int       modAction)
{
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);

    int action = modAction;

    if (i != _buildMods.end()) {
        QMap<int, int>::iterator ai = i.value()._modActions.find(stepIndex);
        if (ai != i.value()._modActions.end())
            i.value()._modActions.remove(stepIndex);
        i.value()._modActions.insert(stepIndex, modAction);

        insertBuildModStep(modKey, stepIndex);

        QString modFileName = getBuildModStepKeyModelName(modKey);
        setModified(modFileName, true);
        for (const int modelIndex : i.value()._modSubmodelStack)
            setModified(getSubmodelName(modelIndex), true);

        action = i.value()._modActions.value(stepIndex);

#ifdef QT_DEBUG_MODE
        bool change = modified(modFileName);
        emit gui->messageSig(LOG_DEBUG, QString("Set BuildMod Action: %1, ModKey: '%2', StepIndex: %3, Changed: %4, ModelFile: %5")
                                                .arg(action ? action == BuildModApplyRc ? "Apply(64)" : "Remove(65)" : "None(0)")
                                                .arg(modKey)
                                                .arg(stepIndex)
                                                .arg(change ? "True" : "False")
                                                .arg(modFileName));
#endif
    }

    return action;
}

QMap<int, int>LDrawFile::getBuildModActions(const QString &buildModKey)
{
    QMap<int, int> empty;
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
        return i.value()._modActions;
    }

    return empty;
}

void LDrawFile::clearBuildModAction(const QString &buildModKey,const int stepIndex)
{
    QString  modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
#ifdef QT_DEBUG_MODE
        bool change = false;
        int action = BuildModNoActionRc;
#endif
        QString modFileName = getBuildModStepKeyModelName(modKey);
        QMap<int, int>::iterator a = i.value()._modActions.find(stepIndex);
        if (a != i.value()._modActions.end()) {
#ifdef QT_DEBUG_MODE
            action = i.value()._modActions.value(stepIndex);
#endif
            i.value()._modActions.remove(stepIndex);

            clearBuildModStep(modKey, stepIndex);

            QMap<QString, LDrawSubFile>::iterator s = _subFiles.find(modFileName);
            if (s != _subFiles.end()) {
              s.value()._modified = true;
              s.value()._changedSinceLastWrite = true;
#ifdef QT_DEBUG_MODE
              change = true;
#endif
            }
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_TRACE, QString("Remove BuildMod Action: %1, StepIndex: %2, Changed: %3, ModelFile: %4")
                                                .arg(action ? action == BuildModApplyRc ? "Apply(64)" : "Remove(65)" : "None(0)")
                                                .arg(stepIndex)
                                                .arg(change ? "True" : "False")
                                                .arg(modFileName));
#endif
        }
    }
}

/* This call is used exclusively to navigate backwards when buildMod is enabled */

void LDrawFile::setBuildModNavBackward()
{
#ifdef QT_DEBUG_MODE
    QElapsedTimer timer;
    timer.start();
    int count = 0;
    QString keys;
#endif
    for (const QString &modKey : _buildMods.keys()) {
        for (int stepIndex : _buildMods[modKey]._modActions.keys()) {
            if (stepIndex > _buildModNextStepIndex && stepIndex <= _buildModPrevStepIndex) {
                int action = _buildMods[modKey]._modActions.last();
#ifdef QT_DEBUG_MODE
                int actionStepIndex = _buildMods[modKey]._modActions.lastKey();
                keys.append(QString("'%1' StepIndex: [%2], ").arg(modKey).arg(stepIndex));
                emit gui->messageSig(LOG_TRACE, QString("Get BuildMod Last Action: %1, StepIndex: %2, ActionStepIndex: %3, ModKey: '%4'")
                                                        .arg(action == BuildModApplyRc ? "Apply(64)" : "Remove(65)")
                                                        .arg(stepIndex)
                                                        .arg(actionStepIndex)
                                                        .arg(modKey));
                count++;
#endif
                if (action == BuildModRemoveRc)
                    setBuildModAction(modKey, stepIndex, BuildModApplyRc);
                else
                    setBuildModAction(modKey, stepIndex, BuildModRemoveRc);
            }
        }
    }
#ifdef QT_DEBUG_MODE
    keys.chop(1);
    emit gui->messageSig(LOG_TRACE, QString("BuildMod Jump Backward Updated %1 %2 %3 %4")
                                            .arg(count).arg(count == 1 ? "Key:" : "Keys:").arg(keys).arg(lpub->elapsedTime(timer.elapsed())));
#endif
}

/* Returns index for BEGIN, APPLY and REMOVE BuildMod commands, requires valid TopOfStep */

int LDrawFile::getBuildModStepIndex(const int _modelIndex, const int _lineNumber, bool indexCheck)
{
    LogType logType = LOG_DEBUG;
    QString insert = QString("Get BuildMod");

    int modelIndex = _modelIndex;
    int lineNumber = _lineNumber;
    int stepIndex = BM_INVALID_INDEX;

    // If we are processing the first step, _buildModStepIndexes is not yet initialized.
    if (!_buildModStepIndexes.size() && !modelIndex) {
#ifdef QT_DEBUG_MODE
        insert = QString("Get BuildMod (FIRST STEP)");
#endif
        stepIndex = BM_FIRST_INDEX;

    } else if (modelIndex > BM_INVALID_INDEX) {
        QVector<int> indexKey = { modelIndex, lineNumber };
        stepIndex = _buildModStepIndexes.indexOf(indexKey);
        if (stepIndex == BM_INVALID_INDEX) {
            if (indexCheck) {
                insert = QString("Index Check BuildMod (INVALID)");
            } else {
                logType = LOG_ERROR;
                insert = QString("Get BuildMod (INVALID)");
            }
        }
    }
    else {
        logType = LOG_ERROR;
        insert = QString("Get BuildMod (INVALID)");
    }

    if (logType == LOG_ERROR)
        emit gui->messageSig(logType, QString("%1 StepIndex: %2, ModelIndex: %3, LineNumber %4, ModelName: %5")
                                              .arg(insert)
                                              .arg(stepIndex)
                                              .arg(modelIndex)
                                              .arg(lineNumber)
                                              .arg(getSubmodelName(modelIndex)));
#ifdef QT_DEBUG_MODE
    else
        emit gui->messageSig(logType, QString("%1 StepIndex: %2, ModelIndex: %3, LineNumber %4, ModelName: %5")
                                              .arg(insert)
                                              .arg(stepIndex)
                                              .arg(modelIndex)
                                              .arg(lineNumber)
                                              .arg(getSubmodelName(modelIndex)));
#endif

    return stepIndex;
}

/*  This call uses the global step index (versus the viewer StepKey) */

int LDrawFile::getBuildModStepLineNumber(int stepIndex, bool bottom)
{
    LogType logType = LOG_DEBUG;
    QString message;

    int lineNumber = 0;
    if (stepIndex  > BM_INVALID_INDEX && stepIndex < _buildModStepIndexes.size()) {
        if (bottom) {
            const int bottomStepIndex = stepIndex + 1;
            const int topModelIndex = _buildModStepIndexes.at(stepIndex).at(BM_STEP_MODEL_KEY);
            const int bottomModelIndex = _buildModStepIndexes.at(bottomStepIndex).at(BM_STEP_MODEL_KEY);
            if (bottomModelIndex != topModelIndex) // bottom of step so return number of lines
                lineNumber = size(getSubmodelName(topModelIndex));
            else
                lineNumber = _buildModStepIndexes.at(bottomStepIndex).at(BM_LINE_NUMBER);
        } else /*if (top)*/ {
            lineNumber = _buildModStepIndexes.at(stepIndex).at(BM_LINE_NUMBER);
        }
#ifdef QT_DEBUG_MODE
        message = QString("Get BuildMod %1 LineNumber: %2, StepIndex: %3, ModelName: %4")
                          .arg(bottom ? "BottomOfStep," : "TopOfStep,")
                          .arg(lineNumber).arg(stepIndex)
                          .arg(getSubmodelName(_buildModStepIndexes.at(bottom ? stepIndex + 1 : stepIndex).at(BM_MODEL_NAME)));
#endif
    } else {
        logType = LOG_ERROR;
        message = QString("Get BuildMod (INVALID) %1 LineNumber: %2, StepIndex: %3")
                          .arg(bottom ? "BottomOfStep," : "TopOfStep,")
                          .arg(lineNumber).arg(stepIndex);
    }

    if (logType == LOG_ERROR)
        emit gui->messageSig(logType, message);
#ifdef QT_DEBUG_MODE
    else
        emit gui->messageSig(logType, message);
#endif

    return lineNumber;
}

/* Returns index for BEGIN BuildMod command, requires BuildMod Key*/

int LDrawFile::getBuildModStepIndex(const QString &buildModKey)
{
    QString modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end()) {
      return i.value()._modStepIndex;
    }

    return -1;
}

bool LDrawFile::getBuildModStepIndexWhere(const int stepIndex, QString &modelName,int &modelIndex, int &lineNumber)
{
    LogType logType = LOG_DEBUG;
    QString insert  = QString("Get BuildMod Where");

    bool validIndex = false;
    if (stepIndex > BM_INVALID_INDEX && stepIndex < _buildModStepIndexes.size()) {
        modelIndex = _buildModStepIndexes.at(stepIndex).at(BM_MODEL_NAME);
        lineNumber = _buildModStepIndexes.at(stepIndex).at(BM_LINE_NUMBER);
        modelName  = getSubmodelName(modelIndex);
        validIndex = ! modelName.isEmpty() && lineNumber > 0;
    } else {
        logType = LOG_ERROR;
        insert = QString("Get BuildMod (INVALID) Where");
    }

    if (logType == LOG_ERROR)
        emit gui->messageSig(logType, QString("%1 StepIndex: %2, ModelIndex: %3, LineNumber %4, ModelName: %5")
                                              .arg(insert)
                                              .arg(stepIndex)
                                              .arg(modelIndex)
                                              .arg(lineNumber)
                                              .arg(modelName));
#ifdef QT_DEBUG_MODE
    else
        emit gui->messageSig(logType, QString("%1 StepIndex: %2, ModelIndex: %3, LineNumber %4, ModelName: %5")
                                              .arg(insert)
                                              .arg(stepIndex)
                                              .arg(modelIndex)
                                              .arg(lineNumber)
                                              .arg(modelName));
#endif

  return validIndex;
}

/* Returns the first step index that contains a build modification declaration */
int LDrawFile::getBuildModFirstStepIndex()
{
    if (_buildMods.size())
        return _buildMods.first()._modStepIndex;
    return BM_INVALID_INDEX;
}

int LDrawFile::getBuildModPrevStepIndex()
{
    return _buildModPrevStepIndex;
}

int LDrawFile::getBuildModNextStepIndex()
{
    int lineNumber  = 0;
    LogType logType = LOG_DEBUG;
    QString message;
#ifdef QT_DEBUG_MODE
    bool validIndex = false;
    bool firstIndex = false;
#endif

    int stepIndex   = BM_FIRST_INDEX;

    if (!_buildModStepIndexes.size()) {

        skipHeader(getSubmodelName(0), lineNumber);

#ifdef QT_DEBUG_MODE
        firstIndex = _buildModNextStepIndex == stepIndex;
        validIndex = true;
#endif
    } else if (_buildModNextStepIndex > BM_INVALID_INDEX && _buildModStepIndexes.size() > _buildModNextStepIndex) {

        stepIndex  = _buildModNextStepIndex;
#ifdef QT_DEBUG_MODE
        validIndex = stepIndex != BM_INVALID_INDEX;
#endif
    } else {
#ifdef QT_DEBUG_MODE
        stepIndex = BM_INVALID_INDEX;
        validIndex = false;
#endif
        logType = LOG_ERROR;
        message = QString("Get BuildMod (INVALID) StepIndex: %1")
                          .arg(_buildModNextStepIndex);
    }

    if (logType == LOG_ERROR) {
        emit gui->messageSig(logType, message);
    }
#ifdef QT_DEBUG_MODE
    else
    {
        if (validIndex) {
            QVector<int> topOfStep = { 0,0 };

            if (!firstIndex) {
                topOfStep     = _buildModStepIndexes.at(_buildModNextStepIndex);
                lineNumber    = topOfStep.at(BM_LINE_NUMBER);
            }

            QString modelName = getSubmodelName(topOfStep.at(BM_MODEL_NAME));
            QString insert    = firstIndex ? "First" : "Next";
            validIndex        = !modelName.isEmpty() && lineNumber > BM_BEGIN_LINE_NUM;

            if (!validIndex) {
                insert.prepend("(INVALID) ");
                if (modelName.isEmpty())
                    modelName = "undefined";
                if (lineNumber == -1)
                    lineNumber = BM_BEGIN_LINE_NUM;
            }
            message = QString("Get BuildMod %1 "
                              "StepIndex: %2, "
                              "PrevStepIndex: %3, "
                              "ModelName: %4, "
                              "LineNumber: %5, "
                              "Result: %6")
                              .arg(insert)
                              .arg(_buildModNextStepIndex)
                              .arg(_buildModPrevStepIndex)
                              .arg(modelName)
                              .arg(lineNumber)
                              .arg(validIndex ? "OK" : "KO");
        }

        emit gui->messageSig(logType, message);
    }
#endif

    return stepIndex;

}

bool LDrawFile::setBuildModNextStepIndex(const QString &modelName, const int &lineNumber)
{
    int  newStepIndex = BM_INVALID_INDEX;

    if (!_buildModStepIndexes.size() && modelName == topLevelFile())
        newStepIndex = BM_FIRST_INDEX;
    else
        newStepIndex = getStepIndex(modelName,lineNumber);

    bool validIndex  = newStepIndex > BM_INVALID_INDEX;
    bool firstIndex  = _buildModNextStepIndex == BM_INVALID_INDEX;
    bool nextIndex   = validIndex && !firstIndex;

    _buildModPrevStepIndex = nextIndex ? _buildModNextStepIndex : BM_FIRST_INDEX;
    _buildModNextStepIndex = newStepIndex;

#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_TRACE, QString("Set BuildMod %1 "
                                            "StepIndex: %2, "
                                            "PrevStepIndex: %3, "
                                            "ModelName: %4, "
                                            "LineNumber: %5, "
                                            "Result: %6")
                                            .arg(firstIndex ? "First" : "Next")
                                            .arg(_buildModNextStepIndex)
                                            .arg(_buildModPrevStepIndex)
                                            .arg(modelName)
                                            .arg(lineNumber)
                                            .arg(validIndex ? "OK" : "KO"));
#endif

    return validIndex;
}

int LDrawFile::getStepIndex(const QString &modelName, const int &lineNumber)
{
    int modelIndex = getSubmodelIndex(modelName);
    int stepIndex  = BM_INVALID_INDEX;

    if (!_buildModStepIndexes.size() && !modelIndex)
        stepIndex = BM_FIRST_INDEX;
    else {
        QVector<int> topOfStep = { modelIndex, lineNumber };
        stepIndex = _buildModStepIndexes.indexOf(topOfStep);
        if (stepIndex == BM_INVALID_INDEX) {
            int topLineNumber = lineNumber;
            stepIndex = getTopOfStep(modelName, modelIndex, topLineNumber);
        }
    }

    return stepIndex;
}

int LDrawFile::getTopOfStep(const QString &modelName, int &modelIndex, int &lineNumber)
{
    if (modelIndex == BM_INVALID_INDEX)
        modelIndex = getSubmodelIndex(modelName);

    QVector<int> topOfStep = { modelIndex, lineNumber };
    int stepIndex = _buildModStepIndexes.indexOf(topOfStep);

    if (stepIndex == BM_INVALID_INDEX) {
        int topLineNumber = 0;
        for (QVector<int> &topOfStep : _buildModStepIndexes) {
            if (topOfStep.at(BM_STEP_MODEL_KEY) == modelIndex) {
                topLineNumber = topOfStep.at(BM_STEP_LINE_KEY);
                if (topLineNumber == lineNumber) {
                    stepIndex = _buildModStepIndexes.indexOf(topOfStep);
                } else if (topLineNumber > lineNumber) {
                    stepIndex = _buildModStepIndexes.indexOf(topOfStep) - 1;
                    if (stepIndex > BM_INVALID_INDEX) {
                        int topModelIndex = _buildModStepIndexes[stepIndex].at(BM_STEP_MODEL_KEY);
                        if (topModelIndex == modelIndex) {
                            lineNumber = _buildModStepIndexes[stepIndex].at(BM_STEP_LINE_KEY);
                        } else {
                            emit gui->messageSig(LOG_ERROR, QObject::tr("TopOfStep modelIndex %1 (%2) - lineNumber %3 is below model start, returned modelIndex %4.")
                                                                        .arg(modelIndex).arg(modelName).arg(lineNumber).arg(topModelIndex));
                            return BM_INVALID_INDEX;
                        }
                    }
                }
            }
            // When lineNumber is greater than last step topOfStep lineNumber but
            // less than model max lines, set the stepIndex of the last step topOfStep
            else if (stepIndex == BM_INVALID_INDEX && topLineNumber) {
                int modelLines = size(modelName);
                if (lineNumber <= modelLines) {
                    lineNumber = topLineNumber;
                    stepIndex = getTopOfStep(modelName, modelIndex, lineNumber);
                } else {
                    emit gui->messageSig(LOG_ERROR, QObject::tr("TopOfStep modelIndex %1 (%2) - lineNumber %3 exceedes model lines %4.")
                                                                .arg(modelIndex).arg(modelName).arg(lineNumber).arg(modelLines));
                    return stepIndex;
                }
            }
            if (stepIndex > BM_INVALID_INDEX) {
                break;
            }
        }
    }

    return stepIndex;
}

QString LDrawFile::getViewerStepKey(const int stepIndex)
{
    QString stepKey;
    if (stepIndex > BM_INVALID_INDEX) {
        QVector<int> topOfStep =  _buildModStepIndexes.at(stepIndex);
        const int lineNumber = topOfStep.at(BM_STEP_LINE_KEY);
        const int modelIndex = topOfStep.at(BM_STEP_MODEL_KEY);
        stepKey = QString("%1;%2;0").arg(modelIndex).arg(lineNumber);

        QMap<QString, ViewerStep>::const_iterator i = _viewerSteps.constBegin();
        while (i != _viewerSteps.constEnd()) {
            bool validType = i->_viewType == Options::CSI || i->_viewType == Options::SMI;
            if (validType && i->_stepKey.modIndex == modelIndex && i->_stepKey.lineNum == lineNumber) {
                if (i->_stepKey.stepNum) {
                    stepKey.chop(1);
                    stepKey.append(QString::number(i->_stepKey.stepNum));
                }
                if (!i->_keySuffix.isEmpty())
                    stepKey.append(i->_keySuffix);
                break;
            }
            ++i;
        }
    }
    return stepKey;
}

QString LDrawFile::getViewerStepKeyWhere(const int modelIndex, const int lineNumber)
{
    Where here(getSubmodelName(modelIndex),modelIndex,lineNumber);
    int stepIndex = getTopOfStep(here.modelName, here.modelIndex, here.lineNumber);

    return getViewerStepKey(stepIndex);
}

QString LDrawFile::getViewerStepKeyFromRange(const int modelIndex, const int lineNumber, const int topModelIndex, const int topLineNumber, const int bottomModelIndex, const int bottomLineNumber)
{
    Where here(getSubmodelName(modelIndex), modelIndex, lineNumber);
    int stepIndex = getTopOfStep(here.modelName, here.modelIndex, here.lineNumber);
    here = Where(getSubmodelName(topModelIndex), topModelIndex, topLineNumber);
    int topStepIndex = getTopOfStep(here.modelName, here.modelIndex, here.lineNumber);
    here = Where(getSubmodelName(bottomModelIndex), bottomModelIndex, bottomLineNumber);
    int bottomStepIndex = getTopOfStep(here.modelName, here.modelIndex, here.lineNumber);

    if (stepIndex >= topStepIndex && stepIndex <= bottomStepIndex)
        return getViewerStepKeyWhere(modelIndex, lineNumber);

    return QString();
}

/* This function returns the buildModKey for the specified topOfStep */

QString LDrawFile::getBuildModKey(const QString &modelName, const int &lineNumber)
{
    int stepIndex = getStepIndex(modelName, lineNumber);
    QMap<int, BuildModStep>::iterator i = _buildModSteps.find(stepIndex);
    while (i != _buildModSteps.end()) {
        if (i.key() == stepIndex && i.value()._buildModStepIndex == stepIndex)
            return i.value()._buildModKey;
        ++i;
    }
    return QString();
}

/* This function returns the equivalent of the ViewerStepKey */

QString LDrawFile::getBuildModStepKey(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end()) {
    QString stepKey = QString("%1;%2;%3")
                              .arg(i.value()._modAttributes.at(BM_MODEL_NAME_INDEX))
                              .arg(i.value()._modAttributes.at(BM_MODEL_LINE_NUM))
                              .arg(i.value()._modAttributes.at(BM_MODEL_STEP_NUM));
#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_DEBUG, QString("Get BuildMod StepKey: %1").arg(stepKey));
#endif
    return stepKey;
  }

  return QString();
}

QString LDrawFile::getBuildModStepKeyModelName(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_MODEL_NAME_INDEX) {
    int index = i.value()._modAttributes.at(BM_MODEL_NAME_INDEX);
    return getSubmodelName(index);
  }

  return QString();
}

int LDrawFile::getBuildModStepKeyLineNum(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_MODEL_LINE_NUM) {
    return i.value()._modAttributes.at(BM_MODEL_LINE_NUM);
  }

  return 0;
}

int LDrawFile::getBuildModStepKeyStepNum(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_MODEL_STEP_NUM) {
    return i.value()._modAttributes.at(BM_MODEL_STEP_NUM);
  }

  return 0;
}

int LDrawFile::getBuildModStepKeyModelIndex(const QString &buildModKey)
{
  QString modKey = buildModKey.toLower();
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_MODEL_NAME_INDEX) {
    int index = i.value()._modAttributes.at(BM_MODEL_NAME_INDEX);
    return index;
  }

  return 0;
}

/* return paths - using buildMod key - to the end of the submodel and set parent submodels and viewer steps to modified */

QStringList LDrawFile::getPathsFromBuildModKeys(const QStringList &buildModKeys)
{
#ifdef QT_DEBUG_MODE
    int subModelStackCount = 0;
#endif
  QStringList imageFilePaths;
  for (const QString &buildModKey : buildModKeys) {
    const QString modKey = buildModKey.toLower();
    QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
    if (i != _buildMods.end() && i.value()._modAttributes.size() > BM_MODEL_STEP_NUM) {
      ViewerStep::StepKey viewerStepKey = { i.value()._modAttributes.at(BM_MODEL_NAME_INDEX),
                                            i.value()._modAttributes.at(BM_MODEL_LINE_NUM),
                                            i.value()._modAttributes.at(BM_MODEL_STEP_NUM) };

      setModified(getSubmodelName(viewerStepKey.modIndex), true);
      if (i.value()._modSubmodelStack.size())
        for (const int index : i.value()._modSubmodelStack)
          setModified(getSubmodelName(index), true);

#ifdef QT_DEBUG_MODE
      subModelStackCount = i.value()._modSubmodelStack.size() + 1;
#endif

      QMap<QString, ViewerStep>::iterator si = _viewerSteps.begin();
      while (si != _viewerSteps.end()) {
        if (viewerStepKey.modIndex == si->_stepKey.modIndex && si->_viewType == Options::CSI) {
          if (viewerStepKey.stepNum <= si->_stepKey.stepNum) {
            if (modified(getSubmodelName(viewerStepKey.modIndex))) {
              si->_modified = true;
              if (QFileInfo(si->_imagePath).exists()) {
                imageFilePaths.append(si->_imagePath);
              }
            }
          }
        }
        ++si;
      }
    }
  }

  if (imageFilePaths.size() > 1)
    imageFilePaths.removeDuplicates();

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Get BuildMod Modified Subfiles: %1, Image File Paths %2, %3 %4")
                                                .arg(subModelStackCount)
                                                .arg(imageFilePaths.size())
                                                .arg(buildModKeys.size() == 1 ? "ModelKey:" : "ModelKeys:")
                                                .arg(buildModKeys.join(" ")));
#endif

  return imageFilePaths;
}

/* return the number of build mods within the submodel file */

int LDrawFile::getBuildModsCount(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._buildMods;
  }
  return 0;
}

void LDrawFile::setBuildModsCount(const QString &mcFileName, const int value)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    if (value == BM_NONE)
      i.value()._buildMods += 1;
    else
      i.value()._buildMods = value;
  }
}

bool LDrawFile::getBuildModExists(const QString &mcFileName, const QString &buildModKey)
{
    QString fileName = mcFileName.toLower();
    QRegExp buildModBeginRx("^0 !LPUB BUILD_MOD BEGIN ");
    for(const QString &line : lpub->ldrawFile.contents(fileName)) {
        if (line[0] == '1')
            continue;
        if (line.contains(buildModBeginRx))
            if (line.contains(buildModKey,Qt::CaseInsensitive))
                return true;
    }
    return false;
}

bool LDrawFile::buildModContains(const QString &buildModKey)
{
  QString modKey = buildModKey;
  return _buildModList.contains(modKey,Qt::CaseInsensitive);
}

QStringList LDrawFile::getBuildModsList()
{
  return _buildModList;
}

int LDrawFile::buildModsCount()
{
  return _buildMods.size();
}

void LDrawFile::clearBuildModSteps()
{
    _buildModSteps.clear();
}

/* Visual Editor routines */

void LDrawFile::insertViewerStep(const QString     &stepKey,
                                 const QStringList &rotatedViewerContents,
                                 const QStringList &rotatedContents,
                                 const QStringList &unrotatedContents,
                                 const QString     &filePath,
                                 const QString     &imagePath,
                                 const QString     &csiKey,
                                 bool               multiStep,
                                 bool               calledOut,
                                 int                viewType)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    _viewerSteps.erase(i);
  }

  const QStringList keyList = stepKey.split("_");
  const QStringList keys = keyList.size() > 1 ? keyList.first().split(";") : stepKey.split(";");

  ViewerStep viewerStep(keys,rotatedViewerContents,rotatedContents,unrotatedContents,filePath,imagePath,csiKey,multiStep,calledOut,viewType);

  viewerStep._keySuffix = keyList.size() > 1 ? QString("_%1").arg(keyList.last()) : QString();
  viewerStep._partCount = rotatedContents.size();

  _viewerSteps.insert(stepKey,viewerStep);

#ifdef QT_DEBUG_MODE
  const QString debugMessage =
          QString("Insert %1 ViewerStep Key: '%2' [%3 %4 StepNumber: %5], Type: [%6]")
                  .arg(viewType == Options::PLI ? "PLI" : viewType == Options::CSI ? "CSI" : "SMI")
                  .arg(stepKey)
                  .arg(viewType == Options::PLI ? QString("PartName: %1,").arg(keys.at(BM_STEP_MODEL_KEY)) :
                                                  QString("ModelIndex: %1 (%2),").arg(keys.at(BM_STEP_MODEL_KEY)).arg(gui->getSubmodelName(keys.at(BM_STEP_MODEL_KEY).toInt())))
                  .arg(viewType == Options::PLI ? QString("Colour: %1,").arg(keys.at(BM_STEP_LINE_KEY)) :
                                                  QString("LineNumber: %1,").arg(keys.at(BM_STEP_LINE_KEY)))
                  .arg(keys.at(BM_STEP_NUM_KEY))
                  .arg(calledOut ? "called out" : multiStep ? "step group" : viewType == Options::PLI ? "part" : "single step");
  emit gui->messageSig(LOG_DEBUG, debugMessage);
#endif
}

/* Viewer Step Exist */

void LDrawFile::updateViewerStep(const QString &stepKey, const QStringList &contents, bool rotated)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);

  if (i != _viewerSteps.end()) {
    if (rotated)
      i.value()._rotatedViewerContents = contents;
    else
      i.value()._unrotatedContents = contents;
    i.value()._partCount = 0;
    for (const QString &line : contents)
      if (line[0] == '1')
        i.value()._partCount++;
    i.value()._modified = true;
  }
}

/* return viewer step Content line */

QString LDrawFile::getViewerStepContentLine(const QString &stepKey, const int lineTypeIndex, bool rotated, bool relative)
{
  if (lineTypeIndex == BM_INVALID_INDEX)
      return QString();

  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    if (rotated) {
      if (i.value()._rotatedContents.size()) {
        if (relative) {
          if (i.value()._rotatedContents.size() > lineTypeIndex)
            return i.value()._rotatedContents.at(lineTypeIndex);
        } else {
          for (int j = 0; j < i.value()._rotatedContents.size(); ++j)
            if (j == lineTypeIndex)
              return i.value()._rotatedContents.at(j);
        }
      }
    } else {
      if (i.value()._unrotatedContents.size()) {
        if (relative) {
          if (i.value()._unrotatedContents.size() > lineTypeIndex)
            return i.value()._unrotatedContents.at(lineTypeIndex);
        } else {
          for (int j = 0; j < i.value()._unrotatedContents.size(); ++j)
            if (j == lineTypeIndex)
              return i.value()._unrotatedContents.at(j);
        }
      }
    }
  }
  return QString();
}

/* return viewer step Contents. */

QStringList LDrawFile::getViewerStepContents(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._rotatedContents;
  }
  return _emptyList;
}

/* return viewer step rotatedContents - Called by viewer project*/

QStringList LDrawFile::getViewerStepRotatedContents(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._rotatedViewerContents;
  }
  return _emptyList;
}

/* return viewer step unrotatedContents */

QStringList LDrawFile::getViewerStepUnrotatedContents(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._unrotatedContents;
  }
  return _emptyList;
}

/* return viewer step file path */

QString LDrawFile::getViewerStepFilePath(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._filePath;
  }
  return _emptyString;
}

/* return paths - using viewer step key - to the end of the submodel and set parent submodels and viewer steps to modified */

QStringList LDrawFile::getPathsFromViewerStepKey(const QString &stepKey)
{
  QStringList list = stepKey.split(";");
  if (list.size() == BM_SUBMODEL_STACK) {
    QStringList submodelStack = list.takeFirst().split(":");
    if (submodelStack.size())
      for (const QString &index : submodelStack)
        setModified(getSubmodelName(index.toInt()),true);
  }

  ViewerStep::StepKey viewerStepKey = { list.at(BM_STEP_MODEL_KEY).toInt(),
                                        list.at(BM_STEP_LINE_KEY).toInt(),
                                        list.at(BM_STEP_NUM_KEY).toInt() };

  setModified(getSubmodelName(viewerStepKey.modIndex), true);

  list.clear();
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.begin();
  while (i != _viewerSteps.end()) {
    if (viewerStepKey.modIndex == i->_stepKey.modIndex && i->_viewType == Options::CSI) {
      if (viewerStepKey.stepNum <= i->_stepKey.stepNum) {
        if (modified(getSubmodelName(viewerStepKey.modIndex))) {
          i->_modified = true;
          if (QFileInfo(i->_imagePath).exists()) {
            list.append(i->_imagePath);
          }
        }
      }
    }
    ++i;
  }

  if (list.size() > 1)
    list.removeDuplicates();

  return list;
}

/* return viewer step image path */

QString LDrawFile::getViewerStepImagePath(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._imagePath;
  }
  return _emptyString;
}

/* return viewer step CSI key */

QString LDrawFile::getViewerConfigKey(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._csiKey;
  }
  return _emptyString;
}

/* return viewer step part count */

int LDrawFile::getViewerStepPartCount(const QString &stepKey)
{
    QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
    if (i != _viewerSteps.end()) {
      return i.value()._partCount;
    }
    return 0;
}

/* Viewer Step Exist */

bool LDrawFile::viewerStepContentExist(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);

  if (i != _viewerSteps.end()) {
    return true;
  }
  return false;
}

/* Delete Viewer Step */

bool LDrawFile::deleteViewerStep(const QString &stepKey)
{
    QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
    if (i != _viewerSteps.end()) {
        _viewerSteps.erase(i);
        return true;
    }
    return false;
}

/* return viewer step is multiStep */

bool LDrawFile::isViewerStepMultiStep(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._multiStep;
  } else {
    return false;
  }
}

/* return viewer step is calledOut */

bool LDrawFile::isViewerStepCalledOut(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._calledOut;
  } else {
    return false;
  }
}

/* return viewer step is currently performing a build modification action  */

bool LDrawFile::getViewerStepHasBuildModAction(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._hasBuildModAction;
  } else {
    return false;
  }
}

bool LDrawFile::setViewerStepHasBuildModAction(const QString &stepKey, bool value)
{
  bool rc = false;
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if ((rc = i != _viewerSteps.end())) {
    i.value()._hasBuildModAction = value;
#ifdef QT_DEBUG_MODE
    int viewType = i.value()._viewType;
    const QString debugMessage = QString("Set %1 ViewerStep Key: '%2', ModelIndex: %3 (%4), LineNumber: %5, StepNumber: %6, HasBuildModAction: %7.")
                                         .arg(viewType == Options::PLI ? "PLI" : viewType == Options::CSI ? "CSI" : "SMI")
                                         .arg(stepKey)
                                         .arg(i.value()._stepKey.modIndex)
                                         .arg(getSubmodelName(i.value()._stepKey.modIndex))
                                         .arg(i.value()._stepKey.lineNum)
                                         .arg(i.value()._stepKey.stepNum)
                                         .arg(value ? "Yes" : "No");
    emit gui->messageSig(LOG_DEBUG, debugMessage);
    //qDebug() << qPrintable(QString("DEBUG: %1").arg(debugMessage));
#endif
  }
#ifdef QT_DEBUG_MODE
  else if (!stepKey.isEmpty()) {
    const QStringList Keys = stepKey.split(";");
    const QString noticeMessage = QString("Cannot set ViewerStep BuildMod Action for Key: '%5', ModelIndex: %1 (%2), LineNumber: %3, StepNumber: %4. Key does not exist.")
                                           .arg(Keys.at(BM_STEP_MODEL_KEY))
                                           .arg(getSubmodelName(Keys.at(BM_STEP_MODEL_KEY).toInt()))
                                           .arg(Keys.at(BM_STEP_LINE_KEY))
                                           .arg(Keys.at(BM_STEP_NUM_KEY))
                                           .arg(stepKey);
    emit gui->messageSig(LOG_NOTICE, noticeMessage);
  }
#endif
  return rc;
}


bool LDrawFile::viewerStepModified(const QString &stepKey, bool reset)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    bool modified = i.value()._modified;
    if (reset)
      i.value()._modified = false;
#ifdef QT_DEBUG_MODE
    const QStringList keys = stepKey.split(";");
    int viewType = i.value()._viewType;
    const QString debugMessage =
            QString("%1%2 ViewerStep Key: '%3' [%4 %5 StepNumber: %6] %7")
                    .arg(reset && modified ? "Reset " : "")
                    .arg(viewType == Options::PLI ? "PLI" : viewType == Options::CSI ? "CSI" : "SMI")
                    .arg(stepKey)
                    .arg(viewType == Options::PLI ? QString("PartName: %1,").arg(keys.at(BM_STEP_MODEL_KEY)) :
                                                    QString("ModelIndex: %1 (%2),").arg(keys.at(BM_STEP_MODEL_KEY)).arg(gui->getSubmodelName(keys.at(BM_STEP_MODEL_KEY).toInt())))
                    .arg(viewType == Options::PLI ? QString("Colour: %1,").arg(keys.at(BM_STEP_LINE_KEY)) :
                                                    QString("LineNumber: %1,").arg(keys.at(BM_STEP_LINE_KEY)))
                    .arg(keys.at(BM_STEP_NUM_KEY))
                    .arg(i.value()._modified ? ", Modified: [Yes]." :", Modified: [No].");
    emit gui->messageSig(LOG_DEBUG, debugMessage);
#endif
    return modified;
  } else {
    return false;
  }
}

void LDrawFile::setViewerStepModified(const QString &stepKey)
{
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(stepKey);
  if (i != _viewerSteps.end()) {
    i.value()._modified = true;
#ifdef QT_DEBUG_MODE
    int viewType = i.value()._viewType;
    const QString debugMessage =
            QString("Set %1 ViewerStep Key: '%2', ModelIndex: %3 (%4), LineNumber: %5, StepNumber: %6, Modified: [Yes].")
                    .arg(viewType == Options::PLI ? "PLI" : viewType == Options::CSI ? "CSI" : "SMI")
                    .arg(stepKey)
                    .arg(i.value()._stepKey.modIndex)
                    .arg(getSubmodelName(i.value()._stepKey.modIndex))
                    .arg(i.value()._stepKey.lineNum)
                    .arg(i.value()._stepKey.stepNum);
    emit gui->messageSig(LOG_DEBUG, debugMessage);
#endif
  }
#ifdef QT_DEBUG_MODE
  else {
    const QStringList Keys = stepKey.split(";");
    const QString warnMessage =
            QString("Cannot modify, ViewerStep  Key: '%5', ModelIndex: %1 (%2), LineNumber: %3, StepNumber: %4. Key does not exist.")
                    .arg(Keys.at(BM_STEP_MODEL_KEY))
                    .arg(getSubmodelName(Keys.at(BM_STEP_MODEL_KEY).toInt()))
                    .arg(Keys.at(BM_STEP_LINE_KEY))
                    .arg(Keys.at(BM_STEP_NUM_KEY))
                    .arg(stepKey);
    emit gui->messageSig(LOG_WARNING, warnMessage);
    //qDebug() << qPrintable(QString("WARNING: %1").arg(warnMessage));
  }
#endif
}

/* Clear ViewerSteps */

void LDrawFile::clearViewerSteps()
{
  _viewerSteps.clear();
}

void LDrawFile::skipHeader(const QString &modelName, int &lineNumber)
{
    int numLines = size(modelName);
    for ( ; lineNumber < numLines; lineNumber++) {
        QString line = readLine(modelName,lineNumber);
        if (line.isEmpty())
            continue;
        int p;
        for (p = 0; p < line.size(); ++p) {
            if (line[p] != ' ') {
                break;
            }
        }
        if (line[p] >= '1' && line[p] <= '5') {
            if (lineNumber > 0) {
                --lineNumber;
            }
            break;
        } else if ( ! isHeader(line)) {
            if (lineNumber != 0) {
                --lineNumber;
                break;
            }
        }
    }
}

// -- -- Utility Functions -- -- //

int split(const QString &line, QStringList &argv)
{
  QString     chopped = line;
  int         p = 0;
  int         length = chopped.length();

  // line length check
  if (p == length) {
      return 0;
    }
  // eol check
  while (chopped[p] == ' ') {
      if (++p == length) {
          return -1;
        }
    }

  argv.clear();

  // if line starts with 1 (part line)
  if (chopped[p] == '1') {

      // line length check
      argv << "1";
      p += 2;
      if (p >= length) {
          return -1;
        }
      // eol check
      while (chopped[p] == ' ') {
          if (++p >= length) {
              return -1;
            }
        }

      // color x y z a b c d e f g h i //

      // populate argv with part line tokens
      for (int i = 0; i < 13; i++) {
          QString token;

          while (chopped[p] != ' ') {
              token += chopped[p];
              if (++p >= length) {
                  return -1;
                }
            }
          argv << token;
          while (chopped[p] == ' ') {
              if (++p >= length) {
                  return -1;
                }
            }
        }

      argv << chopped.mid(p);

      if (argv.size() > 1 && argv[1] == "WRITE") {
          argv.removeAt(1);
        }

    } else if (chopped[p] >= '2' && chopped[p] <= '5') {
      chopped = chopped.mid(p);
      argv << chopped.split(" ", SkipEmptyParts);
    } else if (chopped[p] == '0') {

      /* Parse the input line into argv[] */

      int soq = validSoQ(chopped,chopped.indexOf("\""));
      if (soq == -1) {
          argv << chopped.split(" ", SkipEmptyParts);
        } else {
          // quotes found
          while (chopped.size()) {
              soq = validSoQ(chopped,chopped.indexOf("\""));
              if (soq == -1) {
                  argv << chopped.split(" ", SkipEmptyParts);
                  chopped.clear();
                } else {
                  QString left = chopped.left(soq);
                  left = left.trimmed();
                  argv << left.split(" ", SkipEmptyParts);
                  chopped = chopped.mid(soq+1);
                  soq = validSoQ(chopped,chopped.indexOf("\""));
                  if (soq == -1) {
                      argv << left;
                      return -1;
                    }
                  argv << chopped.left(soq);
                  chopped = chopped.mid(soq+1);
                  if (chopped == "\"") {
                    }
                }
            }
        }

      if (argv.size() > 1 && argv[0] == "0" && argv[1] == "GHOST") {
          argv.removeFirst();
          argv.removeFirst();
        }
    }

  return 0;
}

// check for escaped quotes
int validSoQ(const QString &line, int soq){

  int nextq;
//  logTrace() << "\n  A. START VALIDATE SoQ"
//             << "\n SoQ (at Index):   " << soq
//             << "\n Line Content:     " << line;
  if(soq > 0 && line.at(soq-1) == '\\' ){
      nextq = validSoQ(line,line.indexOf("\"",soq+1));
      soq = nextq;
    }
//  logTrace() << "\n  D. END VALIDATE SoQ"
//             << "\n SoQ (at Index):   " << soq;
  return soq;
}

LDrawFile::LDrawFile() : ldrawMutex(QMutex::Recursive)
{
  _loadedParts.clear();
  {
    _fileRegExp
        << QRegExp("^0\\s+FILE\\s+(.*)$",Qt::CaseInsensitive)       //SOF_RX
        << QRegExp("^0\\s+NOFILE\\s*$",Qt::CaseInsensitive)         //EOF_RX
        << QRegExp("^1\\s+.*$",Qt::CaseInsensitive)                 //LDR_RX
        << QRegExp("^0\\s+AUTHOR:?\\s+(.*)$",Qt::CaseInsensitive)   //AUT_RX
        << QRegExp("^0\\s+NAME:?\\s+(.*)$",Qt::CaseInsensitive)     //NAM_RX
        << QRegExp("^0\\s+!?CATEGORY\\s+(.*)$",Qt::CaseInsensitive) //CAT_RX
        << QRegExp("^0\\s+!?LPUB\\s+INCLUDE\\s+[\"']?([^\"']*)[\"']?$",Qt::CaseSensitive) //INC_RX
        << QRegExp("^(?!0 !?LPUB|0 FILE |0 NOFILE|0 !?LEOCAD|0 !?LDCAD|0 MLCAD|0 GHOST|0 !?SYNTH|[1-5]).*$",Qt::CaseSensitive) //DES_RX
        << QRegExp("^0\\s+!?LDCAD\\s+GROUP_DEF.*\\s+\\[LID=(\\d+)\\]\\s+\\[GID=([\\d\\w]+)\\]\\s+\\[name=(.[^\\]]+)\\].*$",Qt::CaseInsensitive) //LDG_RX
        << QRegExp("^0\\s+!?LDCAD\\s+(CONTENT|PATH_POINT|PATH_SKIN|GENERATED)[^\n]*") // LDC_RX
        ;
  }

  {
    LDrawHeaderRegExp
        << QRegExp("^0\\s+FILE\\s+(.*)$",Qt::CaseInsensitive)
        << QRegExp("^0\\s+AUTHOR:?[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+BFC[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+!?CATEGORY[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+CLEAR[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+!?COLOUR[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+!?CMDLINE[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+!?HELP[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+!?HISTORY[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+!?KEYWORDS[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+!?LDRAW_ORG[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+!?LICENSE[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+NAME:?[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+OFFICIAL[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+ORIGINAL\\s+LDRAW[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+PAUSE[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+PRINT[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+ROTATION[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+SAVE[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+UNOFFICIAL[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+UN-OFFICIAL[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+WRITE[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+~MOVED\\s+TO[^\n]*",Qt::CaseInsensitive)
        << QRegExp("^0\\s+NOFILE\\s*$",Qt::CaseInsensitive)
           ;
  }

  {
      LDrawUnofficialPartRegExp
              << QRegExp("^0\\s+!?UNOFFICIAL\\s+PART[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Part)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Part Alias)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Part Physical_Colour)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Part Physical Colour)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Part)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Part Alias)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Part Physical_Colour)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Part Physical Colour)[^\n]*",Qt::CaseInsensitive)
              ;
  }

  {
      LDrawUnofficialSubPartRegExp
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Subpart)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Subpart)[^\n]*",Qt::CaseInsensitive)
                 ;
  }

  {
      LDrawUnofficialPrimitiveRegExp
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Primitive)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_8_Primitive)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_48_Primitive)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Primitive)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial 8_Primitive)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial 48_Primitive)[^\n]*",Qt::CaseInsensitive)
              ;
  }

  {
      LDrawUnofficialShortcutRegExp
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Shortcut)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Shortcut Alias)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Shortcut Physical_Colour)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Shortcut Physical Colour)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Shortcut)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Shortcut Alias)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Shortcut Physical_Colour)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Shortcut Physical Colour)[^\n]*",Qt::CaseInsensitive)
              ;
  }
}

bool isHeader(QString &line)
{
  int size = LDrawHeaderRegExp.size();

  for (int i = 0; i < size; i++) {
    if (line.contains(LDrawHeaderRegExp[i])) {
      return true;
    }
  }
  return false;
}

bool isComment(QString &line){
  QRegExp commentLine("^\\s*0\\s+\\/\\/\\s*.*$");
  if (line.contains(commentLine))
      return true;
  return false;
}

/*
 * Assume extensions are up to 4 chars in length so part.lfx_01
 * is not considered as having an extension, but part.dat
 * is considered as having extensions
 */
bool isSubstitute(QString &line, QString &lineOut){
  QRegExp substitutePartRx("\\sBEGIN\\sSUB\\s(.*(?:\\.dat|\\.ldr)|[^.]{5})",Qt::CaseInsensitive);
  if (line.contains(substitutePartRx)) {
      lineOut = substitutePartRx.cap(1);
      emit gui->messageSig(LOG_NOTICE,QString("Part [%1] is a SUBSTITUTE").arg(lineOut));
      return true;
  }
  lineOut = QString();
  return false;
}

int getUnofficialFileType(QString &line)
{
  int size = LDrawUnofficialPartRegExp.size();
  for (int i = 0; i < size; i++) {
    if (line.contains(LDrawUnofficialPartRegExp[i])) {
      return UNOFFICIAL_PART;
    }
  }
  size = LDrawUnofficialSubPartRegExp.size();
  for (int i = 0; i < size; i++) {
    if (line.contains(LDrawUnofficialSubPartRegExp[i])) {
      return UNOFFICIAL_SUBPART;
    }
  }
  size = LDrawUnofficialPrimitiveRegExp.size();
  for (int i = 0; i < size; i++) {
    if (line.contains(LDrawUnofficialPrimitiveRegExp[i])) {
      return UNOFFICIAL_PRIMITIVE;
    }
  }
  size = LDrawUnofficialShortcutRegExp.size();
  for (int i = 0; i < size; i++) {
    if (line.contains(LDrawUnofficialShortcutRegExp[i])) {
      return UNOFFICIAL_SHORTCUT;
    }
  }

  return UNOFFICIAL_UNKNOWN;
}

bool isGhost(QString &line){
  QRegExp ghostMeta("^\\s*0\\s+GHOST\\s+.*$");
  if (line.contains(ghostMeta))
      return true;
  return false;
}
