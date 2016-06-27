 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

#include <QFile>
#include <QList>
#include <QRegExp>
#include "paths.h"

#include "lc_application.h"
#include "lc_library.h"
#include "pieceinf.h"

#include "lpub.h"

QString LDrawFile::_file        = "";
QString LDrawFile::_name        = "";
QString LDrawFile::_author      = "";
QString LDrawFile::_description = "";
QString LDrawFile::_category    = "";
int     LDrawFile::_emptyInt;
int     LDrawFile::_pieces      = 0;

LDrawSubFile::LDrawSubFile(
  const QStringList &contents,
  QDateTime   &datetime,
  bool         unofficialPart,
  bool         generated)
{
  _contents << contents;
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
  _fadePosition = 0;
  _startPageNumber = 0;
}

void LDrawFile::empty()
{
  _subFiles.clear();
  _subFileOrder.clear();
  _mpd = false;
}

/* Add a new subFile */

void LDrawFile::insert(
                      const QString &mcFileName,
                      QStringList    &contents,
                      QDateTime      &datetime,
                      bool            unofficialPart,
                      bool            generated)
{
  QString    fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    _subFiles.erase(i);
  }
  LDrawSubFile subFile(contents,datetime,unofficialPart,generated);
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
bool LDrawFile::isUnofficialPart(const QString &name)
{
  QString fileName = name.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    bool _unofficialPart = i.value()._unofficialPart;
    return _unofficialPart;
  }
  return false;
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

/* return the last fade position value */

int LDrawFile::getFadePosition(const QString &mcFileName)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
  if (i != _subFiles.end()) {
    return i.value()._fadePosition;
  }
  return 0;
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

void LDrawFile::setFadePosition(const QString     &mcFileName,
                 const int &fadePosition)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._modified = true;
    //i.value()._datetime = QDateTime::currentDateTime();
    i.value()._fadePosition = fadePosition;
    //i.value()._changedSinceLastWrite = true;  // remarked on build 491 28/12/2015
  }
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

bool LDrawFile::older(const QStringList &submodelStack, 
           const QDateTime &datetime)
{
  QString fileName;
  foreach (fileName, submodelStack) {
    QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
    if (i != _subFiles.end()) {
      QDateTime fileDatetime = i.value()._datetime;
      if (fileDatetime > datetime) {
        return false;
      }
    }
  }
  return true;
}

QStringList LDrawFile::subFileOrder() {
  return _subFileOrder;
}

QString LDrawFile::readLine(const QString &mcFileName, int lineNumber)
{                   ;
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    return i.value()._contents[lineNumber];
  }
  QString empty;
  return empty;
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
  }
}

void LDrawFile::setRendered(const QString &mcFileName, bool mirrored)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    if (mirrored) {
      i.value()._mirrorRendered = true;
    } else {
      i.value()._rendered = true;
    }
  }
}

bool LDrawFile::rendered(const QString &mcFileName, bool mirrored)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    if (mirrored) {
      return i.value()._mirrorRendered;
    } else {
      return i.value()._rendered;
    }
  }
  return false;
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

void LDrawFile::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(NULL, 
                             QMessageBox::tr(VER_PRODUCTNAME_STR),
                             QMessageBox::tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    // get rid of what's there before we load up new stuff

    empty();
    
    // allow files ldr suffix to allow for MPD
    
    bool mpd = false;

    QTextStream in(&file);

    QRegExp sof("^\\s*0\\s+FILE\\s+(.*)$");
    QRegExp part("^\\s*1\\s+.*$");

    while ( ! in.atEnd()) {
      const QString line = in.readLine(0);
      if (line.contains(sof)) {
        mpd = true;
        break;
      }
      if (line.contains(part)) {
        mpd = false;
        break;
      }
    }
    
    file.close();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    if (mpd) {
      QDateTime datetime = QFileInfo(fileName).lastModified();
      loadMPDFile(fileName,datetime);
    } else {
      QFileInfo fileInfo(fileName);
      loadLDRFile(fileInfo.absolutePath(),fileInfo.fileName());
    }
    
    QApplication::restoreOverrideCursor();
}

