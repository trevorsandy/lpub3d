
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
 * This file describes a class that is used to implement backannotation
 * of user Gui input into the LDraw file.  Furthermore it implements
 * some functions to provide higher level editing capabilities, such
 * as adding and removing steps from step groups, adding, moving and
 * deleting dividers.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef METAITEM_H
#define METAITEM_H

#include <QString>
#include "meta.h"

enum ScanMask {

  StepMask = (1 << StepRc)|(1 << RotStepRc),
  CalloutBeginMask = (1 << CalloutBeginRc),
  CalloutDividerMask = (1 << CalloutDividerRc),
  CalloutEndMask = (1 << CalloutEndRc),
  CalloutMask = CalloutBeginMask|CalloutDividerMask|CalloutEndMask,

  StepGroupBeginMask = (1 << StepGroupBeginRc),
  StepGroupDividerMask = (1 << StepGroupDividerRc),
  StepGroupEndMask = (1 << StepGroupEndRc),
  StepGroupMask = StepGroupBeginMask|StepGroupDividerMask|StepGroupEndMask,

  PageMask = (1 << InsertPageRc) | (1 << InsertCoverPageRc),
  InsertMask = (1 << InsertRc)
};

class Steps;
class StepGroup;
class Page;

class MetaItem
{
public:
  void setGlobalMeta(QString &topLevelFile, LeafMeta *leaf);

  bool canConvertToCallout(Meta *);
  void convertToCallout(   Meta *, const QString &, bool isMirrored, bool assembled = false, bool pointerless = false);
  void addCalloutMetas(    Meta *, const QString &, bool isMirrored, bool assembled = false, bool pointerless = false);
  void addPointerTip(      Meta *, const Where &, const Where &, PlacementEnc, Rc);
  void addPointerTipMetas( Meta *, const Where &, const Where &, PlacementEnc, Rc);
  void writeCsiAnnotationMeta(QStringList &list, const Where &, const Where &, Meta *,bool = false);
  void updateCsiAnnotationIconMeta(const Where &here, CsiAnnotationIconMeta *caim);
   int setCsiAnnotationMetas(Steps *steps);
  bool offsetPoint(        Meta &, const Where &, const Where &, int (&)[2], int (&)[2], int (&)[2], int = -1);

  int  nestCallouts(Meta *, const QString &, bool isMirrored, bool pointerless = false);
  QString makeMonoName(const QString &fileName, const QString &color);
  int monoColorSubmodel(const QString &modelName, const QString &monoOutName, const QString &color);
  QPointF defaultPointerTip(
    Meta &meta,QString &modelName,int lineNumber,
    const QString &subModel,int instance,bool isMirrored);

  void removeCallout(          const QString &, const Where &, const Where &);
  void unnestCallouts(         const QString &);
  void updatePointer(          const Where &,   PointerMeta *pointer);
  void deletePointerAttribute( const Where &, bool = false);
  void deletePointer(          const Where &, bool, bool, bool);
  void changeRotation(         const Where &);

  int countInstancesInModel(   Meta *, const QString &);
  int countInstancesInStep(    Meta *, const QString &);

  void addNextMultiStep(       const Where &topOfSteps, const Where &bottomOfSteps);
  void addPrevMultiStep(       const Where &topOfSteps, const Where &bottomOfSteps);
  void addNextStepsMultiStep(  const Where &topOfSteps, const Where &bottomOfSteps, const int &numOfSteps);

  void movePageToEndOfStepGroup(const Where &topOfSteps);
  void movePageToBeginOfStepGroup(const Where &topOfSteps);

  int  removeFirstStep(        const Where &topOfSteps);
  int  removeLastStep(         const Where &topOfSteps, const Where &lastStep);

  void deleteFirstMultiStep(   const Where &);
  void deleteLastMultiStep(    const Where &, const Where &);

  void addDivider(   PlacementType parentRelativeType, const Where &, RcMeta *divider, AllocEnc alloc, bool offerStepDivider);
  void deleteDivider(PlacementType parentRelativeType, const Where &divider);

  void addToNext(    PlacementType parentRelativeType, const Where &topOfStep);
  void addToPrev(    PlacementType parentRelativeType, const Where &topOfStep);

  void calloutAddToPrev(       const Where &);
  void calloutAddToNext(       const Where &);
  void stepGroupAddToPrev(     const Where &);
  void stepGroupAddToNext(     const Where &);

