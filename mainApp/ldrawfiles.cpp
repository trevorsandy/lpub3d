
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2021 Trevor SANDY. All rights reserved.
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
        // add level object to 'all' levels
        HiarchLevel* level = addLevel(key, true);

        // if there are 'current' level objects...
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
            // remove last level object from the 'current' list - reset to parent level if exists
            LDrawFile::_currentLevels.removeAt(LDrawFile::_currentLevels.size() - 1);
        }
    }

    return LDrawFile::_currentLevels.size();
}

/********************************************
 *
 ********************************************/

QStringList LDrawFile::_loadedParts;
QString LDrawFile::_file           = "";
QString LDrawFile::_name           = "";
QString LDrawFile::_author         = VER_PRODUCTNAME_STR;
QString LDrawFile::_description    = PUBLISH_DESCRIPTION_DEFAULT;
QString LDrawFile::_category       = "";
int     LDrawFile::_emptyInt;
int     LDrawFile::_partCount      = 0;
bool    LDrawFile::_currFileIsUTF8 = false;
bool    LDrawFile::_showLoadMessages = false;
bool    LDrawFile::_loadAborted    = false;

LDrawSubFile::LDrawSubFile(
  const QStringList &contents,
  QDateTime         &datetime,
  int                unofficialPart,
  bool               generated,
  bool               includeFile,
  const QString     &subFilePath)
{
  _contents << contents;
  _subFilePath = subFilePath;
  _datetime = datetime;
  _modified = false;
  _numSteps = 0;
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
                       const QStringList &rotatedContents,
                       const QStringList &unrotatedContents,
                       const QString     &filePath,
                       const QString     &imagePath,
                       const QString     &csiKey,
                       bool               multiStep,
                       bool               calledOut,
                       int                viewType)
{
    _rotatedContents   << rotatedContents;
    _unrotatedContents << unrotatedContents;
    _partCount = 0;
    _filePath  = filePath;
    _imagePath = imagePath;
    _csiKey    = csiKey;
    _modified  = false;
    _multiStep = multiStep;
    _calledOut = calledOut;
    _viewType= viewType;
    _stepKey   = { stepKey[BM_STEP_MODEL_KEY].toInt(), stepKey[BM_STEP_LINE_KEY].toInt(), stepKey[BM_STEP_NUM_KEY].toInt() };
}

void LDrawFile::empty()
{
  _subFiles.clear();
  _configuredSubFiles.clear();
  _subFileOrder.clear();
  _viewerSteps.clear();
  _buildMods.clear();
  _buildModSteps.clear();
  _buildModStepIndexes.clear();
  _buildModRendered.clear();
  _includeFileList.clear();
  _buildModList.clear();
  _loadedParts.clear();
  _mpd                   = false;
  _partCount             = 0;
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
      line = QString("0 Name: %1").arg(fileName);
      insertLine(fileName, lineNumber, line);
      lineNumber++;
      line = QString("0 Author: %1").arg(Preferences::defaultAuthor);
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
                      bool            unofficialPart,
                      bool            generated,
                      bool            includeFile,
                      const QString  &subFilePath)
{
  QString    fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    _subFiles.erase(i);
  }
  LDrawSubFile subFile(contents,datetime,unofficialPart,generated,includeFile,subFilePath);
  _subFiles.insert(fileName,subFile);
  if (includeFile)
      _includeFileList << fileName;
  else
      _subFileOrder << fileName;
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

/* return the number of lines in the file */

int LDrawFile::size(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  int mySize;
      
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i == _subFiles.end()) {
    mySize = 0;
  } else {
    mySize = i.value()._contents.size();
  }
  return mySize;
}

/* Only used to return fade or highlight content size */

int LDrawFile::configuredSubFileSize(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  int mySize;

  QMap<QString, CfgSubFile>::iterator i = _configuredSubFiles.find(fileName);

  if (i == _configuredSubFiles.end()) {
    mySize = 0;
  } else {
    mySize = i.value()._contents.size();
  }
  return mySize;
}

bool LDrawFile::isMpd()
{
  return _mpd;
}

int LDrawFile::isUnofficialPart(const QString &name)
{
  QString fileName = name.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    int _unofficialPart = i.value()._unofficialPart;
    return _unofficialPart;
  }
  return 0;
}

int LDrawFile::isIncludeFile(const QString &name)
{
  QString fileName = name.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    int _includeFile = i.value()._includeFile;
    return _includeFile;
  }
  return 0;
}

/* return the name of the top level file */

QString LDrawFile::topLevelFile()
{
  if (_subFileOrder.size()) {
    return _subFileOrder[0];
  } else {
    return _emptyString;
  }
}

int LDrawFile::fileOrderIndex(const QString &file)
{
  for (int i = 0; i < _subFileOrder.size(); i++) {
    if (_subFileOrder[i].toLower() == file.toLower()) {
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

bool LDrawFile::contains(const QString &file)
{
  for (int i = 0; i < _subFileOrder.size(); i++) {
    if (_subFileOrder[i].toLower() == file.toLower()) {
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
      return ! i.value()._unofficialPart && ! i.value()._generated;
      //return ! i.value()._generated; // added on revision 368 - to generate csiSubModels for 3D render
  }
  return false;
}

bool LDrawFile::modified()
{
  QString key;
  bool    modified = false;
  Q_FOREACH (key,_subFiles.keys()) {
    modified |= _subFiles[key]._modified;
  }
  return modified;
}

bool LDrawFile::modified(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._modified;
  } else {
    return false;
  }
}

void LDrawFile::setModified(const QString &mcFileName, bool modified)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    i.value()._modified = modified;
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

void LDrawFile::setContents(const QString     &mcFileName, 
                 const QStringList &contents)
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

void LDrawFile::setSubFilePath(const QString     &mcFileName,
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
      QString modelName = _subFileOrder[i].toLower();
      QMap<QString, LDrawSubFile>::iterator it = _subFiles.find(modelName);
      if (!it->_unofficialPart && !it->_generated) {
          subModel << modelName;
      }
    }
    return subModel;
}

QStringList LDrawFile::getSubFilePaths()
{
  QStringList subFilesPaths;
  for (int i = 0; i < _subFileOrder.size(); i++) {
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(_subFileOrder[i]);
    if (f != _subFiles.end()) {
        if (!f.value()._subFilePath.isEmpty()) {
            subFilesPaths << f.value()._subFilePath;
        }
    }
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

  if (i != _subFiles.end()) {
    //i.value()._modified = true;
    i.value()._startPageNumber = startPageNumber;
    //i.value()._datetime = QDateTime::currentDateTime();
    //i.value()._changedSinceLastWrite = true; // remarked on build 491 28/12/2015
  }
}

/* return the last fade position value */

int LDrawFile::getPrevStepPosition(
        const QString &mcFileName,
        const int     &mcStepNumber)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
      if (mcStepNumber == i.value()._prevStepPosition.at(PS_STEP_NUM))
          return i.value()._prevStepPosition.at(PS_LAST_POS);
      else
          return i.value()._prevStepPosition.at(PS_POS);
  }
  return 0;
}

void LDrawFile::setPrevStepPosition(
        const QString &mcFileName,
        const int     &mcStepNumber,
        const int     &prevStepPosition)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    int lastStepPosition  = i.value()._prevStepPosition.at(PS_POS);
    if (lastStepPosition != prevStepPosition) {
        //i.value()._modified = true;
        QVector<int> stepPositions = { prevStepPosition, lastStepPosition ,mcStepNumber };
        i.value()._prevStepPosition = stepPositions;
        //i.value()._datetime = QDateTime::currentDateTime();
        //i.value()._changedSinceLastWrite = true;  // remarked on build 491 28/12/2015
    }
  }
}

/* set all previous step positions to 0 */

