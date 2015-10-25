/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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

#ifndef PARTLIST_H
#define PARTLIST_H

#include <QFile>
#include <QList>
#include <QMap>
#include <QObject>
#include <QRegExp>
#include <QElapsedTimer>

#include "ldsearchdirs.h"
#include "ldrawfiles.h"
#include "FadeStepColorParts.h"
#include "archiveparts.h"
#include "version.h"

#include "QsLog.h"

class GlobalFadeStep;
class PartList;
class ColourPart;

enum partTypeDir{
    OFFICIAL_PARTS,
    UNOFFICIAL_PARTS,
    OFFICIAL_SUB_PARTS,
    UNOFFICIAL_SUP_PARTS,
    OFFICIAL_PRIMITIVES,
    UNOFFICIAL_PRIMITIVES,
    OFFICIAL_PRIMITIVES_8,
    UNOFFICIAL_PRIMITIVES_8,
    OFFICIAL_PRIMITIVES_48,
    UNOFFICIAL_PRIMITIVES_48
};

enum headerOption{
    OFFICIAL_PARTS_HEADER,
    UNOFFICIAL_PARTS_HEADER,
    OFFICIAL_PRIMITIVES_HEADER,
    UNOFFICIAL_PRIMITIVES_HEADER,
    FADESTEP_FILE_HEADER,
    FADESTEP_COLOUR_CHILDREN_HEADER
};

class ColourPart {
public:
    QStringList _contents;                        // from retrieveContents
    QString     _fileNameStr;                     // from parseParent
    int         _lineNum;                         // from parseParent
    int         _partType;                        // from retrieveContent

    ColourPart(
            const QStringList   &contents,        // file contents
            const QString       &fileNameStr,     // file name as defined in part content
            const int           &lineNum,         // line number in content
            const int           &partType);       // weather official, unofficial, primitive etc...

    ~ColourPart()
    {
        _contents.clear();
    }
};

class PartList: public QObject
{
   Q_OBJECT

public:
    explicit PartList(QObject *parent = 0);
    ~PartList()
    {
        _colourParts.empty();
        _fadeStepColourParts.clear();
        _partFileContents.clear();

    }

    void insert(
            const QStringList   &contents,
            const QString       &fileNameStr,
            const int           &lineNum,
            const int           &partType);

    int  size(
            const QString       &fileNameStr,
            const int           &lineNum);

    QStringList contents(
            const QString       &fileNameStr,
            const int           &lineNum);

    void remove(
            const QString       &fileNameStr,
            const int           &lineNum);

      int size(){return         _partList.size();}

     void empty();

     bool saveFadeFile(
    const QString               &fileName,
    const QStringList           &fadePartContent);

     void createFadePartFiles();                       // convert static color files // replace color code with fade color

     void createFadePartContent(                       // parse provided colour file to colour children
         const QString   &fileNameComboStr,
         const int       &lineNum);

     void retrieveContent(                             // parse provided colour file to colour children
               QStringList     &inputContents,
         const QString         &fileAbsPathStr,
         const QString         &fileNameStr,
         const int             &lineNum);

     QStringList              _partList;

     void processFadeColorParts();                      // scan LDraw file for static colored parts and create fade copy

     void processLDSearchDirParts();

     void processPartsArchive(
         const QString         &comment);

signals:
     void progressBarInitSig();

     void progressMessageSig(
            const QString       &text);

     void progressRangeSig(
             const int          &min,
             const int          &max);

     void progressSetValueSig(
             const int          &value);

     void progressResetSig();

     void messageSig(
            bool                status,
            QString             message);

     void removeProgressStatusSig();

     void finishedSig();

private:
    QMap<QString, ColourPart> _colourParts;
    QStringList               _emptyList;
    QString                   _emptyString;
    QStringList               _fadeStepColourParts;
    QStringList               _partFileContents;
    LDrawFile                 ldrawFile;           // contains MPD or all files used in model
    ArchiveParts              archiveParts;        // add contente to unofficial zip archive (for LeoCAD)
    QStringList               _partsDirs;

};

// Custom Thread

class PartListWorker: public QObject
{
   Q_OBJECT

public:
    explicit PartListWorker(QObject *parent = 0);
    ~PartListWorker()
    {
        _colourParts.empty();
        _fadeStepColourParts.clear();
        _partFileContents.clear();
    }

    void insert(
            const QStringList   &contents,
            const QString       &fileNameStr,
            const int           &lineNum,
            const int           &partType);

    void remove(
            const QString       &fileNameStr,
            const int           &lineNum);

    bool endThreadEventLoopNow();

    QStringList                 _partList;

public slots:
     void                       scanDir();
     void                       requestEndThreadNow();

signals:
     void progressBarInitSig();

     void progressMessageSig(
            const QString       &text);

     void progressRangeSig(
             const int          &min,
             const int          &max);

     void progressSetValueSig(
             const int          &value);

     void progressResetSig();

     void messageSig(
            bool                status,
            QString             message);

     void removeProgressStatusSig();

     void finishedSig();

private:
    bool                      _endThreadNowRequested;
    QMap<QString, ColourPart> _colourParts;
    QStringList               _emptyList;
    QString                   _emptyString;
    int                       _cpLines;

    QStringList               _fadeStepColourParts;
    QStringList               _partFileContents;
    QElapsedTimer             _timer;

    void colourChildren();
    void writeFile();

    void scanDir(             QDir dir);
    void getParts(const  QFileInfo &fileInfo);
    void buildList(const QFileInfo &fileInfo);
    void fileHeader(const      int &option);

};

#endif // PARTLIST_H