  void convertToIgnore(        Meta *);
  void convertToPart(          Meta *);

  void changeCsiAnnotationPlacement(
                    PlacementType          parentType,
                    PlacementType          placedType,
                    QString                title,
                    const Where           &topOf,
                    const Where           &bottomOf,
                    PlacementMeta         *placement,
                    CsiAnnotationIconMeta *icon,
                    bool useTop     =  true,
                    int  append     =  1,
                    bool local      =  true,
                    bool checkLocal =  true,
                    int  onPageType =  ContentPage);

  bool setPointerPlacement(
                    PlacementMeta       *,
                    const PlacementType parentType,
                    const PlacementType relativeType,
                    QString             title);

  void setPlacement(PlacementMeta       *,
                    const PlacementType parentType,
                    const PlacementType relativeType,
                    QString             title,
                    bool                onPageType);

  void changePlacement( PlacementType parentType,
                        PlacementType placedType,
                        QString title,
                        const Where &top,
                        const Where &bottom,
                        PlacementMeta *,
                        bool useTop = true,
                        int  append = 1,
                        bool local = true,
                        bool checkLocal = true,
                        int  onPageType = ContentPage);

  void changePlacement( PlacementType parentType,
                        bool          pliPerStep,
                        PlacementType placedType,
                        QString title,
                        const Where &top,
                        const Where &bottom,
                        PlacementMeta *,
                        bool useTop = true,
                        int  append = 1,
                        bool local = true,
                        bool checkLocal = true,
                        int  onPageType = ContentPage);

  void changePlacementOffset(
                        Where defaultconst,
                        PlacementMeta *placement,
                        PlacementType type,
                        bool useTop = true,
                        bool local = true,
                        bool global = false);

  void changeAlloc(     const Where &,
                        const Where &,
                        AllocMeta   &,
                        int   append = 1);

  void changeBool(      const Where &,
                        const Where &,
                        BoolMeta    *,
                        bool  useTop = true,
                        int   append = 1,
                        bool allowLocal = false,
                        bool askLocal = true);

  void changeFont(      const Where &,
                        const Where &,
                        FontMeta  *,
                        int   append = 1,
                        bool checkLocal = true,
                        bool  useTop = true);

  void changeColor(     const Where &,
                        const Where &,
                        StringMeta  *,
                        int   append = 1,
                        bool checkLocal = true,
                        bool  useTop = true);


  void changeSubModelColor(QString title,
                            const Where &,
                            const Where &,
                            StringListMeta  *,
                            bool checkLocal = true,
                            int   append = 1,
                            bool  useTop = true);

  void changeBackground(QString,
                        const Where &,
                        const Where &,
                        BackgroundMeta*,
                        bool  useTop = true,
                        int   append = 1,
                        bool checkLocal = true,
                        bool picSettings = true);

  void changeSizeAndOrientation(QString,
                        const Where &,
                        const Where &,
                        PageSizeMeta   *,
                        PageOrientationMeta *,
                        bool  useTop = true,
                        int   append = 1,
                        bool checkLocal = true);
/*
  void changePageSize(QString,
                        const Where &,
                        const Where &,
                        UnitsMeta *,
                        bool  useTop = true,
                        int   append = 1,
                        bool checkLocal = true);

  void changePageOrientation(QString,
                        const Where &,
                        const Where &,
                        PageOrientationMeta *,
                        bool  useTop = true,
                        int   append = 1,
                        bool checkLocal = true);
*/
  void changeImageItemSize(QString,
                        const Where &,
                        const Where &,
                        UnitsMeta *,
                        bool  useTop = true,
                        int   append = 1,
                        bool checkLocal = true);

  void changePliSort(   QString,
                        const Where &,
                        const Where &,
                        PliSortOrderMeta *,
                        int   append = 1,
                        bool checkLocal = true);

  void changePliAnnotation(
                        QString,
                        const Where &,
                        const Where &,
                        PliAnnotationMeta *,
                        int  append = 1,
                        bool  local = true);

  void changeBorder(    QString,
                        const Where &,
                        const Where &,
                        BorderMeta  *,
                        bool useTop = true,
                        int  append = 1,
                        bool checkLocal = true,
                        bool rotateArrow = false,
                        bool corners = false);

