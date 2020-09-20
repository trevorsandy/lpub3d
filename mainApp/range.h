 
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
 * This class implements a set of consecutive steps within either a row
 * or column of a callout or multi-step.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef rangeH
#define rangeH

#include <QtGlobal>
#include <QtWidgets>
#include "meta.h"
#include "ranges_element.h"

class Step;
class QGraphicsItem;
class AbstractRangeElement;
class Pointer;
class Range : public AbstractStepsElement {
  public:
    int                 allocType;
    FreeFormMeta        freeform;
    SepMeta             sepMeta;
    int                 stepSpacing;
    int                 leftAdjust;              // for freeform multi_step only
    QList<Pointer *>    rangeDividerPointerList; // for range divider pointers and pointer tips data
    QList<Pointer *>    stepDividerPointerList;  // for step divider pointers and pointer tips data
    QGraphicsView      *view;                    // for divider pointers

    Range(Steps        *_parent,
          AllocEnc      _allocType);

    Range(Steps        *_parent,
          AllocEnc      _allocType,
          FreeFormMeta  _freeform);

    virtual ~Range();

    void append(AbstractRangeElement *gi);

    virtual void sizeMargins(int cols[], int colMargins[][2], int margins[], int element = 0/*cols*/);
    virtual int sizeRangeDividers(int axis);
    virtual void sizeitVert();
    virtual void sizeitHoriz();
    virtual void placeit(int max, int x, int y);

    virtual void sizeitFreeform(
                   int  xx,
                   int  yy,
                   int  base,
                   int  justification);

    virtual void placeitFreeform(
                   int  xx,
                   int  yy,
                   int  max,
                   int  justification);

    virtual void addGraphicsItems(
                   int  xx,
                   int  yy,
                   Meta *meta,
                   PlacementType,
                   QGraphicsItem *parent);

    virtual void appendDividerPointer(
                   const Where       &here,
                   PointerMeta       &pointerMeta,
                   PointerAttribMeta &pointerAttrib,
                   QGraphicsView     *view,
                   int                stepNum,
                   bool               sd);

};

/******************************************************************************
 * Step background item
 *****************************************************************************/

class MultiStepStepBackgroundItem : public QGraphicsRectItem, public MetaItem
{
  Step *step;

public:
  MultiStepStepBackgroundItem(
    Step *step,
    QGraphicsItem *parent);
  int stepNumber;
  Where top, bottom;
protected:
  virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  bool isHovered;
  bool mouseIsDown;
};

#endif