void LDrawFile::loadMPDFile(const QString &fileName, QDateTime &datetime)
{    
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(NULL, 
                             QMessageBox::tr(VER_PRODUCTNAME_STR),
                             QMessageBox::tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QStringList stageContents;
    QStringList contents;
    QString     mpdName;
    QRegExp sofRE("^\\s*0\\s+FILE\\s+(.*)$");
    QRegExp eofRE("^\\s*0\\s+NOFILE\\s*$");

    QRegExp upAUT("^\\s*0\\s+AUTHOR(.*)|Author(.*)|author(.*)$");
    QRegExp upNAM("^\\s*0\\s+Name(.*)|name(.*)|NAME(.*)$");
    QRegExp upCAT("^\\s*0\\s+!CATEGORY(.*)|!Category(.*)|!category(.*)$");
    QRegExp upDAT("\\.dat|\\.DAT$");

    bool topLevelFileNotCaptured        = true;
    bool topLevelNameNotCaptured        = true;
    bool topLevelAuthorNotCaptured      = true;
    bool topLevelDescriptionNotCaptured = true;
    bool topLevelCategoryNotCaptured    = true;
    bool unofficialPart                 = false;
    int  descriptionLine                = 0;
    int  pieces                         = 0;

    /* Read content into temperory content file the first time to put into fileList in order of
       appearance and stage for later processing */
    while ( ! in.atEnd()) {
      QString sLine = in.readLine(0);
      stageContents << sLine.trimmed();
    }

    emit gui->progressBarPermInitSig();
    emit gui->progressPermRangeSig(1, stageContents.size());
    emit gui->progressPermMessageSig("Processing...");
    emit gui->messageSig(true, "Loading mpd model file.");

    for (int i = 0; i < stageContents.size(); i++) {

        QString smLine = stageContents.at(i);

        emit gui->progressPermSetValueSig(i);

        bool sof = smLine.contains(sofRE);  //start of file
        bool eof = smLine.contains(eofRE);  //end of file

        QStringList tokens;        
        split(smLine,tokens);

        if (tokens.size() == 15 && tokens[0] == "1" && tokens[14].contains(upDAT)){
            QFileInfo info(tokens[14]);
            PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(info.baseName().toUpper().toLatin1().constData(), NULL, false);
            if (pieceInfo && pieceInfo->IsPartType())
              pieces++;
          }

        if (topLevelFileNotCaptured) {
            if (sof){
                _file = sofRE.cap(1).replace(".ldr","");
                descriptionLine = i+1;      //next line will be description
                topLevelFileNotCaptured = false;
            }
        }

        if (topLevelAuthorNotCaptured) {
           if (smLine.contains(upAUT)) {
               _author = upAUT.cap(1).replace(": ","");
               topLevelAuthorNotCaptured = false;
            }
        }

        if (topLevelNameNotCaptured) {
            if (smLine.contains(upNAM)) {
                _name = upNAM.cap(1).replace(": ","");
                topLevelNameNotCaptured = false;
            }
        }

        if (topLevelCategoryNotCaptured) {
            if (smLine.contains(upCAT)) {
                _category = upCAT.cap(1);
                topLevelCategoryNotCaptured = false;
            }
        }

        if (topLevelDescriptionNotCaptured && i == descriptionLine) {
            _description = smLine;
            topLevelDescriptionNotCaptured = false;
        }

        bool alreadyInserted = LDrawFile::contains(mpdName.toLower());

        /* - if at start of file marker, populate mpdName
         * - if at end of file marker, clear mpdName
         */
        if (sof || eof) {
            /* - if at end of file marker
             * - insert items if mpdName not empty
             * - as mpdName not empty, unofficial part = false
             * - after insert, clear contents item
             */
            if (! mpdName.isEmpty() && ! alreadyInserted) {
//                logTrace() << "Inserted Subfile: " << mpdName;
                insert(mpdName,contents,datetime,unofficialPart);
                unofficialPart = false;
            }
            contents.clear();

            /* - if at start of file marker
             * - set mpdName of new file
             * - else if at end of file marker, clear mpdName
             */
            if (sof) {
                mpdName = sofRE.cap(1).toLower();
                emit gui->messageSig(true, "Loading submodel " + mpdName);
            } else {
                mpdName.clear();
            }

        } else if ( ! mpdName.isEmpty() && smLine != "") {
            /* - after start of file - mpdName not empty
             * - if line contains unofficial tag set unofficial part = true
             * - add line to contents
             */
            if (isUnofficialFileType(smLine)) {
                unofficialPart = true;
            }

            contents << smLine;
        }
    }

    if ( ! mpdName.isEmpty() && ! contents.isEmpty()) {
      insert(mpdName,contents,datetime,unofficialPart);
    }

    _mpd = true;
    _pieces = pieces;

    emit gui->progressPermSetValueSig(stageContents.size());
    emit gui->removeProgressPermStatusSig();
    emit gui->messageSig(true, "Model file loaded.");

//    logInfo() << "MPD File: "         << _file
//              << ", Name: "           << _name
//              << ", Author: "         << _author
//              << ", Description: "    << _description
//              << ", Pieces: "         << _pieces
//              << ", Category: "       << _category
//                 ;
}

void LDrawFile::loadLDRFile(const QString &path, const QString &fileName)
{
    if (_subFiles[fileName]._contents.isEmpty()) {

      QString fullName(path + "/" + fileName);

      QFile file(fullName);
      if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(NULL, 
                             QMessageBox::tr(VER_PRODUCTNAME_STR),
                             QMessageBox::tr("Cannot read file %1:\n%2.")
                             .arg(fullName)
                             .arg(file.errorString()));
        return;
      }

      /* Read it in the first time to put into fileList in order of 
         appearance */

      QTextStream in(&file);
      QStringList contents;

      QRegExp sofRE("^\\s*0\\s+FILE\\s+(.*)$");
      QRegExp upAUT("^\\s*0\\s+AUTHOR(.*)|Author(.*)|author(.*)$");
      QRegExp upNAM("^\\s*0\\s+Name(.*)|name(.*)|NAME(.*)$");
      QRegExp upCAT("^\\s*0\\s+!CATEGORY(.*)|!Category(.*)|!category(.*)$");
      QRegExp upDAT("\\.dat|\\.DAT$");

      bool topLevelFileNotCaptured        = true;
      bool topLevelNameNotCaptured        = true;
      bool topLevelAuthorNotCaptured      = true;
      bool topLevelDescriptionNotCaptured = true;
      bool topLevelCategoryNotCaptured    = true;
      bool unofficialPart                 = true;
      int  descriptionLine                = 0;
      int  pieces                         = 0;

      while ( ! in.atEnd()) {
        QString line = in.readLine(0);
        contents << line;
      }

      emit gui->progressBarPermInitSig();
      emit gui->progressPermRangeSig(1, contents.size());
      emit gui->progressPermMessageSig("Processing...");
      emit gui->messageSig(true, "Loading ldr model file.");

      QDateTime datetime = QFileInfo(fullName).lastModified();
    
      insert(fileName,contents,datetime,unofficialPart);
      //writeToTmp(fileName,contents);

      /* read it a second time to find submodels */

      for (int i = 0; i < contents.size(); i++) {

        QString line = contents.at(i);

        emit gui->progressPermSetValueSig(i);

        QStringList tokens;
        split(line,tokens);

        if (tokens.size() == 15 && tokens[0] == "1" && tokens[14].contains(upDAT)){
            QFileInfo info(tokens[14]);
            PieceInfo* pieceInfo = lcGetPiecesLibrary()->FindPiece(info.baseName().toUpper().toLatin1().constData(), NULL, false);
            if (pieceInfo && pieceInfo->IsPartType())
              pieces++;
          }

        if (topLevelFileNotCaptured) {
            if (line.contains(sofRE)){
                _file = sofRE.cap(1).replace(".ldr","");
                descriptionLine = i+1;      //next line will be description
                topLevelFileNotCaptured = false;
            }
        }

        if (topLevelAuthorNotCaptured) {
           if (line.contains(upAUT)) {
               _author = upAUT.cap(1).replace(": ","");
               topLevelAuthorNotCaptured = false;
            }
        }

        if (topLevelNameNotCaptured) {
            if (line.contains(upNAM)) {
                _name = upNAM.cap(1).replace(": ","");
                topLevelNameNotCaptured = false;
            }
        }

        if (topLevelCategoryNotCaptured) {
            if (line.contains(upCAT)) {
                _category = upCAT.cap(1);
                topLevelCategoryNotCaptured = false;
            }
        }

        if (topLevelDescriptionNotCaptured && i == descriptionLine) {
            _description = line;
            topLevelDescriptionNotCaptured = false;
        }

        if (line[0] == '1' && tokens.size() == 15) {
          const QString subModel = tokens[tokens.size()-1];
          fullName = path + "/" + subModel;
          if (QFile::exists(fullName)) {
            loadLDRFile(path,subModel);
          }
        }
      }
      _mpd = false;
      _pieces = pieces;

      emit gui->progressPermSetValueSig(contents.size());
      emit gui->removeProgressPermStatusSig();
      emit gui->messageSig(true, "Model file loaded.");

//      logInfo() << "LDR File: "         << _file
//                << ", Name: "           << _name
//                << ", Author: "         << _author
//                << ", Description: "    << _description
//                << ", Pieces: "         << _pieces
//                << ", Category: "       << _category
//                   ;
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

void LDrawFile::countInstances(const QString &mcFileName, bool isMirrored, bool callout)
{
  QString fileName = mcFileName.toLower();
  bool partsAdded = false;
  bool noStep = false;
  bool stepIgnore = false;
  
  QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(fileName);
  if (f != _subFiles.end()) {
    if (f->_beenCounted) {
      if (isMirrored) {
        ++f->_mirrorInstances;
      } else {
        ++f->_instances;
      }
      return;
    }
    int j = f->_contents.size();
    f->_numSteps = 0;
    for (int i = 0; i < j; i++) {
      QStringList tokens;
      QString line = f->_contents[i];
      split(line,tokens);
      
      /* Sorry, but models that are callouts are not counted as instances */
      
      if (tokens.size() == 4 && 
          tokens[0] == "0" && 
          (tokens[1] == "LPUB" || tokens[1] == "!LPUB") && 
          tokens[2] == "CALLOUT" && 
          tokens[3] == "BEGIN") {
        partsAdded = true;

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
      } else if (tokens.size() == 5 &&
                 tokens[0] == "0" &&
                 (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
                 tokens[2] == "PART" &&
                 tokens[3] == "BEGIN"  &&
                 tokens[4] == "IGN") {
        stepIgnore = true;
      } else if (tokens.size() == 4 &&
                 tokens[0] == "0" &&
                 (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
                 tokens[2] == "PART"&&
                 tokens[3] == "END") {
        stepIgnore = false;
      } else if (tokens.size() == 3 && tokens[0] == "0" &&
                (tokens[1] == "LPUB" || tokens[1] == "!LPUB") &&
                 tokens[2] == "NOSTEP") {
        noStep = true;
      } else if (tokens.size() >= 2 && tokens[0] == "0" &&
                (tokens[1] == "STEP" || tokens[1] == "ROTSTEP")) {
        if (partsAdded && ! noStep) {
          int incr = (isMirrored && f->_mirrorInstances == 0) ||
                     (!isMirrored && f->_instances == 0);
          f->_numSteps += incr;
        }
        partsAdded = false;
        noStep = false;
      } else if (tokens.size() == 4 && tokens[0] == "0"
                                     && tokens[1] == "BUFEXCHG") {
      } else if (tokens.size() == 15 && tokens[0] == "1") {
        bool containsSubFile = contains(tokens[14]);

        if (containsSubFile && ! stepIgnore) {
          countInstances(tokens[14],mirrored(tokens),false);
        }
        partsAdded = true;
      }
    }
    f->_numSteps += partsAdded && ! noStep &&
                       ( (isMirrored && f->_mirrorInstances == 0) ||
                       (!isMirrored && f->_instances == 0) );
    if ( ! callout) {
      if (isMirrored) {
        ++f->_mirrorInstances;
      } else {
        ++f->_instances;
      }
    }
  }
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
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(NULL, 
                             QMessageBox::tr(VER_PRODUCTNAME_STR),
                             QMessageBox::tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);

    for (int i = 0; i < _subFileOrder.size(); i++) {
      QString subFileName = _subFileOrder[i];
      QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(subFileName);
      if (f != _subFiles.end() && ! f.value()._generated) {
        out << "0 FILE " << subFileName << endl;
        for (int j = 0; j < f.value()._contents.size(); j++) {
          out << f.value()._contents[j] << endl;
        }
        out << "0 NOFILE " << endl;
      }
    }
    return true;
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
        writeFileName = path + "/" + _subFileOrder[i];
      }
      file.setFileName(writeFileName);

      QMap<QString, LDrawSubFile>::iterator f = _subFiles.find(_subFileOrder[i]);
      if (f != _subFiles.end() && ! f.value()._generated) {
        if (f.value()._modified) {
          if (!file.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(NULL, 
              QMessageBox::tr(VER_PRODUCTNAME_STR),
              QMessageBox::tr("Cannot write file %1:\n%2.")
              .arg(writeFileName)
              .arg(file.errorString()));
            return false;
          }
          QTextStream out(&file);
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
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);
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
QList<QRegExp> LDrawUnofficialFileTypeRegExp;

LDrawFile::LDrawFile()
{
  {
    LDrawHeaderRegExp
        << QRegExp("^\\s*0\\s+Author[^\n]*")
        << QRegExp("^\\s*0\\s+!CATEGORY[^\n]*")
        << QRegExp("^\\s*0\\s+!CMDLINE[^\n]*")
        << QRegExp("^\\s*0\\s+!COLOUR[^\n]*")
        << QRegExp("^\\s*0\\s+!HELP[^\n]*")
        << QRegExp("^\\s*0\\s+!HISTORY[^\n]*")
        << QRegExp("^\\s*0\\s+!KEYWORDS[^\n]*")
        << QRegExp("^\\s*0\\s+!LDRAW_ORG[^\n]*")
        << QRegExp("^\\s*0\\s+LDRAW_ORG[^\n]*")
        << QRegExp("^\\s*0\\s+!LICENSE[^\n]*")
        << QRegExp("^\\s*0\\s+Name[^\n]*")
        << QRegExp("^\\s*0\\s+Official[^\n]*")
        << QRegExp("^\\s*0\\s+Unofficial[^\n]*")
        << QRegExp("^\\s*0\\s+Un-official[^\n]*")
        << QRegExp("^\\s*0\\s+Original LDraw[^\n]*")
        << QRegExp("^\\s*0\\s+~Moved to[^\n]*")
        << QRegExp("^\\s*0\\s+ROTATION[^\n]*");
  }

  {
    LDrawUnofficialFileTypeRegExp
        << QRegExp("^\\s*0\\s+UNOFFICIAL PART[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial_Part)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial_Subpart)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial_Shortcut)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial_Primitive)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial_8_Primitive)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial_48_Primitive)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial_Part Alias)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial_Shortcut Alias)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial_Part Physical_Colour)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial_Shortcut Physical_Colour)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial Part)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial Subpart)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial Shortcut)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial Primitive)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial 8_Primitive)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial 48_Primitive)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial Part Alias)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial Shortcut Alias)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial Part Physical_Colour)[^\n]*")
        << QRegExp("^\\s*0\\s+!*(?:LDRAW_ORG)* (Unofficial Shortcut Physical_Colour)[^\n]*")
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

bool isUnofficialFileType(QString &line)
{
  int size = LDrawUnofficialFileTypeRegExp.size();
  for (int i = 0; i < size; i++) {
    if (line.contains(LDrawUnofficialFileTypeRegExp[i])) {
      return true;
    }
  }
  return false;
}