  void changeImage(     QString,
                        const Where &,
                        const Where &,
                        StringMeta  *,
                        bool useTop = true,
                        int  append = 1,
                        bool allowLocal = true,
                        bool checkLocal = false);

  void changeImageScale(QString title,
                        QString label,
                        const Where &,
                        const Where &,
                        FloatMeta   *,
                        float step = 0.01,
                        bool  useTop = true,
                        int   append = 1,
                        bool  askLocal = true);

  void changeImageFill( const Where &,
                        const Where &,
                        FillMeta    *,
                        bool  useTop = true,
                        int   append = 1,
                        bool allowLocal = false,
                        bool askLocal = true);

  void changeCameraAngles( QString,
                        const Where &,
                        const Where &,
                        FloatPairMeta *,
                        int  append = 1,
                        bool checkLocal = true);

  void changeConstraint(QString,
                        const Where &,
                        const Where &,
                        ConstrainMeta *,
                        int   append = 1,
                        bool checkLocal = true);

  void changeStepSize(  QString,
                        const Where &,
                        const Where &,
                        const QString &,
                        UnitsMeta *,
                        int  sizeX,
                        int  sizeY,
                        int  append = 1,
                        bool local = false,
                        bool askLocal = true);

  void changeDivider(   QString,
                        const Where &,
                        const Where &,
                        SepMeta *,
                        int  append = 1,
                        bool checkLocal = true);

  void changeMargins(   QString,
                        const Where &,
                        const Where &,
                        MarginsMeta *,
                        bool  useTop = true,
                        int   append = 1,
                        bool checkLocal = true);

  void changeFloat(     QString,
                        QString,
                        const Where &,
                        const Where &,
                        FloatMeta *,
                        int   append = 1,
                        bool checkLocal = true);

  void changeFloat(     const Where &,
                        const Where &,
                        FloatMeta *,
                        int   append = 1,
                        bool checkLocal = true);

  void changeFloatSpin( QString,
                        QString,
                        const Where &,
                        const Where &,
                        FloatMeta *,
                        float step = 0.01f,
                        int  append = 1,
                        bool checkLocal = true);

  void changeUnits(     QString,
                        const Where &,
                        const Where &,
                        UnitsMeta *,
                        int  append = 1,
                        bool checkLocal = true);

  void setMeta(         const Where &,
                        const Where &,
                        LeafMeta *,
                        bool  useTop = true,
                        int   append = 1,
                        bool  local = false,
                        bool  askLocal = true,
                        bool  global = false);
  void setMetaTopOf(    const Where &,
                        const Where &,
                        LeafMeta *,
                        int   append = 1,
                        bool  local = false,
                        bool  askLocal = true,
                        bool  global = false);

  void setMetaBottomOf( const Where &,
                        const Where &,
                        LeafMeta *,
                        int  append = 1,
                        bool local = false,
                        bool askLocal = true,
                        bool global = true);

  void hideSubmodel( const Where &,
                     const Where &,
                     BoolMeta *show,
                     bool useTop = true,
                     int append = 1,
                     bool local = true);

  void changeSubmodelRotStep(QString title,
                             const Where &,
                             const Where &,
                             RotStepMeta *rotStep,
                             bool useTop = true,
                             int append = 1,
                             bool local = true);

  void changeConstraint(const Where &,
                        const Where &,
                        ConstrainMeta *constraint,
                        int append = 1,
                        bool useBot = false);

  void changeConstraintStepGroup(const Where &,
                                 const Where &,
                                 ConstrainMeta *constraint,
                                 int append = 1);
  void setPointerAttrib(QString,
                        const Where &,
                        const Where &,
                        PointerAttribMeta *,
                        bool useTop = true,
                        int  append = 1,
                        bool local = true,
                        bool isCallout = false);

  void togglePartGroups(
                     const    Where &,
                     const    Where &,
                     bool     bom,
                     BoolMeta *,
                     bool     useTop = true,
                     int      append = 1,
                     bool     local = true);

  void setRendererArguments(
                     const Where &,
                     const Where &,
                     const QString &,
                     StringMeta *,
                     bool  useTop = true,
                     int   append = 1,
                     bool  local = false);

  void changeInsertOffset(InsertMeta *placement);

  void setPliPartGroupOffset(PliPartGroupMeta *meta);

  //void changePageAttributeImageOffset(Where default const,PageAttributeImageMeta *imageMeta,bool local = false,bool global = true);

