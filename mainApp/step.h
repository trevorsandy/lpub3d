 
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
 * This class represents one step including a step number, and assembly
 * image, possibly a parts list image and zero or more callouts needed for
 * the step.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef stepH
#define stepH

#include <QGraphicsRectItem>
#include <QString>
#include <QStringList>
#include <QHash>

#include "range_element.h"
#include "pli.h"
#include "meta.h"
#include "csiitem.h"
#include "callout.h"
#include "pagepointer.h"
#include "submodelitem.h"
#include "rotateiconitem.h"

class Meta;
class Callout;
class Range;
class MetaItem;
class ImageMatt;
class PagePointer;
class QGraphicsView;
class CsiAnnotationItem;

class Step : public AbstractRangeElement
{
  public:
    bool                  calledOut;
    bool                  multiStep;
    bool                  placeSubModel;
    bool                  placeRotateIcon;
    bool                  placeCsiAnnotation;
    DividerType           dividerType;
    QList<Callout *>      list;
    QList<CsiAnnotation*> csiAnnotations;
    Pli                   pli;
    SubModel              subModel;
    CsiItem              *csiItem;
    Placement             csiPlacement;
    QPixmap               csiPixmap;
    RotateIconMeta        rotateIconMeta;
    RotateIconItem        rotateIcon;
    UnitsMeta             stepSize;
    PlacementNum          stepNumber;
    NumberPlacementMeta   numberPlacemetMeta;
    RotStepMeta           rotStepMeta;
    SettingsMeta          csiStepMeta;
    StringMeta            ldviewParms;
    StringMeta            ldgliteParms;
    StringMeta            povrayParms;
    JustifyStepMeta       justifyStep;
    bool                  showStepNumber;
    int                   submodelLevel;
    bool                  pliPerStep;
    bool                  csiOutOfDate;
    bool                  modelDisplayOnlyStep;
    bool                  fadeSteps;
    bool                  highlightStep;
    bool                  adjustOnItemOffset;
    QString               ldrName;
    QString               pngName;
    QString               csiKey;
    QString               viewerCsiKey;
    ViewerOptions        *viewerOptions;
    PlacementHeader       plPageHeader;
    PlacementFooter       plPageFooter;

    QString               foo;

    MultiStepStepBackgroundItem *stepBackground;

    Step(
      Where                 &topOfStep,
      AbstractStepsElement *_parent,
      int                    num,
      Meta                  &_meta,
      bool                   calledOut,
      bool                   multiStep);

    virtual ~Step();

    void append(
      Callout *callout)
    {
      list.append(callout);
      callout->parentStep = this;
    }

    bool displayModelOnlyStep(){
      return ! modelDisplayOnlyStep;
    }

    Step  *nextStep();
    Range *range();
    Page  *page();

    bool loadTheViewer();

    MetaItem *mi(int which = -1)
    {
        switch(which){
        case Render::Mt::PLI:
            return static_cast<MetaItem *>(pli.background);
        case Render::Mt::CSI:
            return static_cast<MetaItem *>(csiItem);
        case Render::Mt::SMP:
            return static_cast<MetaItem *>(subModel.background);
        default:
           return static_cast<MetaItem *>(csiItem);
        }
    }

    void getStepLocation(Where &top, Where &bottom);

    void setCsiAnnotationMetas(Meta &_meta,bool = false);

    void appendCsiAnnotation(
            const Where       &here,
            CsiAnnotationMeta &caMeta);

    int  createCsi(
           QString      const &addLine,
           QStringList  const &csiParts,
           QPixmap            *pixmap,
           Meta               &meta,
           bool               bfxLoad = false);

    int  sizeit(int  rows[],
                int  cols[],
                int  rowsMargin[][2],
                int  colsMargin[][2],
                int  x,
                int  y);

    bool adjustSize(
      Placement &pl1, // placement with offset
      int  rows[],    // accumulate sub-row heights here
      int  cols[]);   // accumulate sub-col widths here

    bool collide(int square[][NumPlaces],
                 int tbl[],
                 int x,
                 int y);

    void maxMargin(MarginsMeta &margin, int tbl[2], int r[][2], int c[][2]);
    int maxMargin(int &top, int &bot, int y = YY);

    void placeit(int rows[],
                     int margin[],
                     int y,
                     bool shared = false);

    void placeInside();

    void sizeitFreeform(
      int xx,
      int yy,
      int relativeBase,
      int relativeJustification,
      int &left,
      int &right);

    virtual void addGraphicsItems(int ox, int oy, Meta *, PlacementType, QGraphicsItem *, bool);

};

extern class Step* gStep;

#endif // stepH
