
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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

#include "paths.h"

#include "lpub.h"
#include "ldrawfilesload.h"
#include "lc_application.h"
#include "lc_library.h"
#include "project.h"
#include "pieceinf.h"

QStringList LDrawFile::_loadedParts;
QString LDrawFile::_file           = "";
QString LDrawFile::_name           = "";
QString LDrawFile::_author         = "";
QString LDrawFile::_description    = "";
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
  _prevStepPosition = 0;
  _startPageNumber = 0;
  _lineTypeIndexes.clear();
}

/* initialize viewer step*/
ViewerStep::ViewerStep(const QStringList &rotatedContents,
                       const QStringList &unrotatedContents,
                       const QString     &filePath,
                       const QString     &csiKey,
                       bool               multiStep,
                       bool               calledOut){
    _rotatedContents << rotatedContents;
    _unrotatedContents << unrotatedContents;
    _filePath  = filePath;
    _csiKey    = csiKey;
    _modified  = false;
    _multiStep = multiStep;
    _calledOut = calledOut;
}

void LDrawFile::empty()
{
  _subFiles.clear();
  _subFileOrder.clear();
  _viewerSteps.clear();
  _loadedParts.clear();
  _mpd = false;
  _partCount = 0;
}

/* Add a new subFile */

void LDrawFile::insert(const QString &mcFileName,
                      QStringList    &contents,
                      QDateTime      &datetime,
                      bool            unofficialPart,
                      bool            generated,
                      const QString  &subFilePath)
{
  QString    fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    _subFiles.erase(i);
  }
  LDrawSubFile subFile(contents,datetime,unofficialPart,generated,subFilePath);
  _subFiles.insert(fileName,subFile);
  _subFileOrder << fileName;
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
  foreach(key,_subFiles.keys()) {
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

QStringList LDrawFile::getSubFilePaths()
{
  QStringList subFiles;
  for (int i = 0; i < _subFileOrder.size(); i++) {
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(_subFileOrder[i]);
    if (f != _subFiles.end()) {
        if (!f.value()._subFilePath.isEmpty()) {
            subFiles << f.value()._subFilePath;
        }
    }
  }
  return subFiles;
}

void LDrawFile::setModelStartPageNumber(const QString     &mcFileName,
                 const int &startPageNumber)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._modified = true;
    //i.value()._datetime = QDateTime::currentDateTime();
    i.value()._startPageNumber = startPageNumber;
    //i.value()._changedSinceLastWrite = true; // remarked on build 491 28/12/2015
  }
}

/* return the last fade position value */

int LDrawFile::getPrevStepPosition(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._prevStepPosition;
  }
  return 0;
}

void LDrawFile::setPrevStepPosition(const QString     &mcFileName,
                 const int &prevStepPosition)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._modified = true;
    //i.value()._datetime = QDateTime::currentDateTime();
    i.value()._prevStepPosition = prevStepPosition;
    //i.value()._changedSinceLastWrite = true;  // remarked on build 491 28/12/2015
  }
}

/* set all previous step positions to 0 */

