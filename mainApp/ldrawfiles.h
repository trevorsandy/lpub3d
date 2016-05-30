 
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

#ifndef LDRAWFILES_H
#define LDRAWFILES_H

#include <QMessageBox>
#include <QStringList>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <QList>
#include <QRegExp>
#include <QHash>

#include "QsLog.h"

extern QList<QRegExp> LDrawHeaderRegExp;

class LDrawSubFile {
  public:
    QStringList _contents;
    bool        _modified;
    QDateTime   _datetime;
    int         _numSteps;
    bool        _beenCounted;
    int         _instances;
    int         _mirrorInstances;
    bool        _rendered;
    bool        _mirrorRendered;
    bool        _changedSinceLastWrite;
    bool        _unofficialPart;
    bool        _generated;
    int         _fadePosition;
    int         _startPageNumber;

    LDrawSubFile()
    {
      _unofficialPart = false;
    }
    LDrawSubFile(
      const QStringList &contents,
            QDateTime   &datetime,
            bool         unofficialPart,
            bool         generated = false);
    ~LDrawSubFile()
    {
      _contents.clear();
    }
};

class LDrawFile {
  private:
    QMap<QString, LDrawSubFile> _subFiles;
    QStringList                 _emptyList;
    QString                     _emptyString;
    bool                        _mpd;
    static int                  _emptyInt;
  public:
    LDrawFile();
    ~LDrawFile()
    {
      _subFiles.empty();
    }

    QStringList                 _subFileOrder;
    static QString              _file;
    static QString              _name;
    static QString              _author;
    static QString              _description;
    static QString              _category;
    static int                  _pieces;

    bool saveFile(const QString &fileName);
    bool saveMPDFile(const QString &filename);
    bool saveLDRFile(const QString &filename);

    void insert(const QString     &fileName, 
                      QStringList &contents, 
                      QDateTime   &datetime,
                      bool         unofficialPart,
                      bool         generated = false);

    int  size(const QString &fileName);
    void empty();

    QStringList contents(const QString &fileName);
    void setContents(const QString     &fileName, 
                     const QStringList &contents);
    void setFadePosition(const QString &mcFileName,
                         const int     &fadePosition);
    int getFadePosition(const QString &mcFileName);
    void setModelStartPageNumber(const QString &mcFileName,
                         const int     &startPageNumber);
    int getModelStartPageNumber(const QString &mcFileName);
    void subFileLevels(QStringList &contents, int &level);
    void loadFile(const QString &fileName);
    void loadMPDFile(const QString &fileName, QDateTime &datetime);
    void loadLDRFile(const QString &path, const QString &fileName);
    QStringList subFileOrder();
    
    QString readLine(const QString &fileName, int lineNumber);
    void insertLine( const QString &fileName, int lineNumber, const QString &line);
    void replaceLine(const QString &fileName, int lineNumber, const QString &line);
    void deleteLine( const QString &fileName, int lineNumber);
    void changeContents(const QString &fileName, 
                              int      position, 
                              int      charsRemoved, 
                        const QString &charsAdded);

    bool isMpd();
    QString topLevelFile();
    bool isUnofficialPart(const QString &name);
    int numSteps(const QString &fileName);
    QDateTime lastModified(const QString &fileName);
    bool contains(const QString &file);
    bool isSubmodel(const QString &file);
    bool modified();
    bool modified(const QString &fileName);
    bool older(const QStringList &submodelStack, 
               const QDateTime &datetime);
    static bool mirrored(const QStringList &tokens);
    void unrendered();
    void setRendered(const QString &fileName, bool mirrored);
    bool rendered(const QString &fileName, bool mirrored);
    int instances(const QString &fileName, bool mirrored);
    void countInstances();
    void countInstances(const QString &fileName, bool mirrored, const bool callout = false);
    bool changedSinceLastWrite(const QString &fileName);
    void tempCacheCleared();
};

int split(const QString &line, QStringList &argv);
int validSoQ(const QString &line, int soq);
bool isHeader(QString &line);
bool isUnofficialFileType(QString &line);

#endif