void LDrawFile::clearPrevStepPositions()
{
  QString key;
  Q_FOREACH (key,_subFiles.keys()) {
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
  QString fileName;
  Q_FOREACH (fileName, parsedStack) {
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
  return _subFileOrder;
}

QStringList LDrawFile::includeFileList() {
  return _includeFileList;
}

QString LDrawFile::getSubmodelName(int submodelIndx)
{
    if (submodelIndx > -1 && submodelIndx < _subFileOrder.size()){
        QString subFileName = _subFileOrder[submodelIndx];
        if (!subFileName.isEmpty())
            return subFileName;
    }
    return QString();
}

int LDrawFile::getSubmodelIndex(const QString &mcFileName)
{
    QString fileName = mcFileName.toLower();
    return _subFileOrder.indexOf(fileName);
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

void LDrawFile::changeContents(const QString &mcFileName, 
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
  QString key;
  Q_FOREACH (key,_subFiles.keys()) {
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
            key.prepend("cp");
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
    if (countPage)
        key.prepend("cp");
    if (mirrored) {
      haveKey = key.isEmpty() || (countPage && key == "cp") ? howCounted == CountAtTop ? true : false :
                  i.value()._mirrorRenderedKeys.contains(key);
      rendered  = i.value()._mirrorRendered;
    } else {
      haveKey = key.isEmpty() || (countPage && key == "cp") ? howCounted == CountAtTop ? true : false :
                  i.value()._renderedKeys.contains(key);
      rendered  = i.value()._rendered;
    }
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
        emit gui->messageSig(LOG_ERROR, QString("Cannot read ldraw file: [%1]<br>%2.")
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

    bool mpd = false;

    QTextStream in(&qba);



    QFileInfo fileInfo(fileName);

    while ( ! in.atEnd()) {
        QString line = in.readLine(0);
        if (line.contains(_fileRegExp[SOF_RX])) {
            emit gui->messageSig(LOG_INFO_STATUS, QString("Model file %1 identified as Multi-Part LDraw System (MPD) Document").arg(fileInfo.fileName()));
            mpd = true;
            break;
        }
        if (line.contains(_fileRegExp[LDR_RX])) {
            emit gui->messageSig(LOG_INFO_STATUS, QString("Model file %1 identified as LDraw Sytem (LDR) Document").arg(fileInfo.fileName()));
            mpd = false;
            break;
        }
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    ldcadGroupsLoaded = false;

    if (mpd) {
      QDateTime datetime = QFileInfo(fileName).lastModified();
      loadMPDFile(QDir::toNativeSeparators(fileName),datetime);
    } else {
      topLevelModel = true;
      loadLDRFile(QDir::toNativeSeparators(fileInfo.absolutePath()),fileInfo.fileName());
    }
    
    QApplication::restoreOverrideCursor();

    addCustomColorParts(topLevelFile());

    buildModLevel = 0 /*false*/;

    countParts(topLevelFile());

    QString loadStatusMessage = QString("%1 model file %2 loaded. Part Count %3. %4")
            .arg(mpd ? "MPD" : "LDR")
            .arg(fileInfo.fileName())
            .arg(_partCount)
            .arg(gui->elapsedTime(t.elapsed()));
    emit gui->messageSig(LOG_INFO_STATUS, QString("%1").arg(loadStatusMessage));

    QApplication::processEvents();

    auto getCount = [] (const LoadMsgType lmt)
    {
        if (lmt == ALL_LOAD_MSG)
            return _loadedParts.size();

        int count = 0;

        for (QString part : _loadedParts)
        {
            if (part.startsWith(int(lmt)))
                count++;
        }

        return count;
    };

    int vpc = getCount(VALID_LOAD_MSG);
    int mpc = getCount(MISSING_LOAD_MSG);
    int ppc = getCount(PRIMITIVE_LOAD_MSG);
    int spc = getCount(SUBPART_LOAD_MSG);
    int apc = _partCount;
    bool delta = apc != vpc;

    emit gui->messageSig(LOG_INFO, QString("Build Modifications are %1")
                                            .arg(Preferences::buildModEnabled ? "Enabled" : "Disabled"));

    switch (Preferences::ldrawFilesLoadMsgs)
    {
    case NEVER_SHOW:
        break;
    case SHOW_ERROR:
        _showLoadMessages = mpc > 0;
        break;
    case SHOW_WARNING:
        _showLoadMessages = ppc > 0 || spc > 0;
        break;
    case SHOW_MESSAGE:
        _showLoadMessages = mpc > 0 || ppc > 0 || spc > 0;
        break;
    case ALWAYS_SHOW:
        _showLoadMessages = true;
    break;
    }

    if (_showLoadMessages && Preferences::modeGUI) {
        QString message = QString("%1 model file <b>%2</b> loaded.%3%4%5%6%7%8%9")
                .arg(mpd ? "MPD" : "LDR")
                .arg(fileInfo.fileName())
                .arg(delta   ? QString("<br>Parts count:            <b>%1</b>").arg(apc) : "")
                .arg(mpc > 0 ? QString("<span style=\"color:red\"><br>Missing parts:          <b>%1</b></span>").arg(mpc) : "")
                .arg(vpc > 0 ? QString("<br>Validated parts:        <b>%1</b>").arg(vpc) : "")
                .arg(ppc > 0 ? QString("<br>Primitive parts:        <b>%1</b>").arg(ppc) : "")
                .arg(spc > 0 ? QString("<br>Subparts:               <b>%1</b>").arg(spc) : "")
                .arg(QString("<br>%1").arg(gui->elapsedTime(t.elapsed())))
                .arg(mpc > 0 ? QString("<br><br>Missing %1 %2 not found in the %3 or %4 archive.<br>"
                                       "If %5 custom %1, be sure %7 location is captured in the LDraw search directory list.<br>"
                                       "If %5 new unofficial %1, be sure the unofficial archive library is up to date.")
                                       .arg(mpc > 1 ? "parts" : "part")                        //1
                                       .arg(mpc > 1 ? "were" : "was")                          //2
                                       .arg(VER_LPUB3D_UNOFFICIAL_ARCHIVE)                     //3
                                       .arg(VER_LDRAW_OFFICIAL_ARCHIVE)                        //4
                                       .arg(mpc > 1 ? "these are" : "this is a")               //5
                                       .arg(mpc > 1 ? "their" : "its") : "");                  //7
        _loadedParts << message;
        if (mpc > 0) {
            if (_showLoadMessages) {
                _showLoadMessages = false;
                if ((_loadAborted = LdrawFilesLoad::showLoadMessages(_loadedParts) == 0)/*0=Rejected,1=Accepted*/) {
                    return 1;
                }
            }
        }
    }

//    logInfo() << (mpd ? "MPD" : "LDR")
//              << " File:"         << _file
//              << ", Name:"        << _name
//              << ", Author:"      << _author
//              << ", Description:" << _description
//              << ", Category:"    << _category
//              << ", Parts:"      << _partCount
//                 ;
    return 0;
}

void LDrawFile::showLoadMessages()
{
    if (_showLoadMessages && Preferences::modeGUI)
        LdrawFilesLoad::showLoadMessages(_loadedParts);
}

void LDrawFile::loadMPDFile(const QString &fileName, QDateTime &datetime)
{    
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        emit gui->messageSig(LOG_ERROR, QString("Cannot read mpd file %1<br>%2")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QFileInfo   fileInfo(fileName);
    QTextStream in(&file);
    in.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));

    QStringList stageContents;
    QStringList stageSubfiles;

    /* Read it in the first time to put into fileList in order of appearance */

    while ( ! in.atEnd()) {
        QString sLine = in.readLine(0);
        stageContents << sLine.trimmed();
    }
    file.close();

    hdrTopFileNotFound  = true;
    hdrDescNotFound     = true;
    hdrNameNotFound     = true;
    hdrAuthorNotFound   = true;
    hdrCategNotFound    = true;

    metaBuildModNotFund      = true;
    metaFinalModelNotFound   = true;
    metaStartPageNumNotFound = true;
    metaStartStepNumNotFound = true;

    unofficialPart      = false;
    topLevelModel       = true;
    descriptionLine     = 0;

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

    std::function<QString()> fileType;
    fileType = [this] ()
    {
        return topLevelModel ? "model" : "submodel";
    };

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

    emit gui->progressBarPermInitSig();

    std::function<void(int)> loadMPDContents;
    loadMPDContents = [
            this,
            &file,
            &fileType,
            &missingHeaders,
            &loadMPDContents,
            &stageContents,
            &stageSubfiles,
            &fileInfo,
            &searchPaths,
            &datetime] (int fileIndx) {
        bool alreadyLoaded;
        QStringList contents;
        QString     subfileName;
        MissingHeader  headerMissing = NoneMissing;

        emit gui->progressPermRangeSig(1, stageContents.size());
        emit gui->progressPermMessageSig(QString("Loading %1 '%2'").arg(fileType()).arg( fileInfo.fileName()));
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Stage Contents Size: %1, Start Index %2")
                             .arg(stageContents.size()).arg(fileIndx));
#endif
        int number;
        bool validNumber;
        bool headerFinished = false;
        bool subFileFound   = false;
        QString stageSubfileName;

        for (; fileIndx < stageContents.size(); fileIndx++) {

            QString smLine = stageContents.at(fileIndx);

            emit gui->progressPermSetValueSig(fileIndx + 1);

            bool sof = smLine.contains(_fileRegExp[SOF_RX]);  //start of file
            bool eof = smLine.contains(_fileRegExp[EOF_RX]);  //end of file

            // load LDCad groups
            if (!ldcadGroupsLoaded) {
                if (smLine.contains(_fileRegExp[LDG_RX])){
                    insertLDCadGroup(_fileRegExp[LDG_RX].cap(3),_fileRegExp[LDG_RX].cap(1).toInt());
                    insertLDCadGroup(_fileRegExp[LDG_RX].cap(2),_fileRegExp[LDG_RX].cap(1).toInt());
                } else if (smLine.contains("0 STEP") || smLine.contains("0 ROTSTEP")) {
                    ldcadGroupsLoaded = true;
                }
            }

            QStringList tokens;
            split(smLine,tokens);

            // subfile check;
            if ((subFileFound = tokens.size() == 15 && tokens.at(0) == "1")) {
                headerFinished = true;
                stageSubfileName = tokens.at(14).toLower();
            } else if (isSubstitute(smLine,stageSubfileName)) {
                subFileFound = !stageSubfileName.isEmpty();
            }
            if (subFileFound) {
                PieceInfo* standardPart = gui->GetPiecesLibrary()->FindPiece(stageSubfileName.toLatin1().constData(), nullptr, false, false);
                if (! standardPart && ! LDrawFile::contains(stageSubfileName.toLower()) && ! stageSubfiles.contains(stageSubfileName)) {
                    stageSubfiles.append(stageSubfileName);
                }
            }

            if (!headerFinished) {

                // One time populate top level file name
                if (hdrTopFileNotFound) {
                    if (sof){
                        _file = _fileRegExp[SOF_RX].cap(1).replace(QFileInfo(_fileRegExp[SOF_RX].cap(1)).suffix(),"");
                        descriptionLine = fileIndx + 1;      //next line should be description
                        hdrTopFileNotFound = false;
                    }
                }

                // One time populate model descriptkon
                if (hdrDescNotFound && fileIndx == descriptionLine && ! isHeader(smLine)) {
                    if (smLine.contains(_fileRegExp[DES_RX]))
                        _description = smLine;
                    else
                        _description = "LDraw model";
                    Preferences::publishDescription = _description;
                    hdrDescNotFound = false;
                }

                if (hdrNameNotFound) {
                    if (smLine.contains(_fileRegExp[NAM_RX])) {
                        _name = _fileRegExp[NAM_RX].cap(1).replace(": ","");
                        hdrNameNotFound = false;
                    }
                }

                if (hdrAuthorNotFound) {
                    if (smLine.contains(_fileRegExp[AUT_RX])) {
                        _author = _fileRegExp[AUT_RX].cap(1).replace(": ","");
                        Preferences::defaultAuthor = _author;
                        hdrAuthorNotFound = false;
                    }
                }

                // One time populate model category
                if (hdrCategNotFound && subfileName == _file) {
                    if (smLine.contains(_fileRegExp[CAT_RX])) {
                        _category = _fileRegExp[CAT_RX].cap(1);
                        hdrCategNotFound = false;
                    }
                }

                // Check if BuildMod is disabled
                if (metaBuildModNotFund) {
                    if (smLine.startsWith("0 !LPUB BUILD_MOD_ENABLED")) {
                        bool state = tokens.last() == "FALSE" ? false : true ;
                        Preferences::buildModEnabled = state;
                        metaBuildModNotFund = false;
                    }
                }

                // Check if insert final model is disabled
                if (metaFinalModelNotFound) {
                    if (!Preferences::enableFadeSteps && !Preferences::enableHighlightStep) {
                        metaFinalModelNotFound = false;
                    } else {
                        if (smLine.startsWith("0 !LPUB FINAL_MODEL_ENABLED")) {
                            bool state = tokens.last() == "FALSE" ? false : true ;
                            Preferences::finalModelEnabled = state;
                            metaFinalModelNotFound = false;
                        }                            
                    }
                }

                // Check if Start Page Number is specified
                if (metaStartPageNumNotFound) {
                    if (smLine.startsWith("0 !LPUB START_PAGE_NUMBER")) {
                        number = tokens.last().toInt(&validNumber);
                        gui->pa = validNumber ? number - 1 : 0;
                        metaStartPageNumNotFound = false;
                    }
                }

                // Check if Start Step Number is specified
                if (metaStartStepNumNotFound) {
                    if (smLine.startsWith("0 !LPUB START_STEP_NUMBER")) {
                        number = tokens.last().toInt(&validNumber);
                        gui->sa = validNumber ? number - 1 : 0;
                        metaStartStepNumNotFound = false;
                    }
                }
            }

            if ((alreadyLoaded = LDrawFile::contains(subfileName.toLower()))) {
                emit gui->messageSig(LOG_TRACE, QString("MPD " + fileType() + " '" + subfileName + "' already loaded."));
                stageSubfiles.removeAt(stageSubfiles.indexOf(subfileName));
            }

            /* - if at start of file marker, populate subfileName
             * - if at end of file marker, clear subfileName
             */
            if (sof || eof) {
                /* - if at end of file marker
                 * - insert items if subfileName not empty
                 * - as subfileName not empty, set unofficial part = false
                 * - after insert, clear contents item
                 */
                if (! subfileName.isEmpty()) {
                    if (! alreadyLoaded) {
                        insert(subfileName,contents,datetime,unofficialPart);
                        if ((headerMissing = MissingHeader(missingHeaders())))
                            normalizeHeader(subfileName, headerMissing);
                        emit gui->messageSig(LOG_TRACE, QString("MPD " + fileType() + " '" + subfileName + "' with " +
                                                                QString::number(size(subfileName)) + " lines loaded."));
                        topLevelModel = false;
                        unofficialPart = false;
                    }
                    stageSubfiles.removeAt(stageSubfiles.indexOf(subfileName));
                }

                contents.clear();

                /* - if at start of file marker
                 * - set subfileName of new file
                 * - else if at end of file marker, clear subfileName
                 */
                if (sof) {
                    hdrNameNotFound   = true;
                    hdrAuthorNotFound = true;

                    subfileName = _fileRegExp[SOF_RX].cap(1).toLower();
                    if (! alreadyLoaded) {
                        headerFinished = false;
                        emit gui->messageSig(LOG_INFO_STATUS, "Loading MPD " + fileType() + " '" + subfileName + "'...");
                    }
                } else {
                    subfileName.clear();
                }

            } else if ( ! subfileName.isEmpty() && !smLine.isEmpty()) {
                /* - after start of file - subfileName not empty
                 * - if line contains unofficial part/subpart/shortcut/primitive/alias tag set unofficial part = true
                 * - add line to contents
                 */
                if (! unofficialPart) {
                    unofficialPart = getUnofficialFileType(smLine);
                    if (unofficialPart)
                        emit gui->messageSig(LOG_TRACE, "Submodel '" + subfileName + "' spcified as Unofficial Part.");
                }

                contents << smLine;
            }
        }

        // at end of file - NOFILE tag not specified
        if ( ! subfileName.isEmpty() && ! contents.isEmpty()) {
            if (LDrawFile::contains(subfileName.toLower())) {
                emit gui->messageSig(LOG_TRACE, QString("MPD submodel '" + subfileName + "' already loaded."));
            } else {
                insert(subfileName,contents,datetime,unofficialPart);
                if ((headerMissing = MissingHeader(missingHeaders())))
                    normalizeHeader(subfileName, headerMissing);
                emit gui->messageSig(LOG_TRACE, QString("MPD submodel '" + subfileName + "' with " +
                                                        QString::number(size(subfileName)) + " lines loaded."));
            }
            stageSubfiles.removeAt(stageSubfiles.indexOf(subfileName));
        }

        // resolve outstanding subfiles
        if (stageSubfiles.size()){
            stageSubfiles.removeDuplicates();
#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_DEBUG, QString("%1 unresolved stage %2 specified.")
                                 .arg(stageSubfiles.size()).arg(stageSubfiles.size() == 1 ? "subfile" : "subfiles"));
#endif
            QString projectPath = QDir::toNativeSeparators(fileInfo.absolutePath());

            // set index to bottom of stageContents
            int stageFileIndx = stageContents.size();
            bool subFileFound =false;
            for (QString subfile : stageSubfiles) {
#ifdef QT_DEBUG_MODE
                emit gui->messageSig(LOG_DEBUG, QString("Processing stage subfile %1...").arg(subfile));
#endif
                if ((alreadyLoaded = LDrawFile::contains(subfileName.toLower()))) {
                    emit gui->messageSig(LOG_TRACE, QString("MPD submodel '" + subfile + "' already loaded."));
                    continue;
                }
                // current path
                if ((subFileFound = QFileInfo(projectPath + QDir::separator() + subfile).isFile())) {
                    fileInfo = QFileInfo(projectPath + QDir::separator() + subfile);
                } else
                // file path
                if ((subFileFound = QFileInfo(subfile).isFile())){
                    fileInfo = QFileInfo(subfile);
                }
                else
                // extended search - LDraw subfolder paths and extra search directorie paths
                if (Preferences::extendedSubfileSearch) {
                    for (QString subFilePath : searchPaths){
                        if ((subFileFound = QFileInfo(subFilePath + QDir::separator() + subfile).isFile())) {
                            fileInfo = QFileInfo(subFilePath + QDir::separator() + subfile);
                            break;
                        }
                    }
                }

                if (!subFileFound) {
                    emit gui->messageSig(LOG_NOTICE, QString("Subfile %1 not found.")
                                         .arg(subfile));
                } else {
                    setSubFilePath(subfile,fileInfo.absoluteFilePath());
                    stageSubfiles.removeAt(stageSubfiles.indexOf(subfile));
                    file.setFileName(fileInfo.absoluteFilePath());
                    if (!file.open(QFile::ReadOnly | QFile::Text)) {
                        emit gui->messageSig(LOG_NOTICE, QString("Cannot read mpd subfile %1<br>%2")
                                             .arg(fileInfo.absoluteFilePath())
                                             .arg(file.errorString()));
                        return;
                    }

                    QTextStream in(&file);
                    in.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
                    while ( ! in.atEnd()) {
                        QString sLine = in.readLine(0);
                        stageContents << sLine.trimmed();
                    }
                }
            }
            if (subFileFound) {
                loadMPDContents(stageFileIndx);
            }
        }
#ifdef QT_DEBUG_MODE
        else {
            emit gui->messageSig(LOG_DEBUG, QString("No staged subfiles specified."));
        }
#endif
    };

    loadMPDContents(0/*fileIndx*/);

#ifdef QT_DEBUG_MODE
    QHashIterator<QString, int> i(_ldcadGroups);
    while (i.hasNext()) {
        i.next();
        emit gui->messageSig(LOG_TRACE, QString("LDCad Groups: Name[%1], LineID[%2].")
                             .arg(i.key()).arg(i.value()));
    }
#endif

    _mpd = true;

    if (metaBuildModNotFund)
        Preferences::buildModEnabled = false;

    if (metaFinalModelNotFound)
        Preferences::finalModelEnabled = true;

    emit gui->progressPermSetValueSig(stageContents.size());
    emit gui->progressPermStatusRemoveSig();
}

void LDrawFile::loadLDRFile(const QString &path, const QString &fileName)
{
    if (_subFiles[fileName]._contents.isEmpty()) {

        QString fullName(path + QDir::separator() + fileName);

        QFile file(fullName);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            emit gui->messageSig(LOG_ERROR,QString("Cannot read ldr file %1<br>%2")
                                 .arg(fullName)
                                 .arg(file.errorString()));
            return;
        }

        QFileInfo   fileInfo(fullName);
        QTextStream in(&file);
        in.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));

        QStringList contents;
        QStringList subfiles;

        int number;
        bool validNumber;
        bool headerFinished = false;

        unofficialPart = false;

        QString fileType = topLevelModel ? "model" : "submodel";

        /* Read it in the first time to put into fileList in order of appearance */

        while ( ! in.atEnd()) {
            QString line = in.readLine(0);
            QString scModelType = fileType[0].toUpper() + fileType.right(fileType.size() - 1);
            if (line.contains(_fileRegExp[SOF_RX])) {
                file.close();
                emit gui->messageSig(LOG_INFO_STATUS, QString(scModelType + " file %1 identified as Multi-Part LDraw System (MPD) Document").arg(fileInfo.fileName()));
                QDateTime datetime = fileInfo.lastModified();
                loadMPDFile(fileInfo.absoluteFilePath(),datetime);
                return;
            } else {
                if (!line.isEmpty())
                    contents << line.trimmed();
                if (isHeader(line) && ! unofficialPart) {
                    unofficialPart = getUnofficialFileType(line);
                }
            }
        }
        file.close();

        if (topLevelModel) {
            hdrTopFileNotFound  = true;
            hdrNameNotFound     = true;
            hdrDescNotFound     = true;
            hdrAuthorNotFound   = true;
            hdrCategNotFound    = true;
            metaBuildModNotFund = true;
            metaFinalModelNotFound   = true;
            metaStartPageNumNotFound = true;
            metaStartStepNumNotFound = true;
            descriptionLine     = 0;
        }

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

        emit gui->progressBarPermInitSig();
        emit gui->progressPermRangeSig(1, contents.size());
        emit gui->progressPermMessageSig(QString("Loading LDR %1 '%2'...").arg(fileType).arg(fileInfo.fileName()));

        QDateTime datetime = fileInfo.lastModified();

        insert(fileInfo.fileName(),contents,datetime,unofficialPart,false/*generated*/,false/*includeFile*/,fileInfo.absoluteFilePath());

        /* read it a second time to find submodels and check for completeness*/

        for (int i = 0; i < contents.size(); i++) {

            QString smLine = contents.at(i);

            emit gui->progressPermSetValueSig(i + 1);

            // load LDCad groups
            if (!ldcadGroupsLoaded) {
                if(smLine.contains(_fileRegExp[LDG_RX])){
                    insertLDCadGroup(_fileRegExp[LDG_RX].cap(3),_fileRegExp[LDG_RX].cap(1).toInt());
                    insertLDCadGroup(_fileRegExp[LDG_RX].cap(2),_fileRegExp[LDG_RX].cap(1).toInt());
                } else if (smLine.contains("0 STEP") || smLine.contains("0 ROTSTEP")) {
                    ldcadGroupsLoaded = true;
                }
            }

            QStringList tokens;
            split(smLine,tokens);


            if (!headerFinished) {

                headerFinished = tokens.size() && tokens[0] != "0";

                // Check if BuildMod is disabled
                if (metaBuildModNotFund) {
                    if (smLine.startsWith("0 !LPUB BUILD_MOD_ENABLED")) {
                        bool state = tokens.last() == "FALSE" ? false : true ;
                        Preferences::buildModEnabled  = state;
                        metaBuildModNotFund = false;
                    }
                }

                // Check if insert final model is disabled
                if (metaFinalModelNotFound) {
                    if (!Preferences::enableFadeSteps && !Preferences::enableHighlightStep) {
                        metaFinalModelNotFound = false;
                    } else {
                        if (smLine.startsWith("0 !LPUB FINAL_MODEL_ENABLED")) {
                            bool state = tokens.last() == "FALSE" ? false : true ;
                            Preferences::finalModelEnabled  = state;
                            metaFinalModelNotFound = false;
                        }                            
                    }
                }

                // Check if Start Page Number is specified

                if (metaStartPageNumNotFound) {
                    if (smLine.startsWith("0 !LPUB START_PAGE_NUMBER")) {
                        number = tokens.last().toInt(&validNumber);
                        gui->pa  = validNumber ? number - 1 : 0;
                        metaStartPageNumNotFound = false;
                    }
                }

                // Check if Start Step Number is specified
                if (metaStartStepNumNotFound) {
                    if (smLine.startsWith("0 !LPUB START_STEP_NUMBER")) {
                        number = tokens.last().toInt(&validNumber);
                        gui->sa  = validNumber ? number - 1 : 0;
                        metaStartStepNumNotFound = false;
                    }
                }

                // One time populate top level file name
                if (hdrTopFileNotFound) {
                    _file = QString(fileName).replace(QFileInfo(fileName).suffix(),"");
                    descriptionLine = i+1;      //next line should be description
                    hdrTopFileNotFound = false;
                }

                if (hdrNameNotFound) {
                    if (smLine.contains(_fileRegExp[NAM_RX])) {
                        _name = _fileRegExp[NAM_RX].cap(1).replace(": ","");
                        hdrNameNotFound = false;
                    }
                }

                // One time populate model descriptkon
                if (hdrDescNotFound && i == descriptionLine && ! isHeader(smLine)) {
                    if (smLine.contains(_fileRegExp[DES_RX]))
                        _description = smLine;
                    else
                        _description = "LDraw model";
                    Preferences::publishDescription = _description;
                    hdrDescNotFound = false;
                }

                if (hdrAuthorNotFound) {
                    if (smLine.contains(_fileRegExp[AUT_RX])) {
                        _author = _fileRegExp[AUT_RX].cap(1).replace(": ","");
                        Preferences::defaultAuthor = _author;
                        hdrAuthorNotFound = false;
                    }
                }

                // One time populate model category
                if (hdrCategNotFound && fileName == topLevelFile()) {
                    if (smLine.contains(_fileRegExp[CAT_RX])) {
                        _category = _fileRegExp[CAT_RX].cap(1);
                        hdrCategNotFound = false;
                    }
                }
            }

            // subfile check;
            QString subfileName;
            bool subFileFound = false;
            if ((subFileFound = tokens.size() == 15 && tokens.at(0) == "1")) {
                subfileName = tokens.at(14);
            } else if (isSubstitute(smLine,subfileName)) {
                subFileFound = !subfileName.isEmpty();
            }

            // resolve outstanding subfiles
            if (subFileFound) {
                QFileInfo subFileInfo = QFileInfo(subfileName);
                PieceInfo* standardPart = gui->GetPiecesLibrary()->FindPiece(subFileInfo.fileName().toLatin1().constData(), nullptr, false, false);
                if (! standardPart && ! LDrawFile::contains(subFileInfo.fileName())) {
                    // current path
                    if ((subFileFound = QFileInfo(path + QDir::separator() + subFileInfo.fileName()).isFile())) {
                        subFileInfo = QFileInfo(path + QDir::separator() + subFileInfo.fileName());
                    } else
                    // file path
                    if ((subFileFound = QFileInfo(subFileInfo.filePath()).isFile())){
                        subFileInfo = QFileInfo(subFileInfo.filePath());
                    }
                    else
                    // extended search - LDraw subfolder paths and extra search directorie paths
                    if (Preferences::extendedSubfileSearch) {
                        for (QString subFilePath : searchPaths){
                            if ((subFileFound = QFileInfo(subFilePath + QDir::separator() + subFileInfo.fileName()).isFile())) {
                                subFileInfo = QFileInfo(subFilePath + QDir::separator() + subFileInfo.fileName());
                                break;
                            }
                        }
                    }

                    if (subFileFound) {
                        emit gui->messageSig(LOG_NOTICE, QString("Subfile %1 detected").arg(subFileInfo.fileName()));
                        topLevelModel         = false;
                        hdrNameNotFound   = true;
                        hdrAuthorNotFound = true;
                        loadLDRFile(subFileInfo.absolutePath(),subFileInfo.fileName());
                    } else {
                        emit gui->messageSig(LOG_NOTICE, QString("Subfile %1 not found.")
                                             .arg(subFileInfo.fileName()));
                    }
                }
            }
        }

        _mpd = false;

        // Check if BuildMod meta is present
        if (metaBuildModNotFund)
            Preferences::buildModEnabled = false;

        // Check if FinalModelEnabled meta is present
        if (metaFinalModelNotFound)
            Preferences::finalModelEnabled = true;

        emit gui->progressPermSetValueSig(contents.size());
        emit gui->progressPermStatusRemoveSig();

        int headerMissing = NoneMissing;
        if (hdrNameNotFound && hdrAuthorNotFound)
            headerMissing = BothMissing;
        else if (hdrNameNotFound)
            headerMissing =  NameMissing;
        else if (hdrAuthorNotFound)
           headerMissing = AuthorMissing;
        if (headerMissing)
            normalizeHeader(fileInfo.fileName(), headerMissing);

        emit gui->messageSig(LOG_TRACE, QString("LDR " + fileType + " '" + fileInfo.fileName() + "' with " +
                                                 QString::number(size(fileInfo.fileName())) + " lines loaded."));
    }
}

