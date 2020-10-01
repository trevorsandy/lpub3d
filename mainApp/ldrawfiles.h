
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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

#include <QStringList>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <QList>

#include "excludedparts.h"
#include "stickerparts.h"
#include "QsLog.h"

extern QList<QRegExp> LDrawHeaderRegExp;
extern QList<QRegExp> LDrawUnofficialPartRegExp;
extern QList<QRegExp> LDrawUnofficialSubPartRegExp;
extern QList<QRegExp> LDrawUnofficialPrimitiveRegExp;
extern QList<QRegExp> LDrawUnofficialOtherRegExp;

class LDrawSubFile {
  public:
    QStringList  _contents;
    QString      _subFilePath;
    bool         _modified;
    QDateTime    _datetime;
    QStringList  _renderedKeys;
    QStringList  _mirrorRenderedKeys;
    QVector<int> _lineTypeIndexes;
    int          _numSteps;
    bool         _beenCounted;
    int          _instances;
    int          _mirrorInstances;
    bool         _rendered;
    bool         _mirrorRendered;
    bool         _changedSinceLastWrite;
    bool         _generated;
    bool         _includeFile;
    int          _prevStepPosition;
    int          _startPageNumber;
    int          _unofficialPart;

    LDrawSubFile()
    {
      _unofficialPart = 0;
    }
    LDrawSubFile(
            const QStringList &contents,
            QDateTime         &datetime,
            int                unofficialPart,
            bool               generated = false,
            bool               includeFile = false,
            const QString     &subFilePath = QString());
    ~LDrawSubFile()
    {
      _contents.clear();
    }
};

class CfgSubFile {
  public:
    QStringList  _contents;
    QString      _subFilePath;
    CfgSubFile(){}
    CfgSubFile(
            const QStringList &contents,
            const QString     &subFilePath = QString());
    ~CfgSubFile()
    {
      _contents.clear();
    }
};

class ViewerStep {
  public:
    QStringList _rotatedContents;
    QStringList _unrotatedContents;
    QString   	_filePath;
    QString     _imagePath;
    QString     _csiKey;
    bool        _modified;
    bool        _multiStep;
    bool        _calledOut;

    ViewerStep()
    {
      _modified = false;
    }
    ViewerStep(
      const QStringList &rotatedContents,
      const QStringList &unrotatedContents,
      const QString     &filePath,
      const QString     &imagePath,
      const QString     &csiKey,
      bool               multiStep,
      bool               calledOut);
    ~ViewerStep()
    {
      _rotatedContents.clear();
      _unrotatedContents.clear();
    }
};

/********************************************
 * this is a utility class that enables nested
 * levels
 ********************************************/

class HiarchLevel
{
public:
    HiarchLevel(const QString &key)
        : level(nullptr),
          key(key){}
    ~HiarchLevel(){}
    HiarchLevel* GetTopLevel()
    {
        return level ? level->GetTopLevel() : this;
    }
    HiarchLevel* level;
    QString key;
};

extern int getLevel(const QString& key, int position);

/********************************************
 * build modification
 ********************************************/

class BuildMod {
  public:
    QString       _modStepKey;
    QVector<int>  _modAttributes;
    QMap<int,int> _modActions;

    BuildMod()
    {
      _modAttributes = { 0, 0, 0, -1 };
    }
    BuildMod(
      const QString            &modStepKey,
      const QVector<int>       &modAttributes,
      int                       modAction,
      int                       stepNumber);
    ~BuildMod()
    {
      _modAttributes.clear();
      _modActions.clear();
    }
};

class LDrawFile {
  private:
    QMap<QString, LDrawSubFile> _subFiles;
    QMap<QString, CfgSubFile>   _configuredSubFiles;
    QMap<QString, ViewerStep>   _viewerSteps;
    QMap<QString, BuildMod>     _buildMods;
    QVector<QVector<int>>       _buildModStepIndexes;
    QMultiHash<QString, int>    _ldcadGroups;
    QStringList                 _emptyList;
    QString                     _emptyString;
    int                         _buildModNextStepIndex;
    int                         _buildModPrevStepIndex;
    bool                        _mpd;
    static int                  _emptyInt;
    static QList<QRegExp>       _fileRegExp;

    ExcludedParts               excludedParts; // internal list of part count excluded parts