  void hidePLIParts(    QList<Where> &parts);

  void substitutePLIPart(const QStringList &, const QList<Where> &, int = 1, const QStringList & = QStringList());

  void resetPartGroup(const Where &);
  
  void removeLPubFormatting(int option, const Where &top, const Where &bottom);

  void setMetaAlt(const Where &, const QString metaString, bool = false, bool = false);

  void replaceMeta(const Where &here, const QString &line);
  void insertMeta( const Where &here, const QString &line);
  void appendMeta( const Where &here, const QString &line);
  void deleteMeta( const Where &here);
  void beginMacro( QString name);
  void endMacro();

  /*--------------------------------------------------------------------------------
    *
    * General purpose things like arbitrary pages, pictures, arrows, text
    *
    *-----------------------------------------------------------------------------*/

  bool currentFile();
  bool okToInsertCoverPage();
  bool okToAppendCoverPage();
  int  displayModelStepExists();
  bool frontCoverPageExist();
  bool backCoverPageExist();

  void insertCoverPage();
  void appendCoverPage();

  bool okToInsertNumberedPage();
  bool okToAppendNumberedPage();
  void insertNumberedPage();
  void appendNumberedPage();

  void insertPage(QString &meta);
  bool appendPage(QString &meta, Where &where, int = 0/*option*/);
  void insertFinalModelStep(int atLine);
  void insertDisplayModelStep(Where &here, bool = false);
  bool deleteFinalModelStep(bool = false);
  void insertFinalModelStep();

  void insertPicture();
  void insertText();
  void insertBOM();
  void insertSplitBOM();
  void deleteBOM();
  void deleteBOMPartGroups();
  void deletePage();
  void updateText(const Where &,
                  const QString &,
                  QString &,
                  QString &,
                  float,
                  float,
                  int,
                  bool,
                  bool = false);
  void changePreferredRenderer(
                  QString,
                  const Where &,
                  const Where &,
                  PreferredRendererMeta *,
                  int  append = 1,
                  bool local = true,
                  bool askLocal = true);
  void setFadeSteps(QString,
                    const Where &,
                    const Where &,
                    FadeStepMeta *,
                    int  append = 1,
                    bool local = true,
                    bool askLocal = true);
  void setHighlightStep(QString,
                    const Where &,
                    const Where &,
                    HighlightStepMeta *,
                    int  append = 1,
                    bool local = true,
                    bool askLocal = true);
  void deleteImageItem(Where &, QString &);
  void deletePLIPartGroups(const Where &,const Where &);

  Where firstLine(QString);
  void firstLine(Where &);
  Where sortedGlobalWhere(QString modelName,QString metaString);
  Where sortedGlobalWhere(Meta &tmpMeta,QString modelName,QString metaString);

  Rc   scanForward( Where &here, int mask, bool &partsAdded, bool noPartCheck = false);
  Rc   scanForward( Where &here, int mask);
  Rc   scanForwardNoParts( Where &here, int mask);
  Rc   scanForwardStepGroup(Where &here, bool & partsAdded);
  Rc   scanForwardStepGroup(Where &here);

  void scanPastGlobal(Where &);
  void scanPastLPubMeta(Where &);
  void writeRotateStep(QString &value);

  Rc   scanBackward(Where &here, int mask, bool &partsAdded, bool noPartCheck = false);
  Rc   scanBackward(Where &here, int mask);
  Rc   scanBackwardNoParts(Where &here, int mask);
  Rc   scanBackwardStepGroup(Where &here, bool & partsAdded);
  Rc   scanBackwardStepGroup(Where &here);

  int  numSteps(QString modelName);

  void setSuspendFileDisplayFlag(bool) const;

  void loadLDSearchDirParts(bool Process = false, bool OnDemand = false, bool Update = false) const;

  void setSceneTheme() const;
  void loadTheme() const;

  void clearAllCaches() const;
  void clearPageCache(PlacementType relativeType, Page *page, int option) const;
  void clearAndReloadModelFile(bool fileReload = false, bool savePrompt = false) const;
  void clearCustomPartCache(bool silent = true) const;
  void clearCsiCache() const;
  void clearSubmodelCache() const;
  void clearPliCache() const;
  void clearTempCache() const;
  void reloadCurrentPage(bool savePrompt = false) const;
  void restartApplication(bool changeLibrary = false, bool prompt = false) const;
};

#endif
