 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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
#include "rotateiconitem.h"

class Meta;
class Callout;
class Range;
class ImageMatt;
class PagePointer;

class Step : public AbstractRangeElement
{
  public: 
    bool                  calledOut;
    bool                  multiStep;
    bool                  placeRotateIcon;
    QList<Callout *>      list;
    Pli                   pli;
    CsiItem              *csiItem;
    RotateIconMeta        rotateIconMeta;
    RotateIcon            rotateIcon;
    Placement             csiPlacement;
    QPixmap               csiPixmap;
    PlacementNum          stepNumber;
    NumberPlacementMeta   numberPlacemetMeta;
    bool                  showStepNumber;
    int                   submodelLevel;
    bool                  pliPerStep;
    bool                  csiOutOfDate;
    bool                  modelDisplayOnlyStep;
    PlacementMeta         placement;
    QString               ldrName;
    QString               pngName;
    QString               csiKey;
    QString               viewerCsiName;
    ViewerOptions         viewerOptions;
    PlacementHeader       plPageHeader;
    PlacementFooter       plPageFooter;

    QString               foo;

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

    bool loadTheViewer();

    int  createCsi(
           QString      const &addLine,
           QStringList  const &csiParts,
           QPixmap            *pixmap,
           Meta               &meta,
           bool               bfxLoad = false);

    int createViewerCSI(QStringList &csiParts,
                  bool doFadeStep,
                  bool doHighlightStep);

    int mergeViewerCSISubModels(QStringList &subModels,
                           QStringList &subModelParts,
                           bool doFadeStep,
                           bool doHighlightStep);

    int  sizeit(int  rows[],
                int  cols[],
                int  rowsMargin[][2],
                int  colsMargin[][2],
                int  x,
                int  y);
                
    bool collide(int square[][NumPlaces],
                 int tbl[],
                 int x,
                 int y);

    void maxMargin(MarginsMeta &marvin, int tbl[2], int r[][2], int c[][2]);
    void maxMargin(int &top, int &bot, int y = YY);

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

#endif
