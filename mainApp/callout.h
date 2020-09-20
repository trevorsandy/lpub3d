 
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
 * This file describes the data structure that represents an LPub callout
 * (the steps of a submodel packed together and displayed next to the
 * assembly where the submodel is used).  
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef CALLOUTH
#define CALLOUTH

#include "ranges.h"
#include "range.h"
#include "placement.h"
#include "where.h"
#include "numberitem.h"

#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsPolygonItem>
#include "borderedlineitem.h"
#include <QGraphicsItemGroup>

class Step;
class AbstractRangeElement;
class Pointer;
class CalloutPointerItem;
class CalloutBackgroundItem;
class QGraphicsView;
class UnderpinningsItem;
/*
 * There can be more than one callout per step, so we add
 * the ability to be in a list here
 */

class Callout : public Steps {
  public:
    Step                  *parentStep;
    PlacementType          parentRelativeType;
    int                    instances;
    PlacementNum           instanceCount;
    bool                   shared;

    QList<Pointer *>            pointerList;                /* Pointers and pointer tips */
    QList<CalloutPointerItem *> graphicsCalloutPointerList; /* Pointer and pointer tips graphics */

    CalloutBackgroundItem *background;
    UnderpinningsItem     *underpinnings;
    Where                  topCallout,bottomCallout;
    
    Where &topOfCallout()
    {
      return topCallout;
    }
    Where &bottomOfCallout()
    {
      return bottomCallout;
    }
    void setTopOfCallout(const Where &topOfCallout)
    {
      topCallout = topOfCallout;
    }
    void setBottomOfCallout(const Where &bottomOfCallout)
    {
      bottomCallout = bottomOfCallout;
    }

    Callout(
      Meta                 &_meta,
      QGraphicsView        *_view);

    virtual AllocEnc allocType();

    virtual AllocMeta &allocMeta();

    virtual ~Callout();

    virtual void appendPointer(const Where &here,
                               PointerMeta &pointerMeta,
                               PointerAttribMeta &pointerAttrib);

    virtual void sizeIt();
            void sizeitFreeform(int xx, int yy);

    void addGraphicsItems(
      int   offsetX, int offsetY, QRect &csiRect,QGraphicsItem *parent, bool movable);

    virtual void addGraphicsItems( 
      AllocEnc, int x, int y, QGraphicsItem *parent, bool movable);

    virtual void addGraphicsPointerItem(
      Pointer *pointer,QGraphicsItem *parent);

    virtual void drawTips(QPoint &delta, int type = 0);
    virtual void updatePointers(QPoint &delta);

    QString wholeSubmodel(Meta &meta,QString &modelName,QString &addLine,int depth);
};

class CalloutInstanceItem : public NumberPlacementItem
{
  Callout *callout;
public:
  int stepNumber;
  Where instanceTop, instanceBottom;
  CalloutInstanceItem(
    Callout             *callout,
    Meta                *meta,
    const char          *format,
    int                  _value,
    QGraphicsItem       *parent);
protected:
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

class UnderpinningsItem : public QGraphicsRectItem
{
public:
  int stepNumber;
  Where top, bottom;
  UnderpinningsItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent = nullptr)
      : QGraphicsRectItem(x,y,w,h,parent){ }
protected:
};

#endif