    bool hdrTopFileNotFound;
    bool hdrNameNotFound;
    bool hdrAuthorNotFound;
    bool hdrDescNotFound;
    bool hdrCategNotFound;
    bool metaBuildModNotFund;
    bool unofficialPart;
    bool topLevelModel;
    bool ldcadGroupsLoaded;
    int  descriptionLine;
    int  buildModLevel;

  public:
    LDrawFile();
    ~LDrawFile()
    {
      _subFiles.empty();
      _configuredSubFiles.empty();
      _viewerSteps.empty();
      _ldcadGroups.empty();
      _buildMods.empty();
      _buildModStepIndexes.empty();
    }

    QStringList                 _subFileOrder;
    QStringList                 _includeFileList;
    QStringList                 _buildModList;
    static QList<HiarchLevel*>  _currentLevels;
    static QList<HiarchLevel*>  _allLevels;
    static QStringList          _loadedParts;
    static QString              _file;
    static QString              _name;
    static QString              _author;
    static QString              _description;
    static QString              _category;
    static bool                 _currFileIsUTF8;
    static int                  _partCount;
    static bool                 _showLoadMessages;
    static bool                 _loadAborted;

    int getPartCount(){
      return _partCount;
    }

    static void showLoadMessages();

    bool saveFile(const QString &fileName);
    bool saveMPDFile(const QString &filename);
    bool saveLDRFile(const QString &filename);
    bool saveIncludeFile(const QString &filename);

    void insert(const QString       &fileName,
                      QStringList   &contents,
                      QDateTime     &datetime,
                      bool           unofficialPart,
                      bool           generated = false,
                      bool           includeFile = false,
                      const QString &subFilePath = QString());

    int  size(const QString &fileName);
    void empty();

    QStringList getSubModels();
    QStringList getSubFilePaths();
    QStringList contents(const QString &fileName);
    void normalizeHeader(const QString &fileName,
                         int missing = 0);
    void setSubFilePath(const QString &mcFileName,
                     const QString &subFilePath);
    void setContents(const QString     &fileName, 
                     const QStringList &contents);
    void setPrevStepPosition(const QString &mcFileName,
                         const int     &prevStepPosition);
    int getPrevStepPosition(const QString &mcFileName);
    void clearPrevStepPositions();
    void setModelStartPageNumber(const QString &mcFileName,
                         const int     &startPageNumber);
    int getModelStartPageNumber(const QString &mcFileName);
    void subFileLevels(QStringList &contents, int &level);
    int loadFile(const QString &fileName);
    void loadMPDFile(const QString &fileName, QDateTime &datetime);
    void loadLDRFile(const QString &path, const QString &fileName);
    QStringList subFileOrder();
    QStringList includeFileList();
    
    QString readLine(const QString &fileName, int lineNumber);
    void insertLine( const QString &fileName, int lineNumber, const QString &line);
    void replaceLine(const QString &fileName, int lineNumber, const QString &line);
    void deleteLine( const QString &fileName, int lineNumber);
    void changeContents(const QString &fileName, 
                              int      position, 
                              int      charsRemoved, 
                        const QString &charsAdded);

    // Only used to insert fade or highlight content
    void insertConfiguredSubFile (const QString &fileName,
                                        QStringList   &contents,
                                        const QString &subFilePath = QString());
    // Only used to read fade or highlight content
    QString readConfiguredLine(const QString &fileName, int lineNumber);
    // Only used to return fade or highlight content size
    int configuredSubFileSize(const QString &mcFileName);

    bool isMpd();
    QString topLevelFile();
    int isUnofficialPart(const QString &name);
    int isIncludeFile(const QString &fileName);
    int numSteps(const QString &fileName);
    QDateTime lastModified(const QString &fileName);
    int fileOrderIndex(const QString &file);
    bool contains(const QString &file);
    bool isSubmodel(const QString &file);
    bool modified();
    bool modified(const QString &fileName);
    bool older(const QStringList &parsedStack,
               const QDateTime &lastModified);
    bool older(const QString &fileName,
               const QDateTime &lastModified);
    static bool mirrored(const QStringList &tokens);
    void unrendered();
    void setRendered(
            const QString &fileName,
            bool           mirrored,
            const QString &renderParentModel,
            int            renderStepNumber,
            int            howCounted);
    bool rendered(
            const QString &fileName,
            bool           mirrored,
            const QString &renderParentModel,
            int            renderStepNumber,
            int            howCounted);
    void addCustomColorParts(const QString &mcFileName, bool autoAdd = false);
    int instances(const QString &fileName, bool mirrored);
    void countParts(const QString &fileName);
    void countInstances();
    void countInstances(const QString &fileName, bool firstStep, bool isMirrored, const bool callout = false);
    bool changedSinceLastWrite(const QString &fileName);
    void tempCacheCleared();

