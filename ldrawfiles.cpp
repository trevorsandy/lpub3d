 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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

#include "ldrawfiles.h"
#include <QtGui>
#include <QFile>
#include <QList>
#include <QRegExp>
#include "name.h"
#include "paths.h"

#include <iostream>
#define DEBUG
#ifndef DEBUG
#define PRINT(x)
#else
#define PRINT(x) \
    std::cout << "- " << x << std::endl; //without expression
#endif
    //std::cout << #x << ":\t" << x << std::endl; //with expression

QHash<QString, int> LDrawFile::_fadePositions;
int                 LDrawFile::_emptyInt;

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
  _level = 0;
}

void LDrawFile::empty()
{
  _subFiles.clear();
  _subFileOrder.clear();
  _mpd = false;
}

/* Add a new subFile */

void LDrawFile::insert(const QString     &mcFileName, 
                      QStringList &contents, 
                      QDateTime   &datetime,
                      bool         unofficialPart,
                      bool         generated)
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

/* return the name and last stored position value */

const int &LDrawFile::fadePositions(QString value)
{
  if (_fadePositions.contains(value.toLower())) {
    return _fadePositions[value.toLower()];
  } else {
    return _emptyInt;
  }
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

// not used
void LDrawFile::setFileLevel(const QString     &mcFileName,
                 const int &level)
{
  QString fileName = mcFileName.toLower();
  QMap<QString, LDrawSubFile>::iterator i = _subFiles.find(fileName);

  if (i != _subFiles.end()) {
    i.value()._modified = true;
    //i.value()._datetime = QDateTime::currentDateTime();
    i.value()._level = level;
    i.value()._changedSinceLastWrite = true;
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
{
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
                             QMessageBox::tr(LPUB),
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

    QStringList contents;
    QString     mpdName;
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
                             QMessageBox::tr(LPUB),
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
    QRegExp subFile("^\\s*1\\s(.+)\\s(.+)\\.((ldr|LDR)|(mpd|MPD))$");
    QRegExp upRE1( "^\\s*0\\s+(LDRAW_ORG|Unofficial Part)");
    QRegExp upRE2( "^\\s*0\\s+!(LDRAW_ORG|Unofficial Part)");
    bool    unofficialPart = false;
    bool    isSubModel = false;
    bool    isMainFile = true;
    int     counter = 0;

    /* Read content into temperory content file the first time to put into fileList in order of
       appearance and stage for later processing */
    while ( ! in.atEnd()) {
      QString sLine = in.readLine(0);
      stageContents << sLine;
      counter++;
    }
    PRINT("485 FINISHED MPD READ with " << stageContents.count() << " records read");

    for (int i = 0; i < stageContents.size(); i++) {
      QString line = stageContents.at(i);
      QString subModel;
      QStringList tokens;

      split(line,tokens);

      // one time read to capture main.ldr file (first file in MPD)
      if (isMainFile) {
          isSubModel = line.contains(sofRE);
          isMainFile = false;
          PRINT("498 One-time subModel Read: " << sofRE.cap(1).toStdString());
      } else {
          isSubModel = line.contains(subFile);
      }

      if (isSubModel) {

          subModel = tokens[tokens.size()-1];

          for (int e = 0; e < stageContents.size(); e++) {
              QString smLine = stageContents.at(e);
              QStringList tokens2;

              bool sof = smLine.contains(sofRE);  //start of file
              bool eof = smLine.contains(eofRE);  //end of file
              split(smLine,tokens2);
              bool alreadyInserted = LDrawFile::contains(mpdName.toLower());

              if (sof || eof) {

                  if (mpdName.toLower() == subModel.toLower() && ! alreadyInserted) {
                      PRINT( "520 INSERT subModel: " << subModel.toStdString() <<
                                   ", mpdName: " << mpdName.toStdString() <<
                                   ", Content Count: " << contents.count());

                      insert(mpdName,contents,datetime,unofficialPart);
                      unofficialPart = false;

                  }
                  contents.clear();

                  if (sof) {
                      mpdName = sofRE.cap(1);
                  } else {
                      mpdName.clear();
                  }

              } else if ( ! mpdName.isEmpty() && smLine != "") {
                  if (smLine.contains(upRE1)||smLine.contains(upRE2)) {
                      unofficialPart = true;
                  }
                  contents << smLine;
              }
          }
      }
    }

    if ( ! mpdName.isEmpty() && ! contents.isEmpty()) {
      insert(mpdName,contents,datetime,unofficialPart);
    }

    /* Process File Levels */
    int level = 0;

    QString levelFileName = topLevelFile().toLower();
    PRINT("551 LEVEL - Top Level File: " << levelFileName.toStdString());
    QMap<QString, LDrawSubFile>::iterator it = _subFiles.find(levelFileName);
    if (it != _subFiles.end()){
        it->_level = level;
        QStringList contents = LDrawFile::contents(levelFileName);
        subFileLevels(contents, level);
    }

    _mpd = true;
}


void LDrawFile::loadLDRFile(const QString &path, const QString &fileName)
{
    if (_subFiles[fileName]._contents.isEmpty()) {

      QString fullName(path + "/" + fileName);

      QFile file(fullName);
      if ( ! file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(NULL, 
                             QMessageBox::tr(LPUB),
                             QMessageBox::tr("Cannot read file %1:\n%2.")
                             .arg(fullName)
                             .arg(file.errorString()));
        return;
      }

      /* Read it in the first time to put into fileList in order of 
         appearance */

      QTextStream in(&file);
      QStringList contents;

      while ( ! in.atEnd()) {
        QString line = in.readLine(0);
        contents << line;
      }

      QDateTime datetime = QFileInfo(fullName).lastModified();
    
      insert(fileName,contents,datetime,false);
      //writeToTmp(fileName,contents);

      /* read it a second time to find submodels */

      for (int i = 0; i < contents.size(); i++) {
        QString line = contents.at(i);
        QStringList tokens;

        split(line,tokens);

        if (line[0] == '1' && tokens.size() == 15) {
          const QString subModel = tokens[tokens.size()-1];
          fullName = path + "/" + subModel;
          if (QFile::exists(fullName)) {
            loadLDRFile(path,subModel);
          }
        }
      }
      _mpd = false;
    }
}

/* write the level of each subfile in the MPD file
 * Levels are used to properly determine the
 * start position for writing step fade files
 * during the initial MPD file load.
 */

void LDrawFile::subFileLevels(QStringList &contents, int &level){
    QRegExp subFile("^\\s*1\\s(.+)\\s(.+)\\.((ldr|LDR)|(mpd|MPD))$");
    int sfLevel = level;

    for (int i = 0; i < contents.size(); i++) {
        QString line = contents.at(i);
        QString subFileName;
        QStringList tokens;
        split(line,tokens);

        bool isSubFile = line.contains(subFile);
        if (isSubFile){

            subFileName = tokens[tokens.size()-1].toLower();

            QMap<QString, LDrawSubFile>::iterator it = _subFiles.find(subFileName);

            if (it != _subFiles.end()){
                int sfLevel2;
                it->_level = sfLevel;
                QStringList contents2;
                contents2 = LDrawFile::contents(subFileName);
                sfLevel2 = sfLevel + 1;

                PRINT("640 LEVEL - Process SubFile " << subFileName.toStdString() << " at Level (" << sfLevel2 << ") with Content2 Count(" << contents2.count() << ")");
                subFileLevels(contents2, sfLevel2);
            }
        }
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
  bool buffExchg = false;
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
            if (contains(tokens[14]) /*&& ! buffExchg */ && ! stepIgnore) {
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
        buffExchg = tokens[3] == "STORE";
      } else if (tokens.size() == 15 && tokens[0] == "1") {
        bool containsSubFile = contains(tokens[14]);
        // Danny: !buffExchg condition prevented from counting steps in submodels
        if (containsSubFile /*&& ! buffExchg*/ && ! stepIgnore) {
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
                             QMessageBox::tr(LPUB),
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
              QMessageBox::tr(LPUB),
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



int split(const QString &line, QStringList &argv)
{
  QString     chopped = line;
  int         p = 0;
  int         length = chopped.length();

  if (p == length) {
    return 0;
  }
  
  while (chopped[p] == ' ') {
    if (++p == length) {
      return -1;
    }
  }
  
  argv.clear();
  
  if (chopped[p] == '1') {

    argv << "1";
    p += 2;
    if (p >= length) {
      return -1;
    }
    while (chopped[p] == ' ') {
      if (++p >= length) {
        return -1;
      }
    }
    
    // color x y z a b c d e f g h i
    
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
  
    int soq = chopped.indexOf("\"");
    if (soq == -1) {
      argv << chopped.split(" ",QString::SkipEmptyParts);
    } else {
      while (chopped.size()) {
        soq = chopped.indexOf("\"");
        if (soq == -1) {
          argv << chopped.split(" ",QString::SkipEmptyParts);
          chopped.clear();
        } else {
        // we found a double quote
          QString left = chopped.left(soq);

          left = left.trimmed();
  
          argv << left.split(" ",QString::SkipEmptyParts);

          chopped = chopped.mid(soq+1);
  
          soq = chopped.indexOf("\"");
  
          if (soq == -1) {
            argv << left;
            return -1;
          }
          argv << chopped.left(soq);
  
          chopped = chopped.mid(soq+1);
  
          if (chopped == "\"") {
            chopped.clear();
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

QList<QRegExp> LDrawHeaderRegExp;

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