bool LDrawFile::saveFile(const QString &fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool rc;
    if (isIncludeFile(fileName)) {
      rc = saveIncludeFile(fileName);
    } else if (_mpd) {
      rc = saveMPDFile(fileName);
    } else {
      rc = saveLDRFile(fileName);
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
        if (gui->ldrawColourParts.isLDrawColourPart(tokens[5])) {
          // parse the part lines for custom sub parts
          for (++i; i < j; i++) {
            split(f->_contents[i],tokens);
            if (tokens.size() == 15 && tokens[0] == "1") {
              if (contains(tokens[14])) {
                // we have custom part so let's add it to the custom part list
                gui->ldrawColourParts.addLDrawColorPart(tokens[14]);
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
                gui->ldrawColourParts.addLDrawColorPart(tokens[14]);
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
  QMutex countMutex;
  countMutex.lock();
  //logTrace() << QString("countInstances, File: %1, Mirrored: %2, Callout: %3").arg(mcFileName,(isMirrored?"Yes":"No"),(callout?"Yes":"No"));

  QString fileName    = mcFileName.toLower();
  int  modelIndex     = getSubmodelIndex(fileName);
  bool partsAdded     = false;
  bool noStep         = false;
  bool stepIgnore     = false;
  bool buildModIgnore = false;

  /*
   * For countInstances, the BuildMod behaviour creates a sequential
   * list (Vector<int>) of all the steps in the loaded model file.
   * Step indices are appended to the _buildModStepIndexes register.
   * Each step index contains the step's parent model and the line number
   * of the STEP meta command indicating the top of the 'next' step.
   * The buildModLevel flag uses getLevel() function to determine the current
   * BuildMod when mods are nested.
   */
  Where topOfStep(fileName, 0);

  gui->skipHeader(topOfStep);

  if (firstStep) {
    if (fileName == topLevelFile()) {
      buildModLevel = 0;
      _currentLevels.clear();
      _buildModStepIndexes.clear();
    }
    if (! _buildModStepIndexes.contains({ modelIndex, topOfStep.lineNumber }))
      _buildModStepIndexes.append({ modelIndex, topOfStep.lineNumber });
    firstStep = false;
  }

  QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
  if (f != _subFiles.end()) {
    // count mirrored instance automatically
    if (f->_beenCounted) {
      if (isMirrored) {
        ++f->_mirrorInstances;
      } else {
        ++f->_instances;
      }
      countMutex.unlock();
      return;
    }

    // get content size and reset numSteps
    int j = f->_contents.size();
    f->_numSteps = 0;

    // process submodel content...
    int i = topOfStep.lineNumber;
    for (; i < j; i++) {
      QStringList tokens;
      QString line = f->_contents[i];
      split(line,tokens);

        //lpub3d ignore part - so set ignore step
      if (tokens.size() == 5 && tokens[0] == "0" &&
                  (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
                  (tokens[2] == "PART"  || tokens[2] == "PLI") &&
                  tokens[3] == "BEGIN"  &&
                  tokens[4] == "IGN") {
        stepIgnore = true;
        // we have a meta command...
      } else if (tokens.size() == 4 && tokens[0] == "0" &&
                (tokens[1] == "!LPUB" || tokens[1] == "LPUB")) {
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
          for (++i; i < j; i++) {
            split(f->_contents[i],tokens);
            if (tokens.size() == 15 && tokens[0] == "1") {
              if (contains(tokens[14]) && ! stepIgnore && ! buildModIgnore) {
                firstStep = true;
                countInstances(tokens[14], firstStep, mirrored(tokens), callout);
              }
            } else if (tokens.size() == 4 && tokens[0] == "0" &&
                      (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
                       tokens[2] == "CALLOUT" &&
                       tokens[3] == "END") {
              callout = false;
              break;
            }
          }
          // build modification - begins at BEGIN command and ends at END_MOD action
        } else if (tokens[2] == "BUILD_MOD") {
          if (tokens[3] == "BEGIN") {
            buildModLevel = getLevel(tokens[4], BM_BEGIN);
          } else if (tokens[3] == "END_MOD") {
            buildModLevel = getLevel("", BM_END);
          }
          buildModIgnore = buildModLevel;
          // page insert, e.g. dispaly model, bom
        } else if (tokens[2] == "INSERT" &&
                  (tokens[3] == "PAGE" || tokens[3] == "COVER_PAGE")) {
          partsAdded = true;
        } else if (tokens[1] == "BUFEXCHG") {}
        // no step
      } else if (tokens.size() == 3 && tokens[0] == "0" &&
                (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
                (tokens[2] == "NOSTEP" || tokens[2] == "NOFILE")) {
        noStep = true;
        // LDraw step or rotstep - so check if parts added
      } else if (tokens.size() >= 2 && tokens[0] == "0" &&
                (tokens[1] == "STEP" || tokens[1] == "ROTSTEP")) {
        if (! noStep) {
          if (partsAdded) {
            // parts added - increment step
            int incr = (isMirrored && f->_mirrorInstances == 0) ||
                       (! isMirrored && f->_instances == 0);
            f->_numSteps += incr;
          }
          // set step index for occurrences of STEP or ROTSTEP not ignored by BuildMod
          if (! buildModIgnore) {
            _buildModStepIndexes.append({ modelIndex, i });
          }
        }
        // reset partsAdded, noStep and emptyLines
        partsAdded = false;
        noStep = false;
        // check if subfile and process
      } else if (tokens.size() == 15 && tokens[0] >= "1" && tokens[0] <= "5") {
        if (contains(tokens[14]) && ! stepIgnore && ! buildModIgnore) {
            firstStep = true;
            countInstances(tokens[14], firstStep, mirrored(tokens), callout);
        }
        partsAdded = true;
      }
    }

    // increment steps and add BuildMod step index if parts added in the last step of the sub/model and not ignored by BuildMod
    if (partsAdded && ! noStep) {
      int incr = (isMirrored && f->_mirrorInstances == 0) ||
                 (! isMirrored && f->_instances == 0);
      f->_numSteps += incr;
      if (! buildModIgnore) {
        _buildModStepIndexes.append({ modelIndex, i });
      }
    }

    if ( ! callout) {
      if (isMirrored) {
        ++f->_mirrorInstances;
      } else {
        ++f->_instances;
      }
    }

  } // file end
  f->_beenCounted = true;
  countMutex.unlock();
}

void LDrawFile::countInstances()
{
  for (int i = 0; i < _subFileOrder.size(); i++) {
    QString fileName = _subFileOrder[i].toLower();
    QMap<QString, LDrawSubFile>::iterator it = _subFiles.find(fileName);
    it->_instances = 0;
    it->_mirrorInstances = 0;
    it->_beenCounted = false;
  }

#ifdef QT_DEBUG_MODE
  QElapsedTimer timer;
  timer.start();
#endif

  emit gui->messageSig(LOG_STATUS, "Counting model steps and instances...");
  QApplication::processEvents();

  /*
   * For countInstances, the BuildMod behaviour creates a sequential
   * list (Vector<QString>) of all the steps in the loaded model file.
   * The buildMod flag uses a multilevel (_currentLevels) framework to
   * determine the current BuildMod when mods are nested.
   */
  //countInstances(topLevelFile(), true/*firstStep*/, false /*isMirrored*/);
  QFuture<void> future = QtConcurrent::run([this]() {
      countInstances(topLevelFile(), true/*firstStep*/, false /*isMirrored*/);
  });

  future.waitForFinished();

  _buildModStepIndexes.append({ 0/*SubmodelIndex*/, size(topLevelFile()) });

/*
#ifdef QT_DEBUG_MODE
  emit gui->messageSig(LOG_DEBUG, QString("CountInstances Step Indexes:"));
  for (int i = 0; i < _buildModStepIndexes.size(); i++)
  {
      const QVector<int> &key = _buildModStepIndexes.at(i);
      emit gui->messageSig(LOG_DEBUG, QString("StepIndex: %1, SubmodelIndex: %2: LineNumber: %3, ModelName: %4")
                                              .arg(i)                            // index
                                              .arg(key.at(0))                    // modelIndex
                                              .arg(key.at(1))                    // lineNumber
                                              .arg(getSubmodelName(key.at(0)))); // modelName
  }
#endif
*/
#ifdef QT_DEBUG_MODE
  emit gui->messageSig(LOG_DEBUG, QString("Count steps and submodel instances - %1")
                                          .arg(gui->elapsedTime(timer.elapsed())));
#endif

}

bool LDrawFile::saveMPDFile(const QString &fileName)
{
    QString writeFileName = fileName;
    QFile file(writeFileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        emit gui->messageSig(LOG_ERROR,QString("Cannot write file %1:<br>%2.")
                             .arg(writeFileName)
                             .arg(file.errorString()));
        return false;
    }

    QStringList saveTopLevelContent = contents(topLevelFile());

    gui->mloadingFile = true;
    gui->deleteFinalModelStep();

    QTextStream out(&file);
    out.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
    for (int i = 0; i < _subFileOrder.size(); i++) {
      QString subFileName = _subFileOrder[i];
      QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(subFileName);
      if (f != _subFiles.end() && ! f.value()._generated) {
        if (!f.value()._subFilePath.isEmpty()) {
            file.close();
            writeFileName = f.value()._subFilePath;
            file.setFileName(writeFileName);
            if (!file.open(QFile::WriteOnly | QFile::Text)) {
                emit gui->messageSig(LOG_ERROR,QString("Cannot write file %1:<br>%2.")
                                    .arg(writeFileName)
                                    .arg(file.errorString()));
                gui->insertFinalModelStep();
                gui->mloadingFile = false;
                return false;
            }

            QTextStream out(&file);
            out.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
        }
        if (!f.value()._includeFile)
          out << "0 FILE " << subFileName << endl;
        for (int j = 0; j < f.value()._contents.size(); j++) {
          out << f.value()._contents[j] << endl;
        }
        if (!f.value()._includeFile)
          out << "0 NOFILE " << endl;
      }
    }

    gui->insertFinalModelStep();
    gui->mloadingFile = false;

    return true;
}

void LDrawFile::countParts(const QString &fileName) {

    emit gui->progressBarPermInitSig();
    emit gui->progressPermRangeSig(1, size(fileName));
    emit gui->progressPermMessageSig("Counting parts for " + fileName + "...");

    int topModelIndx = getSubmodelIndex(fileName);

    std::function<void(int)> countModelParts;
    countModelParts = [this, &countModelParts, &topModelIndx] (int modelIndx)
    {
        QString modelName = getSubmodelName(modelIndx);
        QStringList content = contents(modelName);
        if (content.size()) {
            // initialize model parts count
            int modelPartCount = 0;

            // get content size
            int c = content.size();

            // initialize valid line
            bool lineIncluded  = true;

            // process submodel content...
            for (int i = 0; i < c; i++) {

                QStringList tokens;
                QString line = content.at(i);

                if (modelIndx == topModelIndx)
                    emit gui->progressPermSetValueSig(i);

                // adjust ghost lines
                if (line.startsWith("0 GHOST "))
                    line = line.mid(8).trimmed();

                split(line,tokens);

                // build modification - starts at BEGIN command and ends at END_MOD action
                if (tokens.size() >= 4 && tokens[0] == "0"  &&
                   (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
                    tokens[2] == "BUILD_MOD") {
                    if (tokens[3] == "BEGIN") {
                        buildModLevel = getLevel(tokens[4], BM_BEGIN);
                    } else if (tokens[3] == "END_MOD") {
                        buildModLevel = getLevel("", BM_END);
                    }
                    lineIncluded = ! buildModLevel;
                }

                // ignore parts begin
                if (tokens.size() == 5 && tokens[0] == "0" &&
                   (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
                   (tokens[2] == "PART" || tokens[2] == "PLI") &&
                    tokens[3] == "BEGIN"  &&
                    tokens[4] == "IGN") {
                    lineIncluded = false;
                } else
                // ignore part end
                if (tokens.size() == 4 && tokens[0] == "0" &&
                   (tokens[1] == "!LPUB" || tokens[1] == "LPUB") &&
                   (tokens[2] == "PART" || tokens[2] == "PLI") &&
                    tokens[3] == "END") {
                   lineIncluded = true;
                }

                QString type;
                bool countThisLine = true;
                if ((countThisLine = tokens.size() == 15 && tokens[0] == "1"))
                    type = tokens[14];
                else if (isSubstitute(line, type))
                    countThisLine = !type.isEmpty();

                bool partIncluded = !ExcludedParts::hasExcludedPart(type);

                if (countThisLine && lineIncluded && partIncluded) {
                    QString partString = "|" + type + "|";
                    bool containsSubFile = contains(type.toLower());
                    if (containsSubFile) {
                        LDrawUnofficialFileType subFileType = LDrawUnofficialFileType(isUnofficialPart(type.toLower()));
                        if (subFileType == UNOFFICIAL_SUBMODEL){
                            //emit gui->messageSig(LOG_TRACE,QString("UNOFFICIAL_SUBMODEL %1 LINE %2 MODEL %3").arg(type).arg(i).arg(modelName));
                            countModelParts(getSubmodelIndex(type));
                        } else {
                            switch(subFileType){
                            case  UNOFFICIAL_PART:
                                _partCount++;modelPartCount++;
                                //emit gui->messageSig(LOG_TRACE,QString("UNOFFICIAL_PART %1 LINE %2 MODEL %3 COUNT %4").arg(type).arg(i).arg(modelName).arg(_partCount));
                                //emit gui->messageSig(LOG_STATUS, QString("Part count for [%1] %2").arg(modelName).arg(modelPartCount));
                                partString += QString("Unofficial inlined part");
                                if (!_loadedParts.contains(QString(VALID_LOAD_MSG) + partString)) {
                                    _loadedParts.append(QString(VALID_LOAD_MSG) + partString);
                                    emit gui->messageSig(LOG_NOTICE,QString("Unofficial inlined part %1 [%2] validated.").arg(_partCount).arg(type));
                                }
                                break;
                            case  UNOFFICIAL_SUBPART:
                                partString += QString("Unofficial inlined subpart");
                                //emit gui->messageSig(LOG_DEBUG,QString("UNOFFICIAL_SUBPART %1 LINE %2 MODEL %3").arg(type).arg(i).arg(modelName));
                                if (!_loadedParts.contains(QString(SUBPART_LOAD_MSG) + partString)) {
                                    _loadedParts.append(QString(SUBPART_LOAD_MSG) + partString);
                                    emit gui->messageSig(LOG_NOTICE,QString("Unofficial inlined part [%1] is a SUBPART").arg(type));
                                }
                                break;
                            case  UNOFFICIAL_PRIMITIVE:
                                partString += QString("Unofficial inlined primitive");
                                //emit gui->messageSig(LOG_DEBUG,QString("UNOFFICIAL_PRIMITIVE %1 LINE %2 MODEL %3").arg(type).arg(i).arg(modelName));
                                if (!_loadedParts.contains(QString(PRIMITIVE_LOAD_MSG) + partString)) {
                                    _loadedParts.append(QString(PRIMITIVE_LOAD_MSG) + partString);
                                    emit gui->messageSig(LOG_NOTICE,QString("Unofficial inlined part [%1] is a PRIMITIVE").arg(type));
                                }
                                break;
                            default:
                                break;
                            }
                        }
                    } else {
                        QString partFile = type.toUpper();
                        if (partFile.startsWith("S\\")) {
                            partFile.replace("S\\","S/");
                        }
                        PieceInfo* pieceInfo;
                        if (!_loadedParts.contains(QString(VALID_LOAD_MSG) + partString)) {
                            pieceInfo = gui->GetPiecesLibrary()->FindPiece(partFile.toLatin1().constData(), nullptr, false, false);
                            if (pieceInfo) {
                                partString += pieceInfo->m_strDescription;
                                if (pieceInfo->IsSubPiece()) {
                                    //emit gui->messageSig(LOG_DEBUG,QString("PIECE_SUBPART %1 LINE %2 MODEL %3").arg(type).arg(i).arg(modelName));
                                    if (!_loadedParts.contains(QString(SUBPART_LOAD_MSG) + partString)){
                                        _loadedParts.append(QString(SUBPART_LOAD_MSG) + partString);
                                        emit gui->messageSig(LOG_NOTICE,QString("Part [%1] is a SUBPART").arg(type));
                                    }
                                } else
                                if (pieceInfo->IsPartType()) {
                                    _partCount++;modelPartCount++;;
                                    //emit gui->messageSig(LOG_TRACE,QString("PIECE_PART %1 LINE %2 MODEL %3 COUNT %4").arg(type).arg(i).arg(modelName).arg(_partCount));
                                    //emit gui->messageSig(LOG_STATUS, QString("Part count for [%1] %2").arg(modelName).arg(modelPartCount));
                                    if (!_loadedParts.contains(QString(VALID_LOAD_MSG) + partString)) {
                                        _loadedParts.append(QString(VALID_LOAD_MSG) + partString);
                                        emit gui->messageSig(LOG_NOTICE,QString("Part %1 [%2] validated.").arg(_partCount).arg(type));
                                    }
                                } else
                                if (gui->GetPiecesLibrary()->IsPrimitive(partFile.toLatin1().constData())){
                                    if (pieceInfo->IsSubPiece()) {
                                        //emit gui->messageSig(LOG_DEBUG,QString("PIECE_SUBPART_PRIMITIVE %1 LINE %2 MODEL %3").arg(type).arg(i).arg(modelName));
                                        if (!_loadedParts.contains(QString(SUBPART_LOAD_MSG) + partString)) {
                                            _loadedParts.append(QString(SUBPART_LOAD_MSG) + partString);
                                            emit gui->messageSig(LOG_NOTICE,QString("Part [%1] is a SUBPART").arg(type));
                                        }
                                    } else {
                                        //emit gui->messageSig(LOG_DEBUG,QString("PIECE_PRIMITIVE %1 LINE %2 MODEL %3").arg(type).arg(i).arg(modelName));
                                        if (!_loadedParts.contains(QString(PRIMITIVE_LOAD_MSG) + partString)) {
                                            _loadedParts.append(QString(PRIMITIVE_LOAD_MSG) + partString);
                                            emit gui->messageSig(LOG_NOTICE,QString("Part [%1] is a PRIMITIVE").arg(type));
                                        }
                                    }
                                }
                            } else {
                                partString += QString("Part not found");
                                if (!_loadedParts.contains(QString(MISSING_LOAD_MSG) + partString)) {
                                    _loadedParts.append(QString(MISSING_LOAD_MSG) + partString);
                                    emit gui->messageSig(LOG_NOTICE,QString("Part [%1] not excluded, not a submodel and not found in the %2 library archives.")
                                                         .arg(type).arg(VER_PRODUCTNAME_STR));
                                }
                            }
                        }
                    }  // check archive
                }
            } // process submodel content
        }
    };

    countModelParts(topModelIndx);

    emit gui->messageSig(LOG_STATUS, QString("Parts count for %1 is %2").arg(fileName).arg(_partCount));
    emit gui->progressPermSetValueSig(size(fileName));
    emit gui->progressPermStatusRemoveSig();
}

bool LDrawFile::saveLDRFile(const QString &fileName)
{
    QString path = QFileInfo(fileName).path();
    QFile file;

    gui->mloadingFile = true;
    gui->deleteFinalModelStep();

    for (int i = 0; i < _subFileOrder.size(); i++) {
      QString writeFileName;
      if (i == 0) {
        writeFileName = fileName;
      } else {
        writeFileName = path + QDir::separator() + _subFileOrder[i];
      }
      file.setFileName(writeFileName);
      QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(_subFileOrder[i]);
      if (f != _subFiles.end() && ! f.value()._generated) {
        if (f.value()._modified) {
          if (!f.value()._subFilePath.isEmpty()) {
              writeFileName = f.value()._subFilePath;
              file.setFileName(writeFileName);
          }
          if (!file.open(QFile::WriteOnly | QFile::Text)) {
            emit gui->messageSig(LOG_ERROR,QString("Cannot write file %1:<br>%2.")
                                 .arg(writeFileName)
                                 .arg(file.errorString()));
            gui->insertFinalModelStep();
            gui->mloadingFile = false;
            return false;
          }
          QTextStream out(&file);
          out.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
          for (int j = 0; j < f.value()._contents.size(); j++) {
            out << f.value()._contents[j] << endl;
          }
          file.close();
        }
      }
    }

    gui->insertFinalModelStep();
    gui->mloadingFile = false;

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
          emit gui->messageSig(LOG_ERROR,QString("Cannot write file %1:<br>%2.")
                               .arg(writeFileName)
                               .arg(file.errorString()));
          return false;
        }
        QTextStream out(&file);
        out.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
        for (int j = 0; j < f.value()._contents.size(); j++) {
          out << f.value()._contents[j] << endl;
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
  QString key;
  Q_FOREACH (key,_subFiles.keys()) {
    _subFiles[key]._changedSinceLastWrite = true;
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
  Q_FOREACH (QString lid, lids){
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
  QMap<int,int> modActions;
  QMap<QString, BuildMod>::iterator i = _buildMods.find(modKey);
  if (i != _buildMods.end()) {
    // Preserve actions
    modActions = i.value()._modActions;
    QMap<int,int>::iterator a = modActions.find(stepIndex);
    if (a != modActions.end())
        modActions.erase(a);
    // Remove BuildMod if exist
    _buildMods.erase(i);
  }
  // Initialize new BuildMod
  BuildMod buildMod(modAttributes, stepIndex);
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
  insertBuildModStep(modKey, stepIndex);

  // Set subfile modified
  QString modFileName = getBuildModStepKeyModelName(modKey);
  QMap<QString, LDrawSubFile>::iterator s = _subFiles.find(modFileName);
  if (s != _subFiles.end()) {
    s.value()._modified = true;
    s.value()._changedSinceLastWrite = true;
  }

  // Update BuildMod list
  if (!_buildModList.contains(buildModKey))
      _buildModList.append(buildModKey);
}

/* This call captures steps that have buildMod command(s) */

void LDrawFile::insertBuildModStep(const QString &buildModKey,
                                   const int      stepIndex,
                                   const int      modAction)
{
    QString modKey = buildModKey.toLower();
    BuildModStep newModStep(stepIndex, modAction ? modAction : getBuildModAction(modKey, stepIndex), modKey);
    QMap<int, BuildModStep>::iterator i = _buildModSteps.find(stepIndex);
    while (i != _buildModSteps.end() && i.key() == stepIndex) {
        if (i.value() == newModStep) {
            _buildModSteps.erase(i);
            break;
        }
        ++i;
    }

    _buildModSteps.insert(stepIndex, newModStep);

#ifdef QT_DEBUG_MODE
    int action = modAction ? modAction : newModStep._buildModAction;
    emit gui->messageSig(LOG_DEBUG, QString("Insert BuildModStep ModStepIndex: %1, Action: %2, ModKey: %3")
                         .arg(stepIndex).arg(action == BuildModApplyRc ? "Apply" : "Remove").arg(buildModKey));
#endif
}

int LDrawFile::getBuildModStep(const QString &modelName,
                               const int &lineNumber)
{
    QString modKey = "Undefined";
    int modAction = BuildModNoActionRc;
    int modelIndex = getSubmodelIndex(modelName);
    int modStepIndex = getBuildModStepIndex(modelIndex, lineNumber);
    int modSourceStepIndex = -1;

    QMap<int, BuildModStep>::const_iterator i = _buildModSteps.find(modStepIndex);
    if (i != _buildModSteps.end()) {
        modAction = i.value()._buildModAction;
        modKey = i.value()._buildModKey;
        modSourceStepIndex = getBuildModStepIndex(modKey);
        if (modSourceStepIndex == modStepIndex)
            modAction = BuildModSourceRc;

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Get BuildModStep StepIndex: %1, SourceIndex: %2, ModCount: %3, Action: %4, TopOfPage: ModelIndex %5 Line %6, ModKey: %7")
                        .arg(modStepIndex)
                        .arg(modSourceStepIndex)
                        .arg(_buildModSteps.values(modStepIndex).size())
                        .arg(modAction == BuildModApplyRc ? "Apply" : modAction == BuildModRemoveRc ? "Remove" : modAction == BuildModSourceRc ? "Source" : "None")
                        .arg(modelIndex).arg(lineNumber).arg(modKey));
#endif
    }
/*
    else {
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Get BuildModStep StepIndex: %1, No BuildModStep").arg(modStepIndex));
#endif
    }
*/

    return modAction;
}

void LDrawFile::clearBuildModStep(const QString &buildModKey,const int stepIndex)
{
    QString modKey = buildModKey.toLower();
    int action = BuildModNoActionRc;
    QMap<int, BuildModStep>::iterator i = _buildModSteps.find(stepIndex);
    while (i != _buildModSteps.end() && i.key() == stepIndex) {
        if (i.value()._buildModStepIndex == stepIndex &&
            i.value()._buildModKey == modKey) {
            action = i.value()._buildModAction;
            _buildModSteps.erase(i);
            break;
        }
        ++i;
    }

#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_DEBUG, QString("Remove BuildModStep ModStepIndex: %1, Action: %2, ModKey: %3")
                         .arg(stepIndex)
                         .arg(action == BuildModApplyRc ? "Apply" : action == BuildModRemoveRc ? "Remove" : action == BuildModSourceRc ? "Source" : "None")
                         .arg(buildModKey));
#endif
}

void LDrawFile::clearBuildModSteps(const QString &buildModKey)
{
#ifdef QT_DEBUG_MODE
    emit gui->messageSig(LOG_TRACE, QString("Remove BuildModStep actions for ModKey %1...") .arg(buildModKey));
#endif

    QString modKey = buildModKey.toLower();
    QMultiMap<int, BuildModStep> buildModSteps;
    QMap<int, BuildModStep>::iterator i = _buildModSteps.begin();
    while (i != _buildModSteps.end()) {
        if(i.value()._buildModKey != modKey ) {
            buildModSteps.insert(i.key(),i.value());
        }
#ifdef QT_DEBUG_MODE
        else
        {
            int action = i.value()._buildModAction;
            emit gui->messageSig(LOG_TRACE, QString("Step Index: %1, Action: %2")
                                 .arg(i.value()._buildModStepIndex)
                                 .arg(action == BuildModApplyRc ? "Apply" : action == BuildModRemoveRc ? "Remove" : action == BuildModSourceRc ? "Source" : "None"));

        }
#endif
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
        if (_buildModList.contains(buildModKey, Qt::CaseInsensitive))
            _buildModList.removeAt(_buildModList.indexOf(buildModKey));

        // Set subfile modified
        QString modFileName = getBuildModStepKeyModelName(modKey);
        QMap<QString, LDrawSubFile>::iterator s = _subFiles.find(modFileName);
        if (s != _subFiles.end()) {
          s.value()._modified = true;
          s.value()._changedSinceLastWrite = true;
        }

        // Clear build mod steps
        clearBuildModSteps(modKey);

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Removed BuildMod ModKey: %1").arg(buildModKey));
#endif
        return true;
    }
    return false;
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
        emit gui->messageSig(LOG_DEBUG, QString("Update BuildMod StepKey: %1;%2;%3, StepIndex: %4, Action: %5, ModKey: %6")
                             .arg(i.value()._modAttributes.at(BM_MODEL_NAME_INDEX))
                             .arg(i.value()._modAttributes.at(BM_MODEL_LINE_NUM))
                             .arg(i.value()._modAttributes.at(BM_MODEL_STEP_NUM))
                             .arg(i.value()._modStepIndex)
                             .arg(action ? action == BuildModApplyRc ? "Apply" : "Remove" : "NoActon")
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
        emit gui->messageSig(LOG_DEBUG, QString("Set BuildMod DisplayPageNumber: %1, BuildModKey: %2")
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
        emit gui->messageSig(LOG_DEBUG, QString("Update BuildMod StepPieces: %1, StepIndex: %2, Action: %3, ModKey: %4")
                                                .arg(i.value()._modAttributes.at(BM_STEP_PIECES))
                                                .arg(i.value()._modStepIndex)
                                                .arg(action ? action == BuildModApplyRc ? "Apply" : "Remove" : "NoActon")
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
        emit gui->messageSig(LOG_DEBUG, QString("Insert BuildMod RenderedModel: %1, ModKey: %2").arg(renderedModel).arg(buildModKey));
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
            emit gui->messageSig(LOG_DEBUG, QString("BuildMod RenderedModel: %1, ModKey: %2")
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
            emit gui->messageSig(LOG_DEBUG, QString("Clear BuildMod RenderedModel: %1, Count: %2, ModKey: %3")
                                 .arg(renderedModel).arg(cleared).arg(buildModKey));
#endif
    }
}

void LDrawFile::clearBuildModRendered()
{
    QString key;
    Q_FOREACH (key,_buildModRendered.keys()) {
    }
        _buildModRendered[key].clear();
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
    int unusedIndex;
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
      if (stepIndex == BM_LAST_ACTION) {
          action = i.value()._modActions.last();
          actionStepIndex = i.value()._modActions.lastKey();
          insert = " Last";
      } else {
          QMap<int, int>::iterator a = i.value()._modActions.find(stepIndex);
          if (a != i.value()._modActions.end()) {
              action = i.value()._modActions.value(stepIndex);
          } else if (i.value()._modActions.size()) {
              int keyIndex = stepIndex;
              if (defaultIndex == BM_LAST_ACTION) {
                  action = i.value()._modActions.last();
                  keyIndex = i.value()._modActions.lastKey();
                  insert = " Last";
              } else if (defaultIndex == BM_PREVIOUS_ACTION) {
                  // iterate backward to get the last action index before the specified step index (account for action index gap)
                  for (; keyIndex > 0; keyIndex--) {
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

  if (!action) {
     action = setBuildModAction(buildModKey, stepIndex, BuildModApplyRc);
#ifdef QT_DEBUG_MODE
     insert = " Default";
#endif
  }

#ifdef QT_DEBUG_MODE
  if (!action)
      emit gui->messageSig(LOG_ERROR, QString("Get BuildMod%1 Action: (INVALID), StepIndex: %2, ActionStepIndex: %3, BuildModKey: %4")
                                              .arg(insert)
                                              .arg(stepIndex)
                                              .arg(actionStepIndex)
                                              .arg(buildModKey));
  else
      emit gui->messageSig(LOG_TRACE, QString("Get BuildMod%1 Action: %2, StepIndex: %3, ActionStepIndex: %4, BuildModKey: %5")
                                              .arg(insert)
                                              .arg(action == BuildModApplyRc ? "Apply" : "Remove")
                                              .arg(stepIndex)
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
    bool change = false;
    if (i != _buildMods.end()) {
        QMap<int, int>::iterator ai = i.value()._modActions.find(stepIndex);
        if (ai != i.value()._modActions.end())
            i.value()._modActions.remove(stepIndex);
        i.value()._modActions.insert(stepIndex, modAction);

        insertBuildModStep(modKey, stepIndex);

        QString modFileName = getBuildModStepKeyModelName(modKey);
        QMap<QString, LDrawSubFile>::iterator s = _subFiles.find(modFileName);
        if (s != _subFiles.end()) {
          change = true;
          s.value()._modified = true;
          s.value()._changedSinceLastWrite = true;
        }

        action = i.value()._modActions.value(stepIndex);

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Set BuildMod Action: %1, StepIndex: %2, Changed: %3, ModelFile: %4")
                                                .arg(action == BuildModApplyRc ? "Apply" : "Remove")
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
        bool change = false;
        int action = BuildModNoActionRc;

        QString modFileName = getBuildModStepKeyModelName(modKey);
        QMap<int, int>::iterator a = i.value()._modActions.find(stepIndex);
        if (a != i.value()._modActions.end()) {
            action = i.value()._modActions.value(stepIndex);
            i.value()._modActions.remove(stepIndex);

            clearBuildModStep(modKey, stepIndex);

            QMap<QString, LDrawSubFile>::iterator s = _subFiles.find(modFileName);
            if (s != _subFiles.end()) {
              change = true;
              s.value()._modified = true;
              s.value()._changedSinceLastWrite = true;
            }
        }

#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_TRACE, QString("Remove BuildMod Action: %1, StepIndex: %2, Changed: %3, ModelFile: %4")
                                                .arg(action == BuildModApplyRc ? "Apply" : "Remove")
                                                .arg(stepIndex)
                                                .arg(change ? "True" : "False")
                                                .arg(modFileName));
#endif
    }
}

/* Returns index for BEGIN, APPLY and REMOVE BuildMod commands, requires TopOfStep */

int LDrawFile::getBuildModStepIndex(const int _modelIndex, const int _lineNumber)
{
#ifdef QT_DEBUG_MODE
    LogType logType = LOG_DEBUG;
    QString insert = QString("Get BuildMod");
#endif

    int modelIndex = _modelIndex;
    int lineNumber = _lineNumber;
    int stepIndex = BM_INVALID_INDEX;
    if (modelIndex > BM_INVALID_INDEX) {
        if (!lineNumber && !modelIndex) {
            // If modelIndex and lineNumber is 0, we are processing the first step
            stepIndex = modelIndex = 0;
#ifdef QT_DEBUG_MODE
            insert = QString("Get BuildMod (FIRST STEP)");
#endif
            lineNumber = _buildModStepIndexes.at(stepIndex).at(BM_LINE_NUMBER);
        } else {
            QVector<int> indexKey = { modelIndex, lineNumber };
            stepIndex = _buildModStepIndexes.indexOf(indexKey);
        }
    }
#ifdef QT_DEBUG_MODE
    else {
        logType = LOG_ERROR;
        insert = QString("Get BuildMod (INVALID)");
    }

    emit gui->messageSig(logType,  QString("%1 StepIndex: %2, ModelIndex: %3, LineNumber %4, ModelName: %5")
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
#ifdef QT_DEBUG_MODE
    LogType logType = LOG_DEBUG;
    QString message;
#endif

    int lineNumber = 0;
    if (stepIndex  > BM_INVALID_INDEX && stepIndex < _buildModStepIndexes.size()) {
        if( stepIndex < _buildModStepIndexes.size() - 1) {
            if (bottom) {
                lineNumber = _buildModStepIndexes.at(stepIndex + 1).at(BM_LINE_NUMBER);
            } else /*if (top)*/ {
                lineNumber = _buildModStepIndexes.at(stepIndex).at(BM_LINE_NUMBER);
            }
        } else if (stepIndex == _buildModStepIndexes.size() - 1) { // last step of model
            lineNumber = _buildModStepIndexes.at(stepIndex).at(BM_LINE_NUMBER);
        }
#ifdef QT_DEBUG_MODE
        message = QString("Get BuildMod %1 LineNumber: %2, StepIndex: %3, ModelName: %4")
                          .arg(bottom ? "BottomOfStep," : "TopOfStep,")
                          .arg(lineNumber).arg(stepIndex)
                          .arg(getSubmodelName(_buildModStepIndexes.at(bottom ? stepIndex + 1 : stepIndex).at(BM_MODEL_NAME)));
#endif
    }
#ifdef QT_DEBUG_MODE
    else {
        logType = LOG_ERROR;
        message = QString("Get BuildMod (INVALID) %1 LineNumber: %2, StepIndex: %3")
                          .arg(bottom ? "BottomOfStep," : "TopOfStep,")
                          .arg(lineNumber).arg(stepIndex);
    }

    emit gui->messageSig(logType, message);
#endif

    return lineNumber;
}

/* Returns index for BEGIN BuildMod command, requires BuildModKey*/

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
#ifdef QT_DEBUG_MODE
  LogType logType = LOG_DEBUG;
  QString insert  = QString("Get BuildMod Where");
#endif

  bool validIndex = false;
  if (stepIndex > BM_INVALID_INDEX && stepIndex < _buildModStepIndexes.size()) {
      modelIndex = _buildModStepIndexes.at(stepIndex).at(BM_MODEL_NAME);
      lineNumber = _buildModStepIndexes.at(stepIndex).at(BM_LINE_NUMBER);
      modelName  = getSubmodelName(modelIndex);
      validIndex = ! modelName.isEmpty() && lineNumber > 0;
  }
#ifdef QT_DEBUG_MODE
  else {
      logType = LOG_ERROR;
      insert = QString("Get BuildMod (INVALID) Where");
  }

  emit gui->messageSig(logType, QString("%1 StepIndex: %2, ModelIndex: %3, LineNumber %4, ModelName: %5")
                                        .arg(insert)
                                        .arg(stepIndex)
                                        .arg(modelIndex)
                                        .arg(lineNumber)
                                        .arg(modelName));
#endif

  return validIndex;
}

int LDrawFile::getBuildModPrevStepIndex()
{
    return _buildModPrevStepIndex;
}

int LDrawFile::getBuildModNextStepIndex()
{
#ifdef QT_DEBUG_MODE
    LogType logType = LOG_DEBUG;
    QString message;
    bool validIndex = false;
    bool firstIndex = false;
#endif

    int stepIndex   = 0;

    if (_buildModNextStepIndex > BM_INVALID_INDEX && _buildModStepIndexes.size() > _buildModNextStepIndex) {

        stepIndex  = _buildModNextStepIndex;

#ifdef QT_DEBUG_MODE
        firstIndex = _buildModNextStepIndex == BM_FIRST_INDEX;

        validIndex = true;
#endif
    } else {

        stepIndex = BM_INVALID_INDEX;

#ifdef QT_DEBUG_MODE
        logType = LOG_ERROR;
        message = QString("Get BuildMod (INVALID) StepIndex: %1")
                          .arg(_buildModNextStepIndex);
#endif

    }

#ifdef QT_DEBUG_MODE
    if (validIndex) {
        QVector<int> stepIndex = _buildModStepIndexes.at(_buildModNextStepIndex);
        QString modelName      = getSubmodelName(stepIndex.at(BM_MODEL_NAME));
        QString insert         = firstIndex ? "First" : "Next";
        int lineNumber         = stepIndex.at(BM_LINE_NUMBER);
        validIndex             = !modelName.isEmpty() && lineNumber > BM_BEGIN_LINE_NUM;
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

#endif

    return stepIndex;

}

bool LDrawFile::setBuildModNextStepIndex(const QString &modelName, const int &lineNumber)
{
    int  newStepIndex = getStepIndex(modelName,lineNumber);
    bool validIndex   = newStepIndex > BM_INVALID_INDEX;
    bool firstIndex   = _buildModNextStepIndex == BM_INVALID_INDEX;
    bool nextIndex    = validIndex && !firstIndex;

    _buildModPrevStepIndex = nextIndex ? _buildModNextStepIndex : 0;
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
  QVector<int> topOfStep = { getSubmodelIndex(modelName), lineNumber };
  return _buildModStepIndexes.indexOf(topOfStep);
}

void LDrawFile::skipHeader(const QString &modelName, int &lineNumber)
{
    int numLines = size(modelName);
    for ( ; lineNumber < numLines; lineNumber++) {
        QString line = readLine(modelName,lineNumber);
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

void LDrawFile::getTopOfStepWhere(const QString &modelName, int &modelIndex, int &lineNumber)
{
    const QStringList keys = getViewerStepKeyWhere(getSubmodelIndex(modelName), lineNumber).split(";");
    bool valid = false;
    if (keys.size() > 2) {
        bool ok[2];
        modelIndex = keys.at(BM_STEP_MODEL_KEY).toInt(&ok[0]);
        lineNumber = keys.at(BM_STEP_LINE_KEY).toInt(&ok[1]);
        valid = ok[0] && ok[1];
    }
    if (!valid) {
        modelIndex = -1;
        lineNumber =  0;
    }
}

QString LDrawFile::getViewerStepKeyWhere(const int modelIndex, const int lineNumber)
{
    QVector<QVector<int>> stepIndexes;
    for (QVector<int> &topOfStep : _buildModStepIndexes) {
        if (topOfStep.at(BM_STEP_MODEL_KEY) == modelIndex) {
            stepIndexes.append(topOfStep);
            if (topOfStep.at(BM_STEP_LINE_KEY) > lineNumber)
                break;
        }
    }

    QString stepKey;
    int index = BM_INVALID_INDEX;
    for (QVector<int> &topOfStep : stepIndexes) {
        if (topOfStep.at(BM_STEP_MODEL_KEY) == modelIndex) {
            if (topOfStep.at(BM_STEP_LINE_KEY) == lineNumber)
                index = stepIndexes.indexOf(topOfStep);
            else if (topOfStep.at(BM_STEP_LINE_KEY) > lineNumber)
                index = stepIndexes.indexOf(topOfStep) - 1;
            if (index > BM_INVALID_INDEX) {
                int lineNumber = stepIndexes.at(index).at(BM_STEP_LINE_KEY);
                stepKey = QString("%1;%2;0").arg(modelIndex).arg(lineNumber);
                QMap<QString, ViewerStep>::const_iterator i = _viewerSteps.constBegin();
                while (i != _viewerSteps.constEnd()) {
                    if (i->_viewType == Options::CSI && i->_stepKey.modIndex == modelIndex && i->_stepKey.lineNum == lineNumber) {
                        if (i->_stepKey.stepNum) {
                            stepKey.chop(1);
                            stepKey.append(QString::number(i->_stepKey.stepNum));
                        }
                        break;
                    }
                    ++i;
                }
                break;
            }
        }
    }

    return stepKey;
}

QString LDrawFile::getViewerStepKeyFromRange(const int modelIndex, const int lineNumber, const int top, const int bottom)
{
    if (lineNumber >= top && lineNumber <= bottom)
        return getViewerStepKeyWhere(modelIndex, lineNumber);
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

/* this call gets the paths from the specified submodel step to the end of the submodel */

QStringList LDrawFile::getBuildModPathsFromStep(const QString &modStepKey, const int image)
{
  QStringList list = modStepKey.split(";");
  ViewerStep::StepKey stepKey = { list.at(BM_STEP_MODEL_KEY).toInt(), list.at(BM_STEP_LINE_KEY).toInt(), list.at(BM_STEP_NUM_KEY).toInt() };
  list.clear();
  QMap<QString, ViewerStep>::const_iterator i = _viewerSteps.constBegin();
  while (i != _viewerSteps.constEnd()) {
    if (stepKey.modIndex == i->_stepKey.modIndex && i->_viewType == Options::CSI) {
      if (stepKey.stepNum <= i->_stepKey.stepNum)
        list.append(image ? i->_imagePath : i->_filePath);
    }
    ++i;
  }

  list.removeDuplicates();

  return list;
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

/* 3DViewer routines */

void LDrawFile::insertViewerStep(const QString     &stepKey,
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
  ViewerStep viewerStep(stepKey.split(";"),rotatedContents,unrotatedContents,filePath,imagePath,csiKey,multiStep,calledOut,viewType);
  Q_FOREACH(QString line, rotatedContents)
    if (line[0] == '1')
      viewerStep._partCount++;
  _viewerSteps.insert(stepKey,viewerStep);
}

/* Viewer Step Exist */

void LDrawFile::updateViewerStep(const QString &stepKey, const QStringList &contents, bool rotated)
{
  QString    mStepKey = stepKey.toLower();
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(mStepKey);

  if (i != _viewerSteps.end()) {
    if (rotated)
      i.value()._rotatedContents = contents;
    else
      i.value()._unrotatedContents = contents;
    i.value()._partCount = 0;
    Q_FOREACH(QString line, contents)
      if (line[0] == '1')
        i.value()._partCount++;
    i.value()._modified = true;
  }
}

/* return viewer step rotatedContents */

QStringList LDrawFile::getViewerStepRotatedContents(const QString &stepKey)
{
  QString mStepKey = stepKey.toLower();
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(mStepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._rotatedContents;
  }
  return _emptyList;
}

/* return viewer step unrotatedContents */

QStringList LDrawFile::getViewerStepUnrotatedContents(const QString &stepKey)
{
  QString mStepKey = stepKey.toLower();
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(mStepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._unrotatedContents;
  }
  return _emptyList;
}

/* return viewer step file path */

QString LDrawFile::getViewerStepFilePath(const QString &stepKey)
{
  QString mStepKey = stepKey.toLower();
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(mStepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._filePath;
  }
  return _emptyString;
}

/* return viewer step image path */

QString LDrawFile::getViewerStepImagePath(const QString &stepKey)
{
  QString mStepKey = stepKey.toLower();
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(mStepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._imagePath;
  }
  return _emptyString;
}

/* return viewer step CSI key */

QString LDrawFile::getViewerConfigKey(const QString &stepKey)
{
  QString mStepKey = stepKey.toLower();
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(mStepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._csiKey;
  }
  return _emptyString;
}

/* return viewer step part count */

int LDrawFile::getViewerStepPartCount(const QString &stepKey)
{
    QString mStepKey = stepKey.toLower();
    QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(mStepKey);
    if (i != _viewerSteps.end()) {
      return i.value()._partCount;
    }
    return 0;
}

/* Viewer Step Exist */

bool LDrawFile::viewerStepContentExist(const QString &stepKey)
{
  QString    mStepKey = stepKey.toLower();
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(mStepKey);

  if (i != _viewerSteps.end()) {
    return true;
  }
  return false;
}

/* Delete Viewer Step */

bool LDrawFile::deleteViewerStep(const QString &stepKey)
{
    QString mStepKey = stepKey.toLower();
    QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(mStepKey);
    if (i != _viewerSteps.end()) {
        _viewerSteps.erase(i);
        return true;
    }
    return false;
}

/* return viewer step is multiStep */

bool LDrawFile::isViewerStepMultiStep(const QString &stepKey)
{
  QString mStepKey = stepKey.toLower();
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(mStepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._multiStep;
  } else {
    return false;
  }
}

/* return viewer step is calledOut */

bool LDrawFile::isViewerStepCalledOut(const QString &stepKey)
{
  QString mStepKey = stepKey.toLower();
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(mStepKey);
  if (i != _viewerSteps.end()) {
    return i.value()._calledOut;
  } else {
    return false;
  }
}

/* Clear ViewerSteps */

void LDrawFile::clearViewerSteps()
{
  _viewerSteps.clear();
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
      argv << chopped.split(" ",QString::SkipEmptyParts);
    } else if (chopped[p] == '0') {

      /* Parse the input line into argv[] */

      int soq = validSoQ(chopped,chopped.indexOf("\""));
      if (soq == -1) {
          argv << chopped.split(" ",QString::SkipEmptyParts);
        } else {
          // quotes found
          while (chopped.size()) {
              soq = validSoQ(chopped,chopped.indexOf("\""));
              if (soq == -1) {
                  argv << chopped.split(" ",QString::SkipEmptyParts);
                  chopped.clear();
                } else {
                  QString left = chopped.left(soq);
                  left = left.trimmed();
                  argv << left.split(" ",QString::SkipEmptyParts);
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

QList<QRegExp> LDrawFile::_fileRegExp;
QList<QRegExp> LDrawHeaderRegExp;
QList<QRegExp> LDrawUnofficialPartRegExp;
QList<QRegExp> LDrawUnofficialSubPartRegExp;
QList<QRegExp> LDrawUnofficialPrimitiveRegExp;
QList<QRegExp> LDrawUnofficialOtherRegExp;

LDrawFile::LDrawFile()
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
        << QRegExp("^0\\s+[^\\s].*$",Qt::CaseInsensitive)           //DES_RX
        << QRegExp("^0\\s+!?LDCAD\\s+GROUP_DEF.*\\s+\\[LID=(\\d+)\\]\\s+\\[GID=([\\d\\w]+)\\]\\s+\\[name=(.[^\\]]+)\\].*$",Qt::CaseInsensitive) //LDG_RX
        ;
  }

  {
    LDrawHeaderRegExp
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
           ;
  }

  {
      LDrawUnofficialPartRegExp
              << QRegExp("^0\\s+!?UNOFFICIAL\\s+PART[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Part)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Part Alias)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Part)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Part Alias)[^\n]*",Qt::CaseInsensitive)
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
      LDrawUnofficialOtherRegExp
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Shortcut)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Shortcut Alias)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Part Physical_Colour)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial_Shortcut Physical_Colour)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Shortcut)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Shortcut Alias)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Part Physical_Colour)[^\n]*",Qt::CaseInsensitive)
              << QRegExp("^0\\s+!?(?:LDRAW_ORG)*\\s?(Unofficial Shortcut Physical_Colour)[^\n]*",Qt::CaseInsensitive)
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
  QRegExp commentLine("^\\s*0\\s+\\/\\/\\s*.*");
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
  size = LDrawUnofficialOtherRegExp.size();
  for (int i = 0; i < size; i++) {
    if (line.contains(LDrawUnofficialOtherRegExp[i])) {
      return UNOFFICIAL_OTHER;
    }
  }
  return UNOFFICIAL_SUBMODEL;
}

bool isGhost(QString &line){
  QRegExp ghostMeta("^\\s*0\\s+GHOST\\s+.*$");
  if (line.contains(ghostMeta))
      return true;
  return false;
}