    void insertLDCadGroup(const QString &name, int lid);
    bool ldcadGroupMatch(const QString &name, const QStringList &lids);

    /* Build Modification functions */
    void insertBuildMod(const QString      &buildModKey,
                        const QString      &modStepKey,
                        const QVector<int> &modAttributes,
                        int                 action,
                        int                 stepIndex);
    int setBuildModAction(const QString &buildModKey,
                          int            stepIndex,
                          int            modAction);
    int getBuildModStepIndex(int modelIndex, int &lineNumber);
    int getBuildModStepIndexHere(int stepIndex, int which);
    int getBuildModStepLineNumber(int stepIndex, bool bottom);
    int getBuildModBeginLineNumber(const QString &buildModKey);
    int getBuildModEndLineNumber(const QString &buildModKey);
    int getBuildModActionLineNumber(const QString &buildModKey);
    int getBuildModAction(const QString &buildModKey, int stepIndex);
    int getBuildModDisplayPageNumber(const QString &buildModKey);
    int setBuildModDisplayPageNumber(const QString &buildModKey, int displayPageNum);
    int getBuildModStepPieces(const QString &buildModKey);
    int setBuildModStepPieces(const QString &buildModKey, int pieces);
    int getBuildModPrevStepIndex();
    int getBuildModNextStepIndex();
    int buildModsSize();
    void setBuildModStepKey(const QString &buildModKey, const QString &modStepKey);
    bool getBuildModStepIndexHere(int stepIndex, QString &modelName, int &lineNumber);
    bool setBuildModNextStepIndex(const QString &modelName, const int &lineNumber);
    bool buildModContains(const QString &buildModKey);
    bool deleteBuildMod(const QString &buildModKey);
    QString getBuildModStepKey(const QString &buildModKey);
    QString getBuildModModelName(const QString &buildModKey);
    QMap<int, int> getBuildModActions(const QString &buildModKey);
    QStringList getBuildModsList();

    /* ViewerStep functions */
    void insertViewerStep(const QString     &stepKey,
                          const QStringList &rotatedContents,
                          const QStringList &unrotatedContents,
                          const QString     &filePath,
                          const QString     &imagePath,
                          const QString     &csiKey,
                          bool               multiStep,
                          bool               calledOut);
    void updateViewerStep(const QString     &fileName,
                          const QStringList &contents,
                          bool rotated = true);
    QStringList getViewerStepRotatedContents(const QString &stepKey);
    QStringList getViewerStepUnrotatedContents(const QString &stepKey);
    QString     getViewerStepFilePath(const QString &stepKey);
    QString     getViewerStepImagePath(const QString &stepKey);
    QString     getViewerConfigKey(const QString &stepKey);
    bool        isViewerStepMultiStep(const QString &stepKey);
    bool        isViewerStepCalledOut(const QString &stepKey);
    bool        viewerStepContentExist(const QString &stepKey);
    bool        deleteViewerStep(const QString &stepKey);
    void        clearViewerSteps();

    QString     getSubmodelName(int submodelIndx);
    int         getSubmodelIndex(const QString &fileName);
    int         getLineTypeIndex(int submodelIndx, int relativeTypeIndx);
    int         getLineTypeRelativeIndex(int submodelIndx, int lineTypeIndx);
    void        setLineTypeRelativeIndex(int submodelIndx, int relativeTypeIndx); // not used
    void        setLineTypeRelativeIndexes(int submodelIndx, QVector<int> &relativeTypeIndxes);
    void        resetLineTypeRelativeIndex(const QString &fileName);              // not used
    QVector<int>*getLineTypeRelativeIndexes(int submodelIndx);
};

int split(const QString &line, QStringList &argv);
int validSoQ(const QString &line, int soq);
int  getUnofficialFileType(QString &line);
bool isHeader(QString &line);
bool isComment(QString &line);
bool isGhost(QString &line);
bool isSubstitute(QString &line, QString &lineOut);

#endif
