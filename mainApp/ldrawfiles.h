
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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
    QStringList  _smiContents;
    QString      _subFilePath;
    bool         _modified;
    QDateTime    _datetime;
    QStringList  _renderedKeys;
    QStringList  _mirrorRenderedKeys;
    QVector<int> _lineTypeIndexes;
    QVector<int> _prevStepPosition;
    QVector<int> _subFileIndexes;
    int          _numSteps;
    bool         _beenCounted;
    int          _instances;
    int          _mirrorInstances;
    bool         _rendered;
    bool         _mirrorRendered;
    bool         _changedSinceLastWrite;
    bool         _generated;
    bool         _includeFile;
    int          _startPageNumber;
    int          _unofficialPart;

    LDrawSubFile()
    {
      _unofficialPart = 0;
      _prevStepPosition = { 0,0,0 };
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
      _smiContents.clear();
      _lineTypeIndexes.clear();
      _subFileIndexes.clear();
      _prevStepPosition.clear();
      _renderedKeys.clear();
      _mirrorRenderedKeys.clear();
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
    struct StepKey {
       int modIndex;
       int lineNum;
       int stepNum;
    };
    QStringList _rotatedContents;
    QStringList _unrotatedContents;
    QString   	_filePath;
    QString     _imagePath;
    QString     _csiKey;
    StepKey     _stepKey;
    int         _partCount;
    bool        _modified;
    bool        _multiStep;
    bool        _calledOut;
    int         _viewType;

    ViewerStep()
    {
      _modified = false;
    }
    ViewerStep(
      const QStringList &stepKey,
      const QStringList &rotatedContents,
      const QStringList &unrotatedContents,
      const QString     &filePath,
      const QString     &imagePath,
      const QString     &csiKey,
      bool               multiStep,
      bool               calledOut,
      int                viewType);
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

class BuildModStep {
  public:
    BuildModStep(
       const int      buildModStepIndex,
       const int      buildModAction,
       const QString& buildModKey);
    ~BuildModStep()
    { }
    bool operator==(const BuildModStep &other) const
    {
        return  _buildModStepIndex == other._buildModStepIndex &&
                _buildModAction == other._buildModAction &&
                _buildModKey == other._buildModKey;
    }
    bool operator<(const BuildModStep &other) const
    {
        if (_buildModStepIndex != other._buildModStepIndex)
            return _buildModStepIndex < other._buildModStepIndex;
        if (_buildModAction != other._buildModAction)
            return _buildModAction < other._buildModAction;
        return _buildModKey < other._buildModKey;
    }
    int         _buildModStepIndex;
    int         _buildModAction;
    QString     _buildModKey;
};

class BuildMod {
  public:
    QVector<int>  _modAttributes;
    QVector<int>  _modSubmodelStack;
    QMap<int,int> _modActions;
    int           _modStepIndex;

    BuildMod()
    {
       // Attributes:
       // 0 BM_BEGIN_LINE_NUM    0
       // 1 BM_ACTION_LINE_NUM   0
       // 2 BM_END_LINE_NUM      0
       // 3 BM_DISPLAY_PAGE_NUM  0
       // 4 BM_STEP_PIECES       0
       // 5 BM_MODEL_NAME_INDEX -1
       // 6 BM_MODEL_LINE_NUM    0
       // 7 BM_MODEL_STEP_NUM    0
      _modAttributes = { 0, 0, 0, 0, 0, -1, 0, 0 };
      _modSubmodelStack = {};
      _modActions = {};
      _modStepIndex = -1;
    }
    BuildMod(
      const QVector<int>       &modAttributes,
      int                       stepIndex);
    ~BuildMod()
    {
      _modSubmodelStack.clear();
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
    QMap<QString, QStringList>  _buildModRendered;
    QMultiMap<int, BuildModStep> _buildModSteps;
    QMultiHash<QString, int>    _ldcadGroups;
    QStringList                 _emptyList;
    QString                     _emptyString;
    int                         _buildModNextStepIndex;
    int                         _buildModPrevStepIndex;
    bool                        _mpd;
    static int                  _emptyInt;
    static QList<QRegExp>       _fileRegExp;

    ExcludedParts               excludedParts; // internal list of part count excluded parts

    bool topFileNotFound;
    bool loadUnoffPartsNotFound;
    bool hdrFILENotFound;
    bool hdrNameNotFound;
    bool hdrAuthorNotFound;
    bool hdrDescNotFound;
    bool hdrCategNotFound;
    bool metaBuildModNotFund;
    bool metaFinalModelNotFound;
    bool metaStartPageNumNotFound;
    bool metaStartStepNumNotFound;
    bool topLevelModel;
    bool topHeaderFinished;
    bool ldcadGroupsLoaded;
    int  unofficialPart;
    int  descriptionLine;
    int  buildModLevel;

  public:
    LDrawFile();
    ~LDrawFile()
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
      _includeFileList.clear();
      _buildModList.clear();
      _loadedParts.clear();
    }

    QStringList                 _subFileOrder;
    QStringList                 _subFileOrderNoUnoff;
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
    static bool                 _loadBuildMods;
    static bool                 _loadUnofficialParts;
    static bool                 _hasUnofficialParts;

    int getPartCount(){
      return _partCount;
    }

    void setLoadBuildMods(bool b) {
        _loadBuildMods = b;
    }

    static void showLoadMessages();

    bool saveFile(const QString &fileName);
    bool saveModelFile(const QString &filename);
    bool saveIncludeFile(const QString &filename);

    void insert(const QString       &fileName,
                      QStringList   &contents,
                      QDateTime     &datetime,
                      int            unofficialPart,
                      bool           generated = false,
                      bool           includeFile = false,
                      const QString &subFilePath = QString());

    int  size(const QString &fileName);
    void empty();

    QStringList getSubModels();
    QStringList getSubFilePaths();
    QStringList contents(const QString &fileName);
    QStringList smiContents(const QString &fileName);
    QString getSubFilePath(const QString &fileName);
    void normalizeHeader(const QString &fileName,
                         int missing = 0);
    void setSubFilePath(const QString &mcFileName,
                     const QString &subFilePath);
    void setContents(const QString     &fileName, 
                     const QStringList &contents);
    void setPrevStepPosition(const QString &mcFileName,
                             const int &mcStepNumber,
                             const int &prevStepPosition);
    int getPrevStepPosition(const QString &mcFileName,
                            const int &mcStepNumber);
    void clearPrevStepPositions();
    void setModelStartPageNumber(const QString &mcFileName,
                         const int     &startPageNumber);
    int getModelStartPageNumber(const QString &mcFileName);
    void subFileLevels(QStringList &contents, int &level);
    int loadFile(const QString &fileName);
    void loadMPDFile(const QString &fileName,
                           bool externalFile = false);
    void loadLDRFile(const QString &filePath,
                     const QString &fileName = QString(),
                           bool externalFile = false);
    int subFileOrderSize();
    QStringList subFileOrder();
    QStringList includeFileList();
    QVector<int> getSubmodelIndexes(const QString &fileName);
    
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
                                        QStringList &contents,
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
    bool contains(const QString &file, bool = true);
    bool isSubmodel(const QString &file);
    bool modified();
    bool modified(const QString &fileName);
    bool modified(const QStringList &parsedStack);
    bool older(const QStringList &parsedStack,
               const QDateTime &lastModified);
    bool older(const QString &fileName,
               const QDateTime &lastModified);
    static bool mirrored(const QStringList &tokens);
    void setModified(const QString &fileName, bool modified);
    void unrendered();
    void setRendered(
            const QString &fileName,
            bool           mirrored,
            const QString &renderParentModel,
            int            renderStepNumber,
            int            howCounted,
            bool           countPage = false);
    bool rendered(
            const QString &fileName,
            bool           mirrored,
            const QString &renderParentModel,
            int            renderStepNumber,
            int            howCounted,
            bool           countPage = false);
    void addCustomColorParts(const QString &mcFileName, bool autoAdd = false);
    int instances(const QString &fileName, bool mirrored);
    void countParts(const QString &fileName);
    void countInstances();
    void countInstances(
            const QString &fileName,
                  bool     firstStep,
                  bool     isMirrored,
                  bool     callout = false);
    bool changedSinceLastWrite(const QString &fileName);
    void tempCacheCleared();

    void setSmiContent(const QString &fileName, const QStringList &smiContents);
    void insertLDCadGroup(const QString &name, int lid);
    bool ldcadGroupMatch(const QString &name, const QStringList &lids);
    int getStepIndex(const QString &modelName, const int &lineNumber);
    int getStepIndex(const int &modelIndex, const int &lineNumber);
    void getTopOfStepWhere(const QString &modelName, int &modelIndex, int &lineNumber);
    void skipHeader(const QString &modelName, int &lineNumber);

    /* Build Modification functions */

    void insertBuildMod(const QString      &buildModKey,
                        const QVector<int> &modAttributes,
                        int                 stepIndex);
    void insertBuildModStep(const QString  &buildModKey,
                            const int       stepIndex,
                            const int       modAction = 0);
    void setBuildModSubmodelStack(const QString &buildModKey,
                                  const QStringList &submodelStack);
    int setBuildModAction(const QString  &buildModKey,
                          const int       stepIndex,
                          const int       modAction);
    int setBuildModRendered(const QString &buildModKey, const QString &renderedModel);
    int getBuildModStep(const QString &modelName, const int &lineNumber);
    int getBuildModStepIndex(const int modelIndex, const int lineNumber); // last application step index
    int getBuildModStepIndex(const QString &buildModKey);                 // creation step index
    int getBuildModStepLineNumber(int stepIndex, bool bottom);
    int getBuildModBeginLineNumber(const QString &buildModKey);
    int getBuildModEndLineNumber(const QString &buildModKey);
    int getBuildModActionLineNumber(const QString &buildModKey);
    int getBuildModAction(const QString &buildModKey, const int stepIndex);
    int getBuildModAction(const QString &buildModKey, const int stepIndex, const int defaultIndex);
    int getBuildModAction(const QString &buildModKey, const int stepIndex, const int defaultIndex, int &actionStepIndex);
    int getBuildModActionPrevIndex(const QString &buildModKey, const int stepIndex, const int action);
    int getBuildModDisplayPageNumber(const QString &buildModKey);
    int setBuildModDisplayPageNumber(const QString &buildModKey, int displayPageNum);
    int getBuildModStepPieces(const QString &buildModKey);
    int setBuildModStepPieces(const QString &buildModKey, int pieces);
    int getBuildModStepKeyLineNum(const QString &buildModKey);
    int getBuildModStepKeyStepNum(const QString &buildModKey);
    int getBuildModStepKeyModelIndex(const QString &buildModKey);
    int getBuildModPrevStepIndex();
    int getBuildModNextStepIndex();
    int buildModsCount();
    bool getBuildModRendered(const QString &buildModKey, const QString &renderedModel, bool countPage = false);
    bool getBuildModStepIndexWhere(const int stepIndex, QString &modelName, int &modelIndex, int &lineNumber);
    bool setBuildModNextStepIndex(const QString &modelName, const int &lineNumber);
    bool buildModContains(const QString &buildModKey);
    bool deleteBuildMod(const QString &buildModKey);
    void clearBuildModRendered(bool countPage = false);
    void clearBuildModRendered(const QString &buildModKey, const QString &renderedModel);
    void clearBuildModAction(const QString &buildModKey, const int stepIndex);
    void clearBuildModStep(const QString &buildModKey,const int stepIndex);
    void clearBuildModSteps(const QString &buildModKey);
    void clearBuildModSteps();
    void setBuildModStepKey(const QString &buildModKey, const QString &modStepKey);
    void deleteBuildMods(const int stepIndex);
    QString getBuildModStepKey(const QString &buildModKey);
    QString getBuildModStepKeyModelName(const QString &buildModKey);
    QStringList getPathsFromBuildModKeys(const QStringList &buildModKeys);
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
                          bool               calledOut,
                          int                viewType/*CSI,PLI,SMP*/);
    void updateViewerStep(const QString     &fileName,
                          const QStringList &contents,
                          bool rotated = true);
    QString getViewerStepKeyFromRange(const int modelIndex,
                                      const int lineNumber,
                                      const int topModelIndex,
                                      const int topLineNumber,
                                      const int bottomModelIndex,
                                      const int bottomLineNumber);
    QStringList getPathsFromViewerStepKey(const QString &stepKey);
    QStringList getViewerStepRotatedContents(const QString &stepKey);
    QStringList getViewerStepUnrotatedContents(const QString &stepKey);
    QString     getViewerStepKeyWhere(const int modelIndex, const int lineNumber);
    QString     getViewerStepFilePath(const QString &stepKey);
    QString     getViewerStepImagePath(const QString &stepKey);
    QString     getViewerConfigKey(const QString &stepKey);
    int         getViewerStepPartCount(const QString &stepKey);
    bool        isViewerStepMultiStep(const QString &stepKey);
    bool        isViewerStepCalledOut(const QString &stepKey);
    bool        viewerStepContentExist(const QString &stepKey);
    bool        viewerStepModified(const QString &stepKey);
    bool        deleteViewerStep(const QString &stepKey);
    void        clearViewerSteps();

    /* Line index functions */

    QString     getSubmodelName(int submodelIndx);
    int         getSubmodelIndex(const QString &fileName);
    int         getLineTypeIndex(int submodelIndx, int relativeTypeIndx);
    int         getLineTypeRelativeIndex(int submodelIndx, int lineTypeIndx);
    int         getLineTypeRelativeIndexCount(int submodelIndx);
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