void LDrawFile::clearPrevStepPositions()
{
  QString key;
  foreach(key,_subFiles.keys()) {
    _subFiles[key]._prevStepPosition = 0;
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
  foreach (fileName, parsedStack) {
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

QString LDrawFile::getSubmodelName(int index)
{
    if (index > -1 && index < _subFileOrder.size()){
        QString subFileName = _subFileOrder[index];
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
    QVector<int> *lineTypeIndexes = new QVector<int>;
    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(getSubmodelName(submodelIndx).toLower());
    if (f != _subFiles.end() && f.value()._lineTypeIndexes.size()) {
        lineTypeIndexes = &f.value()._lineTypeIndexes;
    }
    return lineTypeIndexes;
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

void LDrawFile::unrendered()
{
  QString key;
  foreach(key,_subFiles.keys()) {
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
        int            howCounted)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
      QString key =
        howCounted == CountAtStep ?
          QString("%1 %2").arg(renderParentModel).arg(renderStepNumber) :
        howCounted > CountFalse && howCounted < CountAtStep ?
          renderParentModel : QString();
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

bool LDrawFile::rendered(
        const QString &mcFileName,
        bool           mirrored,
        const QString &renderParentModel,
        int            renderStepNumber,
        int            howCounted)
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
    if (mirrored) {
      haveKey = key.isEmpty() ? howCounted == CountAtTop ? true : false :
                  i.value()._mirrorRenderedKeys.contains(key);
      rendered  = i.value()._mirrorRendered;
    } else {
      haveKey = key.isEmpty() ? howCounted == CountAtTop ? true : false :
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
        emit gui->messageSig(LOG_ERROR, QString("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return 1;
    }
    QByteArray qba(file.readAll());
    file.close();

    QTime t; t.start();

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

    QRegExp sof("^0\\s+FILE\\s+(.*)$",Qt::CaseInsensitive);
    QRegExp part("^\\s*1\\s+.*$",Qt::CaseInsensitive);

    QFileInfo fileInfo(fileName);

    while ( ! in.atEnd()) {
        QString line = in.readLine(0);
        if (line.contains(sof)) {
            emit gui->messageSig(LOG_INFO_STATUS, QString("Model file %1 identified as Multi-Part LDraw System (MPD) Document").arg(fileInfo.fileName()));
            mpd = true;
            break;
        }
        if (line.contains(part)) {
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

    countParts(topLevelFile());

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

    QString statusMessage = QString("%1 model file %2 loaded. "
                                    "Part Count %3. %4")
            .arg(mpd ? "MPD" : "LDR")
            .arg(fileInfo.fileName())
            .arg(apc)
            .arg(gui->elapsedTime(t.elapsed()));

    emit gui->messageSig(LOG_INFO_STATUS, QString("%1").arg(statusMessage));

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
        emit gui->messageSig(LOG_ERROR, QString("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QFileInfo   fileInfo(fileName);
    QTextStream in(&file);
    in.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));

    QStringList stageContents;
    QStringList stageSubfiles;

    /* Read it in the first time to put into fileList in order of
     appearance */

    while ( ! in.atEnd()) {
        QString sLine = in.readLine(0);
        stageContents << sLine.trimmed();
    }
    file.close();

    topLevelFileNotCaptured        = true;
    topLevelNameNotCaptured        = true;
    topLevelAuthorNotCaptured      = true;
    topLevelDescriptionNotCaptured = true;
    topLevelCategoryNotCaptured    = true;
    unofficialPart                 = false;
    topLevelModel                  = true;
    descriptionLine                = 0;

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

    std::function<QString()> modelType;
    modelType = [this] ()
    {
        return topLevelModel ? "model" : "submodel";
    };

    std::function<void(int)> loadMPDContents;
    loadMPDContents = [
            this,
            &file,
            &modelType,
            &loadMPDContents,
            &stageContents,
            &stageSubfiles,
            &fileInfo,
            &searchPaths,
            &datetime] (int i) {
        bool alreadyLoaded;
        QStringList contents;
        QString     subfileName;
        QRegExp sofRE("^0\\s+FILE\\s+(.*)$",Qt::CaseInsensitive);
        QRegExp eofRE("^0\\s+NOFILE\\s*$",Qt::CaseInsensitive);

        QRegExp upAUT("^0\\s+Author:?\\s+(.*)$",Qt::CaseInsensitive);
        QRegExp upNAM("^0\\s+Name:?\\s+(.*)$",Qt::CaseInsensitive);
        QRegExp upCAT("^0\\s+!?CATEGORY\\s+(.*)$",Qt::CaseInsensitive);

        QRegExp ldcGRP( "^\\s*0\\s+!?LDCAD\\s+GROUP_DEF.*\\s+\\[LID=(\\d+)\\]\\s+\\[GID=([\\d\\w]+)\\]\\s+\\[name=(.[^\\]]+)\\].*$");

        emit gui->progressBarPermInitSig();
        emit gui->progressPermRangeSig(1, stageContents.size());
        emit gui->progressPermMessageSig("Processing " + modelType() + " file " + fileInfo.fileName() + "...");
        emit gui->messageSig(LOG_INFO, "Loading MPD " + modelType() + " file " + fileInfo.fileName() + "...");
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Stage Contents Size: %1, Start Index %2")
                             .arg(stageContents.size()).arg(i));
#endif
        for (; i < stageContents.size(); i++) {

            QString smLine = stageContents.at(i);

            emit gui->progressPermSetValueSig(i);

            bool sof = smLine.contains(sofRE);  //start of file
            bool eof = smLine.contains(eofRE);  //end of file

            // load LDCad groups
            if (!ldcadGroupsLoaded && smLine.contains(ldcGRP)){
               insertLDCadGroup(ldcGRP.cap(3),ldcGRP.cap(1).toInt());
               insertLDCadGroup(ldcGRP.cap(2),ldcGRP.cap(1).toInt());
            } else if (smLine.contains("0 STEP")) {
               ldcadGroupsLoaded = true;
            }

            QStringList tokens;
            split(smLine,tokens);

            // subfile check;
            QString stageSubfileName;
            bool subFileFound = false;
            if ((subFileFound = tokens.size() == 15 && tokens.at(0) == "1")) {
                stageSubfileName = tokens.at(14).toLower();
            } else if (isSubstitute(smLine,stageSubfileName)) {
                subFileFound = !stageSubfileName.isEmpty();
            }
            if (subFileFound) {
                PieceInfo* standardPart = lcGetPiecesLibrary()->FindPiece(stageSubfileName.toLatin1().constData(), nullptr, false, false);
                if (! standardPart && ! LDrawFile::contains(stageSubfileName.toLower()) && ! stageSubfiles.contains(stageSubfileName)) {
                    stageSubfiles.append(stageSubfileName);
                }
            }

            if (topLevelFileNotCaptured) {
                if (sof){
                    _file = sofRE.cap(1).replace(QFileInfo(sofRE.cap(1)).suffix(),"");
                    descriptionLine = i+1;      //next line should be description
                    topLevelFileNotCaptured = false;
                }
            }

            if (topLevelAuthorNotCaptured) {
                if (smLine.contains(upAUT)) {
                    _author = upAUT.cap(1).replace(": ","");
                    Preferences::defaultAuthor = _author;
                    topLevelAuthorNotCaptured = false;
                }
            }

            if (topLevelNameNotCaptured) {
                if (smLine.contains(upNAM)) {
                    _name = upNAM.cap(1).replace(": ","");
                    topLevelNameNotCaptured = false;
                }
            }

            if (topLevelCategoryNotCaptured && subfileName == topLevelFile()) {
                if (smLine.contains(upCAT)) {
                        _category = upCAT.cap(1);
                    topLevelCategoryNotCaptured = false;
                }
            }

            if (topLevelDescriptionNotCaptured && i == descriptionLine && ! isHeader(smLine)) {
                _description = smLine;
                Preferences::publishDescription = _description;
                topLevelDescriptionNotCaptured = false;
            }

            if ((alreadyLoaded = LDrawFile::contains(subfileName.toLower()))) {
                emit gui->messageSig(LOG_TRACE, QString("MPD " + modelType() + " '" + subfileName + "' already loaded."));
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
                        emit gui->messageSig(LOG_TRACE, QString("MPD " + modelType() + " '" + subfileName + "' with " +
                                                                QString::number(contents.size()) + " lines loaded."));
                        insert(subfileName,contents,datetime,unofficialPart);
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
                    subfileName = sofRE.cap(1).toLower();
                    if (! alreadyLoaded)
                        emit gui->messageSig(LOG_INFO, "Loading MPD " + modelType() + " '" + subfileName + "'...");
                } else {
                    subfileName.clear();
                }

            } else if ( ! subfileName.isEmpty() && smLine != "") {
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

        if ( ! subfileName.isEmpty() && ! contents.isEmpty()) {
            if (LDrawFile::contains(subfileName.toLower())) {
                emit gui->messageSig(LOG_TRACE, QString("MPD submodel '" + subfileName + "' already loaded."));
            } else {
                emit gui->messageSig(LOG_TRACE, QString("MPD submodel '" + subfileName + "' with " +
                                                        QString::number(contents.size()) + " lines loaded."));
                insert(subfileName,contents,datetime,unofficialPart);
            }
            stageSubfiles.removeAt(stageSubfiles.indexOf(subfileName));
        }

        // resolve outstanding subfiles
        if (stageSubfiles.size()){
            stageSubfiles.removeDuplicates();
            // Debug
            emit gui->messageSig(LOG_DEBUG, QString("%1 unresolved stage %2 specified.")
                                 .arg(stageSubfiles.size()).arg(stageSubfiles.size() == 1 ? "subfile" : "subfiles"));

            QString projectPath = QDir::toNativeSeparators(fileInfo.absolutePath());

            bool subFileFound =false;
            int i = stageContents.size();
            for (QString subfile : stageSubfiles) {
                // Debug
                emit gui->messageSig(LOG_DEBUG, QString("Processing stage subfile %1...").arg(subfile));

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
                        emit gui->messageSig(LOG_NOTICE, QString("Cannot read file %1:\n%2.")
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
                loadMPDContents(i);
            }
        } else {
            // Debug
            emit gui->messageSig(LOG_DEBUG, QString("No stage subfiles specified."));
        }
    };

    loadMPDContents(0);

#ifdef QT_DEBUG_MODE
    QHashIterator<QString, int> i(_ldcadGroups);
    while (i.hasNext()) {
        i.next();
        emit gui->messageSig(LOG_TRACE, QString("LDCad Groups: Name[%1], LineID[%2].")
                             .arg(i.key()).arg(i.value()));
    }
#endif

    _mpd = true;

    emit gui->progressPermSetValueSig(stageContents.size());
    emit gui->progressPermStatusRemoveSig();
}

void LDrawFile::loadLDRFile(const QString &path, const QString &fileName)
{
    if (_subFiles[fileName]._contents.isEmpty()) {

        QString fullName(path + QDir::separator() + fileName);

        QFile file(fullName);
        if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
            emit gui->messageSig(LOG_ERROR,QString("Cannot read file %1:\n%2.")
                                 .arg(fullName)
                                 .arg(file.errorString()));
            return;
        }

        QFileInfo   fileInfo(fullName);
        QTextStream in(&file);
        in.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));

        QStringList contents;
        QStringList subfiles;

        unofficialPart                 = false;

        QString modelType = topLevelModel ? "model" : "submodel";

        /* Read it in the first time to put into fileList in order of
         appearance */

        QRegExp mpdMeta("^0\\s+FILE\\s+(.*)$",Qt::CaseInsensitive);
        while ( ! in.atEnd()) {
            QString line = in.readLine(0);
            QString scModelType = modelType[0].toUpper() + modelType.right(modelType.size() - 1);
            if (line.contains(mpdMeta)) {
                file.close();
                emit gui->messageSig(LOG_INFO_STATUS, QString(scModelType + " file %1 identified as Multi-Part LDraw System (MPD) Document").arg(fileInfo.fileName()));
                QDateTime datetime = fileInfo.lastModified();
                loadMPDFile(fileInfo.absoluteFilePath(),datetime);
                return;
            } else {
                contents << line.trimmed();
                if (isHeader(line) && ! unofficialPart) {
                    unofficialPart = getUnofficialFileType(line);
                }
            }
        }
        file.close();

        if (topLevelModel) {
            topLevelDescriptionNotCaptured = true;
            topLevelNameNotCaptured        = true;
            topLevelAuthorNotCaptured      = true;
            topLevelCategoryNotCaptured    = true;
            descriptionLine                = 0;
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

        QRegExp upAUT("^0\\s+Author:?\\s+(.*)$",Qt::CaseInsensitive);
        QRegExp upNAM("^0\\s+Name:?\\s+(.*)$",Qt::CaseInsensitive);
        QRegExp upCAT("^0\\s+!?CATEGORY\\s+(.*)$",Qt::CaseInsensitive);

        QRegExp ldcGRP( "^\\s*0\\s+!?LDCAD\\s+GROUP_DEF.*\\s+\\[LID=(\\d+)\\]\\s+\\[GID=([\\d\\w]+)\\]\\s+\\[name=(.[^\\]]+)\\].*$");

        emit gui->progressBarPermInitSig();
        emit gui->progressPermRangeSig(1, contents.size());
        emit gui->progressPermMessageSig("Processing model file...");

        emit gui->messageSig(LOG_INFO, "Loading LDR " + modelType + " file '" + fileInfo.fileName() + "'...");

        QDateTime datetime = fileInfo.lastModified();

        insert(fileInfo.fileName(),contents,datetime,unofficialPart,false,fileInfo.absoluteFilePath());

        /* read it a second time to find submodels */

        for (int i = 0; i < contents.size(); i++) {

            QString line = contents.at(i);

            emit gui->progressPermSetValueSig(i);

            // load LDCad groups
            if (!ldcadGroupsLoaded && line.contains(ldcGRP)){
               insertLDCadGroup(ldcGRP.cap(3),ldcGRP.cap(1).toInt());
               insertLDCadGroup(ldcGRP.cap(2),ldcGRP.cap(1).toInt());
            } else if (line.contains("0 STEP")) {
               ldcadGroupsLoaded = true;
            }

            QStringList tokens;
            split(line,tokens);

            if (topLevelModel) {

                if (topLevelDescriptionNotCaptured && i == descriptionLine && ! isHeader(line)) {
                    _description = line;
                    topLevelDescriptionNotCaptured = false;
                }

                if (topLevelNameNotCaptured) {
                    if (line.contains(upNAM)) {
                        _name = upNAM.cap(1).replace(": ","");
                        topLevelNameNotCaptured = false;
                    }
                }

                if (topLevelAuthorNotCaptured) {
                    if (line.contains(upAUT)) {
                        _author = upAUT.cap(1).replace(": ","");
                        topLevelAuthorNotCaptured = false;
                    }
                }

                if (topLevelCategoryNotCaptured) {
                    if (line.contains(upCAT)) {
                        _category = upCAT.cap(1);
                        topLevelCategoryNotCaptured = false;
                    }
                }
            }

            // subfile check;
            QString subfileName;
            bool subFileFound = false;
            if ((subFileFound = tokens.size() == 15 && tokens.at(0) == "1")) {
                subfileName = tokens.at(14);
            } else if (isSubstitute(line,subfileName)) {
                subFileFound = !subfileName.isEmpty();
            }

            // resolve outstanding subfiles
            if (subFileFound) {
                QFileInfo subFileInfo = QFileInfo(subfileName);
                PieceInfo* standardPart = lcGetPiecesLibrary()->FindPiece(subFileInfo.fileName().toLatin1().constData(), nullptr, false, false);
                if (! standardPart && ! LDrawFile::contains(subFileInfo.fileName())) {
                    subFileFound = false;
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
                        topLevelModel = false;
                        loadLDRFile(subFileInfo.absolutePath(),subFileInfo.fileName());
                    } else {
                        emit gui->messageSig(LOG_NOTICE, QString("Subfile %1 not found.")
                                             .arg(subFileInfo.fileName()));
                    }
                }
            }
        }

        _mpd = false;

        emit gui->progressPermSetValueSig(contents.size());
        emit gui->progressPermStatusRemoveSig();
        emit gui->messageSig(LOG_TRACE, QString("LDR " + modelType + " file '" + fileInfo.fileName() + "' with " +
                                                 QString::number(contents.size()) + " lines loaded."));
    }
}

bool LDrawFile::saveFile(const QString &fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool rc;
    if (_mpd) {
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
         (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
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
                      (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
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

void LDrawFile::countInstances(const QString &mcFileName, bool isMirrored, bool callout)
{
  //logTrace() << QString("countInstances, File: %1, Mirrored: %2, Callout: %3").arg(mcFileName,(isMirrored?"Yes":"No"),(callout?"Yes":"No"));

  QString fileName = mcFileName.toLower();
  bool partsAdded = false;
  bool noStep = false;
  bool stepIgnore = false;
  
  QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
  if (f != _subFiles.end()) {
    // count mirrored instance automatically
    if (f->_beenCounted) {
      if (isMirrored) {
        ++f->_mirrorInstances;
      } else {
        ++f->_instances;
      }
      return;
    }
    // get content size and reset numSteps
    int j = f->_contents.size();
    f->_numSteps = 0;

    // process submodel content...
    for (int i = 0; i < j; i++) {
      QStringList tokens;
      QString line = f->_contents[i];
      split(line,tokens);
      
      /* Sorry, but models that are callouts are not counted as instances */
          // called out
      if (tokens.size() == 4 && 
          tokens[0] == "0" && 
          (tokens[1] == "LPUB" || tokens[1] == "!LPUB") && 
          tokens[2] == "CALLOUT" && 
          tokens[3] == "BEGIN") {
        partsAdded = true;
           //process callout content
        for (++i; i < j; i++) {
          split(f->_contents[i],tokens);
          if (tokens.size() == 15 && tokens[0] == "1") {
            if (contains(tokens[14]) && ! stepIgnore) {
              countInstances(tokens[14],mirrored(tokens),true);
            }
          } else if (tokens.size() == 4 &&
              tokens[0] == "0" && 
              (tokens[1] == "LPUB" || tokens[1] == "!LPUB") && 
              tokens[2] == "CALLOUT" && 
              tokens[3] == "END") {
            
            break;
          }
        }
        //lpub3d ignore part - so set ignore step
      } else if (tokens.size() == 5 &&
                 tokens[0] == "0" &&
                 (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
                 (tokens[2] == "PART" || tokens[2] == "PLI") &&
                 tokens[3] == "BEGIN"  &&
                 tokens[4] == "IGN") {
        stepIgnore = true;
        // lpub3d part - so set include step
      } else if (tokens.size() == 4 &&
                 tokens[0] == "0" &&
                 (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
                 (tokens[2] == "PART" || tokens[2] == "PLI") &&
                 tokens[3] == "END") {
        stepIgnore = false;
        // no step
      } else if (tokens.size() == 3 && tokens[0] == "0" &&
                (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
                 tokens[2] == "NOSTEP") {
        noStep = true;
        // LDraw step or rotstep - so check if parts added
      } else if (tokens.size() >= 2 && tokens[0] == "0" &&
                (tokens[1] == "STEP" || tokens[1] == "ROTSTEP")) {
        // parts added - increment step
        if (partsAdded && ! noStep) {
          int incr = (isMirrored && f->_mirrorInstances == 0) ||
                     (!isMirrored && f->_instances == 0);
          f->_numSteps += incr;
        }
        // reset partsAdded
        partsAdded = false;
        noStep = false;
        // buffer exchange - do nothing
      } else if (tokens.size() == 4 && tokens[0] == "0"
                                     && tokens[1] == "BUFEXCHG") {
        // check if subfile and process...
      } else if (tokens.size() == 15 && tokens[0] == "1") {
        bool containsSubFile = contains(tokens[14]);
        if (containsSubFile && ! stepIgnore) {
          countInstances(tokens[14],mirrored(tokens),false);
        }
        partsAdded = true;
      }
    }
    //add step if parts added
    f->_numSteps += partsAdded && ! noStep &&
                       ( (isMirrored && f->_mirrorInstances == 0) ||
                       (!isMirrored && f->_instances == 0) );
    //
    if ( ! callout) {
      if (isMirrored) {
        ++f->_mirrorInstances;
      } else {
        ++f->_instances;
      }
    }

  } // file end
  f->_beenCounted = true;
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
  countInstances(topLevelFile(),false);
}

bool LDrawFile::saveMPDFile(const QString &fileName)
{
    QString writeFileName = fileName;
    QFile file(writeFileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        emit gui->messageSig(LOG_ERROR,QString("Cannot write file %1:\n%2.")
                             .arg(writeFileName)
                             .arg(file.errorString()));
        return false;
    }

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
                emit gui->messageSig(LOG_ERROR,QString("Cannot write file %1:\n%2.")
                                    .arg(writeFileName)
                                    .arg(file.errorString()));
                return false;
            }

            QTextStream out(&file);
            out.setCodec(_currFileIsUTF8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForName("System"));
        }
        out << "0 FILE " << subFileName << endl;
        for (int j = 0; j < f.value()._contents.size(); j++) {
          out << f.value()._contents[j] << endl;
        }
        out << "0 NOFILE " << endl;
      }
    }
    return true;
}

void LDrawFile::countParts(const QString &fileName){

    //logDebug() << QString("  Subfile: %1, Subfile Parts Count: %2").arg(fileName).arg(count);
    emit gui->messageSig(LOG_STATUS, QString("Processing subfile '%1'").arg(fileName));

    int sfCount = 0;

    QRegExp validExtRx("\\.DAT|\\.LDR|\\.MPD$",Qt::CaseInsensitive);

    QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName.toLower());
    if (f != _subFiles.end()) {
        // get content size and reset numSteps
        int j = f->_contents.size();

        // process submodel content...
        for (int i = 0; i < j; i++) {
            QStringList tokens;
            QString line = f->_contents[i];

            split(line,tokens);

            bool doCountPart = true;

// interrogate each line
//          if (tokens[0] != "1") {
//              logNotice() << QString("     Line: [%1] %2").arg(fileName).arg(line);
//            }

            if (tokens.size() == 5 &&
                tokens[0] == "0" &&
               (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
               (tokens[2] == "PART" || tokens[2] == "PLI") &&
                tokens[3] == "BEGIN"  &&
                tokens[4] == "IGN") {
                doCountPart = false;
            } else
            if (tokens.size() == 4 &&
                tokens[0] == "0" &&
               (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
               (tokens[2] == "PART" || tokens[2] == "PLI") &&
                tokens[3] == "END") {
               doCountPart = true;
            }

            QString partToken;
            if (doCountPart && tokens.size() == 15 && tokens[0] == "1" && (tokens[14].contains(validExtRx))) {
                partToken = tokens[14];
            } else if ((doCountPart = isSubstitute(line,partToken))) {
                doCountPart = !partToken.isEmpty() && partToken.contains(validExtRx);
            }
            if (doCountPart) {
                QString partString = "|" + partToken + "|";
                bool containsSubFile = contains(partToken.toLower());
                if (containsSubFile) {
                    int subFileType = isUnofficialPart(partToken.toLower());
                    if (subFileType == UNOFFICIAL_SUBMODEL){
                        countParts(partToken);
                    } else {
                        switch(subFileType){
                        case  UNOFFICIAL_PART:
                            _partCount++;sfCount++;
                            partString += QString("Unofficial inlined part");
                            if (!_loadedParts.contains(QString(VALID_LOAD_MSG) + partString)) {
                                _loadedParts.append(QString(VALID_LOAD_MSG) + partString);
                                emit gui->messageSig(LOG_NOTICE,QString("Unofficial inlined part %1 [%2] validated.").arg(_partCount).arg(partToken));
                            }
                            break;
                        case  UNOFFICIAL_SUBPART:
                            partString += QString("Unofficial inlined subpart");
                            if (!_loadedParts.contains(QString(SUBPART_LOAD_MSG) + partString)) {
                                _loadedParts.append(QString(SUBPART_LOAD_MSG) + partString);
                                emit gui->messageSig(LOG_NOTICE,QString("Unofficial inlined part [%1] is a subpart").arg(partToken));
                            }
                            break;
                        case  UNOFFICIAL_PRIMITIVE:
                            partString += QString("Unofficial inlined primitive");
                            if (!_loadedParts.contains(QString(PRIMITIVE_LOAD_MSG) + partString)) {
                                _loadedParts.append(QString(PRIMITIVE_LOAD_MSG) + partString);
                                emit gui->messageSig(LOG_NOTICE,QString("Unofficial inlined part [%1] is a primitive").arg(partToken));
                            }
                            break;
                        default:
                            break;
                        }
                    }
                } else if (! ExcludedParts::hasExcludedPart(partToken)) {
                    QString partFile = partToken.toUpper();
                    if (partFile.startsWith("S\\")) {
                        partFile.replace("S\\","S/");
                    }
                    PieceInfo* pieceInfo;
                    if (!_loadedParts.contains(QString(VALID_LOAD_MSG) + partString)) {
                        pieceInfo = lcGetPiecesLibrary()->FindPiece(partFile.toLatin1().constData(), nullptr, false, false);
                        if (pieceInfo) {
                            partString += pieceInfo->m_strDescription;
                            if (pieceInfo->IsSubPiece()) {
                                if (!_loadedParts.contains(QString(SUBPART_LOAD_MSG) + partString)){
                                    _loadedParts.append(QString(SUBPART_LOAD_MSG) + partString);
                                    emit gui->messageSig(LOG_NOTICE,QString("Part [%1] is a subpart").arg(partToken));
                                }
                            } else
                            if (pieceInfo->IsPartType()) {
                                _partCount++;sfCount++;
                                if (!_loadedParts.contains(QString(VALID_LOAD_MSG) + partString)) {
                                    _loadedParts.append(QString(VALID_LOAD_MSG) + partString);
                                    emit gui->messageSig(LOG_NOTICE,QString("Part %1 [%2] validated.").arg(_partCount).arg(partToken));
                                }
                            } else
                            if (lcGetPiecesLibrary()->IsPrimitive(partFile.toLatin1().constData())){
                                if (pieceInfo->IsSubPiece()) {
                                    if (!_loadedParts.contains(QString(SUBPART_LOAD_MSG) + partString)) {
                                        _loadedParts.append(QString(SUBPART_LOAD_MSG) + partString);
                                        emit gui->messageSig(LOG_NOTICE,QString("Part [%1] is a subpart").arg(partToken));
                                    }
                                } else {
                                    if (!_loadedParts.contains(QString(PRIMITIVE_LOAD_MSG) + partString)) {
                                        _loadedParts.append(QString(PRIMITIVE_LOAD_MSG) + partString);
                                        emit gui->messageSig(LOG_NOTICE,QString("Part [%1] is a primitive part").arg(partToken));
                                    }
                                }
                            }
                        } else {
                            partString += QString("Part not found");
                            if (!_loadedParts.contains(QString(MISSING_LOAD_MSG) + partString)) {
                                _loadedParts.append(QString(MISSING_LOAD_MSG) + partString);
                                emit gui->messageSig(LOG_NOTICE,QString("Part [%1] not excluded, not a submodel and not found in the %2 library archives.")
                                                     .arg(partToken)
                                        .arg(VER_PRODUCTNAME_STR));
                            }
                        }
                    }
                }  // check excluded
            }
        }
    }
}

bool LDrawFile::saveLDRFile(const QString &fileName)
{
    QString path = QFileInfo(fileName).path();
    QFile file;

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
            emit gui->messageSig(LOG_ERROR,QString("Cannot write file %1:\n%2.")
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
  foreach(key,_subFiles.keys()) {
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
  foreach(QString lid, lids){
    if (values.contains(lid.toInt()))
      return true;
  }
  return false;
}

/* Add a new Viewer Step */

void LDrawFile::insertViewerStep(const QString     &stepKey,
                                 const QStringList &rotatedContents,
                                 const QStringList &unrotatedContents,
                                 const QString     &filePath,
                                 const QString     &csiKey,
                                 bool               multiStep,
                                 bool               calledOut)
{
  QString    mStepKey = stepKey.toLower();
  QMap<QString, ViewerStep>::iterator i = _viewerSteps.find(mStepKey);

  if (i != _viewerSteps.end()) {
    _viewerSteps.erase(i);
  }
  ViewerStep viewerStep(rotatedContents,unrotatedContents,filePath,csiKey,multiStep,calledOut);
  _viewerSteps.insert(mStepKey,viewerStep);
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

QList<QRegExp> LDrawHeaderRegExp;
QList<QRegExp> LDrawUnofficialPartRegExp;
QList<QRegExp> LDrawUnofficialSubPartRegExp;
QList<QRegExp> LDrawUnofficialPrimitiveRegExp;
QList<QRegExp> LDrawUnofficialOtherRegExp;

LDrawFile::LDrawFile()
{
    _loadedParts.clear();
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

bool isSubstitute(QString &line, QString &lineOut){
  QRegExp substitutePart("BEGIN\\sSUB\\s([A-Za-z0-9\\s_-]+.[dat|mpd|ldr]+)",Qt::CaseInsensitive);
  if (line.contains(substitutePart)) {
      lineOut = substitutePart.cap(1);
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
