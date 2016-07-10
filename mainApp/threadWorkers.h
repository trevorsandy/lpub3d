/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

#ifndef THREADWORKERS_H
#define THREADWORKERS_H

#include <QFile>
#include <QList>
#include <QMap>
#include <QObject>
#include <QRegExp>
#include <QElapsedTimer>

#include "lpub_preferences.h"
#include "ldrawfiles.h"
#include "FadeStepColorParts.h"
#include "archiveparts.h"
#include "version.h"
#include "ldsearchdirs.h"

#include "QsLog.h"

class GlobalFadeStep;
class PartWorker;
class ColourPart;
class LDPartsDirs;

enum partTypeDir{
    LD_PARTS,
    LD_SUB_PARTS,
    LD_PRIMITIVES,
    LD_PRIMITIVES_8,
    LD_PRIMITIVES_48
};

enum headerOption{
    FADESTEP_INTRO_HEADER,
    FADESTEP_FILE_HEADER,
    FADESTEP_COLOUR_PARTS_HEADER,
    FADESTEP_COLOUR_CHILDREN_PARTS_HEADER,
    FADESTEP_FILE_STATUS
};

class ColourPart {
public:
    QStringList _contents;                        // from retrieveContents
    QString     _fileNameStr;                     // from parseParent
    int         _partType;                        // from retrieveContent

    ColourPart(
            const QStringList   &contents,        // file contents
            const QString       &fileNameStr,     // file name as defined in part content
            const int           &partType);

    ~ColourPart()
    {
        _contents.clear();
    }
};

class PartWorker: public QObject
{
   Q_OBJECT

public:
    explicit PartWorker(QObject *parent = 0);
    ~PartWorker()
    {
        _colourParts.empty();
        _fadeStepColourParts.clear();
        _partFileContents.clear();

    }

    void insert(
            const QStringList   &contents,
            const QString       &fileNameStr,
            const int           &partType);

    bool partAlreadyInList(
        const QString           &fileNameStr);

    void remove(
        const QString           &fileNameStr);

    bool endThreadEventLoopNow();

    void setDoFadeStep(
        bool                     doFadeStep)
        {_doFadeStep           = doFadeStep;}

    void setDidInitLDSearch(
        bool                     initLDSearch)
        {_didInitLDSearch      = initLDSearch;}

    void setDoReloadUnoffLib(
        bool                     doReload)
        {_doReload             = doReload;}

    void resetSearchDirSettings()
        {_resetSearchDirSettings = true;
         ldsearchDirPreferences();}

    void ldsearchDirPreferences();

    bool loadLDrawSearchDirs();

    QStringList                _partList;

public slots:
     void processFadeColourParts();                      // scan LDraw file for static colored parts and create fade copy

     void processLDSearchDirParts();

     void populateLdgLiteSearchDirs();

     void requestEndThreadNow();

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

     void fadeColourFinishedSig();

     //2 below not used
     void requestFinishSig();
     void ldSearchDirFinishedSig();

private:
    bool                      _endThreadNowRequested;
    QMap<QString, ColourPart> _colourParts;
    QStringList               _emptyList;
    QString                   _emptyString;
    QStringList               _fadeStepColourParts;
    QStringList               _partFileContents;
    QStringList               _excludedSearchDirs;
//    bool                      _partsArchived;
    bool                      _doFadeStep;
    bool                      _doReload;
    bool                      _didInitLDSearch;
    bool                      _resetSearchDirSettings;

    LDPartsDirs                ldPartsDirs;                     // automatically load LDraw.ini parameters
    LDrawFile                  ldrawFile;                       // contains MPD or all files used in model
    ArchiveParts               archiveParts;                    // add contente to unofficial zip archive (for LeoCAD)


    QStringList contents(
        const QString       &fileNameStr);

    int  size(const QString &fileNameStr);

    int size(){return       _partList.size();}

   void empty();

   bool saveFadeFile(
        const QString        &fileName,
        const QStringList    &fadePartContent);

   void createFadePartFiles();                       // convert static color files // replace color code with fade color

   void createFadePartContent(                       // parse provided colour file to colour children
       const QString         &fileNameComboStr);

   void retrieveContent(                             // parse provided colour file to colour children
             QStringList     &inputContents,
       const QString         &fileAbsPathStr,
       const QString         &fileNameStr);

   void processFadeColourPartsArchive(
       const QString         &comment,
       bool                   silent = false);

   void processLDSearchPartsArchive(
       const QStringList     &ldSearchPartsDirs,
       const QString         &comment,
       bool                   silent = false);

   void processPartsArchive(
       const QStringList     &ldPartsDirs,
       const QString         &comment);

   bool doFadeStep()
      {return                _doFadeStep;}

   bool doReloadUnoffLib()
      {return                _doReload;}

   bool didInitLDSearch()
      {return                _didInitLDSearch;}

   bool okToEmitToProgressBar()
      {return               (doFadeStep() || doReloadUnoffLib());}

};


class ColourPartListWorker: public QObject
{
   Q_OBJECT

public:
    explicit ColourPartListWorker(QObject *parent = 0);
    ~ColourPartListWorker()
    {
        _colourParts.empty();
        _fadeStepColourParts.clear();
        _partFileContents.clear();
    }

    void insert(
            const QStringList   &contents,
            const QString       &fileNameStr,
            const int           &partType);

    bool partAlreadyInList(
            const QString       &fileNameStr);

    void remove(
            const QString       &fileNameStr);

    bool endThreadEventLoopNow();

    QStringList                 _partList;

public slots:
     void                       generateFadeColourPartsList();
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
     void colourPartListFinishedSig();

     //not used
     void requestFinishSig();

private:
    bool                      _endThreadNowRequested;
    QMap<QString, ColourPart> _colourParts;
    QStringList               _emptyList;
    QString                   _emptyString;
    int                       _cpLines;
    int                       _columnWidth;

    QStringList               _fadeStepColourParts;
    QStringList               _partFileContents;
    QElapsedTimer             _timer;

    LDPartsDirs                ldPartsDirs;                     // automatically load LDraw.ini parameters

    void processChildren();
    void writeFadeFile(bool append = false);

    bool processArchiveParts(const QString &archiveFile);
    void processFileContents(const QFileInfo &fileInfo);
    void fileSectionHeader(const int &option,
                           const QString &heading = "");

};

#endif // THREADWORKERS_H














